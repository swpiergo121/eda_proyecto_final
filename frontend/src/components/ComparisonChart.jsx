const COLOR_VAR = {
	trie: "--chart-trie",
	xfast: "--chart-xfast",
	bktree: "--chart-bktree",
};

const SCALE_LABEL = {
	medium: "demo",
	large: "50k",
};

function formatBytes(bytes) {
	if (bytes == null) return "N/D";
	if (bytes < 1024) return `${bytes} B`;
	if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(0)} KB`;
	return `${(bytes / (1024 * 1024)).toFixed(2)} MB`;
}

function formatMs(ms) {
	if (ms == null) return "N/D";
	return `${ms.toFixed(ms < 1 ? 3 : 1)} ms`;
}

function LatencyBars({ structures }) {
	const rows = [];
	for (const s of structures) {
		for (const scale of ["medium", "large"]) {
			const ms = s.latencyMs[scale];
			if (ms != null) {
				rows.push({ key: `${s.key}-${scale}`, structKey: s.key, label: s.name, scale, ms });
			}
		}
	}
	const maxLog = Math.max(...rows.map((r) => Math.log10(r.ms + 1)), 1);

	return (
		<div className="comparison-bars">
			{rows.map((r) => {
				const widthPct = Math.max((Math.log10(r.ms + 1) / maxLog) * 100, 3);
				return (
					<div key={r.key} className="comparison-row">
						<div className="comparison-row-label">
							<span>{r.label}</span>
							<span className="comparison-row-sub">dataset {SCALE_LABEL[r.scale]}</span>
						</div>
						<div className="comparison-track">
							<div
								className="comparison-fill"
								style={{ width: `${widthPct}%`, background: `var(${COLOR_VAR[r.structKey]})` }}
							/>
						</div>
						<span className="comparison-value">{formatMs(r.ms)}</span>
					</div>
				);
			})}
		</div>
	);
}

function MemoryBars({ structures }) {
	const rows = structures
		.filter((s) => s.memoryBytes.medium != null || s.memoryBytes.large != null)
		.flatMap((s) =>
			["medium", "large"]
				.filter((scale) => s.memoryBytes[scale] != null)
				.map((scale) => ({ key: `${s.key}-${scale}`, structKey: s.key, label: s.name, scale, bytes: s.memoryBytes[scale] }))
		);
	const max = Math.max(...rows.map((r) => r.bytes), 1);

	return (
		<div className="comparison-bars">
			{rows.map((r) => (
				<div key={r.key} className="comparison-row">
					<div className="comparison-row-label">
						<span>{r.label}</span>
						<span className="comparison-row-sub">dataset {SCALE_LABEL[r.scale]}</span>
					</div>
					<div className="comparison-track">
						<div
							className="comparison-fill"
							style={{ width: `${Math.max((r.bytes / max) * 100, 3)}%`, background: `var(${COLOR_VAR[r.structKey]})` }}
						/>
					</div>
					<span className="comparison-value">{formatBytes(r.bytes)}</span>
				</div>
			))}
			<p className="panel-footnote">
				X-fast trie no aparece aquí: su estructura interna (LSS) es un miembro privado y no se puede
				instrumentar sin modificar el código fuente de la estructura.
			</p>
		</div>
	);
}

function SummaryTable({ structures }) {
	return (
		<div className="comparison-table-wrap">
			<table className="comparison-table">
				<thead>
					<tr>
						<th>Estructura</th>
						<th>Tipo</th>
						<th>Latencia (demo)</th>
						<th>Latencia (50k)</th>
						<th>Indexadas</th>
						<th>Memoria estimada</th>
					</tr>
				</thead>
				<tbody>
					{structures.map((s) => (
						<tr key={s.key}>
							<td>
								<span className="table-dot" style={{ background: `var(${COLOR_VAR[s.key]})` }} />
								{s.name}
							</td>
							<td>{s.type === "prefijo" ? "prefijo" : "fuzzy (radio 2)"}</td>
							<td>{formatMs(s.latencyMs.medium)}</td>
							<td>{formatMs(s.latencyMs.large)}</td>
							<td>{s.indexed.medium != null ? s.indexed.medium.toLocaleString("es") : "N/D"}</td>
							<td>
								{formatBytes(s.memoryBytes.medium)}
								<span className="table-note"> · {s.memoryNote}</span>
							</td>
						</tr>
					))}
				</tbody>
			</table>
		</div>
	);
}

function ComparisonChart({ benchmark, loading }) {
	if (loading) {
		return (
			<div className="comparison-panel">
				<h3 className="panel-title">Comparación de estructuras</h3>
				<div className="empty-state">Calculando benchmark…</div>
			</div>
		);
	}

	if (!benchmark) {
		return (
			<div className="comparison-panel">
				<h3 className="panel-title">Comparación de estructuras</h3>
				<div className="empty-state">No se pudo cargar el benchmark del backend.</div>
			</div>
		);
	}

	const { structures } = benchmark;

	return (
		<div className="comparison-panel">
			<h3 className="panel-title">Comparación de estructuras</h3>
			<p className="panel-subtitle">
				Consultas de ejemplo: {benchmark.queries.join(", ")} — demo = {benchmark.datasetSize.toLocaleString("es")}{" "}
				palabras, 50k = {benchmark.datasetSizeLarge.toLocaleString("es")} palabras. Cada medición incluye
				reconstruir la estructura desde cero (no hay estado persistente entre consultas).
			</p>

			<h4 className="panel-section-title">Latencia (construcción + consulta), escala logarítmica</h4>
			<LatencyBars structures={structures} />

			<h4 className="panel-section-title">Memoria estimada</h4>
			<MemoryBars structures={structures} />

			<h4 className="panel-section-title">Resumen</h4>
			<SummaryTable structures={structures} />
		</div>
	);
}

export default ComparisonChart;
