FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git libpq-dev libpqxx-dev ca-certificates && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)

ENV PORT=10000
EXPOSE 10000
CMD ["./build/boda_stage"]
