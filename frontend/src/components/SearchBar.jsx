function SearchBar({ value, onChange }) {
	return (
		<div className="search-bar">
			<span className="search-icon" aria-hidden="true">
				<svg width="22" height="22" viewBox="0 0 24 24" fill="none">
					<circle cx="11" cy="11" r="7" stroke="currentColor" strokeWidth="2" />
					<line x1="16.5" y1="16.5" x2="21" y2="21" stroke="currentColor" strokeWidth="2" strokeLinecap="round" />
				</svg>
			</span>
			<input
				type="text"
				className="search-input"
				placeholder="Escribe una palabra en español…"
				value={value}
				onChange={(e) => onChange(e.target.value)}
				autoFocus
				spellCheck={false}
				autoComplete="off"
			/>
			{value && (
				<button className="clear-btn" onClick={() => onChange("")} type="button" aria-label="Limpiar">
					×
				</button>
			)}
		</div>
	);
}

export default SearchBar;
