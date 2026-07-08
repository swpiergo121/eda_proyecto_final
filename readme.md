# String Data Structures Implementation

This project contains implementations and benchmarks for two different string-indexing data structures: the **BK Tree** and the **X-Fast Trie**.

# Interfaz
The interface is found on the dev branch.

# Directory structure

.
├── bk_tree/                  # BK-Tree implementation
│   ├── bk_tree.cpp
│   ├── bk_tree.h
│   ├── time.cpp              # Benchmarking / timing
│   └── para_front.cpp        # Parameter-front test / usage demo
├── x_fast_trie/              # X-Fast Trie implementation
│   ├── xfast.h
│   ├── str_xfast.h
│   ├── time.cpp              # Benchmarking / timing
│   └── test.cpp              # Test / usage demo
├── utils/                    # Shared utilities
│   ├── text_utils.cpp
│   ├── text_utils.h
│   ├── dataset_loader.cpp
│   ├── dataset_loader.h
│   ├── json_utils.cpp
│   └── json_utils.h
├── data/                     # Datasets (paths hardcoded in test files)
└── README.md
# Requirements


Needs to have a C++ compiler installed, like g++.

There are no external libraries.

python can be used to format the output.


# Commands
## BK tree
### Time test

```bash
g++ -o bktree.out bk_tree/time.cpp bk_tree/bk_tree.cpp utils/text_utils.cpp utils/dataset_loader.cpp utils/json_utils.cpp
 ./bktree.out
```

### Usage test

```bash
g++ -o para_front bk_tree/para_front.cpp bk_tree/bk_tree.cpp utils/text_utils.cpp utils/dataset_loader.cpp utils/json_utils.cpp
./bktree.out
```

If python is installed, the next code can be used to make the json prettier.

```bash

./bktree.out abandina 3 data/palabras_frecuencias_1000.csv | python3 -m json.tool
```

The first word is the word to search, then the range, then the dataset.

## X-fast trie

### Time test
```bash
g++ -o xfast.out x_fast_trie/time.cpp x_fast_trie/str_xfast.h x_fast_trie/xfast.h utils/text_utils.cpp utils/dataset_loader.cpp utils/json_utils.cpp
 ./xfast.out
```

### Usage test

```bash
g++ -o xfast.out x_fast_trie/test.cpp x_fast_trie/str_xfast.h x_fast_trie/xfast.h utils/text_utils.cpp utils/dataset_loader.cpp utils/json_utils.cpp
./xfast.out
```

If python is installed, the next code can be used to make the json prettier.

```bash
./xfast.out abandina 3 data/palabras_frecuencias_1000.csv | python3 -m json.tool
```

The first word is the word to search, then the number of elements to return that match and then the dataset.

