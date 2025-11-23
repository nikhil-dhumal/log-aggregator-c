# Log Aggregator - System Architecture

This document describes the architecture of the **Log Aggregator** (Phase 2).

## Overview
A log aggregator is a system for collecting and storing data from multiple sources in a centralized location. It supports monitoring, debugging, and analyzing system behavior. This version includes asynchronous database writes, caching, and a multi-threaded load generator for benchmarking.

## Architecture Diagram
![System Architecture](architecture.png)

## Architecture

### 1. Load Generator
- Simulates multiple clients sending requests to the Application Server.
- Supports workload types:
  - **write-heavy:** 100% POST 
  - **read-heavy:** 80% GET, 20% POST  
  - **mixed:** 50% POST, 50% GET  
- Generates configurable concurrent load using multiple threads.
- Closed-loop design: a new request is sent only after receiving the previous response.
- Measures:
  - throughput
  - latency
  - success/failure counts
  - failure rate  
- Outputs results in CSV format.

---

### 2. Application Server (Tier 1)

#### **HTTP Server**
- Multithreaded HTTP server built using the **CivetWeb** library.
- Endpoints:
  - `GET /health`
  - `POST /logs` — accepts `level` and `message`
  - `GET /logs` — supports `page`, `offset`, `limit`, `level` filters

#### **Queue**
- All `POST /logs` requests are pushed to a queue.
- A **single worker thread** persists logs to the database asynchronously.
- Prevents blocking under write-heavy workloads.

#### **In-Memory Cache**
- Stores the **last N logs** in a ring buffer.
- `GET /logs` checks the cache first.
- Falls back to database on cache misses.
- N is configurable based on memory limits.

---

### 3. Database Server (Tier 2)
- PostgreSQL database using libpq.
- Logs table:
  - `id` (integer, primary key)
  - `timestamp` (bigint)
  - `level` (varchar)
  - `message` (varchar)
- Persistent storage for all logs.
- Queried only when logs are not found in cache.

---

### 4. Request Flow

#### **POST /logs**
Client → Application Server → Queue → Worker Thread → PostgreSQL → Acknowledgment

#### **GET /logs**
Client → Application Server → Cache → (if cache miss) → PostgreSQL → Acknowledgment

---

### 5. Design Choices
- **Caching last N logs:**  
  Maximizes speed for recent-log reads while controlling memory use.

- **Asynchronous writes via queue:**  
  Improves availability by preventing writes from blocking other requests.

- **Eventual consistency:**  
  Reads may not instantly show the newest writes from the queue, but system responsiveness is maintained.

- **Closed-loop load generation:**  
  Ensures accurate CPU/disk bottleneck measurement without overwhelming the server.

