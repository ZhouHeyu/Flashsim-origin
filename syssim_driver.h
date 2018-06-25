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
 * Definitions for simple system simulator that uses DiskSim as a slave.
 *
 * Contributed by Eran Gabber of Lucent Technologies - Bell Laboratories
 *
 */

typedef	double SysTime;		/* system time in seconds.usec */

typedef	struct	{		/* System level request */
        char type;		/* 'R' denotes read; 'W' denotes write */
        short devno;
        unsigned long blkno;
        int bytecount;
        SysTime start;
        /* added may be removed later*/
        int sectorcount;
        double last_seek;
        double last_latency;
        int last_cylno;
        double last_angle;
        int background;
        int async;
        /* end added */
} Request;

/* routines for translating between the system-level simulation's simulated */
/* time format (whatever it is) and disksim's simulated time format (a      */
/* double, representing the number of milliseconds from the simulation's    */
/* initialization).                                                         */

/* In this example, system time is in seconds since initialization */
#define SYSSIMTIME_TO_MS(syssimtime)    (syssimtime*1e3)
#define MS_TO_SYSSIMTIME(curtime)       (curtime/1e3)

/* routine for determining a read request */
#define	isread(r)	((r->type == 'R') || (r->type == 'r'))

/* exported by disksim_interface.c */
struct disksim;
struct disksim * disksim_interface_initialize (void *addr, int len, const char *pfile, const char *ofile);
struct disksim * disksim_interface_initialize_latency (void *addr, int len, const char *pfile, const char *ofile, int latency_weight, char *paramval, char *paramname, int sythio, char *sched_alg);
void disksim_interface_shutdown (struct disksim *disksim_in, SysTime syssimtime);
void disksim_interface_dump_stats (struct disksim *disksim_in, SysTime syssimtime);
void disksim_interface_internal_event (void *disksim_in, SysTime t);
void disksim_interface_request_arrive (struct disksim *disksim_in, SysTime syssimtime, Request *requestdesc);

/* exported by syssim_driver.c */
void syssim_report_completion(SysTime syssimtime, Request *r);
void syssim_schedule_callback(void (*f)(void *,double), SysTime t);
void syssim_report_completion(SysTime t, Request *r);
void syssim_deschedule_callback(void (*f)());

