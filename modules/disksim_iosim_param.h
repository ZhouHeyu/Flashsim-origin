#include <libparam/libparam.h>

#ifndef _DISKSIM_IOSIM_PARAM_H
#define _DISKSIM_IOSIM_PARAM_H  


/* prototype for disksim_iosim param loader function */
   int disksim_iosim_loadparams(struct lp_block *b);

typedef enum {
   DISKSIM_IOSIM_IO_TRACE_TIME_SCALE,
   DISKSIM_IOSIM_IO_MAPPINGS
} disksim_iosim_param_t;

#define DISKSIM_IOSIM_MAX_PARAM		DISKSIM_IOSIM_IO_MAPPINGS


static struct lp_varspec disksim_iosim_params [] = {
   {"I/O Trace Time Scale", D, 0 },
   {"I/O Mappings", LIST, 0 },
   {0,0,0}
};
#define DISKSIM_IOSIM_MAX 2
static struct lp_mod disksim_iosim_mod = { "disksim_iosim", disksim_iosim_params, DISKSIM_IOSIM_MAX, (lp_modloader_t)disksim_iosim_loadparams,  0,0 };
#endif // _DISKSIM_IOSIM_PARAM_H
