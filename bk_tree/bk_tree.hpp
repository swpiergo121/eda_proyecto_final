#ifndef BKTREE_BK_TREE_HPP
#define BKTREE_BK_TREE_HPP

#include "types.hpp"

#include <string>
#include <vector>

class BKTree {
public:
	~BKTree();
	void insertar(const std::string& palabra, long long frecuencia);
	std::vector<ResultadoBusqueda> busqueda(const std::string& palabraBuscada, int radio) const;

private:
	struct Nodo;
	Nodo* raiz = nullptr;

	void insertar(Nodo* nodo, const std::string& palabra, long long frecuencia);
	void busqueda(Nodo* nodo, const std::string& palabraBuscada, int radio, std::vector<ResultadoBusqueda>& resultados) const;
	void destruir(Nodo* nodo);
};

#endif