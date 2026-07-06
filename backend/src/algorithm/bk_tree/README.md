# BK-tree para búsqueda de palabras

- `para_front.cpp`: versión no interactiva pensada para integrarse con una aplicación web y devolver JSON.

## Estructura

- `bk_tree.cpp` y `bk_tree.hpp`: lógica del árbol BK-tree.
- `dataset_loader.cpp` y `dataset_loader.hpp`: carga del archivo CSV.
- `text_utils.cpp` y `text_utils.hpp`: normalización de texto y distancia de Levenshtein.
- `json_utils.cpp` y `json_utils.hpp`: construcción de la respuesta JSON.
- `types.hpp`: estructuras compartidas.

## Requisitos

- Compilador C++ con soporte para C++17 o superior.
- El archivo CSV de datos en la carpeta `data/` del proyecto.

## Compilar

Desde `bk-tree`, compila la versión modular con:

```bash
g++ -o para_front para_front.cpp bk_tree.cpp text_utils.cpp dataset_loader.cpp json_utils.cpp
```

## Usar la versión JSON

La versión para frontend recibe tres argumentos:

```bash
./para_front <palabra> <distancia> <ruta_csv>
```

Ejemplo:

```bash
./para_front abandeno 2 data/palabras_frecuencias_1000.csv
```

Salida esperada:

- `ok`: indica si la ejecución fue correcta.
- `query`: palabra buscada.
- `radio`: distancia máxima.
- `archivo`: ruta del CSV usado.
- `datasetTamano`: número de palabras cargadas.
- `cantidadResultados`: cantidad de coincidencias.
- `resultados`: lista de resultados ordenados por distancia, frecuencia y palabra.
