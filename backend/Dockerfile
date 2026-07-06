FROM python:3.11-slim

WORKDIR /app

RUN apt-get update \
	&& apt-get install -y --no-install-recommends g++ \
	&& rm -rf /var/lib/apt/lists/*

COPY requirements.txt /app/
RUN pip install --no-cache-dir -r requirements.txt

COPY src /app/src
RUN mkdir -p /app/data /app/out /app/bin \
	&& g++ -O2 -std=gnu++17 -o /app/bin/bk_front \
		src/algorithm/bk_tree/para_front.cpp \
		src/algorithm/bk_tree/bk_tree.cpp \
		src/algorithm/utils/text_utils.cpp \
		src/algorithm/utils/dataset_loader.cpp \
		src/algorithm/utils/json_utils.cpp \
	&& g++ -O2 -std=gnu++17 -o /app/bin/xfast_front \
		src/algorithm/x_fast_trie/xfast_front.cpp \
		src/algorithm/utils/text_utils.cpp \
		src/algorithm/utils/dataset_loader.cpp \
	&& g++ -O2 -std=gnu++17 -o /app/bin/trie_front \
		src/algorithm/trie/trie_front.cpp \
		src/algorithm/trie/trie.cpp \
		src/algorithm/utils/text_utils.cpp \
		src/algorithm/utils/dataset_loader.cpp

EXPOSE 8000
CMD ["python", "-m", "uvicorn", "src.app:app", "--host", "0.0.0.0", "--port", "8000"]