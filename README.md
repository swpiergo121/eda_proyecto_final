# Sistema de Autocompletado y Búsqueda Aproximada de Alto Rendimiento

Proyecto final del curso EDA (Estructura de Datos Avanzados). Implementa y compara
experimentalmente tres estructuras de datos para resolver dos problemas de
autocompletado de texto:

- **Completar por prefijo**: dado lo que el usuario escribió, sugerir palabras que
  empiezan con ese prefijo.
- **Corregir errores de tipeo**: si la palabra no existe en el vocabulario, sugerir
  la más cercana.

## Autores

- Piero Guerrero
- Eddison Pinedo
- Alair Catacora

## Estructuras implementadas

| Estructura | Resuelve | Ubicación |
|---|---|---|
| **Trie de caracteres** | Prefijo (baseline, sin optimizaciones) | `backend/src/algorithm/trie/` |
| **X-fast trie** | Prefijo (adaptado de enteros a palabras) | `backend/src/algorithm/x_fast_trie/` |
| **BK-tree** | Errores de tipeo (distancia de Levenshtein, radio 2) | `backend/src/algorithm/bk_tree/` |

La demo web corre las tres estructuras en paralelo sobre cada consulta y muestra
sus resultados y latencias lado a lado, en vez de elegir una sola respuesta. Más
detalles de diseño y hallazgos experimentales en `backend/src/algorithm/bk_tree/README.md`
y en el informe del proyecto.

## Estructura del repositorio

```
backend/
  src/
    algorithm/        # implementaciones en C++ (trie, x_fast_trie, bk_tree, utils compartidos)
    search_service.py # orquesta los binarios compilados y arma las respuestas de la API
    routes.py          # endpoints de FastAPI
    app.py              # app de FastAPI
  Dockerfile           # instala g++ y compila los binarios C++ al construir la imagen
frontend/
  src/
    components/        # UI: barra de búsqueda, paneles por algoritmo, métricas, benchmark
    pages/HomePage.jsx  # composición principal de la demo
    utils/api.js        # cliente HTTP hacia el backend
compose.yml            # levanta frontend + backend
```

## Requisitos

- Docker.

## Cómo ejecutar

```bash
docker compose up --build
```

- Frontend: http://localhost:5173
- Backend (API): http://localhost:8000

Para detener:

```bash
docker compose down
```

## API

| Endpoint | Descripción |
|---|---|
| `GET /health` | chequeo de salud |
| `GET /api/suggest?q=<texto>&limit=8` | corre Trie, X-fast trie y BK-tree sobre `q` y devuelve los tres resultados por separado, con latencia y estado de cada uno |
| `GET /api/top?limit=8` | palabras más frecuentes del vocabulario (usado para el estado inicial del buscador, sin invocar ninguna estructura) |
| `GET /api/benchmark` | comparación agregada (latencia, nodos indexados, memoria estimada) de las tres estructuras sobre un lote de consultas de ejemplo, incluyendo el dataset completo de 50k palabras |
