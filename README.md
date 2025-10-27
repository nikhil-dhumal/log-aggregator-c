# Log Aggregator in C

A multi-threaded HTTP-based log aggregation and query system implemented in C.

## Overview
This project is part of **CS744: Design and Engineering of Computer Systems (Autumn 2025, IIT Bombay)**.  
It implements a basic log analytics backend similar to Splunk, featuring an HTTP server, in-memory caching, a persistent database, and a load generator for performance testing.

## Features
- HTTP REST API for log ingestion and querying (`POST /log`, `GET /logs`, `DELETE /logs`)
- Multi-threaded server using pthreads
- In-memory LRU cache for recent logs
- Persistent storage using PostgreSQL or MySQL
- Custom load generator for throughput and latency benchmarking
- Analysis of CPU-bound and I/O-bound workloads

## Components
- **Server:** Multi-threaded HTTP server handling concurrent client requests  
- **Cache:** LRU cache to reduce repeated database lookups  
- **Database:** Stores logs with timestamp, level, and message  
- **Load Generator:** Multi-threaded client to simulate concurrent logging and querying

## Project Goals
1. Build a functionally correct log aggregation system in C  
2. Integrate caching and database for persistence  
3. Perform load tests with different workloads  
4. Identify and analyze performance bottlenecks

## Tech Stack
- **Language:** C (C11)
- **Libraries:** pthread, libmicrohttpd or civetweb, PostgreSQL/MySQL C connector
- **Tools:** gcc, make, htop, iostat, perf
- **Platform:** Linux (Ubuntu)

## Author
**Nikhil Rajendra Dhumal**  
M.Tech CSE, IIT Bombay  
Course: CS744 - Design and Engineering of Computer Systems
