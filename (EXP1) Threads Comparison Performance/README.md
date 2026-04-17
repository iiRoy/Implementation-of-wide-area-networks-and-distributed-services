# pi_parallel.c — Documentación Técnica

Implementación en C del cálculo de π mediante integración numérica paralelizada con OpenMP. Forma parte de un benchmark de rendimiento multihilo.

---

## Algoritmo

Se aproxima π usando la identidad:

$$\pi = \int_0^1 \frac{4}{1+x^2} \, dx$$

---

## Paralelización

El trabajo se distribuye entre hilos con un esquema de **stride por ID de hilo**:
```c
for (i = id, sum[id] = 0.0; i < num_pasos; i = i + nthrds)
{
    x = (i + 0.5) * paso;
    sum[id] += 4.0 / (1.0 + x * x);
}
```

Cada hilo escribe únicamente en `sum[id]`, su posición exclusiva del arreglo, lo que elimina la necesidad de secciones `critical` o cláusulas `reduction`.

Al finalizar el bloque paralelo, el hilo principal acumula los resultados:
```c
for (i = 0, pi = 0.0; i < nthreads; i++)
    pi += sum[i] * paso;
```

---

## Compilación
```bash
gcc -fopenmp -o pi_parallel pi_parallel.c
```

---

## Ejecución
```bash
./pi_parallel
```

Para modificar el número de hilos, edita la constante y recompila:
```c
#define NUM_THREADS 1000
```

### Salida esperada
```
pi = (3.141593)
Tiempo = 1.850639
```

---

## Resultados del Benchmark

| Hilos | Abigail (s) | Rodrigo (s) | Álvaro (s) |
|-------|-------------|-------------|------------|
| 1 | 5.555951 | 6.447261 | 10.17 |
| 5 | 18.089424 | 35.169669 | 16.63 |
| 10 | 18.77275 | 25.513639 | 23.25 |
| 20 | 11.611307 | 12.20619 | 7.92 |
| 50 | 5.701553 | 5.011657 | 8.09 |
| 75 | 5.035934 | 4.520729 | 6.2 |
| 100 | 3.183143 | 2.759085 | 3.58 |
| 200 | 2.329701 | 1.740351 | 3.93 |
| 500 | 2.039387 | 1.407489 | 3.1 |
| 750 | 1.850639 | 1.343163 | 2.69 |
| 1000 | 2.335648 | 1.27477 | 2.518 |
| 1500 | 2.005587 | 1.316799 | 2.49 |
| 2000 | 2.060329 | 1.462688 | 2.51 |
| 5000 | 2.334864 | 2.541903 | 2.18 |
| 7500 | 3.466168 | 2.729314 | 2.46 |
| 10000 | 3.638604 | 3.451208 | 2.43 |

---

## Consideraciones técnicas

### Anomalía en hilos bajos (5–20 hilos)

Los tres equipos registran tiempos **más altos con 5 y 10 hilos que con 1 hilo**. Esto es contraintuitivo pero tiene explicación:

- Con pocos hilos, el overhead de inicialización de OpenMP supera la ganancia de paralelismo
- En conteos bajos los hilos compiten por recursos de caché de forma menos eficiente que un solo hilo secuencial

### Límite de hilos del sistema

Con valores muy altos (`NUM_THREADS > 5000`), el OS puede:
- Limitar la creación de hilos según `ulimit -u` (Linux)
- Aumentar el overhead de context switching hasta degradar el rendimiento
- En casos extremos, lanzar un error de recursos insuficientes

### Estándar C

`void main()` es aceptado por la mayoría de compiladores pero no es estrictamente estándar:
```c
int main() { ... return 0; }
```

---

## Dependencias

| Herramienta | Versión mínima |
|---|---|
| GCC | 4.9+ |
| OpenMP | 3.1+ |

Compatible con Linux, Windows (MinGW / WSL) y macOS.
