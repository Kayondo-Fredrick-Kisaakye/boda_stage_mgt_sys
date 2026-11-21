# Boda Boda Stage Manager

A basic C++17 OOP application with a browser GUI, REST API, and PostgreSQL database.

## Features

- Register boda boda drivers
- Record check-in and check-out activity
- View recent stage records
- Neon PostgreSQL storage
- Docker deployment to Render

## Local setup

1. Install build dependencies:

```bash
sudo apt-get update
sudo apt-get install -y cmake build-essential libpq-dev libpqxx-dev git
```

2. Create a PostgreSQL database and set `DATABASE_URL`:

```bash
export DATABASE_URL='postgresql://USER:PASSWORD@HOST/DATABASE?sslmode=require'
cmake -S . -B build
cmake --build build
./build/boda_stage
```

Open `http://localhost:8080`.

## Neon

1. Create a Neon project and database.
2. Copy the pooled or direct PostgreSQL connection string.
3. Set it as `DATABASE_URL` in Render. Add `sslmode=require` if it is absent.

## Render

1. Push this folder to GitHub.
2. In Render, choose **New > Blueprint** and select the repository.
3. Set `DATABASE_URL` to the Neon connection string.
4. Deploy; the Dockerfile exposes the configured `PORT` and `/api/health` provides a health check.
