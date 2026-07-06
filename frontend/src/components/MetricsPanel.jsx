function average(values) {
	if (!values.length) return null;
	return values.reduce((a, b) => a + b, 0) / values.length;
}

const STRUCTURES = [
	{ key: "trie", name: "Trie", colorVar: "--chart-trie", hitLabel: "prefijo encontrado" },
	{ key: "xfast", name: "X-fast trie", colorVar: "--chart-xfast", hitLabel: "prefijo encontrado" },
	{ key: "bktree", name: "BK-tree", colorVar: "--chart-bktree", hitLabel: "corrección encontrada" },
];

function MetricsPanel({ history, datasetSize }) {
	const queriesCount = history.length;

	const perStructure = STRUCTURES.map((s) => {
		const latencies = history.map((h) => h[s.key]?.latencyMs).filter((v) => v != null);
		const hits = history.filter((h) => h[s.key]?.matchType && h[s.key].matchType !== "none").length;
		return {
			...s,
			avgLatency: average(latencies),
			hitRate: queriesCount ? (hits / queriesCount) * 100 : 0,
		};
	});

	return (
		<div className="metrics-panel">
			<h3 className="panel-title">Métricas de rendimiento (esta sesión)</h3>
			<div className="metrics-grid">
				<div className="metric-tile">
					<span className="metric-value">{datasetSize ? datasetSize.toLocaleString("es") : "–"}</span>
					<span className="metric-label">Vocabulario indexado</span>
				</div>
				<div className="metric-tile">
					<span className="metric-value">{queriesCount}</span>
					<span className="metric-label">Consultas realizadas</span>
				</div>
			</div>

			<table className="metrics-table">
				<thead>
					<tr>
						<th>Estructura</th>
						<th>Latencia promedio</th>
						<th>Tasa de acierto</th>
					</tr>
				</thead>
				<tbody>
					{perStructure.map((s) => (
						<tr key={s.key}>
							<td>
								<span className="table-dot" style={{ background: `var(${s.colorVar})` }} />
								{s.name}
							</td>
							<td>{s.avgLatency != null ? `${s.avgLatency.toFixed(1)} ms` : "–"}</td>
							<td>
								{queriesCount ? `${s.hitRate.toFixed(0)} %` : "–"}
								<span className="table-note">{s.hitLabel}</span>
							</td>
						</tr>
					))}
				</tbody>
			</table>
		</div>
	);
}

export default MetricsPanel;
