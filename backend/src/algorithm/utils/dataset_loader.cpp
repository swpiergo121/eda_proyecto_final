#include "dataset_loader.hpp"

#include "text_utils.hpp"

#include <fstream>
#include <sstream>

std::vector<InfoPalabra> cargarDatos(const std::string& rutaArchivo) {
	std::vector<InfoPalabra> palabras;

	std::ifstream archivo(rutaArchivo);
	if (!archivo.is_open()) {
		return palabras;
	}

	std::string linea;
	std::getline(archivo, linea);

	while (std::getline(archivo, linea)) {
		linea = recortar(linea);
		if (linea.empty()) {
			continue;
		}

		std::stringstream flujo(linea);
		std::string palabra;
		std::string textoFrecuencia;

		if (!std::getline(flujo, palabra, ',')) {
			continue;
		}

		if (!std::getline(flujo, textoFrecuencia, ',')) {
			continue;
		}

		palabra = recortar(palabra);
		textoFrecuencia = recortar(textoFrecuencia);

		if (palabra.empty() || textoFrecuencia.empty()) {
			continue;
		}

		long long frecuencia = 0;
		try {
			frecuencia = std::stoll(textoFrecuencia);
		} catch (...) {
			continue;
		}

		palabras.push_back({aMinusculasEspanolUtf8Basico(palabra), frecuencia});
	}

	return palabras;
}