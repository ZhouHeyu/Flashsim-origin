#include <libparam/libparam.h>

#ifndef _DISKSIM_IODRIVER_PARAM_H
#define _DISKSIM_IODRIVER_PARAM_H  


/* prototype for disksim_iodriver param loader function */
   struct iodriver *disksim_iodriver_loadparams(struct lp_block *b);

typedef enum {
   DISKSIM_IODRIVER_TYPE,
   DISKSIM_IODRIVER_CONSTANT_ACCESS_TIME,
   DISKSIM_IODRIVER_USE_QUEUEING_IN_SUBSYSTEM,
   DISKSIM_IODRIVER_SCHEDULER
} disksim_iodriver_param_t;

#define DISKSIM_IODRIVER_MAX_PARAM		DISKSIM_IODRIVER_SCHEDULER


static struct lp_varspec disksim_iodriver_params [] = {
   {"type", I, 1 },
   {"Constant access time", D, 1 },
   {"Use queueing in subsystem", I, 1 },
   {"Scheduler", BLOCK, 1 },
   {0,0,0}
};
#define DISKSIM_IODRIVER_MAX 4
static struct lp_mod disksim_iodriver_mod = { "disksim_iodriver", disksim_iodriver_params, DISKSIM_IODRIVER_MAX, (lp_modloader_t)disksim_iodriver_loadparams,  0,0 };
#endif // _DISKSIM_IODRIVER_PARAM_H
