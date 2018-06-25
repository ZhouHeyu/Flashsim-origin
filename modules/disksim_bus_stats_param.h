#include <libparam/libparam.h>

#ifndef _DISKSIM_BUS_STATS_PARAM_H
#define _DISKSIM_BUS_STATS_PARAM_H  


/* prototype for disksim_bus_stats param loader function */
   int disksim_bus_stats_loadparams(struct lp_block *b);

typedef enum {
   DISKSIM_BUS_STATS_PRINT_BUS_IDLE_STATS,
   DISKSIM_BUS_STATS_PRINT_BUS_ARBWAIT_STATS
} disksim_bus_stats_param_t;

#define DISKSIM_BUS_STATS_MAX_PARAM		DISKSIM_BUS_STATS_PRINT_BUS_ARBWAIT_STATS


static struct lp_varspec disksim_bus_stats_params [] = {
   {"Print bus idle stats", I, 1 },
   {"Print bus arbwait stats", I, 1 },
   {0,0,0}
};
#define DISKSIM_BUS_STATS_MAX 2
static struct lp_mod disksim_bus_stats_mod = { "disksim_bus_stats", disksim_bus_stats_params, DISKSIM_BUS_STATS_MAX, (lp_modloader_t)disksim_bus_stats_loadparams,  0,0 };
#endif // _DISKSIM_BUS_STATS_PARAM_H
