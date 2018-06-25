/*Adapted from disksim_simpledisk.c
 * Adaptation Authors: Kanishk Jain
*/ 

/*
 * DiskSim Storage Subsystem Simulation Environment (Version 3.0)
 * Revision Authors: John Bucy, Greg Ganger
 * Contributors: John Griffin, Jiri Schindler, Steve Schlosser
 *
 * Copyright (c) of Carnegie Mellon University, 2001, 2002, 2003.
 *
 * This software is being provided by the copyright holders under the
 * following license. By obtaining, using and/or copying this software,
 * you agree that you have read, understood, and will comply with the
 * following terms and conditions:
 *
 * Permission to reproduce, use, and prepare derivative works of this
 * software is granted provided the copyright and "No Warranty" statements
 * are included with all reproductions and derivative works and associated
 * documentation. This software may also be redistributed without charge
 * provided that the copyright and "No Warranty" statements are included
 * in all redistributions.
 *
 * NO WARRANTY. THIS SOFTWARE IS FURNISHED ON AN "AS IS" BASIS.
 * CARNEGIE MELLON UNIVERSITY MAKES NO WARRANTIES OF ANY KIND, EITHER
 * EXPRESSED OR IMPLIED AS TO THE MATTER INCLUDING, BUT NOT LIMITED
 * TO: WARRANTY OF FITNESS FOR PURPOSE OR MERCHANTABILITY, EXCLUSIVITY
 * OF RESULTS OR RESULTS OBTAINED FROM USE OF THIS SOFTWARE. CARNEGIE
 * MELLON UNIVERSITY DOES NOT MAKE ANY WARRANTY OF ANY KIND WITH RESPECT
 * TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.
 * COPYRIGHT HOLDERS WILL BEAR NO LIABILITY FOR ANY USE OF THIS SOFTWARE
 * OR DOCUMENTATION.
 *
 */



/*
 * DiskSim Storage Subsystem Simulation Environment (Version 2.0)
 * Revision Authors: Greg Ganger
 * Contributors: Ross Cohen, John Griffin, Steve Schlosser
 *
 * Copyright (c) of Carnegie Mellon University, 1999.
 *
 * Permission to reproduce, use, and prepare derivative works of
 * this software for internal use is granted provided the copyright
 * and "No Warranty" statements are included with all reproductions
 * and derivative works. This software may also be redistributed
 * without charge provided that the copyright and "No Warranty"
 * statements are included in all redistributions.
 *
 * NO WARRANTY. THIS SOFTWARE IS FURNISHED ON AN "AS IS" BASIS.
 * CARNEGIE MELLON UNIVERSITY MAKES NO WARRANTIES OF ANY KIND, EITHER
 * EXPRESSED OR IMPLIED AS TO THE MATTER INCLUDING, BUT NOT LIMITED
 * TO: WARRANTY OF FITNESS FOR PURPOSE OR MERCHANTABILITY, EXCLUSIVITY
 * OF RESULTS OR RESULTS OBTAINED FROM USE OF THIS SOFTWARE. CARNEGIE
 * MELLON UNIVERSITY DOES NOT MAKE ANY WARRANTY OF ANY KIND WITH RESPECT
 * TO FREEDOM FROM PATENT, TRADEMARK, OR COPYRIGHT INFRINGEMENT.
 */


#ifndef DISKSIM_SIMPLEFLASH_H
#define DISKSIM_SIMPLEFLASH_H

/* externalized disksim_simpleflash.c functions */

void    simpleflash_read_toprints (FILE *parfile);
void    simpleflash_read_specs (FILE *parfile, int devno, int copies);
void    simpleflash_set_syncset (int setstart, int setend);
void    simpleflash_param_override (char *paramname, char *paramval, int first, int last);
void    simpleflash_setcallbacks (void);
void    simpleflash_initialize (void);
void    simpleflash_resetstats (void);
void    simpleflash_printstats (void);
void    simpleflash_printsetstats (int *set, int setsize, char *sourcestr);
void    simpleflash_cleanstats (void);
int     simpleflash_set_depth (int devno, int inbusno, int depth, int slotno);
int     simpleflash_get_depth (int devno);
int     simpleflash_get_inbus (int devno);
int     simpleflash_get_busno (ioreq_event *curr);
int     simpleflash_get_slotno (int devno);
int     simpleflash_get_number_of_blocks (int devno);
int     simpleflash_get_maxoutstanding (int devno);
int     simpleflash_get_numdisks (void);
int     simpleflash_get_numcyls (int devno);
double  simpleflash_get_blktranstime (ioreq_event *curr);
int     simpleflash_get_avg_sectpercyl (int devno);
void    simpleflash_get_mapping (int maptype, int devno, int blkno, int *cylptr, int *surfaceptr, int *blkptr);
void    simpleflash_event_arrive (ioreq_event *curr);
int     simpleflash_get_distance (int devno, ioreq_event *req, int exact, int direction);
double  simpleflash_get_servtime (int devno, ioreq_event *req, int checkcache, double maxtime);
//kanishk begin
//double  simpleflash_get_acctime (int devno, ioreq_event *req, double maxtime);
double  simpleflash_get_acctime ();
//kanishk end
void    simpleflash_bus_delay_complete (int devno, ioreq_event *curr, int sentbusno);
void    simpleflash_bus_ownership_grant (int devno, ioreq_event *curr, int busno, double arbdelay);

struct simpleflash *getsimpleflash (int devno);

/* default simpleflash dev header */
extern struct device_header simpleflash_hdr_initializer;

#endif   /* DISKSIM_SIMPLEFLASH_H */

