#if !defined(STATS_H)
#define STATS_H

#include <stdio.h>

typedef struct core_stat_t {
    uint8_t core_id;
    uint8_t utilization;
} core_stat_t;

typedef struct utilization_t {
    size_t core_num;
    core_stat_t* core[];
} utilization_t;

utilization_t* stats_get();

void init_stats(void);

#endif  // STATS_H
