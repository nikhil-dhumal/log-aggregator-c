# Log Aggregator in C

A multi-tier HTTP-based log aggregation and query system implemented in C.

## Overview
This project is part of **CS744: Design and Engineering of Computer Systems (Autumn 2025, IIT Bombay)**.  
It implements a log aggregator that collects logs from multiple clients, stores them in PostgreSQL, and provides fast access to recent logs using an in-memory cache. Future work includes a load generator and additional performance features.

## Features
- HTTP REST API for log ingestion and querying (`POST /logs`, `GET /logs`)
- Multi-threaded HTTP server using civetweb (handles concurrent requests internally)
- In-memory cache storing the last N logs for fast reads
- Persistent storage using PostgreSQL
- Queue for asynchronous writes to database
- Future: Custom load generator for performance testing

## Components
- **Server:** HTTP server handling multiple clients concurrently
- **Cache:** In-memory cache for quick access to recent logs
- **Queue:** Handles asynchronous writes to database
- **Database:** Stores all logs persistently
- **Client:** Multiple clients send HTTP requests concurrently
- **Future:** Load generator to simulate high traffic and test performance

## Installation

### Prerequisites
- Linux (Ubuntu recommended)
- GCC (C compiler)
- PostgreSQL
- `libpq` C library
- `libcur` C library
- `make`

### Setup PostgreSQL
```bash
-- Log in as postgres user
sudo -u postgres psql

-- Create a database
CREATE DATABASE logsdb;

-- Create a user
CREATE USER loguser WITH PASSWORD 'logpassword';

-- Grant privileges
GRANT ALL PRIVILEGES ON DATABASE logsdb TO loguser;

-- Switch to the database
\c logsdb

-- Grant schema privileges
GRANT ALL PRIVILEGES ON SCHEMA public TO loguser;

-- Exit
\q
```

### Set Environment Variable
```bash
export LOGDB_CONN="host=localhost dbname=logsdb user=loguser password=logpassword"
```

> NOTE: Replace 'logpassword' with your own password

### Build
```bash
git clone https://github.com/nikhil-dhumal/log-aggregator-c.git
cd log-aggregator-c
make
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

## Author
**Nikhil Rajendra Dhumal**  
M.Tech CSE, IIT Bombay  
Course: CS744 - Design and Engineering of Computer Systems