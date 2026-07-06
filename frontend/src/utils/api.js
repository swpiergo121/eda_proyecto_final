import axios from "axios";

const API_BASE_URL = import.meta.env.VITE_API_URL || "http://localhost:8000";

export async function getHealth() {
	const response = await axios.get(`${API_BASE_URL}/health`);
	return response.data;
}

export async function getSuggestions(query, limit = 8, signal) {
	const response = await axios.get(`${API_BASE_URL}/api/suggest`, {
		params: { q: query, limit },
		signal,
	});
	return response.data;
}

export async function getBenchmark() {
	const response = await axios.get(`${API_BASE_URL}/api/benchmark`);
	return response.data;
}

export async function getTopWords(limit = 10) {
	const response = await axios.get(`${API_BASE_URL}/api/top`, { params: { limit } });
	return response.data;
}
