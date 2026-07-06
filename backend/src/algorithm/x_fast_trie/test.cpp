#include "../utils/dataset_loader.hpp"
#include "../utils/json_utils.hpp"
#include "../utils/text_utils.hpp"
#include "str_xfast.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

int main(int argc, char *argv[]) {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  if (argc < 4) {
    std::cout << construirErrorJson(
                     "Uso: <programa> <palabra> <distancia> <ruta_csv>")
              << '\n';
    return 1;
  }

  std::string palabraBuscada = aMinusculasEspanolUtf8Basico(recortar(argv[1]));
  std::string textoRadio = recortar(argv[2]);
  std::string rutaArchivo = argv[3];

  if (palabraBuscada.empty()) {
    std::cout << construirErrorJson(
                     "La palabra de búsqueda no puede estar vacía.")
              << '\n';
    return 1;
  }

  int n_closest = 5;
  try {
    size_t parsed = 0;
    n_closest = std::stoi(textoRadio, &parsed);
    if (parsed != textoRadio.size() || n_closest < 0) {
      throw std::invalid_argument("invalid radius");
    }
  } catch (...) {
    std::cout << construirErrorJson(
                     "La distancia debe ser un entero mayor o igual a 0.")
              << '\n';
    return 1;
  }

  std::vector<InfoPalabra> palabras = cargarDatos(rutaArchivo);
  if (palabras.empty()) {
    std::cout << construirErrorJson("No se pudo cargar el dataset o el archivo "
                                    "no contiene datos válidos.")
              << '\n';
    return 1;
  }

  std::sort(palabras.begin(), palabras.end(),
            [](const InfoPalabra &a, const InfoPalabra &b) {
              return a.frecuencia > b.frecuencia;
            });

  StringXFastTrie trie(16); // 4 chars * 8 bits = W = 32
  vector<string> words_skip;
  for (const InfoPalabra &item : palabras) {
    try {
      trie.insert(item.palabra);
    } catch (const std::runtime_error) {
      words_skip.push_back(item.palabra);
      continue;
    }
  }

  std::vector<string> resultados =
      trie.get_closest_strings(palabraBuscada, n_closest);

  std::cout << construirRespuestaJsonXFastTrie(palabraBuscada, n_closest,
                                               rutaArchivo, palabras.size(),
                                               resultados, words_skip)
            << '\n';
  return 0;
}
