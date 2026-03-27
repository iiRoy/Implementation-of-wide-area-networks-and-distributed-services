# OpenMP π Benchmark

Proyecto de benchmark diseñado para medir y comparar el rendimiento de distintos hardwares y sistemas operativos bajo diferentes niveles de paralelismo, usando el cálculo de π como carga de trabajo controlada.

---

##  Wiki

Para documentación extendida, análisis detallado y notas de cada experimento, consulta la [Wiki del proyecto](https://github.com/iiRoy/Implementation-of-wide-area-networks-and-distributed-services/wiki/%5BEXP%5D-%E2%80%90-Rendimiento-de-comparaci%C3%B3n-de-Hilos-en-Paralelo).

---

## ¿Qué hace este proyecto?

Aproxima el valor de π mediante integración numérica, paralelizada con **OpenMP**. Al ser una tarea puramente CPU-bound y matemáticamente simple, sirve como métrica limpia para evaluar cómo escala el rendimiento de un sistema al aumentar el número de hilos.

---

## Entornos evaluados

| Persona | Sistema Operativo | Procesador | Cores | Frecuencia | RAM |
|---------|-------------------|------------|-------|------------|-----|
| Abigail | Linux (CachyOS) | AMD Ryzen 5 7535HS | 6 (12 hilos) | 4.6 GHz | 30.59 GB |
| Rodrigo | Linux (CachyOS) | AMD Ryzen 7 6800HS | 8 (16 hilos) | 4.79 GHz | 14.86 GB |
| Álvaro | Windows 11 | Intel i7-11800H 11ª gen | 8 (16 hilos) | 2.3 GHz | 31.7 GB |

---

## Resultados — Tiempo de ejecución por número de hilos (segundos)

| Hilos | Abigail | Rodrigo | Álvaro |
|-------|---------|---------|--------|
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

## Conclusiones principales

- **Los equipos Linux (AMD)** alcanzan su pico de rendimiento antes (~750–1000 hilos) y muestran mayor variación con hilos altos.
- **El equipo Windows (Intel)** tiene un baseline más lento por frecuencia de reloj, pero se mantiene más estable con conteos de hilos extremos (5000–10000).
- **El punto óptimo** en todos los equipos se encuentra entre **750 y 1500 hilos**, independientemente de los núcleos físicos disponibles.
- Superar los ~2000 hilos no aporta mejora real — el overhead de scheduling del OS neutraliza cualquier ganancia de paralelismo.
- Los resultados de **5 y 10 hilos** muestran un comportamiento inesperado (tiempos más altos que con 1 hilo), posiblemente por overhead de inicialización de hilos y false sharing en conteos bajos.

---

## Estructura del repositorio
```
📁 /
├── 📄 README.md          ← Este archivo
└── 📁 Threads Comparison Perfomance/
    ├── 📄 README.md      ← Documentación técnica del código
    ├── 📄 pi_parallel.c       ← Implementación en C con OpenMP
    └── 📁 Images/
        ├── 📄 LaptopA.png     ← Imagen del proceso
        ├── 📄 LaptopB.png     ← Imagen del proceso
        └── 📄 LaptopC.png     ← Imagen del proceso

```

---

## Requisitos para reproducir

- GCC 4.9+ con soporte OpenMP (`-fopenmp`)
- Linux, Windows (MinGW/WSL) o macOS

Consulta el README dentro de `/Threads Comparison Perfomance` para instrucciones de compilación y ejecución.
