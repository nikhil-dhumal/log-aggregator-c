#ifndef LOADGEN_H
#define LOADGEN_H

typedef struct
{
    int rate;
    int threads;
    int duration_sec;
    const char *url;
    const char *csv_path;
} loadgen_config;

int run_loadgen(loadgen_config *cfg);

#endif