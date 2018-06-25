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

/*
 * DiskSim Storage Subsystem Simulation Environment
 * Authors: Greg Ganger, Bruce Worthington, Yale Patt
 *
 * Copyright (C) 1993, 1995, 1997 The Regents of the University of Michigan 
 *
 * This software is being provided by the copyright holders under the
 * following license. By obtaining, using and/or copying this software,
 * you agree that you have read, understood, and will comply with the
 * following terms and conditions:
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose and without fee or royalty is
 * hereby granted, provided that the full text of this NOTICE appears on
 * ALL copies of the software and documentation or portions thereof,
 * including modifications, that you make.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS," AND COPYRIGHT HOLDERS MAKE NO
 * REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED. BY WAY OF EXAMPLE,
 * BUT NOT LIMITATION, COPYRIGHT HOLDERS MAKE NO REPRESENTATIONS OR
 * WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR
 * THAT THE USE OF THE SOFTWARE OR DOCUMENTATION WILL NOT INFRINGE ANY
 * THIRD PARTY PATENTS, COPYRIGHTS, TRADEMARKS OR OTHER RIGHTS. COPYRIGHT
 * HOLDERS WILL BEAR NO LIABILITY FOR ANY USE OF THIS SOFTWARE OR
 * DOCUMENTATION.
 *
 *  This software is provided AS IS, WITHOUT REPRESENTATION FROM THE
 * UNIVERSITY OF MICHIGAN AS TO ITS FITNESS FOR ANY PURPOSE, AND
 * WITHOUT WARRANTY BY THE UNIVERSITY OF MICHIGAN OF ANY KIND, EITHER
 * EXPRESSED OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE REGENTS
 * OF THE UNIVERSITY OF MICHIGAN SHALL NOT BE LIABLE FOR ANY DAMAGES,
 * INCLUDING SPECIAL , INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * WITH RESPECT TO ANY CLAIM ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OF OR IN CONNECTION WITH THE USE OF THE SOFTWARE, EVEN IF IT HAS
 * BEEN OR IS HEREAFTER ADVISED OF THE POSSIBILITY OF SUCH DAMAGES
 *
 * The names and trademarks of copyright holders or authors may NOT be
 * used in advertising or publicity pertaining to the software without
 * specific, written prior permission. Title to copyright in this software
 * and any associated documentation will at all times remain with copyright
 * holders.
 */

/*****************************************************************************/
/*

This file offers a suggested interface between disksim and a full system
simulator.  Using this interface (which assumes only a few characteristics
of the system simulator), disksim will act as a slave of the system simulator,
providing disk request completion indications in time for an interrupt to be
generated in the system simulation.  Specifically, disksim code will only be
executed when invoked by one of the procedures in this file.

Using ths interface requires only two significant functionalities of the
system simulation environment:

1. the ability to function correctly without knowing when a disk request
will complete at the time that it is initiated.  Via the code in this file,
the system simulation will be informed at some later point in its view of
time that the request is completed.  (At this time, the appropriate "disk
request completion" interrupt could be inserted into the system simulation.)

2. the ability for disksim to register a callback with the system simulation
environment.  That is, this interface code must be able to say, "please
invoke this function when the simulated time reaches X".  It is also helpful
to be able to "deschedule" a callback at a later time -- but lack of this
support can certainly be worked around.

(NOTE: using this interface requires compiling disksim.c with -DEXTERNAL.)

*/
/*****************************************************************************/

#include "sim.h"
#include "simtypes.h"
#include "eventcallback.h"
#include "hd.h"
#include "cpu_interface.h"
#include "simutil.h"
#include "sim_error.h"
#include "machine_params.h"

#include "disksim_global.h"
#include "disksim_ioface.h"
#include "disksim_simos_interface.h"

#ifdef DISKSIM_DEBUG
     //#define DEBUG_TRACE(_x_)  { CPUPrint("disksim:  "); CPUPrint _x_; }
#define DEBUG_TRACE(_x_)  { fprintf(disk_access_dump_file, _x_); }
#else
#define DEBUG_TRACE(_x_)  { }
#endif

typedef struct DiskSimInternalCallback {
  EventCallbackHdr hdr;
  int num;
} DiskSimInternalCallback;

DiskSimInternalCallback *InternalCallback;

double diskTICSperMSEC;

extern void DiskDoneRoutine(int);
extern void DMATransfer(int, SimTime, void (*done)(int), int);
unsigned long MSECtoTICS(double MSEC) {
  return (unsigned long)(MSEC / MSECperTICS);
}

double TICStoMSEC(unsigned long TICS) {
  return (double)(TICS / TICSperMSEC);
}

FILE *disk_access_dump_file;

/* This is the disksim callback for reporting completion of a disk request */
/* to the system-level simulation -- the system-level simulation should    */
/* incorporate this completion as appropriate (probably by inserting a     */
/* simulated "disk completion interrupt" at the specified simulated time). */
/* Based on the requestdesc pointed to by "curr->buf" (below), the         */
/* system-level simulation should be able to determine which request       */
/* completed.  (A ptr to the system-level simulator's request structure    */
/* is a reasonable use of "curr->buf".)                                    */

static void disksim_interface_io_done_notify (ioreq_event *curr)
{
  Request *r = curr->buf;  /*  this is a pointer to the original request  */
  int arg = r->doneArg;
  
  DEBUG_TRACE(("disksim_interface_io_done_notify\n"));

  free(r);

  DEBUG_TRACE(("disksim_interface_io_done_notify::  calling DMATransfer.  time = %d\n",
	       CPUVec.CycleCount(0)));

  DMATransfer(r->bytecount, CPUVec.CycleCount(0),
	      DiskDoneRoutine, arg);

  //  DiskDoneRoutine(arg);

  DEBUG_TRACE(("disksim_interface_io_done_notify done\n"));
}


/* called once at simulation initialization time */

struct disksim * disksim_interface_initialize (void *addr, int len, const char *pfile, const char *ofile)
{
   const char *argv[6];

   disksim = disksim_initialize_disksim_structure (addr, len);

   DEBUG_TRACE(("disksim_interface_initialize\n"));

   diskTICSperMSEC = (1000.0 * CPU_CLOCK);

   DEBUG_TRACE(("disksim_interface_initialize::  diskTICSperMSEC = %f\n", diskTICSperMSEC));

   InternalCallback =
     (DiskSimInternalCallback *)ZMALLOC(sizeof(DiskSimInternalCallback), "InternalCallback");

   argv[0] = "disksim";
   argv[1] = pfile;
   argv[2] = ofile;
   argv[3] = "external";
   argv[4] = "0";
   argv[5] = "0";
   disksim_setup_disksim (6, (char **)argv);

   /* Note that this call must be redone anytime a disksim checkpoint is */
   /* restored -- this prevents old function addresses from polluting    */
   /* the restored execution...                                          */
   disksim_set_external_io_done_notify (disksim_interface_io_done_notify);

#ifdef DISKSIM_DEBUG
   disk_access_dump_file = fopen("disk_access_dump_file", "RW");
#endif

   DEBUG_TRACE(("disksim_interface_initialize done\n"));

   return (disksim);
}


/* called once at simulation shutdown time */

void disksim_interface_shutdown (struct disksim *disksim_in, TICS systime)
{
  double curtime = TICStoMSEC(CPUVec.CycleCount(0));
  
  disksim = disksim_in;
  
  DEBUG_TRACE(("disksim_interface_shutdown::  curtime = %f msec, %d tics\n",
	       curtime, CPUVec.CycleCount(0)));
  
  if ((curtime + DISKSIM_TIME_THRESHOLD) < simtime) {
    fprintf (stderr, "external time is ahead of disksim time: %f > %f\n", curtime, simtime);
    exit(1);
  }

  free(InternalCallback);
  
  simtime = curtime;
  disksim_cleanup_and_printstats ();

#ifdef DISKSIM_DEBUG
  fclose(disk_access_dump_file);
#endif

  DEBUG_TRACE(("disksim_interface_shutdown done\n"));
}


/* Prints the current disksim statistics.  This call does not reset the    */
/* statistics, so the simulation can continue to run and calculate running */
/* totals.  "syssimtime" should be the current simulated time of the       */
/* system-level simulation.                                                */

void disksim_interface_dump_stats (struct disksim *disksim_in, TICS systime)
{
   double curtime = TICStoMSEC(CPUVec.CycleCount(0));

   disksim = disksim_in;

   DEBUG_TRACE(("disksim_interface_dump_stats:: curtime = %f msec, %d tics\n",
		curtime, CPUVec.CycleCount(0)));

   if ((disksim->intq) && (disksim->intq->time < curtime) && ((disksim->intq->time + DISKSIM_TIME_THRESHOLD) >= curtime)) {
      curtime = disksim->intq->time;
   }
   if (((curtime + DISKSIM_TIME_THRESHOLD) < simtime) || ((disksim->intq) && (disksim->intq->time < curtime))) {
      fprintf (stderr, "external time is mismatched with disksim time: %f vs. %f (%f)\n", curtime, simtime, ((disksim->intq) ? disksim->intq->time : 0.0));
      exit (0);
   }

   simtime = curtime;
   disksim_cleanstats();
   disksim_printstats();

   DEBUG_TRACE(("disksim_interface_dump_stats done\n"));
}


/* This is the callback for handling internal disksim events while running */
/* as a slave of a system-level simulation.  "syssimtime" should be the    */
/* current simulated time of the system-level simulation.                  */

void disksim_interface_internal_event (int cpuNum, EventCallbackHdr *hdr, void *disksim_in)
{
  TICS systime = CPUVec.CycleCount(0);
  double curtime = TICStoMSEC(systime);
  DiskSimInternalCallback *cback = (DiskSimInternalCallback *)hdr;

  disksim = disksim_in;
  
  DEBUG_TRACE(("disksim_interface_internal_event:: curtime = %f msec, %d tics\n", curtime, systime));
  DEBUG_TRACE(("disksim_interface_internal_event::  hdr->num = %d\n", cback->num));
    
  
  /* if next event time is less than now, error.  Also, if no event is  */
  /* ready to be handled, then this is a spurious callback -- it should */
  /* not be possible with the descheduling below (allow it if it is not */
  /* possible to deschedule.                                            */
  
  if ((disksim->intq == NULL) || ((disksim->intq->time + DISKSIM_TIME_THRESHOLD) < curtime)) {
    fprintf (stderr, "external time is ahead of disksim time: %f > %f\n", curtime, ((disksim->intq) ? disksim->intq->time : 0.0));
    exit (0);
  }
  
  DEBUG_TRACE(("disksim_interface_internal_event:: disksim->intq->time=%f curtime=%f, diff = %f\n",
	       disksim->intq->time, curtime, curtime - disksim->intq->time));
  
  DEBUG_TRACE(("disksim_interface_internal_event:: disksim->intq->time=%d curtime=%d, diff = %d\n",
	       MSECtoTICS(disksim->intq->time), systime, curtime - MSECtoTICS(disksim->intq->time)));
    
  /* while next event time is same as now, handle next event */
  ASSERT (disksim->intq->time >= simtime);
  while ((disksim->intq != NULL) && (disksim->intq->time <= (curtime + DISKSIM_TIME_THRESHOLD))) { 
    DEBUG_TRACE(("disksim_interface_internal_event::  handling internal event: type %d\n", disksim->intq->type));    
    disksim_simulate_event();
  }
  
  if (disksim->intq != NULL) {
    /* Note: this could be a dangerous operation when employing checkpoint */
    /* and, specifically, restore -- functions move around when programs   */
    /* are changed and recompiled...                                       */
    (cback->num)++;
    EventDoCallback(0, disksim_interface_internal_event,
		    (EventCallbackHdr *)cback, (void *)disksim, MSECtoTICS(disksim->intq->time) - systime);
    DEBUG_TRACE(("disksim_interface_internal_event::  scheduled new internal event %d at %d\n",
		 cback->num, MSECtoTICS(disksim->intq->time)));
  }
  
  DEBUG_TRACE(("disksim_interface_internal_event:: done\n"));
}


/* This function should be called by the system-level simulation when    */
/* it wants to issue a request into disksim.  "syssimtime" should be the */
/* system-level simulation time at which the request should "arrive".    */
/* "requestdesc" is the system-level simulator's description of the      */
/* request (device number, block number, length, etc.).                  */

void disksim_interface_request_arrive (struct disksim *disksim_in, Request *r)
{
  TICS systime = CPUVec.CycleCount(0);
  double curtime = TICStoMSEC(systime);

  ioreq_event *new = (ioreq_event *) getfromextraq();

  DEBUG_TRACE(("disksim_interface_request_arrive:: curtime = %f msec, %d tics\n", curtime, systime));

  disksim = disksim_in;
  
  //  new = (ioreq_event *) getfromextraq();

  assert (new != NULL);
  
  new->type = IO_REQUEST_ARRIVE;
  new->time = curtime;
  new->busno = 0;
  new->devno = r->device_num;
  new->blkno = r->block_num;
  new->bcount = (r->bytecount) / 512;
  new->flags = (r->read_write == 'R') ? READ : WRITE;
  new->cause = 0;
  new->opid = 0;
  new->buf = r;

  io_map_trace_request (new);

  /* issue it into simulator */
  if (disksim->intq) {
    DEBUG_TRACE(("disksim_interface_request_arrive::  removing InternalCallback\n"));
    EventCallbackRemove((EventCallbackHdr *)InternalCallback);
  }
  addtointq ((event *)new);

  /* while next event time is same as now, handle next event */
  while ((disksim->intq != NULL) && (disksim->intq->time <= (curtime + DISKSIM_TIME_THRESHOLD))) {
    disksim_simulate_event ();
  }
  
  if (disksim->intq) {
    /* Note: this could be a dangerous operation when employing checkpoint */
    /* and, specifically, restore -- functions move around when programs   */
    /* are changed and recompiled...                                       */
    (InternalCallback->num)++;
    DEBUG_TRACE(("disksim_interface_request_arrive::  scheduling new internal event %d at %d\n",
		 InternalCallback->num, MSECtoTICS(disksim->intq->time)));
    EventDoCallback(0, disksim_interface_internal_event,
		    (EventCallbackHdr *)InternalCallback, (void *)disksim,
		    (MSECtoTICS(disksim->intq->time) - systime));   
    
  }
  
  DEBUG_TRACE(("disksim_interface_request_arrive done\n"));
}

