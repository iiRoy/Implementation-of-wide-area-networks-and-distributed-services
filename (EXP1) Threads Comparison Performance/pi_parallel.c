#include <stdio.h>
#include <omp.h>
static long num_pasos=1.9e9;
double paso;
//#define NUM_THREADS 1000
#define NUM_THREADS 450
void main(){
 int i, nthreads;
 double pi, sum[NUM_THREADS];
 paso=1.0/(double) num_pasos;
 omp_set_num_threads(NUM_THREADS);
 const double ST = omp_get_wtime();
#pragma omp parallel
 {
  int i, id, nthrds;
  double x;
  id = omp_get_thread_num();
  nthrds = omp_get_num_threads();
  if (id==0) nthreads=nthrds;

  for (i=id, sum[id]=0.0;i<num_pasos;i=i+nthrds)
  {
    x=(i+0.5)*paso;
    sum[id]+=4.0/(1.0+x*x);
  }
 }

 for (i=0,pi=0.0;i<nthreads;i++)
 {
   pi+=sum[i]*paso;
 }
   printf("pi = (%lf)\n",pi);
   const double STOP = omp_get_wtime();
   printf("Tiempo = %lf \n", (STOP - ST));
}
