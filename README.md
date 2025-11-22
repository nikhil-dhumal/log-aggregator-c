# Log Aggregator in C

A multi-tier HTTP-based log aggregation and query system implemented in C.

## Overview
This project is part of **CS744: Design and Engineering of Computer Systems (Autumn 2025, IIT Bombay)**.  
It implements a multi-tier log aggregation system in C that collects logs from multiple clients, stores them in PostgreSQL, and provides fast access to recent logs via an in-memory cache.  
The system also includes a custom load generator to simulate different workloads and measure system performance.  

## Features
- HTTP REST API for log ingestion and querying (`POST /logs`, `GET /logs`)
- Multi-threaded HTTP server using CivetWeb for handling concurrent requests
- In-memory cache storing the last N logs for fast reads
- Persistent storage using PostgreSQL
- Queue for asynchronous writes to the database to reduce write latency
- Custom load generator to simulate different workloads and measure performance

## Components
- **Server:** HTTP server handling multiple clients concurrently
- **Cache:** In-memory cache storing recent logs for fast reads
- **Queue:** Manages asynchronous writes to the database
- **Database:** Persistent storage using PostgreSQL
- **Client:** Generates HTTP requests to test the server
- **Load Generator:** Simulates different workloads (write-heavy, read-heavy, mixed) and measures performance

## Installation

### Prerequisites
- Linux (Ubuntu recommended)
- GCC (C compiler)
- PostgreSQL
- `libpq` C library
- `libcurl` C library
- `make`

### Setup PostgreSQL
```bash
# Log in as postgres user
sudo -u postgres psql

# Create a database
CREATE DATABASE logsdb;

# Create a user
CREATE USER loguser WITH PASSWORD 'logpassword';

# Grant privileges
GRANT ALL PRIVILEGES ON DATABASE logsdb TO loguser;

# Switch to the database
\c logsdb

# Grant schema privileges
GRANT ALL PRIVILEGES ON SCHEMA public TO loguser;

# Exit
\q
```

### Set Environment Variable
```bash
export LOGDB_CONN="host=localhost dbname=logsdb user=loguser password=logpassword"
```

> NOTE: Replace 'logpassword' with your own password

### Build
```bash
# Clone the repository
git clone https://github.com/nikhil-dhumal/log-aggregator-c.git
cd log-aggregator-c

# Build the project (this builds both the server and the load generator)
make all
```

### Run
#### 1. Run the server:
```bash
./log_aggregator
```

> NOTE: Verify environment variable is set correctly.
```bash 
echo $LOGDB_CONN
``` 

#### 2. Test using curl or Postman:
```bash
# Post log
curl -X POST http://localhost:8080/logs \
  -H "Content-Type: application/json" \
  -d '{"level":"INFO","message":"This is a test log"}'

# Get logs
curl http://localhost:8080/logs?limit=10
```

#### 3. Run the Load Generator:
```bash
./loadgen \
  --url http://localhost:8080/logs \
  --rate 100 \
  --threads 10 \
  --duration 30 \
  --csv results.csv \
  --workload mixed
```

> NOTE: The CSV file (`results.csv`) will contain metrics for the test, including:
> - Success count
> - Failure count
> - Average response time (ms)
> - Average throughput (requests/sec)
> - Failure rate (%)

#### Command-line Flags

- `--url <server_url>` : **Required**. The base URL of the log aggregator server (e.g., `http://localhost:8080/logs`).
- `--rate <requests_per_second>` : **Required**. Number of requests **per second per thread**.
- `--threads <number_of_threads>` : **Required**. Number of concurrent worker threads generating load.
- `--duration <seconds>` : **Required**. Total duration of the test in seconds.
- `--csv <output_csv_file>` : **Required**. Path to save the CSV metrics output.
- `--workload <type>` : **Optional**. Workload type, default is `mixed`.

#### Workload Types

The workload type determines the **ratio of POST (write) vs GET (read) requests** each thread generates:

- `write-heavy` : 80% POST (log ingestion), 20% GET (log querying)  
- `read-heavy` : 80% GET, 20% POST  
- `mixed` : 50% POST, 50% GET  

## Author
**Nikhil Rajendra Dhumal**  
M.Tech CSE, IIT Bombay  
Course: CS744 - Design and Engineering of Computer Systems