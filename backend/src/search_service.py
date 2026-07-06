"""Orquesta las estructuras nativas (BK-tree y X-fast trie) desde Python.

No reimplementa los algoritmos: invoca los binarios compilados a partir del
codigo en src/algorithm/{bk_tree,x_fast_trie} y combina sus salidas JSON.
"""
import json
import subprocess
import time
from functools import lru_cache
from pathlib import Path

ALGORITHM_DIR = Path(__file__).parent / "algorithm"
DATA_DIR = ALGORITHM_DIR / "bk_tree" / "data"

BIN_DIR = Path("/app/bin")
if not BIN_DIR.exists():
    # desarrollo local fuera de Docker: buscar binarios ya compilados en el repo
    BIN_DIR = ALGORITHM_DIR

BK_BIN_CANDIDATES = [BIN_DIR / "bk_front", ALGORITHM_DIR / "bk_tree" / "para_front"]
XFAST_BIN_CANDIDATES = [BIN_DIR / "xfast_front"]
TRIE_BIN_CANDIDATES = [BIN_DIR / "trie_front"]

DATASETS = {
    # muestra curada (~35 palabras mas frecuentes por letra inicial) para que
    # el x-fast trie tenga variedad de prefijos y siga siendo interactivo:
    # con las 50k palabras completas, las claves generadas por
    # StringXFastTrie::encode comparten muchos bits bajos en cero (padding),
    # lo que degrada el unordered_map<__int128,...> interno a colisiones
    # masivas y la construccion deja de ser interactiva (varios segundos).
    "medium": DATA_DIR / "palabras_frecuencias_demo.csv",
    "large": DATA_DIR / "palabras_frecuencias.csv",
}
DEFAULT_DATASET = "medium"
# el BK-tree si escala bien a 50k palabras (ver README del proyecto), se usa
# solo para el punto de comparacion "a gran escala" del benchmark, nunca para
# consultas en vivo del x-fast trie.
LARGE_DATASET = "large"
FUZZY_RADIUS = 2


def _first_existing(paths):
    for p in paths:
        if p.exists():
            return p
    return None


BK_BIN = _first_existing(BK_BIN_CANDIDATES)
XFAST_BIN = _first_existing(XFAST_BIN_CANDIDATES)
TRIE_BIN = _first_existing(TRIE_BIN_CANDIDATES)


class StructureUnavailable(RuntimeError):
    pass


def _run(binary: Path, args: list[str]) -> tuple[dict, float]:
    if binary is None:
        raise StructureUnavailable("Binario no compilado. Reconstruye la imagen del backend.")
    started = time.perf_counter()
    proc = subprocess.run(
        [str(binary), *args],
        capture_output=True,
        text=True,
        timeout=15,
    )
    elapsed_ms = (time.perf_counter() - started) * 1000
    try:
        payload = json.loads(proc.stdout.strip() or "{}")
    except json.JSONDecodeError as exc:
        raise StructureUnavailable(f"Salida invalida de {binary.name}: {exc}") from exc
    return payload, elapsed_ms


def dataset_path(dataset: str) -> Path:
    return DATASETS.get(dataset, DATASETS[DEFAULT_DATASET])


def run_prefix_search(query: str, limit: int, dataset: str = DEFAULT_DATASET):
    payload, elapsed_ms = _run(XFAST_BIN, [query, str(limit), str(dataset_path(dataset))])
    return payload, elapsed_ms


def run_fuzzy_search(query: str, radius: int, dataset: str = DEFAULT_DATASET):
    payload, elapsed_ms = _run(BK_BIN, [query, str(radius), str(dataset_path(dataset))])
    return payload, elapsed_ms


def run_trie_search(query: str, limit: int, dataset: str = DEFAULT_DATASET):
    payload, elapsed_ms = _run(TRIE_BIN, [query, str(limit), str(dataset_path(dataset))])
    return payload, elapsed_ms


def suggest(query: str, limit: int = 8, dataset: str = DEFAULT_DATASET) -> dict:
    """Corre trie, x-fast trie y BK-tree de forma independiente sobre la
    misma consulta y devuelve los tres resultados por separado (no hay
    fallback ni 'ganador'): la demo muestra las estructuras lado a lado para
    que se vea su comportamiento real bajo el mismo escenario (prefijo
    valido, typo, etc.), tal como pide la seccion 5 del informe."""
    query = query.strip().lower()
    started_total = time.perf_counter()

    if not query:
        empty = {"status": "skipped", "latencyMs": 0.0, "matchType": "none", "suggestions": []}
        return {
            "ok": True,
            "query": query,
            "trie": {**empty},
            "xfast": {**empty},
            "bktree": {**empty},
            "totalLatencyMs": 0.0,
            "datasetSize": 0,
        }

    trie_payload, trie_ms = run_trie_search(query, limit, dataset)
    trie_suggestions = []
    if trie_payload.get("ok"):
        for item in trie_payload.get("resultados", []):
            trie_suggestions.append({"word": item["palabra"], "frequency": item["frecuencia"]})

    trie_result = {
        "status": "ok" if trie_payload.get("ok") else "error",
        "latencyMs": round(trie_ms, 3),
        "matchType": "prefix" if trie_suggestions else "none",
        "suggestions": trie_suggestions,
    }

    xfast_payload, xfast_ms = run_prefix_search(query, limit, dataset)
    es_prefijo_real = xfast_payload.get("esPrefijoReal", False)
    xfast_suggestions = []
    if xfast_payload.get("ok") and es_prefijo_real:
        for item in xfast_payload.get("resultados", []):
            xfast_suggestions.append({"word": item["palabra"], "frequency": item["frecuencia"]})

    xfast_result = {
        "status": "ok" if xfast_payload.get("ok") else "error",
        "latencyMs": round(xfast_ms, 3),
        "matchType": "prefix" if xfast_suggestions else "none",
        "suggestions": xfast_suggestions,
    }

    bk_payload, bk_ms = run_fuzzy_search(query, FUZZY_RADIUS, dataset)
    bktree_suggestions = []
    if bk_payload.get("ok"):
        for item in bk_payload.get("resultados", [])[:limit]:
            bktree_suggestions.append({
                "word": item["palabra"],
                "frequency": item["frecuencia"],
                "distance": item["distancia"],
            })

    bktree_result = {
        "status": "ok" if bk_payload.get("ok") else "error",
        "latencyMs": round(bk_ms, 3),
        "matchType": "fuzzy" if bktree_suggestions else "none",
        "suggestions": bktree_suggestions,
    }

    dataset_size = (
        trie_payload.get("datasetTamano")
        or xfast_payload.get("datasetTamano")
        or bk_payload.get("datasetTamano")
        or 0
    )
    total_ms = (time.perf_counter() - started_total) * 1000

    return {
        "ok": True,
        "query": query,
        "trie": trie_result,
        "xfast": xfast_result,
        "bktree": bktree_result,
        "totalLatencyMs": round(total_ms, 3),
        "datasetSize": dataset_size,
    }


@lru_cache(maxsize=1)
def top_words(limit: int = 8, dataset: str = DEFAULT_DATASET) -> dict:
    """Palabras mas frecuentes del vocabulario, para poblar el estado inicial
    del buscador antes de que el usuario escriba algo (no requiere invocar
    ninguna estructura, es una lectura directa del dataset)."""
    pairs = []
    with open(dataset_path(dataset), encoding="utf-8") as fh:
        next(fh, None)
        for line in fh:
            parts = line.strip().split(",")
            if len(parts) < 2:
                continue
            word, freq_text = parts[0].strip().lower(), parts[1].strip()
            if not word:
                continue
            try:
                pairs.append((word, int(freq_text)))
            except ValueError:
                continue
    pairs.sort(key=lambda p: -p[1])
    top = pairs[:limit]
    return {
        "ok": True,
        "words": [{"word": w, "frequency": f} for w, f in top],
        "datasetSize": len(pairs),
    }


# Formula de memoria del propio informe: cada nodo del BK-tree guarda la
# palabra + frecuencia (8 bytes) + un unordered_map<int,Nodo*> de hijos.
BKTREE_BYTES_PER_NODE = 80
# Estimacion para el trie: unordered_map<char,Nodo*> vacio (~56 B en libstdc++)
# mas ~40 B por cada arista realmente insertada. Nodos reales vienen del
# propio binario (Trie::contarNodos), no es una suposicion sobre el conteo.
TRIE_BYTES_PER_NODE = 56


def _avg(values):
    return round(sum(values) / len(values), 3) if values else None


def _collect(fn, queries, *args):
    """Corre fn(q, *args) para cada query y junta (payloads, latencias)."""
    payloads, latencies = [], []
    for q in queries:
        try:
            payload, ms = fn(q, *args)
            if payload.get("ok"):
                payloads.append(payload)
                latencies.append(ms)
        except StructureUnavailable:
            pass
    return payloads, latencies


@lru_cache(maxsize=1)
def benchmark() -> dict:
    """Compara latencia, tamano indexado y memoria de las tres formas de
    resolver autocompletado/correccion sobre el mismo lote de consultas.
    Se cachea porque cada corrida reconstruye las estructuras nativas desde
    cero (la construccion domina el tiempo total, tal como documenta el
    informe)."""
    sample_queries = ["casa", "amor", "texto", "programa", "computadora"]

    trie_payloads, trie_lat = _collect(run_trie_search, sample_queries, 8, DEFAULT_DATASET)
    trie_large_payloads, trie_large_lat = _collect(run_trie_search, sample_queries, 8, LARGE_DATASET)
    xfast_payloads, xfast_lat = _collect(run_prefix_search, sample_queries, 8, DEFAULT_DATASET)
    bktree_payloads, bktree_lat = _collect(run_fuzzy_search, sample_queries, FUZZY_RADIUS, DEFAULT_DATASET)
    bktree_large_payloads, bktree_large_lat = _collect(run_fuzzy_search, sample_queries, FUZZY_RADIUS, LARGE_DATASET)

    def max_field(payloads, field):
        values = [p[field] for p in payloads if field in p]
        return max(values) if values else None

    dataset_size = len(_load_words_plain(DEFAULT_DATASET))
    dataset_size_large = len(_load_words_plain(LARGE_DATASET))

    trie_nodes = max_field(trie_payloads, "nodosTotales")
    trie_nodes_large = max_field(trie_large_payloads, "nodosTotales")
    bktree_words = max_field(bktree_payloads, "datasetTamano") or dataset_size
    bktree_words_large = max_field(bktree_large_payloads, "datasetTamano") or dataset_size_large
    xfast_leaves = max_field(xfast_payloads, "datasetTamano")

    structures = [
        {
            "key": "trie",
            "name": "Trie ingenuo",
            "type": "prefijo",
            "latencyMs": {"medium": _avg(trie_lat), "large": _avg(trie_large_lat)},
            "indexed": {"medium": dataset_size, "large": dataset_size_large},
            "nodes": {"medium": trie_nodes, "large": trie_nodes_large},
            "memoryBytes": {
                "medium": trie_nodes * TRIE_BYTES_PER_NODE if trie_nodes else None,
                "large": trie_nodes_large * TRIE_BYTES_PER_NODE if trie_nodes_large else None,
            },
            "memoryNote": f"estimado: nodos reales × ~{TRIE_BYTES_PER_NODE} B (unordered_map por nodo)",
        },
        {
            "key": "xfast",
            "name": "X-fast trie",
            "type": "prefijo",
            "latencyMs": {"medium": _avg(xfast_lat), "large": None},
            "indexed": {"medium": xfast_leaves, "large": None},
            "memoryBytes": {"medium": None, "large": None},
            "memoryNote": "no instrumentado: la tabla de niveles (LSS) es un miembro privado",
        },
        {
            "key": "bktree",
            "name": "BK-tree",
            "type": "fuzzy",
            "latencyMs": {"medium": _avg(bktree_lat), "large": _avg(bktree_large_lat)},
            "indexed": {"medium": bktree_words, "large": bktree_words_large},
            "nodes": {"medium": bktree_words, "large": bktree_words_large},
            "memoryBytes": {
                "medium": bktree_words * BKTREE_BYTES_PER_NODE if bktree_words else None,
                "large": bktree_words_large * BKTREE_BYTES_PER_NODE if bktree_words_large else None,
            },
            "memoryNote": f"formula del informe: 1 nodo/palabra × ~{BKTREE_BYTES_PER_NODE} B/nodo",
        },
    ]

    return {
        "ok": True,
        "queries": sample_queries,
        "datasetSize": dataset_size,
        "datasetSizeLarge": dataset_size_large,
        "structures": structures,
    }


@lru_cache(maxsize=4)
def _load_words_plain(dataset: str) -> tuple[str, ...]:
    path = dataset_path(dataset)
    words = []
    with open(path, encoding="utf-8") as fh:
        next(fh, None)
        for line in fh:
            word = line.split(",", 1)[0].strip().lower()
            if word:
                words.append(word)
    return tuple(words)
