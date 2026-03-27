# Cálculo de π con OpenMP — Benchmark de Rendimiento Multihilo

Implementación en C que aproxima el valor de π usando integración numérica por el método del punto medio (regla del rectángulo), paralelizada con **OpenMP**. Este proyecto fue diseñado como **benchmark de rendimiento** para evaluar el comportamiento de distintos hardwares y sistemas operativos bajo diferentes niveles de paralelismo.

---

## ¿Cómo funciona?

La idea se basa en que:

$$\pi = \int_0^1 \frac{4}{1+x^2} \, dx$$

Se divide el intervalo `[0, 1]` en `1,900,000,000` pasos y se calcula la suma de las áreas de los rectángulos. El trabajo se reparte entre múltiples hilos usando un esquema de **suma parcial por hilo** (`sum[id]`), evitando condiciones de carrera sin necesidad de `critical` ni `reduction`.

---

## Estructura del código

- Cada hilo procesa los índices que le corresponden (`i = id, id + nthrds, id + 2*nthrds, ...`)
- Cada hilo acumula su resultado en `sum[id]` (posición exclusiva → sin race condition)
- Al final, el hilo principal suma todos los parciales para obtener π
- El tiempo se mide con `omp_get_wtime()` para máxima precisión

---

## Configuración

| Parámetro | Valor |
|---|---|
| Número de pasos | 1,900,000,000 |
| Método | Punto medio (regla del rectángulo) |

Para cambiar el número de hilos en las pruebas, modifica la constante:
```c
#define NUM_THREADS 1   // o 10, 750, 5000...
```

---

## Compilación

Requiere un compilador con soporte para OpenMP (GCC recomendado):
```bash
gcc -fopenmp -o pi_omp pi_omp.c
```

---

## Ejecución
```bash
./pi_omp
```

### Salida esperada
```
pi = (3.141593)
Tiempo = 2.38XXXX
```

---

## Resultados del Benchmark

Se ejecutó la prueba con **1, 10, 50, 75, 100, 200, 500, 750 y 1000, 2000, 5000, 10000 hilos** en distintos entornos tanto hardware como software.

| Hilos | Tiempo (s) | Observaciones |
|-------|-----------|---------------|
| 1 | — | Baseline secuencial |
| 10 | — | Aceleración cercana al número de núcleos físicos |
| 750 | — | Overhead de context switching comienza a notarse |
| 5000 | — | El OS limita el paralelismo real; posible degradación |

---


## Factores que afectan los resultados por plataforma

| Factor | Impacto |
|---|---|
| Número de núcleos físicos | Determina el paralelismo real máximo |
| Hyper-Threading / SMT | Puede dar ganancia moderada más allá de los núcleos físicos |
| Sistema Operativo | Linux suele tener menor overhead de scheduling que Windows |
| Caché L1/L2/L3 | Con muchos hilos, el arreglo `sum[]` puede causar *false sharing* |
| Frecuencia del CPU | Afecta el baseline de hilo único |

---

## Requisitos

- GCC 4.9+ (o cualquier compilador compatible con OpenMP 3.1+)
- Sistema operativo: Linux / Windows (con MinGW o WSL) / macOS

---
