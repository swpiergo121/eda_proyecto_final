#include "bk_tree.hpp"

#include "text_utils.hpp"

#include <algorithm>
#include <unordered_map>

struct BKTree::Nodo {
	std::string palabra;
	long long frecuencia;
	std::unordered_map<int, Nodo*> hijos;

	Nodo(const std::string& palabraInicial, long long frecuenciaInicial)
		: palabra(palabraInicial), frecuencia(frecuenciaInicial) {}
};

BKTree::~BKTree() {
	destruir(raiz);
}

void BKTree::destruir(Nodo* nodo) {
	if (nodo == nullptr) {
		return;
	}

	for (auto& hijo : nodo->hijos) {
		destruir(hijo.second);
	}

	delete nodo;
}

void BKTree::insertar(Nodo* nodo, const std::string& palabra, long long frecuencia) {
	int distancia = distanciaLevenshtein(palabra, nodo->palabra);
	auto it = nodo->hijos.find(distancia);

	if (it == nodo->hijos.end()) {
		nodo->hijos[distancia] = new Nodo(palabra, frecuencia);
	} else {
		insertar(it->second, palabra, frecuencia);
	}
}

void BKTree::busqueda(Nodo* nodo, const std::string& palabraBuscada, int radio, std::vector<ResultadoBusqueda>& resultados) const {
	if (nodo == nullptr) return;

	int distancia = distanciaLevenshtein(palabraBuscada, nodo->palabra);

	if (distancia <= radio) {
		resultados.push_back({nodo->palabra, nodo->frecuencia, distancia, distanciaLevenshteinNormalizada(palabraBuscada, nodo->palabra)});
	}

	int limiteInferior = distancia - radio;
	int limiteSuperior = distancia + radio;

	for (const auto& hijo : nodo->hijos) {
		int distanciaArista = hijo.first;
		if (distanciaArista >= limiteInferior && distanciaArista <= limiteSuperior) {
			busqueda(hijo.second, palabraBuscada, radio, resultados);
		}
	}
}

void BKTree::insertar(const std::string& palabra, long long frecuencia) {
	if (raiz == nullptr) {
		raiz = new Nodo(palabra, frecuencia);
		return;
	}

	insertar(raiz, palabra, frecuencia);
}

std::vector<ResultadoBusqueda> BKTree::busqueda(const std::string& palabraBuscada, int radio) const {
	std::vector<ResultadoBusqueda> resultados;
	busqueda(raiz, palabraBuscada, radio, resultados);

	std::sort(resultados.begin(), resultados.end(), [](const ResultadoBusqueda& a, const ResultadoBusqueda& b) {
		if (a.distancia != b.distancia) {
			return a.distancia < b.distancia;
		}

		if (a.frecuencia != b.frecuencia) {
			return a.frecuencia > b.frecuencia;
		}

		return a.palabra < b.palabra;
	});

	return resultados;
}