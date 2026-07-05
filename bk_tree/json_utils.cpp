#include "json_utils.hpp"

#include <sstream>

static std::string escaparJson(const std::string& texto) {
	std::string escapado;
	escapado.reserve(texto.size() + 8);

	for (unsigned char c : texto) {
		switch (c) {
			case '"': escapado += "\\\""; break;
			case '\\': escapado += "\\\\"; break;
			case '\b': escapado += "\\b"; break;
			case '\f': escapado += "\\f"; break;
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
				break;
		}
	}

	return escapado;
}

std::string construirErrorJson(const std::string& mensaje) {
	std::ostringstream salida;
	salida << "{"
		   << "\"ok\":false,"
		   << "\"error\":\"" << escaparJson(mensaje) << "\""
		   << "}";
	return salida.str();
}

std::string construirRespuestaJson(const std::string& palabraBuscada,
							   int radio,
							   const std::string& rutaArchivo,
							   size_t tamanoDataset,
							   const std::vector<ResultadoBusqueda>& resultados) {
	std::ostringstream salida;
	salida << "{";
	salida << "\"ok\":true,";
	salida << "\"query\":\"" << escaparJson(palabraBuscada) << "\",";
	salida << "\"radio\":" << radio << ",";
	salida << "\"archivo\":\"" << escaparJson(rutaArchivo) << "\",";
	salida << "\"datasetTamano\":" << tamanoDataset << ",";
	salida << "\"cantidadResultados\":" << resultados.size() << ",";
	salida << "\"resultados\":";
	salida << "[";

	for (size_t i = 0; i < resultados.size(); ++i) {
		if (i > 0) {
			salida << ",";
		}

		const ResultadoBusqueda& resultado = resultados[i];
		salida << "{";
		salida << "\"palabra\":\"" << escaparJson(resultado.palabra) << "\",";
		salida << "\"frecuencia\":" << resultado.frecuencia << ",";
		salida << "\"distancia\":" << resultado.distancia << ",";
		salida << "\"distanciaNormalizada\":" << resultado.distanciaNormalizada;
		salida << "}";
	}

	salida << "]";
	salida << "}";
	return salida.str();
}