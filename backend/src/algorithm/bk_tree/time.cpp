#include "../utils/dataset_loader.hpp"
#include "../utils/json_utils.hpp"
#include "../utils/text_utils.hpp"
#include "bk_tree.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace std::chrono;

struct BenchmarkResult {
  string fileName;
  size_t datasetSize;
  double constructionTimeMs;
  double queryTimeMs;
};

// New function to save results to a CSV file
void saveResultsToFile(const string &outputFileName,
                       const vector<BenchmarkResult> &report) {
  ofstream outFile(outputFileName);
  if (!outFile.is_open()) {
    cerr << "Could not open " << outputFileName << " for writing." << endl;
    return;
  }

  // CSV Header
  outFile << "File,DatasetSize,BuildTimeMs,QueryTimeMs" << endl;

  for (const auto &r : report) {
    outFile << r.fileName << "," << r.datasetSize << "," << r.constructionTimeMs
            << "," << r.queryTimeMs << "\n";
  }

  outFile.close();
  cout << "Results successfully saved to " << outputFileName << endl;
}

int main() {
  // 1. Configuration
  vector<string> files = {
      "data/palabras_frecuencias_100.csv",
      "data/palabras_frecuencias_1000.csv",
      "data/palabras_frecuencias_10000.csv",
      "data/palabras_frecuencias_20000.csv",
      // "data/palabras_frecuencias_30000.csv",
      // "data/palabras_frecuencias_40000.csv",
      // "data/palabras_frecuencias.csv",
  };
  string queryWord = "hola";
  int radio = 4;
  vector<BenchmarkResult> report;

  cout << left << setw(20) << "File" << setw(15) << "Size" << setw(20)
       << "Build Time (ms)" << setw(20) << "Query Time (ms)" << endl;
  cout << string(75, '-') << endl;

  for (const string &filePath : files) {
    try {
      // --- MEASURE CONSTRUCTION ---
      auto startBuild = high_resolution_clock::now();

      // Load data
      vector<InfoPalabra> palabras = cargarDatos(filePath);
      if (palabras.empty())
        continue;

      // Sort by frequency (as per your requirement)
      sort(palabras.begin(), palabras.end(),
           [](const InfoPalabra &a, const InfoPalabra &b) {
             return a.frecuencia > b.frecuencia;
           });

      // Build Trie
      BKTree tree;
      vector<string> skipped;
      for (const auto &item : palabras) {
        try {
          tree.insertar(item.palabra, item.frecuencia);
        } catch (...) {
          skipped.push_back(item.palabra);
        }
      }

      auto endBuild = high_resolution_clock::now();
      duration<double, std::milli> buildTime = endBuild - startBuild;

      // --- MEASURE QUERY ---
      auto startQuery = high_resolution_clock::now();

      vector<ResultadoBusqueda> results = tree.busqueda(queryWord, radio);
      std::vector<ResultadoBusqueda> resultados =
          tree.busqueda(queryWord, radio);

      auto endQuery = high_resolution_clock::now();
      duration<double, std::milli> queryTime = endQuery - startQuery;

      // Store and Print results
      report.push_back(
          {filePath, palabras.size(), buildTime.count(), queryTime.count()});

      cout << left << setw(20) << filePath << setw(15) << palabras.size()
           << setw(20) << fixed << setprecision(3) << buildTime.count()
           << setw(20) << fixed << setprecision(3) << queryTime.count() << endl;

    } catch (const exception &e) {
      cerr << "Error processing " << filePath << ": " << e.what() << endl;
    }
  }

  // Save all results to the file
  string outputCsv = "results/bk_tree_benchmark_results1.csv";
  saveResultsToFile(outputCsv, report);

  return 0;
}
