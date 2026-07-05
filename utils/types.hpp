#ifndef BKTREE_TYPES_HPP
#define BKTREE_TYPES_HPP

#include <string>

struct InfoPalabra {
	std::string palabra;
	long long frecuencia;
};

struct ResultadoBusqueda {
	std::string palabra;
	long long frecuencia;
	int distancia;
	double distanciaNormalizada;
};

#endif