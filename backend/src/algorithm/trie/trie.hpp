#ifndef TRIE_TRIE_HPP
#define TRIE_TRIE_HPP

#include "types.hpp"

#include <string>
#include <vector>

// Trie de caracteres estandar (baseline de la industria, sin optimizaciones):
// cada arista es un byte de la palabra normalizada, cada nodo "fin de
// palabra" lleva asociado un peso de frecuencia.
class Trie {
public:
	~Trie();
	void insertar(const std::string& palabra, long long frecuencia);

	// Devuelve hasta `limite` palabras que empiezan con `prefijo`, ordenadas
	// por frecuencia descendente.
	std::vector<ResultadoPrefijo> buscarPorPrefijo(const std::string& prefijo, int limite) const;

	// Numero total de nodos (para el estimado de memoria del reporte).
	size_t contarNodos() const;

private:
	struct Nodo;
	Nodo* raiz = nullptr;

	void recolectar(const Nodo* nodo, std::string& actual, std::vector<ResultadoPrefijo>& resultados) const;
	void destruir(Nodo* nodo);
};

#endif
