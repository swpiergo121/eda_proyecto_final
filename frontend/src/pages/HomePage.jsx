import { useEffect, useRef, useState } from "react";
import SearchBar from "../components/SearchBar";
import AlgorithmPanel from "../components/AlgorithmPanel";
import MetricsPanel from "../components/MetricsPanel";
import ComparisonChart from "../components/ComparisonChart";
import TypoTable from "../components/TypoTable";
import { getSuggestions, getBenchmark, getTopWords } from "../utils/api";

const DEBOUNCE_MS = 150;
const MAX_HISTORY = 50;

function HomePage() {
	const [query, setQuery] = useState("");
	const [result, setResult] = useState(null);
	const [loading, setLoading] = useState(false);
	const [error, setError] = useState(null);
	const [history, setHistory] = useState([]);
	const [corrections, setCorrections] = useState([]);
	const [topWords, setTopWords] = useState([]);
	const [benchmark, setBenchmark] = useState(null);
	const [benchmarkLoading, setBenchmarkLoading] = useState(true);

	const debounceRef = useRef(null);
	const abortRef = useRef(null);
	const lastCorrectionQuery = useRef(null);

	useEffect(() => {
		getTopWords(10).then((data) => setTopWords(data.words)).catch(() => setTopWords([]));
		getBenchmark()
			.then(setBenchmark)
			.catch(() => setBenchmark(null))
			.finally(() => setBenchmarkLoading(false));
	}, []);

	useEffect(() => {
		clearTimeout(debounceRef.current);

		if (!query.trim()) {
			setResult(null);
			setLoading(false);
			setError(null);
			return;
		}

		setLoading(true);
		debounceRef.current = setTimeout(() => {
			abortRef.current?.abort();
			const controller = new AbortController();
			abortRef.current = controller;

			getSuggestions(query.trim(), 8, controller.signal)
				.then((data) => {
					setResult(data);
					setError(null);
					setHistory((prev) => [...prev.slice(-(MAX_HISTORY - 1)), data]);

					const topBk = data.bktree.suggestions[0];
					if (
						data.xfast.matchType === "none" &&
						topBk &&
						lastCorrectionQuery.current !== data.query
					) {
						lastCorrectionQuery.current = data.query;
						setCorrections((prev) => [
							{ query: data.query, suggestion: topBk.word, distance: topBk.distance },
							...prev,
						].slice(0, 12));
					}
				})
				.catch((err) => {
					if (err.name === "CanceledError" || err.name === "AbortError") return;
					setError(err.message);
				})
				.finally(() => setLoading(false));
		}, DEBOUNCE_MS);

		return () => clearTimeout(debounceRef.current);
	}, [query]);

	return (
		<div className="app-shell">
			<header className="header">
				<h1>Autocompletado y Búsqueda Aproximada</h1>
				<span className="header-sub">Trie · X-fast trie · BK-tree</span>
			</header>

			<section className="hero">
				<h2 className="hero-title">Escribe una palabra</h2>
				<p className="hero-subtitle">
					Cada consulta se resuelve con las tres estructuras al mismo tiempo — compara su latencia y
					sus resultados lado a lado.
				</p>
				<SearchBar value={query} onChange={setQuery} />
				{error && <div className="fallback-notice">No se pudo conectar con el backend: {error}</div>}
			</section>

			<section className="algo-grid">
				<AlgorithmPanel
					panelKey="trie"
					title="Trie ingenuo"
					subtitle="prefijo, sin optimizaciones"
					query={query.trim()}
					result={result?.trie}
					idleWords={topWords}
					loading={loading}
					onPickWord={setQuery}
				/>
				<AlgorithmPanel
					panelKey="xfast"
					title="X-fast trie"
					subtitle="predecesor/sucesor por prefijo"
					query={query.trim()}
					result={result?.xfast}
					idleWords={topWords}
					loading={loading}
					onPickWord={setQuery}
				/>
				<AlgorithmPanel
					panelKey="bktree"
					title="BK-tree"
					subtitle="distancia de Levenshtein, radio 2"
					query={query.trim()}
					result={result?.bktree}
					idleWords={topWords}
					loading={loading}
					onPickWord={setQuery}
				/>
			</section>

			<section className="secondary-grid">
				<MetricsPanel history={history} datasetSize={result?.datasetSize || benchmark?.datasetSize || 0} />
				<TypoTable corrections={corrections} />
			</section>

			<section className="deep-dive">
				<h2 className="deep-dive-title">Benchmark completo</h2>
				<p className="deep-dive-subtitle">
					Comparación agregada del Trie, X-fast trie y BK-tree, incluyendo el dataset completo de 50k
					palabras.
				</p>
				<ComparisonChart benchmark={benchmark} loading={benchmarkLoading} />
			</section>
		</div>
	);
}

export default HomePage;
