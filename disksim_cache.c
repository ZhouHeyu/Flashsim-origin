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

#include "disksim_global.h"
#include "disksim_iosim.h"
#include "disksim_ioqueue.h"
#include "disksim_cache.h"
#include "disksim_cachemem.h"
#include "disksim_cachedev.h"
#include "config.h"

static int get_cachetype (struct cache_def *cache)
{
   /* all caches must support this function uniformly, so just call one */
   return cachemem_get_cachetype (cache);
}


int cache_get_maxreqsize (struct cache_def *cache)
{
   if (get_cachetype(cache) == CACHE_MEMORY) {
      return cachemem_get_maxreqsize(cache);
   } else {   /* CACHE_DEVICE */
      return cachedev_get_maxreqsize(cache);
   }
}


/* Gets the appropriate block, locked and ready to be accessed read or write */

int cache_get_block (struct cache_def *cache, ioreq_event *req, void (**donefunc)(void *, ioreq_event *), void *doneparam)
{
   if (get_cachetype(cache) == CACHE_MEMORY) {
      return cachemem_get_block(cache, req, donefunc, doneparam);
   } else {   /* CACHE_DEVICE */
      return cachedev_get_block(cache, req, donefunc, doneparam);
   }
}


/* frees the block after access complete, block is clean so remove locks */
/* and update lru                                                        */

void cache_free_block_clean (struct cache_def *cache, ioreq_event *req)
{
   if (get_cachetype(cache) == CACHE_MEMORY) {
      return cachemem_free_block_clean(cache, req);
   } else {   /* CACHE_DEVICE */
      return cachedev_free_block_clean(cache, req);
   }
}


/* a delayed write - set dirty bits, remove locks and update lru.        */
/* If cache doesn't allow delayed writes, forward this to async          */

int cache_free_block_dirty (struct cache_def *cache, ioreq_event *req, void (**donefunc)(void *, ioreq_event *), void *doneparam)
{
   if (get_cachetype(cache) == CACHE_MEMORY) {
      return cachemem_free_block_dirty(cache, req, donefunc, doneparam);
   } else {   /* CACHE_DEVICE */
      return cachedev_free_block_dirty(cache, req, donefunc, doneparam);
   }
}


int cache_sync (struct cache_def *cache)
{
   if (get_cachetype(cache) == CACHE_MEMORY) {
      return cachemem_sync(cache);
   } else {   /* CACHE_DEVICE */
      return cachedev_sync(cache);
   }
}


struct cacheevent *cache_disk_access_complete (struct cache_def *cache, ioreq_event *curr)
{
   if (get_cachetype(cache) == CACHE_MEMORY) {
      return cachemem_disk_access_complete(cache, curr);
   } else {   /* CACHE_DEVICE */
      return cachedev_disk_access_complete(cache, curr);
   }
}


void cache_wakeup_complete (struct cache_def *cache, struct cacheevent *desc)
{
   if (get_cachetype(cache) == CACHE_MEMORY) {
      cachemem_wakeup_complete(cache, desc);
   } else {   /* CACHE_DEVICE */
      cachedev_wakeup_complete(cache, desc);
   }
}


void cache_resetstats (struct cache_def *cache)
{
   if (get_cachetype(cache) == CACHE_MEMORY) {
      cachemem_resetstats(cache);
   } else {   /* CACHE_DEVICE */
      cachedev_resetstats(cache);
   }
}


void cache_setcallbacks ()
{
   cachemem_setcallbacks();
   cachedev_setcallbacks();
}


void cache_initialize (struct cache_def *cache, 
		       void (**issuefunc)(void *,ioreq_event *), 
		       void *issueparam, 
		       struct ioq * (**queuefind)(void *,int), 
		       void *queuefindparam, 
		       void (**wakeupfunc)(void *,struct cacheevent *), 
		       void *wakeupparam, 
		       int numdevs)
{
   if (get_cachetype(cache) == CACHE_MEMORY) {
      cachemem_initialize(cache, issuefunc, issueparam, 
			  queuefind, queuefindparam, wakeupfunc, 
			  wakeupparam, numdevs);
   } else {   /* CACHE_DEVICE */
      cachedev_initialize(cache, issuefunc, issueparam, 
			  queuefind, queuefindparam, wakeupfunc, 
			  wakeupparam, numdevs);
   }
}


void cache_cleanstats (struct cache_def *cache)
{
   if (get_cachetype(cache) == CACHE_MEMORY) {
      cachemem_cleanstats(cache);
   } else {   /* CACHE_DEVICE */
      cachedev_cleanstats(cache);
   }
}


void cache_printstats (struct cache_def *cache, char *prefix)
{
   if (get_cachetype(cache) == CACHE_MEMORY) {
      cachemem_printstats(cache, prefix);
   } else {   /* CACHE_DEVICE */
      cachedev_printstats(cache, prefix);
   }
}


struct cache_def * cache_copy (struct cache_def *cache)
{
   if (get_cachetype(cache) == CACHE_MEMORY) {
      return cachemem_copy(cache);
   } else {   /* CACHE_DEVICE */
      return cachedev_copy(cache);
   }
}




struct cache_def *disksim_cache_loadparams(struct lp_block *b)
{
  int c = 0;
  struct cache_def *result;


  switch(b->type) {
  case DISKSIM_MOD_CACHEMEM:
    result = disksim_cachemem_loadparams(b);
    break;
  case DISKSIM_MOD_CACHEDEV:
    result = disksim_cachedev_loadparams(b);
    break;
  default:
    fprintf(stderr, "*** error: Invalid cache type (%d) specified.\n", IVAL(b->params[c]));
    return 0;
    break;
  }


  if(!result) {
    fprintf(stderr, "*** error: failed to load cache definition.\n");
    return 0;
  }

  return result;
}

