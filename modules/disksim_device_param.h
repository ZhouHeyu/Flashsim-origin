#include <libparam/libparam.h>

#ifndef _DISKSIM_DEVICE_PARAM_H
#define _DISKSIM_DEVICE_PARAM_H  


/* prototype for disksim_device param loader function */
   void disksim_device_loadparams(void);

#define DISKSIM_DEVICE_MAX_PARAM		


static struct lp_varspec disksim_device_params [] = {
   {0,0,0}
};
#define DISKSIM_DEVICE_MAX 0
static struct lp_mod disksim_device_mod = { "disksim_device", disksim_device_params, DISKSIM_DEVICE_MAX, (lp_modloader_t)disksim_device_loadparams,  0,0 };
#endif // _DISKSIM_DEVICE_PARAM_H
