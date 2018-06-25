#include <libparam/libparam.h>

#ifndef _DISKSIM_LOGORG_PARAM_H
#define _DISKSIM_LOGORG_PARAM_H  


/* prototype for disksim_logorg param loader function */
   struct logorg *disksim_logorg_loadparams(struct lp_block *b);

typedef enum {
   DISKSIM_LOGORG_ADDRESSING_MODE,
   DISKSIM_LOGORG_DISTRIBUTION_SCHEME,
   DISKSIM_LOGORG_REDUNDANCY_SCHEME,
   DISKSIM_LOGORG_COMPONENTS,
   DISKSIM_LOGORG_DEVICES,
   DISKSIM_LOGORG_STRIPE_UNIT,
   DISKSIM_LOGORG_SYNCH_WRITES_FOR_SAFETY,
   DISKSIM_LOGORG_NUMBER_OF_COPIES,
   DISKSIM_LOGORG_COPY_CHOICE_ON_READ,
   DISKSIM_LOGORG_RMW_VS_RECONSTRUCT,
   DISKSIM_LOGORG_PARITY_STRIPE_UNIT,
   DISKSIM_LOGORG_PARITY_ROTATION_TYPE,
   DISKSIM_LOGORG_TIME_STAMP_INTERVAL,
   DISKSIM_LOGORG_TIME_STAMP_START_TIME,
   DISKSIM_LOGORG_TIME_STAMP_STOP_TIME,
   DISKSIM_LOGORG_TIME_STAMP_FILE_NAME
} disksim_logorg_param_t;

#define DISKSIM_LOGORG_MAX_PARAM		DISKSIM_LOGORG_TIME_STAMP_FILE_NAME


static struct lp_varspec disksim_logorg_params [] = {
   {"Addressing mode", S, 1 },
   {"Distribution scheme", S, 1 },
   {"Redundancy scheme", S, 1 },
   {"Components", S, 0 },
   {"devices", LIST, 1 },
   {"Stripe unit", I, 1 },
   {"Synch writes for safety", I, 1 },
   {"Number of copies", I, 1 },
   {"Copy choice on read", I, 1 },
   {"RMW vs. reconstruct", D, 1 },
   {"Parity stripe unit", I, 1 },
   {"Parity rotation type", I, 1 },
   {"Time stamp interval", D, 1 },
   {"Time stamp start time", D, 1 },
   {"Time stamp stop time", D, 1 },
   {"Time stamp file name", S, 1 },
   {0,0,0}
};
#define DISKSIM_LOGORG_MAX 16
static struct lp_mod disksim_logorg_mod = { "disksim_logorg", disksim_logorg_params, DISKSIM_LOGORG_MAX, (lp_modloader_t)disksim_logorg_loadparams,  0,0 };
#endif // _DISKSIM_LOGORG_PARAM_H
