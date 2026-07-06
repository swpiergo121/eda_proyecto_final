// Wrapper no interactivo para integrar el trie de caracteres (baseline) con
// el backend web. Mismo patron que bk_tree/para_front.cpp y
// x_fast_trie/xfast_front.cpp: recibe <prefijo> <cantidad> <ruta_csv> y
// devuelve JSON por stdout.

#include "../utils/dataset_loader.hpp"
#include "../utils/text_utils.hpp"
#include "trie.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {

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

	Trie trie;
	for (const InfoPalabra& item : palabras) {
		trie.insertar(item.palabra, item.frecuencia);
	}

	std::vector<ResultadoPrefijo> resultados = trie.buscarPorPrefijo(prefijo, cantidad);

	std::ostringstream salida;
	salida << "{";
	salida << "\"ok\":true,";
	salida << "\"query\":\"" << escaparJson(prefijo) << "\",";
	salida << "\"esPrefijoReal\":" << (!resultados.empty() ? "true" : "false") << ",";
	salida << "\"archivo\":\"" << escaparJson(rutaArchivo) << "\",";
	salida << "\"datasetTamano\":" << palabras.size() << ",";
	salida << "\"nodosTotales\":" << trie.contarNodos() << ",";
	salida << "\"cantidadResultados\":" << resultados.size() << ",";
	salida << "\"resultados\":[";
	for (size_t i = 0; i < resultados.size(); ++i) {
		if (i > 0) salida << ",";
		salida << "{";
		salida << "\"palabra\":\"" << escaparJson(resultados[i].palabra) << "\",";
		salida << "\"frecuencia\":" << resultados[i].frecuencia;
		salida << "}";
	}
	salida << "]}";

	std::cout << salida.str() << '\n';
	return 0;
}
