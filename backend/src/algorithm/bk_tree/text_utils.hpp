#ifndef BKTREE_TEXT_UTILS_HPP
#define BKTREE_TEXT_UTILS_HPP

#include <string>

double normalizedLevenshteinDistance(const std::string& a, const std::string& b);
std::string recortar(const std::string& texto);
std::string aMinusculasEspanolUtf8Basico(const std::string& texto);
std::u32string utf8ACodepoints(const std::string& texto);
int distanciaLevenshtein(const std::string& textoA, const std::string& textoB);
double distanciaLevenshteinNormalizada(const std::string& textoA, const std::string& textoB);

#endif