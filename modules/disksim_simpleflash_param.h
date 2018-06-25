#include <libparam/libparam.h>

#ifndef _DISKSIM_SIMPLEFLASH_PARAM_H
#define _DISKSIM_SIMPLEFLASH_PARAM_H  


/* prototype for disksim_simpleflash param loader function */
   struct simpleflash *disksim_simpleflash_loadparams(struct lp_block *b);

typedef enum {
   DISKSIM_SIMPLEFLASH_SCHEDULER,
   DISKSIM_SIMPLEFLASH_MAX_QUEUE_LENGTH,
   DISKSIM_SIMPLEFLASH_BLOCK_COUNT,
   DISKSIM_SIMPLEFLASH_EXTRA_BLOCK_COUNT,
   DISKSIM_SIMPLEFLASH_FTL_TYPE,
   DISKSIM_SIMPLEFLASH_BUS_TRANSACTION_LATENCY,
   DISKSIM_SIMPLEFLASH_BULK_SECTOR_TRANSFER_TIME,
   DISKSIM_SIMPLEFLASH_NEVER_DISCONNECT,
   DISKSIM_SIMPLEFLASH_PRINT_STATS
} disksim_simpleflash_param_t;

#define DISKSIM_SIMPLEFLASH_MAX_PARAM		DISKSIM_SIMPLEFLASH_PRINT_STATS


static struct lp_varspec disksim_simpleflash_params [] = {
   {"Scheduler", BLOCK, 1 },
   {"Max queue length", I, 1 },
   {"Block count", I, 1 },
   {"Extra Block count", I, 1 },
   {"FTL type", I, 1 },
   {"Bus transaction latency", D, 0 },
   {"Bulk sector transfer time", D, 1 },
   {"Never disconnect", I, 1 },
   {"Print stats", I, 1 },
   {0,0,0}
};
#define DISKSIM_SIMPLEFLASH_MAX 9
static struct lp_mod disksim_simpleflash_mod = { "disksim_simpleflash", disksim_simpleflash_params, DISKSIM_SIMPLEFLASH_MAX, (lp_modloader_t)disksim_simpleflash_loadparams,  0,0 };
#endif // _DISKSIM_SIMPLEFLASH_PARAM_H
