#include <libparam/libparam.h>

#ifndef _DISKSIM_GLOBAL_PARAM_H
#define _DISKSIM_GLOBAL_PARAM_H  


/* prototype for disksim_global param loader function */
   int disksim_global_loadparams(struct lp_block *b);

typedef enum {
   DISKSIM_GLOBAL_INIT_SEED,
   DISKSIM_GLOBAL_INIT_SEED_WITH_TIME,
   DISKSIM_GLOBAL_REAL_SEED,
   DISKSIM_GLOBAL_REAL_SEED_WITH_TIME,
   DISKSIM_GLOBAL_STATISTIC_WARM_UP_TIME,
   DISKSIM_GLOBAL_STATISTIC_WARM_UP_IOS,
   DISKSIM_GLOBAL_STAT_DEFINITION_FILE,
   DISKSIM_GLOBAL_OUTPUT_FILE_FOR_TRACE_OF_IO_REQUESTS_SIMULATED
} disksim_global_param_t;

#define DISKSIM_GLOBAL_MAX_PARAM		DISKSIM_GLOBAL_OUTPUT_FILE_FOR_TRACE_OF_IO_REQUESTS_SIMULATED


static struct lp_varspec disksim_global_params [] = {
   {"Init Seed", I, 0 },
   {"Init Seed with time", I, 0 },
   {"Real Seed", I, 0 },
   {"Real Seed with time", I, 0 },
   {"Statistic warm-up time", D, 0 },
   {"Statistic warm-up IOs", I, 0 },
   {"Stat definition file", S, 1 },
   {"Output file for trace of I/O requests simulated", S, 0 },
   {0,0,0}
};
#define DISKSIM_GLOBAL_MAX 8
static struct lp_mod disksim_global_mod = { "disksim_global", disksim_global_params, DISKSIM_GLOBAL_MAX, (lp_modloader_t)disksim_global_loadparams,  0,0 };
#endif // _DISKSIM_GLOBAL_PARAM_H
