function highlightPrefix(word, query) {
	if (!query || !word.toLowerCase().startsWith(query.toLowerCase())) {
		return <span>{word}</span>;
	}
	const head = word.slice(0, query.length);
	const rest = word.slice(query.length);
	return (
		<span>
			<span className="suggestion-prefix">{head}</span>
			{rest}
		</span>
	);
}

const EMPTY_MESSAGE = {
	trie: "El prefijo no existe en el vocabulario de la demo.",
	xfast: "El prefijo no existe en el vocabulario de la demo.",
	bktree: "Ninguna palabra del vocabulario cae dentro del radio de edición 2.",
};

function AlgorithmPanel({ panelKey, title, subtitle, query, result, idleWords, loading, onPickWord }) {
	const isIdle = !query;
	const status = loading ? "active" : result?.status ?? "skipped";

	return (
		<div className={`algo-panel algo-${panelKey} status-${status}`}>
			<div className="algo-panel-header">
				<div>
					<h3 className="algo-panel-title">{title}</h3>
					<p className="algo-panel-subtitle">{subtitle}</p>
				</div>
				{!isIdle && (
					<div className="algo-latency">
						{loading ? (
							<span className="latency-spinner" aria-hidden="true" />
						) : (
							<>
								<span className="latency-value">{result?.latencyMs?.toFixed(1) ?? "–"}</span>
								<span className="latency-unit">ms</span>
							</>
						)}
					</div>
				)}
			</div>

			{isIdle ? (
				<>
					<p className="algo-idle-label">Palabras más frecuentes del vocabulario</p>
					<div className="chip-row">
						{idleWords.map((w) => (
							<button key={w.word} className="chip" onClick={() => onPickWord(w.word)} type="button">
								{w.word}
							</button>
						))}
					</div>
				</>
			) : loading ? (
				<div className="empty-state">Consultando…</div>
			) : !result || result.suggestions.length === 0 ? (
				<div className="empty-state">{EMPTY_MESSAGE[panelKey]}</div>
			) : (
				<ul className="suggestions-list">
					{result.suggestions.map((s, i) => (
						<li key={`${s.word}-${i}`} className="suggestion-row">
							<span className="suggestion-word">{highlightPrefix(s.word, query)}</span>
							<span className="suggestion-freq">freq {s.frequency.toLocaleString("es")}</span>
							{s.distance != null && <span className="suggestion-distance">dist {s.distance}</span>}
						</li>
					))}
				</ul>
			)}
		</div>
	);
}

export default AlgorithmPanel;
