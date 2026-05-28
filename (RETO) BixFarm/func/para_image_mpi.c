#include <mpi.h>
#include <omp.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "./lib/imgTrans.h"

#define MAX_LINE 4096
#define MAX_PATH 1024
#define MAX_NAME 512

#define TAG_JOB  100
#define TAG_DONE 101
#define TAG_STOP 102

typedef struct {
    char input[MAX_PATH];
    char output_dir[MAX_PATH];
    char output_name[MAX_NAME];
    char effect[16];
    int value;
    int use_r;
    int use_g;
    int use_b;
    int use_gray;
} Job;

static void usage(const char *prog) {
    fprintf(stderr,
        "Uso:\n"
        "  %s --jobs jobs.tsv --threads N\n\n"
        "Formato jobs.tsv:\n"
        "  input<TAB>output_dir<TAB>output_name<TAB>effect<TAB>value<TAB>r<TAB>g<TAB>b<TAB>gray\n",
        prog
    );
}

static int ensure_dir(const char *path) {
    if (path == NULL || path[0] == '\0') return 0;
    if (mkdir(path, 0775) == 0) return 0;
    if (errno == EEXIST) return 0;
    return -1;
}

static int copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return -1;

    FILE *out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return -1;
    }

    char buffer[8192];
    size_t n;

    while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        if (fwrite(buffer, 1, n, out) != n) {
            fclose(in);
            fclose(out);
            return -1;
        }
    }

    if (ferror(in)) {
        fclose(in);
        fclose(out);
        return -1;
    }

    fclose(in);
    fclose(out);
    return 0;
}

static int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static int move_generated_file(const char *output_name, const char *output_dir) {
    char generated[MAX_PATH];
    char final_path[MAX_PATH];

    snprintf(generated, sizeof(generated), "./Resultados/%s.bmp", output_name);

    if (output_dir == NULL || output_dir[0] == '\0') return 0;

    snprintf(final_path, sizeof(final_path), "%s/%s.bmp", output_dir, output_name);

    if (strcmp(generated, final_path) == 0) return 0;

    if (ensure_dir(output_dir) != 0) {
        fprintf(stderr, "[MPI] No se pudo crear output_dir: %s\n", output_dir);
        return -1;
    }

    if (rename(generated, final_path) == 0) return 0;

    if (copy_file(generated, final_path) == 0) {
        remove(generated);
        return 0;
    }

    fprintf(stderr, "[MPI] No se pudo mover %s -> %s\n", generated, final_path);
    return -1;
}

static int parse_job_line(char *line, Job *job) {
    char *saveptr = NULL;
    char *tokens[9];

    for (int i = 0; i < 9; i++) {
        tokens[i] = strtok_r(i == 0 ? line : NULL, "\t\r\n", &saveptr);
        if (!tokens[i]) return -1;
    }

    snprintf(job->input, sizeof(job->input), "%s", tokens[0]);
    snprintf(job->output_dir, sizeof(job->output_dir), "%s", tokens[1]);
    snprintf(job->output_name, sizeof(job->output_name), "%s", tokens[2]);
    snprintf(job->effect, sizeof(job->effect), "%s", tokens[3]);

    job->value = atoi(tokens[4]);
    job->use_r = atoi(tokens[5]);
    job->use_g = atoi(tokens[6]);
    job->use_b = atoi(tokens[7]);
    job->use_gray = atoi(tokens[8]);

    if (job->input[0] == '\0' || job->output_name[0] == '\0' || job->effect[0] == '\0') return -1;
    if (job->value <= 0) return -1;

    return 0;
}

static int load_jobs(const char *jobs_path, char ***jobs_out) {
    FILE *file = fopen(jobs_path, "r");
    if (!file) return -1;

    int capacity = 128;
    int count = 0;
    char **jobs = (char **)calloc(capacity, sizeof(char *));
    if (!jobs) {
        fclose(file);
        return -1;
    }

    char line[MAX_LINE];

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '#') continue;

        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[len - 1] = '\0';
            len--;
        }

        if (count >= capacity) {
            capacity *= 2;
            char **tmp = (char **)realloc(jobs, capacity * sizeof(char *));
            if (!tmp) {
                fclose(file);
                for (int i = 0; i < count; i++) free(jobs[i]);
                free(jobs);
                return -1;
            }
            jobs = tmp;
        }

        jobs[count] = strdup(line);
        if (!jobs[count]) {
            fclose(file);
            for (int i = 0; i < count; i++) free(jobs[i]);
            free(jobs);
            return -1;
        }

        count++;
    }

    fclose(file);
    *jobs_out = jobs;
    return count;
}

static void free_jobs(char **jobs, int count) {
    if (!jobs) return;
    for (int i = 0; i < count; i++) free(jobs[i]);
    free(jobs);
}

static void send_job(int worker_rank, const char *job_line) {
    int len = (int)strlen(job_line) + 1;
    MPI_Send(&len, 1, MPI_INT, worker_rank, TAG_JOB, MPI_COMM_WORLD);
    MPI_Send((void *)job_line, len, MPI_CHAR, worker_rank, TAG_JOB, MPI_COMM_WORLD);
}

static void send_stop(int worker_rank) {
    int len = 0;
    MPI_Send(&len, 1, MPI_INT, worker_rank, TAG_STOP, MPI_COMM_WORLD);
}

static int process_job(const Job *job, int rank, const char *host) {
    if (ensure_dir("./Resultados") != 0) {
        fprintf(stderr, "[rank %d][host %s] No se pudo crear ./Resultados\n", rank, host);
        return 1;
    }

    printf("[rank %d][host %s] INICIO input=%s output=%s effect=%s value=%d\n",
           rank, host, job->input, job->output_name, job->effect, job->value);
    fflush(stdout);

    double t0 = omp_get_wtime();

    if (strcmp(job->effect, "inv") == 0) {
        inv_img_flags(job->output_name, job->input, job->use_r, job->use_g,
                      job->use_b, job->use_gray, job->value);
    } else if (strcmp(job->effect, "des") == 0) {
        desenfoque_flags(job->input, job->output_name, job->value, job->use_r,
                         job->use_g, job->use_b, job->use_gray);
    } else {
        fprintf(stderr, "[rank %d][host %s] Efecto no valido: %s\n", rank, host, job->effect);
        return 1;
    }

    double t1 = omp_get_wtime();

    char generated[MAX_PATH];
    snprintf(generated, sizeof(generated), "./Resultados/%s.bmp", job->output_name);

    if (!file_exists(generated)) {
        fprintf(stderr, "[rank %d][host %s] No se genero el archivo esperado: %s\n",
                rank, host, generated);
        return 1;
    }

    if (move_generated_file(job->output_name, job->output_dir) != 0) {
        fprintf(stderr, "[rank %d][host %s] Error moviendo salida: %s\n",
                rank, host, job->output_name);
        return 1;
    }

    printf("[rank %d][host %s] FIN output=%s effect=%s value=%d tiempo=%.6f s\n",
           rank, host, job->output_name, job->effect, job->value, t1 - t0);
    fflush(stdout);

    return 0;
}

static int master_run(const char *jobs_path, int world_size, int threads) {
    char **jobs = NULL;
    int total_jobs = load_jobs(jobs_path, &jobs);

    if (total_jobs < 0) {
        fprintf(stderr, "[MPI master] No se pudo cargar jobs file: %s\n", jobs_path);
        return 1;
    }

    int next_job = 0;
    int active_workers = 0;
    int processed = 0;
    int errors = 0;

    double start = MPI_Wtime();

    printf("[MPI master] workers=%d jobs=%d threads_openmp_por_worker=%d\n",
           world_size - 1, total_jobs, threads);
    fflush(stdout);

    /*
       Reparto inicial.
       rank 0 es master y NO procesa imágenes.
       Los workers son ranks 1..world_size-1.
    */
    for (int worker = 1; worker < world_size; worker++) {
        if (next_job < total_jobs) {
            send_job(worker, jobs[next_job]);
            next_job++;
            active_workers++;
        } else {
            send_stop(worker);
        }
    }

    /*
       Reparto dinámico:
       cuando un worker termina, recibe otro job inmediatamente.
       Esto balancea mejor que job_index % size cuando unas imágenes pesan más que otras.
    */
    while (active_workers > 0) {
        int rc = 0;
        MPI_Status status;

        MPI_Recv(&rc, 1, MPI_INT, MPI_ANY_SOURCE, TAG_DONE, MPI_COMM_WORLD, &status);

        int worker = status.MPI_SOURCE;
        processed++;

        if (rc != 0) errors++;

        if (next_job < total_jobs) {
            send_job(worker, jobs[next_job]);
            next_job++;
        } else {
            send_stop(worker);
            active_workers--;
        }
    }

    double stop = MPI_Wtime();

    printf("[MPI master] Procesados=%d Errores=%d Tiempo_total=%.6f s\n",
           processed, errors, stop - start);
    fflush(stdout);

    free_jobs(jobs, total_jobs);
    return errors == 0 ? 0 : 1;
}

static int worker_run(int rank, const char *host) {
    while (1) {
        int len = 0;
        MPI_Status status;

        MPI_Recv(&len, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == TAG_STOP) break;

        if (status.MPI_TAG != TAG_JOB || len <= 0 || len > MAX_LINE) {
            fprintf(stderr, "[rank %d][host %s] Mensaje invalido del master\n", rank, host);
            int rc = 1;
            MPI_Send(&rc, 1, MPI_INT, 0, TAG_DONE, MPI_COMM_WORLD);
            continue;
        }

        char line[MAX_LINE];
        MPI_Recv(line, len, MPI_CHAR, 0, TAG_JOB, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        Job job;
        char line_copy[MAX_LINE];
        snprintf(line_copy, sizeof(line_copy), "%s", line);

        int rc = 0;

        if (parse_job_line(line_copy, &job) != 0) {
            fprintf(stderr, "[rank %d][host %s] Linea invalida: %s\n", rank, host, line);
            rc = 1;
        } else {
            rc = process_job(&job, rank, host);
        }

        MPI_Send(&rc, 1, MPI_INT, 0, TAG_DONE, MPI_COMM_WORLD);
    }

    printf("[rank %d][host %s] STOP\n", rank, host);
    fflush(stdout);
    return 0;
}

int main(int argc, char **argv) {
    int rank = 0;
    int size = 1;

    char jobs_path[MAX_PATH] = {0};
    int threads = 1;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char host[MPI_MAX_PROCESSOR_NAME];
    int host_len = 0;
    MPI_Get_processor_name(host, &host_len);
    host[host_len] = '\0';

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--jobs") == 0 && i + 1 < argc) {
            snprintf(jobs_path, sizeof(jobs_path), "%s", argv[++i]);
        } else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) {
            threads = atoi(argv[++i]);
        }
    }

    if (jobs_path[0] == '\0' || threads < 1) {
        if (rank == 0) usage(argv[0]);
        MPI_Finalize();
        return 1;
    }

    if (size < 2) {
        if (rank == 0) {
            fprintf(stderr, "[MPI] Se requieren al menos 2 ranks: 1 master y 1 worker.\n");
        }
        MPI_Finalize();
        return 1;
    }

    omp_set_num_threads(threads);

    int rc = 0;

    if (rank == 0) {
        rc = master_run(jobs_path, size, threads);
    } else {
        rc = worker_run(rank, host);
    }

    MPI_Finalize();
    return rc;
}
