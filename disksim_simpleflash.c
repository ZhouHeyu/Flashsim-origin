/* 
 * Adapted from disksim_simpledisk.c
 * Adaptation Author: Youngjae Kim (youkim@cse.psu.edu)
 *   
 * Description: This source provides a stub to plug a flash simulator. 
 * 
 */

/*
 * DiskSim Storage Subsystem Simulation Environment (Version 3.0)
 * Revision Authors: John Bucy, Greg Ganger
 * Contributors: John Griffin, Jiri Schindler, Steve Schlosser_
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

#include "disksim_global.h"
#include "disksim_iosim.h"
#include "disksim_stat.h"
#include "disksim_simpleflash.h"
#include "disksim_ioqueue.h"
#include "disksim_bus.h"
#include "config.h"
//flashsim
#include "ssd_interface.h"

#include "modules/disksim_simpleflash_param.h"

//flashsim
int flash_numblocks;
int flash_extrblocks;
int ftl_type = -1;
int req_num = 1;

/* read-only globals used during readparams phase */
static char *statdesc_acctimestats	=	"Access time";

//flashsim: define static global variable to make only one call to device
static double accessTime = 0.0;
//flashsim: end

typedef struct {
   statgen acctimestats;
   double  requestedbus;
   double  waitingforbus;
   int     numbuswaits;
} simpleflash_stat_t;


typedef struct simpleflash {
  struct device_header hdr;

//flashsim
    int type; 
    int extrblocks;

   double acctime;
   double overhead;
   double bus_transaction_latency;
   int numblocks;
   int devno;
   int inited;
   struct ioq *queue;
   int media_busy;
   int reconnect_reason;

   double blktranstime;
   int maxqlen;
   int busowned;
   ioreq_event *buswait;
   int neverdisconnect;
   int numinbuses;
   int inbuses[MAXINBUSES];
   int depth[MAXINBUSES];
   int slotno[MAXINBUSES];

   int printstats;
   simpleflash_stat_t stat;
} simpleflash_t;


typedef struct simpleflash_info {
   struct simpleflash **simpleflashs;
   int numsimpleflashs;
  int simpleflashs_len; /* allocated size of simpleflashs */
} simpleflashinfo_t;


/* private remapping #defines for variables from device_info_t */
#define numsimpleflashs         (disksim->simpleflashinfo->numsimpleflashs)
//#define simpleflashs            (disksim->simpleflashinfo->simpleflashs)



struct simpleflash *getsimpleflash (int devno)
{
   ASSERT1((devno >= 0) && (devno < MAXDEVICES), "devno", devno);
   return (disksim->simpleflashinfo->simpleflashs[0]);//[devno]); //hack by Kanishk : this restricts the number of flash devices in the system to 1 but allows a flash device to be coupled with a different kind of device, thus allowing a hetrogeneous topology 
}


int simpleflash_set_depth (int devno, int inbusno, int depth, int slotno)
{
   simpleflash_t *currflash;
   int cnt;

   currflash = getsimpleflash (devno);
   assert(currflash);
   cnt = currflash->numinbuses;
   currflash->numinbuses++;
   if ((cnt + 1) > MAXINBUSES) {
      fprintf(stderr, "Too many inbuses specified for simpleflash %d - %d\n", devno, (cnt+1));
      exit(1);
   }
   currflash->inbuses[cnt] = inbusno;
   currflash->depth[cnt] = depth;
   currflash->slotno[cnt] = slotno;
   return(0);
}


int simpleflash_get_depth (int devno)
{
   simpleflash_t *currflash;
   currflash = getsimpleflash (devno);
   return(currflash->depth[0]);
}


int simpleflash_get_slotno (int devno)
{
   simpleflash_t *currflash;
   currflash = getsimpleflash (devno);
   return(currflash->slotno[0]);
}


int simpleflash_get_inbus (int devno)
{
   simpleflash_t *currflash;
   currflash = getsimpleflash (devno);
   return(currflash->inbuses[0]);
}


int simpleflash_get_maxoutstanding (int devno)
{
   simpleflash_t *currflash;
   currflash = getsimpleflash (devno);
   return(currflash->maxqlen);
}


double simpleflash_get_blktranstime (ioreq_event *curr)
{
   simpleflash_t *currflash;
   double tmptime;

   currflash = getsimpleflash (curr->devno);
   tmptime = bus_get_transfer_time(simpleflash_get_busno(curr), 1, (curr->flags & READ));
   if (tmptime < currflash->blktranstime) {
      tmptime = currflash->blktranstime;
   }
   return(tmptime);
}


int simpleflash_get_busno (ioreq_event *curr)
{
   simpleflash_t *currflash;
   intchar busno;
   int depth;

   currflash = getsimpleflash (curr->devno);
   busno.value = curr->busno;
   depth = currflash->depth[0];
   return(busno.byte[depth]);
}


/*
 * simpleflash_send_event_up_path()
 *
 * Acquires the bus (if not already acquired), then uses bus_delay to
 * send the event up the path.
 *
 * If the bus is already owned by this device or can be acquired
 * immediately (interleaved bus), the event is sent immediately.
 * Otherwise, disk_bus_ownership_grant will later send the event.
 */
  
static void simpleflash_send_event_up_path (ioreq_event *curr, double delay)
{
   simpleflash_t *currflash;
   int busno;
   int slotno;

   // fprintf (outputfile, "simpleflash_send_event_up_path - devno %d, type %d, cause %d, blkno %d\n", curr->devno, curr->type, curr->cause, curr->blkno);

   currflash = getsimpleflash (curr->devno);

   busno = simpleflash_get_busno(curr);
   slotno = currflash->slotno[0];

   /* Put new request at head of buswait queue */
   curr->next = currflash->buswait;
   currflash->buswait = curr;

   curr->tempint1 = busno;
   curr->time = delay;
   if (currflash->busowned == -1) {

      // fprintf (outputfile, "Must get ownership of the bus first\n");

      if (curr->next) {
         //fprintf(stderr,"Multiple bus requestors detected in flash_send_event_up_path\n");
         /* This should be ok -- counting on the bus module to sequence 'em */
      }
      if (bus_ownership_get(busno, slotno, curr) == FALSE) {
         /* Remember when we started waiting (only place this is written) */
	 currflash->stat.requestedbus = simtime;
      } else {
         bus_delay(busno, DEVICE, curr->devno, delay, curr); /* Never for SCSI */
      }
   } else if (currflash->busowned == busno) {

      //fprintf (outputfile, "Already own bus - so send it on up\n");

      bus_delay(busno, DEVICE, curr->devno, delay, curr);
   } else {
      fprintf(stderr, "Wrong bus owned for transfer desired\n");
      exit(1);
   }
}


/*
  **-simpleflash_bus_ownership_grant

  Calls bus_delay to handle the event that the disk has been granted the bus.  I believe
  this is always initiated by a call to disk_send_even_up_path.

  */

void simpleflash_bus_ownership_grant (int devno, ioreq_event *curr, int busno, double arbdelay)
{
   simpleflash_t *currflash;
   ioreq_event *tmp;

   currflash = getsimpleflash (devno);

   tmp = currflash->buswait;
   while ((tmp != NULL) && (tmp != curr)) {
      tmp = tmp->next;
   }
   if (tmp == NULL) {
      fprintf(stderr, "Bus ownership granted to unknown simpleflash request - devno %d, busno %d\n", devno, busno);
      exit(1);
   }
   currflash->busowned = busno;
   currflash->stat.waitingforbus += arbdelay;
   //ASSERT (arbdelay == (simtime - currflash->stat.requestedbus));
   currflash->stat.numbuswaits++;
   bus_delay(busno, DEVICE, devno, tmp->time, tmp);
}


void simpleflash_bus_delay_complete (int devno, ioreq_event *curr, int sentbusno)
{
   simpleflash_t *currflash;
   intchar slotno;
   intchar busno;
   int depth;

   currflash = getsimpleflash (devno);

   // fprintf (outputfile, "Entered flash_bus_delay_complete\n");

   if (curr == currflash->buswait) {
      currflash->buswait = curr->next;
   } else {
      ioreq_event *tmp = currflash->buswait;
      while ((tmp->next != NULL) && (tmp->next != curr)) {
         tmp = tmp->next;
      }
      if (tmp->next != curr) {
         fprintf(stderr, "Bus delay complete for unknown simpleflash request - devno %d, busno %d\n", devno, busno.value);
         exit(1);
      }
      tmp->next = curr->next;
   }
   busno.value = curr->busno;
   slotno.value = curr->slotno;
   depth = currflash->depth[0];
   slotno.byte[depth] = slotno.byte[depth] >> 4;
   curr->time = 0.0;
   if (depth == 0) {
      intr_request ((event *)curr);
   } else {
      bus_deliver_event(busno.byte[depth], slotno.byte[depth], curr);
   }
}


/* send completion up the line */

static void simpleflash_request_complete(ioreq_event *curr)
{
   simpleflash_t *currflash;

   // fprintf (outputfile, "Entering simpleflash_request_complete: %12.6f\n", simtime);

   currflash = getsimpleflash (curr->devno);

   if ((curr = ioqueue_physical_access_done(currflash->queue,curr)) == NULL) {
      fprintf(stderr, "simpleflash_request_complete:  ioreq_event not found by ioqueue_physical_access_done call\n");
      exit(1);
   }

   /* send completion interrupt */
   curr->type = IO_INTERRUPT_ARRIVE;
   curr->cause = COMPLETION;

   simpleflash_send_event_up_path(curr, currflash->bus_transaction_latency);
}


static void simpleflash_bustransfer_complete (ioreq_event *curr)
{
   simpleflash_t *currflash;

   // fprintf (outputfile, "Entering simpleflash_bustransfer_complete for flash %d: %12.6f\n", curr->devno, simtime);

   currflash = getsimpleflash (curr->devno);

   if (curr->flags & READ) {
      simpleflash_request_complete (curr);

   } else {
      simpleflash_t *currflash = getsimpleflash (curr->devno);

      if (currflash->neverdisconnect == FALSE) {
         /* disconnect from bus */
         ioreq_event *tmp = ioreq_copy (curr);
         tmp->type = IO_INTERRUPT_ARRIVE;
         tmp->cause = DISCONNECT;
         simpleflash_send_event_up_path (tmp, currflash->bus_transaction_latency);
      }

      /* do media access */
      currflash->media_busy = TRUE;
      stat_update (&currflash->stat.acctimestats, accessTime);
      //printf ("Kanishk access time call 0.5\n");
      curr->time = simtime + accessTime;
      //printf ("Kanishk access time call 1\n");
      curr->type = DEVICE_ACCESS_COMPLETE;
      addtointq ((event *) curr);
   }
}


static void simpleflash_reconnect_done (ioreq_event *curr)
{
   simpleflash_t *currflash;

   // fprintf (outputfile, "Entering simpleflash_reconnect_done for flash %d: %12.6f\n", curr->devno, simtime);

   currflash = getsimpleflash (curr->devno);

   if (curr->flags & READ) {
      if (currflash->neverdisconnect) {
         /* Just holding on to bus; data transfer will be initiated when */
         /* media access is complete.                                    */
         addtoextraq((event *) curr);

      } else {
         /* data transfer: curr->bcount, which is still set to original */
         /* requested value, indicates how many blks to transfer.       */
         curr->type = DEVICE_DATA_TRANSFER_COMPLETE;
         simpleflash_send_event_up_path(curr, (double) 0.0);
      }

   } else {
      if (currflash->reconnect_reason == DEVICE_ACCESS_COMPLETE) {
         simpleflash_request_complete (curr);

      } else {
         /* data transfer: curr->bcount, which is still set to original */
         /* requested value, indicates how many blks to transfer.       */
         curr->type = DEVICE_DATA_TRANSFER_COMPLETE;
         simpleflash_send_event_up_path(curr, (double) 0.0);
      }
   }
}


static void simpleflash_request_arrive (ioreq_event *curr)
{
   ioreq_event *intrp;
   simpleflash_t *currflash;

   //printf("Kanishk: New request arrived\n");

   // fprintf (outputfile, "Entering simpleflash_request_arrive: %12.6f\n", simtime);
   // fprintf (outputfile, "simpleflash = %d, blkno = %d, bcount = %d, read = %d\n",curr->devno, curr->blkno, curr->bcount, (READ & curr->flags));

   currflash = getsimpleflash(curr->devno);

   /* verify that request is valid. */
   if ((curr->blkno < 0) || (curr->bcount <= 0) ||
       ((curr->blkno + curr->bcount) > currflash->numblocks)) {
      fprintf(stderr, "Invalid set of blocks requested from simpleflash - blkno %d, bcount %d, numblocks %d\n", curr->blkno, curr->bcount, currflash->numblocks);
      exit(1);
   }

   /* create a new request, set it up for initial interrupt */
   currflash->busowned = simpleflash_get_busno(curr);

   if (ioqueue_get_reqoutstanding (currflash->queue) == 0) {
      ioqueue_add_new_request(currflash->queue, curr);
      curr = ioqueue_get_next_request (currflash->queue);
      intrp = curr;


  //flashsim
  accessTime = simpleflash_get_acctime(curr->devno, curr->blkno, curr->bcount, (curr->flags & READ));
     
  /* initiate media access if request is a READ */
      if (curr->flags & READ) {
         ioreq_event *tmp = ioreq_copy (curr);
         currflash->media_busy = TRUE;
         stat_update (&currflash->stat.acctimestats, accessTime);
      //printf ("Kanishk access time call 1.5\n");
         tmp->time = simtime + accessTime;
         //printf ("Kanishk access time call 2\n");
         tmp->type = DEVICE_ACCESS_COMPLETE;
         addtointq ((event *)tmp);
      }

      /* if not disconnecting, then the READY_TO_TRANSFER is like a RECONNECT */
      currflash->reconnect_reason = IO_INTERRUPT_ARRIVE;
      if (curr->flags & READ) {
         intrp->cause = (currflash->neverdisconnect) ? READY_TO_TRANSFER : DISCONNECT;
      } else {
         intrp->cause = READY_TO_TRANSFER;
      }

   } else {
      intrp = ioreq_copy(curr);
      ioqueue_add_new_request(currflash->queue, curr);
      intrp->cause = DISCONNECT;
   }

   intrp->type = IO_INTERRUPT_ARRIVE;
   simpleflash_send_event_up_path(intrp, currflash->bus_transaction_latency);
}


static void simpleflash_access_complete (ioreq_event *curr)
{
   simpleflash_t *currflash;

   // fprintf (outputfile, "Entering simpleflash_access_complete: %12.6f\n", simtime);

   currflash = getsimpleflash (curr->devno);
   currflash->media_busy = FALSE;

   if (currflash->neverdisconnect) {
      /* already connected */
      if (curr->flags & READ) {
         /* transfer data up the line: curr->bcount, which is still set to */
         /* original requested value, indicates how many blks to transfer. */
         curr->type = DEVICE_DATA_TRANSFER_COMPLETE;
         simpleflash_send_event_up_path(curr, (double) 0.0);

      } else {
         simpleflash_request_complete (curr);
      }

   } else {
      /* reconnect to controller */
      curr->type = IO_INTERRUPT_ARRIVE;
      curr->cause = RECONNECT;
      simpleflash_send_event_up_path (curr, currflash->bus_transaction_latency);
      currflash->reconnect_reason = DEVICE_ACCESS_COMPLETE;
   }
}


/* intermediate disconnect done */

static void simpleflash_disconnect_done (ioreq_event *curr)
{
   simpleflash_t *currflash;

   currflash = getsimpleflash (curr->devno);

   // fprintf (outputfile, "Entering simpleflash_disconnect for flash %d: %12.6f\n", currflash->devno, simtime);

   addtoextraq((event *) curr);

   if (currflash->busowned != -1) {
      bus_ownership_release(currflash->busowned);
      currflash->busowned = -1;
   }
}


/* completion disconnect done */

static void simpleflash_completion_done (ioreq_event *curr)
{
   simpleflash_t *currflash = getsimpleflash (curr->devno);

   // fprintf (outputfile, "Entering simpleflash_completion for flash %d: %12.6f\n", currflash->devno, simtime);

   addtoextraq((event *) curr);

   if (currflash->busowned != -1) {
      bus_ownership_release(currflash->busowned);
      currflash->busowned = -1;
   }

   /* check for and start next queued request, if any */
   curr = ioqueue_get_next_request(currflash->queue);
   if (curr != NULL) {
      ASSERT (currflash->media_busy == FALSE);
      if (curr->flags & READ) {
         currflash->media_busy = TRUE;
         stat_update (&currflash->stat.acctimestats, accessTime);
      //printf ("Kanishk access time call 2.5\n");
         curr->time = simtime + accessTime;
      //printf ("Kanishk access time call 3\n");
         curr->type = DEVICE_ACCESS_COMPLETE;
         addtointq ((event *)curr);

      } else {
         curr->type = IO_INTERRUPT_ARRIVE;
         curr->cause = RECONNECT;
         simpleflash_send_event_up_path (curr, currflash->bus_transaction_latency);
         currflash->reconnect_reason = IO_INTERRUPT_ARRIVE;
      }
   }
}


static void simpleflash_interrupt_complete (ioreq_event *curr)
{
   // fprintf (outputfile, "Entered simpleflash_interrupt_complete - cause %d\n", curr->cause);

   switch (curr->cause) {

      case RECONNECT:
         simpleflash_reconnect_done(curr);
	 break;

      case DISCONNECT:
	 simpleflash_disconnect_done(curr);
	 break;

      case COMPLETION:
	 simpleflash_completion_done(curr);
	 break;

      default:
         ddbg_assert2(0, "bad event type");
   }
}


void simpleflash_event_arrive (ioreq_event *curr)
{
   simpleflash_t *currflash;

   // fprintf (outputfile, "Entered simpleflash_event_arrive: time %f (simtime %f)\n", curr->time, simtime);
   // fprintf (outputfile, " - devno %d, blkno %d, type %d, cause %d, read = %d\n", curr->devno, curr->blkno, curr->type, curr->cause, curr->flags & READ);

   currflash = getsimpleflash (curr->devno);

   switch (curr->type) {

      case IO_ACCESS_ARRIVE:
         curr->time = simtime + currflash->overhead;
         curr->type = DEVICE_OVERHEAD_COMPLETE;
         addtointq((event *) curr);
         break;

      case DEVICE_OVERHEAD_COMPLETE:
         simpleflash_request_arrive(curr);
         break;

      case DEVICE_ACCESS_COMPLETE:
         simpleflash_access_complete (curr);
         break;

      case DEVICE_DATA_TRANSFER_COMPLETE:
         simpleflash_bustransfer_complete(curr);
         break;

      case IO_INTERRUPT_COMPLETE:
         simpleflash_interrupt_complete(curr);
         break;

      case IO_QLEN_MAXCHECK:
         /* Used only at initialization time to set up queue stuff */
         curr->tempint1 = -1;
         curr->tempint2 = simpleflash_get_maxoutstanding(curr->devno);
         curr->bcount = 0;
         break;

      default:
         fprintf(stderr, "Unrecognized event type at simpleflash_event_arrive\n");
         exit(1);
   }

   // fprintf (outputfile, "Exiting simpleflash_event_arrive\n");
}

int simpleflash_get_number_of_blocks (int devno)
{
   simpleflash_t *currflash = getsimpleflash (devno);
   return (currflash->numblocks);
}


int simpleflash_get_numcyls (int devno)
{
   simpleflash_t *currflash = getsimpleflash (devno);

   return (currflash->numblocks);
}


void simpleflash_get_mapping (int maptype, int devno, int blkno, int *cylptr, int *surfaceptr, int *blkptr)
{
   simpleflash_t *currflash = getsimpleflash (devno);

   if ((blkno < 0) || (blkno >= currflash->numblocks)) {
      fprintf(stderr, "Invalid blkno at simpleflash_get_mapping: %d\n", blkno);
      exit(1);
   }

   if (cylptr) {
      *cylptr = blkno;
   }
   if (surfaceptr) {
      *surfaceptr = 0;
   }
   if (blkptr) {
      *blkptr = 0;
   }
}


int simpleflash_get_avg_sectpercyl (int devno)
{
   return (1);
}


int simpleflash_get_distance (int devno, ioreq_event *req, int exact, int direction)
{
   /* just return an arbitrary constant, since acctime is constant */
   return 1;
}


double  simpleflash_get_servtime (int devno, ioreq_event *req, int checkcache, double maxtime)
{
   simpleflash_t *currflash = getsimpleflash (devno);
   return (accessTime);//simpleflash_get_acctime());
}

double  simpleflash_get_acctime (int devno, unsigned int blkno, int bcount, int flags)
{
  double delay = 0;
  double temp; 
  
  delay = callFsim(blkno, bcount, flags); 
  DEVICE_SERVICE_TIME = delay;

  return delay;
}

int simpleflash_get_numdisks (void)
{
   return(numsimpleflashs);
}

void simpleflash_cleanstats (void)
{
   int i;

   for (i=0; i<MAXDEVICES; i++) {
      simpleflash_t *currflash = getsimpleflash (i);
      if (currflash) {
         ioqueue_cleanstats(currflash->queue);
      }
   }
}




static void simpleflash_statinit (int devno, int firsttime)
{
   simpleflash_t *currflash;

   currflash = getsimpleflash (devno);
   if (firsttime) {
      stat_initialize(statdeffile, statdesc_acctimestats, &currflash->stat.acctimestats);
   } else {
      stat_reset(&currflash->stat.acctimestats);
   }

   currflash->stat.requestedbus = 0.0;
   currflash->stat.waitingforbus = 0.0;
   currflash->stat.numbuswaits = 0;
}


static void simpleflash_postpass (void)
{
}


void simpleflash_setcallbacks ()
{
   ioqueue_setcallbacks();
}


static void simpleflash_initialize_diskinfo ()
{
   disksim->simpleflashinfo = malloc (sizeof(simpleflashinfo_t));
   bzero ((char *)disksim->simpleflashinfo, sizeof(simpleflashinfo_t));
   disksim->simpleflashinfo->simpleflashs = malloc(MAXDEVICES * (sizeof(simpleflash_t)));
   disksim->simpleflashinfo->simpleflashs_len = MAXDEVICES;
   bzero ((char *)disksim->simpleflashinfo->simpleflashs, (MAXDEVICES * (sizeof(simpleflash_t))));
}


void simpleflash_initialize (void)
{
   int i;

   if (disksim->simpleflashinfo == NULL) {
      simpleflash_initialize_diskinfo ();
   }
/*
fprintf (outputfile, "Entered simpleflash_initialize - numsimpleflashs %d\n", numsimpleflashs);
*/
   simpleflash_setcallbacks();
   simpleflash_postpass();

   for (i=0; i<MAXDEVICES; i++) {
      simpleflash_t *currflash = getsimpleflash (i);
      if(!currflash) continue;
/*        if (!currflash->inited) { */
         currflash->media_busy = FALSE;
         currflash->reconnect_reason = -1;
         addlisttoextraq ((event **) &currflash->buswait);
         currflash->busowned = -1;
         ioqueue_initialize (currflash->queue, i);
         simpleflash_statinit(i, TRUE);
/*        } */
   }
}


void simpleflash_resetstats (void)
{
   int i;

   for (i=0; i<MAXDEVICES; i++) {
      simpleflash_t *currflash = getsimpleflash (i);
      if (currflash) {
         ioqueue_resetstats(currflash->queue);
         simpleflash_statinit(i, 0);
      }
   }
}



int simpleflash_add(struct simpleflash *d) {
  int c;

  if(!disksim->simpleflashinfo) simpleflash_initialize_diskinfo();
  
  for(c = 0; c < disksim->simpleflashinfo->simpleflashs_len; c++) {
    if(!disksim->simpleflashinfo->simpleflashs[c]) {
      disksim->simpleflashinfo->simpleflashs[c] = d;
      numsimpleflashs++;
      return c;
    }
  }

  /* note that numdisks must be equal to diskinfo->disks_len */
  disksim->simpleflashinfo->simpleflashs = 
    realloc(disksim->simpleflashinfo->simpleflashs, 
	    2 * c * sizeof(struct simpleflash *));

  bzero(disksim->simpleflashinfo->simpleflashs + numsimpleflashs, 
	numsimpleflashs);

  disksim->simpleflashinfo->simpleflashs[c] = d;
  numsimpleflashs++;
  disksim->simpleflashinfo->simpleflashs_len *= 2;
  return c;
}


struct simpleflash *disksim_simpleflash_loadparams(struct lp_block *b)
{
  /* temp vars for parameters */
  struct simpleflash *result;
  int num;

  if(!disksim->simpleflashinfo) simpleflash_initialize_diskinfo();

  result = malloc(sizeof(struct simpleflash));
  if(!result) return 0;
  bzero(result, sizeof(struct simpleflash));
  
  num = simpleflash_add(result);

  result->hdr = simpleflash_hdr_initializer; 
  if(b->name)
    result->hdr.device_name = strdup(b->name);

#include "modules/disksim_simpleflash_param.c"

  flash_numblocks = result->numblocks;
  flash_extrblocks = result->extrblocks;
  ftl_type = result->type;

  device_add((struct device_header *)result, num);
  return result;
}


struct simpleflash *simpleflash_copy(struct simpleflash *orig) {
  struct simpleflash *result = malloc(sizeof(struct simpleflash));
  bzero(result, sizeof(struct simpleflash));
  memcpy(result, orig, sizeof(struct simpleflash));
  result->queue = ioqueue_copy(orig->queue);

  return result;
}

void simpleflash_set_syncset (int setstart, int setend)
{
}


static void simpleflash_acctime_printstats (int *set, int setsize, char *prefix)
{
   int i;
   statgen * statset[MAXDEVICES];

   if (device_printacctimestats) {
      for (i=0; i<setsize; i++) {
         simpleflash_t *currflash = getsimpleflash (set[i]);
         statset[i] = &currflash->stat.acctimestats;
      }
      stat_print_set(statset, setsize, prefix);
   }
}


static void simpleflash_other_printstats (int *set, int setsize, char *prefix)
{
   int i;
   int numbuswaits = 0;
   double waitingforbus = 0.0;

   for (i=0; i<setsize; i++) {
      simpleflash_t *currflash = getsimpleflash (set[i]);
      numbuswaits += currflash->stat.numbuswaits;
      waitingforbus += currflash->stat.waitingforbus;
   }

   fprintf(outputfile, "%sTotal bus wait time: %f\n", prefix, waitingforbus);
   fprintf(outputfile, "%sNumber of bus waits: %d\n", prefix, numbuswaits);
}


void simpleflash_printsetstats (int *set, int setsize, char *sourcestr)
{
   int i;
   struct ioq * queueset[MAXDEVICES];
   int reqcnt = 0;
   char prefix[80];

   sprintf(prefix, "%ssimpleflash ", sourcestr);
   for (i=0; i<setsize; i++) {
      simpleflash_t *currflash = getsimpleflash (set[i]);
      queueset[i] = currflash->queue;
      reqcnt += ioqueue_get_number_of_requests(currflash->queue);
   }
   if (reqcnt == 0) {
      fprintf (outputfile, "\nNo simpleflash requests for members of this set\n\n");
      return;
   }
   ioqueue_printstats(queueset, setsize, prefix);

   simpleflash_acctime_printstats(set, setsize, prefix);
   simpleflash_other_printstats(set, setsize, prefix);
}


void simpleflash_printstats (void)
{
   struct ioq * queueset[MAXDEVICES];
   int set[MAXDEVICES];
   int i;
   int reqcnt = 0;
   char prefix[80];
   int diskcnt;

   fprintf(outputfile, "\nsimpleflash STATISTICS\n");
   fprintf(outputfile, "---------------------\n\n");

   sprintf(prefix, "simpleflash ");

   diskcnt = 0;
   for (i=0; i<MAXDEVICES; i++) {
      simpleflash_t *currflash = getsimpleflash (i);
      if (currflash) {
         queueset[diskcnt] = currflash->queue;
         reqcnt += ioqueue_get_number_of_requests(currflash->queue);
         diskcnt++;
      }
   }
   //assert (diskcnt == numsimpleflashs); //commented by Kanishk to allow hetrogeneous topology

   if (reqcnt == 0) {
      fprintf(outputfile, "No simpleflash requests encountered\n");
      return;
   }

   ioqueue_printstats(queueset, numsimpleflashs, prefix);

   diskcnt = 0;
   for (i=0; i<MAXDEVICES; i++) {
      simpleflash_t *currflash = getsimpleflash (i);
      if (currflash) {
         set[diskcnt] = i;
         diskcnt++;
      }
   }
   //assert (diskcnt == numsimpleflashs);//commented by Kanishk to allow hetrogeneous topology

   simpleflash_acctime_printstats(set, numsimpleflashs, prefix);
   simpleflash_other_printstats(set, numsimpleflashs, prefix);
   fprintf (outputfile, "\n\n");

   if (numsimpleflashs <= 1) {
      return;
   }

   for (i=0; i<numsimpleflashs; i++) {
      simpleflash_t *currflash = getsimpleflash (set[i]);
      if (currflash->printstats == FALSE) {
	 continue;
      }
      if (ioqueue_get_number_of_requests(currflash->queue) == 0) {
	 fprintf(outputfile, "No requests for simpleflash #%d\n\n\n", set[i]);
	 continue;
      }
      fprintf(outputfile, "simpleflash #%d:\n\n", set[i]);
      sprintf(prefix, "simpleflash #%d ", set[i]);
      ioqueue_printstats(&currflash->queue, 1, prefix);
      simpleflash_acctime_printstats(&set[i], 1, prefix);
      simpleflash_other_printstats(&set[i], 1, prefix);
      fprintf (outputfile, "\n\n");
   }
}


double simpleflash_get_seektime (int devno, 
				ioreq_event *req, 
				int checkcache, 
				double maxtime)
{
  fprintf(stderr, "device_get_seektime not supported for simpleflash devno %d\n",  devno);
  assert(0);
}

/* default simpleflash dev header */
struct device_header simpleflash_hdr_initializer = { 
//  DEVICETYPE_simpleflash,
  DEVICETYPE_SIMPLEFLASH,
  sizeof(struct simpleflash),
  "unnamed simpleflash",
  (void *)simpleflash_copy,
  simpleflash_set_depth,
  simpleflash_get_depth,
  simpleflash_get_inbus,
  simpleflash_get_busno,
  simpleflash_get_slotno,
  simpleflash_get_number_of_blocks,
  simpleflash_get_maxoutstanding,
  simpleflash_get_numcyls,
  simpleflash_get_blktranstime,
  simpleflash_get_avg_sectpercyl,
  simpleflash_get_mapping,
  simpleflash_event_arrive,
  simpleflash_get_distance,
  simpleflash_get_servtime,
  simpleflash_get_seektime,
  simpleflash_get_acctime,
  simpleflash_bus_delay_complete,
  simpleflash_bus_ownership_grant
};
