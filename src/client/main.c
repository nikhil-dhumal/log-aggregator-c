#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loadgen.h"

int main(int argc, char *argv[])
{
    loadgen_config cfg;
    cfg.rate = 0;
    cfg.threads = 0;
    cfg.duration_sec = 0;
    cfg.workload = "mixed";

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--url") == 0 && i + 1 < argc)
        {
            cfg.url = argv[++i];
        } 
        else if (strcmp(argv[i], "--rate") == 0 && i + 1 < argc)
        {
            cfg.rate = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc)
        {
            cfg.threads = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--duration") == 0 && i + 1 < argc)
        {
            cfg.duration_sec = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--csv") == 0 && i + 1 < argc)
        {
            cfg.csv_path = argv[++i];
        }
        else if (strcmp(argv[i], "--workload") == 0 && i + 1 < argc)
        {
            cfg.workload = argv[++i];
        }
        else {
            fprintf(stderr, "[loadgen] WARNING: unknown or incomplete argumnet %s\n", argv[i]);
            return 1;
        }
    }

    if (!cfg.url || cfg.rate <= 0 || cfg.threads <= 0 || cfg.duration_sec <= 0 || !cfg.csv_path)
    {
        fprintf(stderr, "[loadgen] ERROR: missing or invalid required arguments\n");
        fprintf(stderr, "Usage: %s --url <url> --rate <rate> --thread <threads> --duration <seconds> --csv <file> [--workload <type>]\n", argv[0]);
        return 1;
    }

    printf("[loadgen] Starting load test: %d req/sec, %d threads, %d seconds, workload = %s\n", cfg.rate, cfg.threads, cfg.duration_sec, cfg.workload);

    int res = run_loadgen(&cfg);

    if (res != 0)
    {
        fprintf(stderr, "[loadgen] ERROR: load generator failed\n");
        return 1;
    }

    printf("[loadgen] Load test completed. Results saved to %s\n", cfg.csv_path);

    return 0;
}