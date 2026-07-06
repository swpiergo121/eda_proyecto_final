// Wrapper no interactivo para integrar StringXFastTrie con el backend web.
// No modifica xfast.h ni str_xfast.h: solo los consume a traves de su API publica.
//
// El x-fast trie resuelve predecesor/sucesor sobre un universo de enteros.
// Para usarlo con palabras, cada palabra se codifica como un entero que
// preserva el orden lexicografico (ver str_xfast.h::encode). Codificar un
// prefijo produce la misma clave que codificar una palabra que empieza con
// ese prefijo rellena con ceros, así que el sucesor de encode(prefijo) es la
// primera palabra (en orden alfabetico) que podria matchear ese prefijo.
// Caminamos alrededor de ese punto con get_closest_strings y filtramos por
// startswith para quedarnos con coincidencias reales de prefijo.

#include "../utils/dataset_loader.hpp"
#include "../utils/text_utils.hpp"
#include "str_xfast.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace {

constexpr size_t kMaxCodedLength = 16; // limite duro de StringXFastTrie (__int128)

std::string escaparJson(const std::string& texto) {
	std::string escapado;
	escapado.reserve(texto.size() + 8);
	for (unsigned char c : texto) {
		switch (c) {
			case '"': escapado += "\\\""; break;
			case '\\': escapado += "\\\\"; break;
			case '\n': escapado += "\\n"; break;
			case '\r': escapado += "\\r"; break;
			case '\t': escapado += "\\t"; break;
			default:
				if (c < 0x20) {
					static const char* hex = "0123456789abcdef";
					escapado += "\\u00";
					escapado += hex[(c >> 4) & 0x0F];
					escapado += hex[c & 0x0F];
				} else {
					escapado.push_back(static_cast<char>(c));
				}
		}
	}
	return escapado;
}

std::string construirError(const std::string& mensaje) {
	std::ostringstream salida;
	salida << "{\"ok\":false,\"error\":\"" << escaparJson(mensaje) << "\"}";
	return salida.str();
}

} // namespace

int main(int argc, char* argv[]) {
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);

	if (argc < 4) {
		std::cout << construirError("Uso: <programa> <prefijo> <cantidad> <ruta_csv>") << '\n';
		return 1;
	}

	std::string prefijo = aMinusculasEspanolUtf8Basico(recortar(argv[1]));
	std::string textoCantidad = recortar(argv[2]);
	std::string rutaArchivo = argv[3];

	if (prefijo.empty()) {
		std::cout << construirError("El prefijo de busqueda no puede estar vacio.") << '\n';
		return 1;
	}

	int cantidad = 8;
	try {
		size_t parsed = 0;
		cantidad = std::stoi(textoCantidad, &parsed);
		if (parsed != textoCantidad.size() || cantidad <= 0) {
			throw std::invalid_argument("invalid n");
		}
	} catch (...) {
		std::cout << construirError("La cantidad debe ser un entero mayor a 0.") << '\n';
		return 1;
	}

	std::vector<InfoPalabra> palabras = cargarDatos(rutaArchivo);
	if (palabras.empty()) {
		std::cout << construirError("No se pudo cargar el dataset o el archivo no contiene datos validos.") << '\n';
		return 1;
	}

	std::sort(palabras.begin(), palabras.end(), [](const InfoPalabra& a, const InfoPalabra& b) {
		return a.frecuencia > b.frecuencia;
	});

	std::unordered_map<std::string, long long> frecuenciaPorPalabra;
	StringXFastTrie trie(kMaxCodedLength);

	size_t indexadas = 0;
	for (const InfoPalabra& item : palabras) {
		if (item.palabra.empty() || item.palabra.size() > kMaxCodedLength) {
			continue; // fuera del universo codificable del x-fast trie
		}
		if (frecuenciaPorPalabra.find(item.palabra) != frecuenciaPorPalabra.end()) {
			continue; // duplicado tras normalizacion
		}
		frecuenciaPorPalabra[item.palabra] = item.frecuencia;
		trie.insert(item.palabra);
		++indexadas;
	}

	if (indexadas == 0) {
		std::cout << construirError("Ninguna palabra del dataset cabe en el universo del x-fast trie.") << '\n';
		return 1;
	}

	std::string prefijoCodificable = prefijo.size() > kMaxCodedLength
		? prefijo.substr(0, kMaxCodedLength)
		: prefijo;

	std::vector<std::string> vecinos;
	try {
		vecinos = trie.get_closest_strings(prefijoCodificable, cantidad * 6);
	} catch (const std::exception& e) {
		std::cout << construirError(std::string("Error de x-fast trie: ") + e.what()) << '\n';
		return 1;
	}

	std::vector<std::string> conPrefijo;
	std::vector<std::string> vecinosLexicograficos;
	for (const std::string& palabra : vecinos) {
		if (palabra.rfind(prefijo, 0) == 0) {
			conPrefijo.push_back(palabra);
		} else {
			vecinosLexicograficos.push_back(palabra);
		}
	}

	bool esPrefijoReal = !conPrefijo.empty();
	std::vector<std::string>& elegidas = esPrefijoReal ? conPrefijo : vecinosLexicograficos;

	std::sort(elegidas.begin(), elegidas.end(), [&](const std::string& a, const std::string& b) {
		return frecuenciaPorPalabra[a] > frecuenciaPorPalabra[b];
	});
	if (elegidas.size() > static_cast<size_t>(cantidad)) {
		elegidas.resize(cantidad);
	}

	std::ostringstream salida;
	salida << "{";
	salida << "\"ok\":true,";
	salida << "\"query\":\"" << escaparJson(prefijo) << "\",";
	salida << "\"esPrefijoReal\":" << (esPrefijoReal ? "true" : "false") << ",";
	salida << "\"archivo\":\"" << escaparJson(rutaArchivo) << "\",";
	salida << "\"datasetTamano\":" << indexadas << ",";
	salida << "\"cantidadResultados\":" << elegidas.size() << ",";
	salida << "\"resultados\":[";
	for (size_t i = 0; i < elegidas.size(); ++i) {
		if (i > 0) salida << ",";
		salida << "{";
		salida << "\"palabra\":\"" << escaparJson(elegidas[i]) << "\",";
		salida << "\"frecuencia\":" << frecuenciaPorPalabra[elegidas[i]];
		salida << "}";
	}
	salida << "]}";

	std::cout << salida.str() << '\n';
	return 0;
}
