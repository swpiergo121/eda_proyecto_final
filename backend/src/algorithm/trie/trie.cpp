#include "trie.hpp"

#include <algorithm>
#include <unordered_map>

struct Trie::Nodo {
	std::unordered_map<char, Nodo*> hijos;
	bool esFinal = false;
	long long frecuencia = 0;
};

Trie::~Trie() {
	destruir(raiz);
}

void Trie::destruir(Nodo* nodo) {
	if (!nodo) return;
	for (auto& [c, hijo] : nodo->hijos) {
		destruir(hijo);
	}
	delete nodo;
}

void Trie::insertar(const std::string& palabra, long long frecuencia) {
	if (palabra.empty()) return;
	if (!raiz) raiz = new Nodo();

	Nodo* actual = raiz;
	for (char c : palabra) {
		auto it = actual->hijos.find(c);
		if (it == actual->hijos.end()) {
			Nodo* nuevo = new Nodo();
			actual->hijos[c] = nuevo;
			actual = nuevo;
		} else {
			actual = it->second;
		}
	}
	actual->esFinal = true;
	actual->frecuencia = frecuencia;
}

void Trie::recolectar(const Nodo* nodo, std::string& actual, std::vector<ResultadoPrefijo>& resultados) const {
	if (!nodo) return;
	if (nodo->esFinal) {
		resultados.push_back({actual, nodo->frecuencia});
	}
	for (const auto& [c, hijo] : nodo->hijos) {
		actual.push_back(c);
		recolectar(hijo, actual, resultados);
		actual.pop_back();
	}
}

std::vector<ResultadoPrefijo> Trie::buscarPorPrefijo(const std::string& prefijo, int limite) const {
	std::vector<ResultadoPrefijo> resultados;
	if (!raiz) return resultados;

	const Nodo* actual = raiz;
	for (char c : prefijo) {
		auto it = actual->hijos.find(c);
		if (it == actual->hijos.end()) {
			return resultados; // ningun nodo tiene ese prefijo
		}
		actual = it->second;
	}

	std::string acumulado = prefijo;
	recolectar(actual, acumulado, resultados);

	std::sort(resultados.begin(), resultados.end(), [](const ResultadoPrefijo& a, const ResultadoPrefijo& b) {
		return a.frecuencia > b.frecuencia;
	});

	if (resultados.size() > static_cast<size_t>(limite)) {
		resultados.resize(limite);
	}
	return resultados;
}

size_t Trie::contarNodos() const {
	std::vector<const Nodo*> pila;
	if (raiz) pila.push_back(raiz);
	size_t total = 0;
	while (!pila.empty()) {
		const Nodo* actual = pila.back();
		pila.pop_back();
		++total;
		for (const auto& [c, hijo] : actual->hijos) {
			pila.push_back(hijo);
		}
	}
	return total;
}
