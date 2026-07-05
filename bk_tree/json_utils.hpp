#ifndef BKTREE_JSON_UTILS_HPP
#define BKTREE_JSON_UTILS_HPP

#include "types.hpp"

#include <string>
#include <vector>

std::string construirErrorJson(const std::string& mensaje);
std::string construirRespuestaJson(const std::string& palabraBuscada,
							   int radio,
							   const std::string& rutaArchivo,
							   size_t tamanoDataset,
							   const std::vector<ResultadoBusqueda>& resultados);

#endif