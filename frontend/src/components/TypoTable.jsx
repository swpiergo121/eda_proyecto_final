function TypoTable({ corrections }) {
	return (
		<div className="typo-panel">
			<h3 className="panel-title">Corrección de tipeos (esta sesión)</h3>
			{corrections.length === 0 ? (
				<div className="empty-state">
					Escribe una palabra sin coincidencias de prefijo (por ejemplo, con un error de tipeo) para
					ver aquí lo que encuentra el BK-tree.
				</div>
			) : (
				<table className="typo-table">
					<thead>
						<tr>
							<th>Entrada</th>
							<th>Corrección (BK-tree)</th>
							<th>Distancia</th>
						</tr>
					</thead>
					<tbody>
						{corrections.map((c, i) => (
							<tr key={`${c.query}-${i}`}>
								<td className="typo-input">{c.query}</td>
								<td className="typo-output">{c.suggestion ?? "—"}</td>
								<td>{c.distance ?? "—"}</td>
							</tr>
						))}
					</tbody>
				</table>
			)}
		</div>
	);
}

export default TypoTable;
