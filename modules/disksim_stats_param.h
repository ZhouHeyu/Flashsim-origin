#include <libparam/libparam.h>

#ifndef _DISKSIM_STATS_PARAM_H
#define _DISKSIM_STATS_PARAM_H  


/* prototype for disksim_stats param loader function */
   int disksim_stats_loadparams(struct lp_block *b);

typedef enum {
   DISKSIM_STATS_IODRIVER_STATS,
   DISKSIM_STATS_BUS_STATS,
   DISKSIM_STATS_CTLR_STATS,
   DISKSIM_STATS_DEVICE_STATS,
   DISKSIM_STATS_PROCESS_FLOW_STATS
} disksim_stats_param_t;

#define DISKSIM_STATS_MAX_PARAM		DISKSIM_STATS_PROCESS_FLOW_STATS


static struct lp_varspec disksim_stats_params [] = {
   {"iodriver stats", BLOCK, 1 },
   {"bus stats", BLOCK, 1 },
   {"ctlr stats", BLOCK, 1 },
   {"device stats", BLOCK, 1 },
   {"process flow stats", BLOCK, 1 },
   {0,0,0}
};
#define DISKSIM_STATS_MAX 5
static struct lp_mod disksim_stats_mod = { "disksim_stats", disksim_stats_params, DISKSIM_STATS_MAX, (lp_modloader_t)disksim_stats_loadparams,  0,0 };
#endif // _DISKSIM_STATS_PARAM_H
