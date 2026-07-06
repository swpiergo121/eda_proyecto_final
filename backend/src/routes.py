from fastapi import APIRouter, HTTPException, Query

from src import search_service

router = APIRouter()


@router.get("/health")
def health():
	return {"status": "ok"}


@router.get("/api/suggest")
def suggest(
	q: str = Query(..., min_length=0, max_length=64),
	limit: int = Query(8, ge=1, le=20),
	dataset: str = Query(search_service.DEFAULT_DATASET),
):
	try:
		return search_service.suggest(q, limit=limit, dataset=dataset)
	except search_service.StructureUnavailable as exc:
		raise HTTPException(status_code=503, detail=str(exc)) from exc


@router.get("/api/benchmark")
def benchmark():
	try:
		return search_service.benchmark()
	except search_service.StructureUnavailable as exc:
		raise HTTPException(status_code=503, detail=str(exc)) from exc


@router.get("/api/top")
def top(limit: int = Query(8, ge=1, le=20)):
	return search_service.top_words(limit=limit)
