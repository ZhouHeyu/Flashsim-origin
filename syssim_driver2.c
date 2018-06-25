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
 * A sample skeleton for a system simulator that calls DiskSim as
 * a slave.
 *
 * Contributed by Eran Gabber of Lucent Technologies - Bell Laboratories
 *
 * Usage:
 *	syssim <parameters file> <output file> <max. block number>
 * Example:
 *	syssim parv.seagate out 2676846
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include "syssim_driver.h"

#include "disksim_global.h"
#include "disksim_iosim.h"
#include "disksim_iodriver.h"
#include "disksim_disk.h"

#include <sys/param.h>      /* for MIN */
#include <assert.h>

#include <sys/time.h>
#include <unistd.h>
#include <getopt.h>

/* #define DEBUG_SEEK */  
/* #define DEBUG_EXTRA */ 
/* #define DEBUG_EXTRASEEK */
/* #define DEBUG_PHYSREQUESTS */
/* #define DEBUG_COMPLETIONS */
/* #define DEBUG_POSSIBLE */
/* #define DEBUG_NOTBORING */
/* #define DEBUG_SEQPROGRESS */
/* #define DEBUG_ISSUE */
/* #define DEBUG_BLOCKMAP */
/* #define OLD */

#define HAVE_X86

#define MAX_TIME 8000
extern char *optarg;
extern int optind;
extern int opterr;
extern int optopt;

/* algorithm defines */
#define SCHED_INIT 0x1
#define SCHED_OUT_VALID 0x2

#define MIN_ALG 1
#define GREEDY_BW_ALG 1
#define GREEDY_BLOCK_COIN_ALG 2
#define SSTF_ALG 3
#define SEPTF_ALG 4
#define GREEDY_BW_PROB_COIN_ALG 5
#define GREEDY_BW_EDGE_COIN_ALG 6
#define GREEDY_BW_SOURCE_ALG 7
#define GREEDY_BLOCK_ALG 8
#define GREEDY_BW_SOURCE_WEIGHT_ALG 9
#define LEAST_LATENCY_ALG 10
#define GREEDY_BW_MAX_DEST_ALG 11
#define GREEDY_BW_COIN_ALG 12
#define GREEDY_BW_SOURCE_DIST_ALG 13
#define GREEDY_BW_TIME_DIST_ALG 14
#define GREEDY_UNIFORM_RATE_ALG 15
#define GREEDY_BW_TRACK_ALG 16
#define GREEDY_AREA_FIRST_ALG 17
#define GREEDY_BW_TRACK_DIST_ALG 18
#define SEPTF_TRACK_ALG 19
#define SEPTF_FRAGMENT_ALG 20
#define MAX_ALG 20

/* end algorithm defines */

#define	BLOCK	4096
#define	SECTOR	512
#define	BLOCK2SECTOR	(BLOCK/SECTOR)
#define	STRMAX	512

typedef	struct	{
	int n;
	double sum;
	double sqr;
} Stat;

/* globals */
static SysTime now = 0;		/* current time */
static SysTime next_event = 0;	/* next event */
//static int completed = 0;	/* last request was completed */
//static int pending = 0;	        /* number of requests pending */
static Stat st;

//#define MAXDISKS 10
//static double last_issued = 0.0;
static double last_foreground;
static int seq_blkno[MAXDISKS];
static int sourcecyl[MAXDISKS];
static int destcyl[MAXDISKS];
static int inside[MAXDISKS];
static int outside[MAXDISKS];
static int zero[MAXDISKS];
static double last_background[MAXDISKS];
static int background_outstanding[MAXDISKS];

/* disk, workload parameters */
static double interarrival = 65.12;
static double readfraction = 2.0 / 3.0;
//static int max_sectors = 4400000;
static int ave_sectors = 10;
static int seq_sectors = 16;
static int servtime = 0;
static int onlyfourpass = 0;
static float servicetime = 0.0;
static int min_sector_pickup = 1;

static double off_cyl_mult = 1.0;

static double threshold = 1.0;

static double conservative = 0.0;

/* inputs */
//static int nsectors;
static int do_background = 0;
static int algorithm = 0;
static int amount_background = 1000;
static int multi_programming = 2;
static double sim_length = 20000.0;
static int pickup_demand_blocks = 0;
static double hot_fraction = 0.0;
static int do_input_trace=0;
static int active_requests=0;
static int done10=0;
static int done20=0;
static int done30=0;
static int done40=0;
static int done50=0;
static int done60=0;
static int done70=0;
static int done80=0;
static int done90=0;
static int done95=0;
static int done100=0;

/* trace file */

static FILE *tracefile;
static FILE *free_block_rate;

static statgen *total_latency_stat;
static statgen *search_count_stat;
static statgen *total_seek_stat;
static statgen *total_transfer_stat;
static statgen *free_bandwidth_stat;
static statgen *extra_seek_stat;
static statgen *total_latency_stat_inst;
static statgen *free_bandwidth_stat_inst;
static statgen *extra_seek_stat_inst;
static statgen **latency_stat;
static statgen **seek_dist_stat;
static statgen **free_count_stat;
static statgen **free_demand_count_stat;
static statgen **take_free_time_stat;
static statgen **free_pop_stat;
static statgen **free_cyl_pop_stat;
static statgen **free_cyl_stat;

static int req_count=0;
static float free_bandwidth;
static int max_band_count=0;
static double max_cyl_prob=0.0;

static float cyl_mean;
static long cyl_dist_count=0;
static long total_seeks=0;

/* weight for algorithm GREEDY_BW_SOURCE_WEIGHT_ALG */
static float FB_weight=1;

static long alg_count=0;

static int area_firstcyl=1;
static int area_lastcyl=2;

/* bucket sizes */

//#define CYL_ENTRIES 1
#define MAX_REQ 3
#define PERIOD 500.0
#define MICROMULT 1000000
#define ANGLE_ENTRIES 360
#define MAX_ANGLE 360
#define MAX_SECTORS 256
//#define MAXSURFACES 4
#define TRACEBATCHSIZE 25
#define SEARCH_COUNT_STAT "Search count"
#define EXTRA_SEEK_STAT "Extra seek time"
#define FREE_BANDWIDTH_STAT "Free bandwidth"
#define LATENCY_STAT "Rotational latency"
#define TRANSFER_STAT "Transfer time"
#define SEEK_STAT "Seek time"
#define SEEK_DIST_STAT "Seek distance"
#define FREE_COUNT_STAT "Free count"
#define FREE_DEMAND_COUNT_STAT "Free demand count"
#define TAKE_FREE_TIME_STAT "Free time"
#define FREE_POP_STAT "Free population"
#define FREE_CYL_POP_STAT "Free cylinder population"
#define FREE_CYL_STAT "Free cylinder"
#define STAT_FILENAME "free.statsdef"
#define LATENCY_WEIGHT "ioqueue_latency_weight"
#define MAX_COUNT_WEIGHT 1.0
#define CYL_DIST_MEAN 1000

#define numiodrivers            (disksim->iodriver_info->numiodrivers)
#define iodrivers               (disksim->iodriver_info->iodrivers)

extern double remote_disk_seek_time(int distnace, int headswitch, int read);
extern event* iotrace_get_ioreq_event(FILE* iotracefile,int traceformat, ioreq_event* new);

/* added disksim variable */

long total_timer;
long free_time;

long time_scale = 550000;

#define BITMAP_CHECK(DEVNO,CYL,SURF,SECT)       ((bitmap[DEVNO][SURF][CYL][SECT >> 5] & (0x1 << (SECT & 0x001F))) >> (SECT & 0x001F)) 
#define BITMAP_SET(DEVNO,CYL,SURF,SECT)       (bitmap[DEVNO][SURF][CYL][SECT >> 5] |= (0x1 << (SECT & 0x001F)))
#define BITMAP_CLEAR(DEVNO,CYL,SURF,SECT)       (bitmap[DEVNO][SURF][CYL][SECT >> 5] &= ((0xFFFFFFFE << (SECT & 0x001F)) | (0xFFFFFFFE >> (0x001F - ((SECT-1) & 0x001F)))))


void panic(const char *s)
{
  perror(s);
  exit(1);
}

void add_statistics(Stat *s, double x)
{
  s->n++;
  s->sum += x;
  s->sqr += x*x;
}

void print_statistics(Stat *s, const char *title)
{
  double avg, std;
  
  avg = s->sum/s->n;
  std = sqrt((s->sqr - 2*avg*s->sum + s->n*avg*avg) / s->n);
  fprintf(stderr,"%s: n=%d average=%f std. deviation=%f\n", title, s->n, avg, std);
}

/* 
 *   For free block figuring
 *
 *
 */

static int nrequests[MAXDISKS];

typedef struct cyl_node_s {
  unsigned short cylinder;
  unsigned char possible_seek;
  unsigned char nsectors;
  float source_seektime;
  float dest_seektime;
  float extra_time;
} cyl_node_t;

typedef struct node_s {
  unsigned short sectors_rotate;
  unsigned char sector;
  unsigned char possible_rotate;
  unsigned char possible;
  unsigned char cumulative_possible;
  unsigned char desired;
  unsigned char completed;
  float source_rotate;
} node_t;

typedef struct bucket_s {
  float angle;
  unsigned short cylinder;
  struct bucket_s *next;
} bucket_t;

/*typedef struct bitmap_s {
  unsigned int bit : 1;
  } bitmap_t;*/


typedef struct touchmap_s {
  float touch;
  float squared;
  float max;
} touchmap_t;

typedef struct entry_s {
  unsigned int *free_block_count;
} entry_t;

typedef struct block_s {
  unsigned short cylinder;
  unsigned char sector;
} block_t;

static int min_fragment_size=1;
static int numcyls;
static int numsurfaces;
static int numblocks;
static int *sectorspercyl;
static int *cylsperzone;
static double *probpercyl;
static float oneRotation;
static int ndrives = 1;
static float extra_settle_angle = 0.0;
static float **sector_angle;
static unsigned short **min_sectors;
static float *seek_angle;
static float *seek_min;
//static bitmap_t ****bitmap;
static unsigned int ****bitmap;
static touchmap_t ***touchmap;
static int *free_blocks;
static int *cyl_order;
static int **free_count;
static unsigned short ***free_count_track;
void free_block_request(unsigned short cyl, unsigned short sector, float angle, unsigned short surface, int devno);


void map_blocknum(int diskno, int blocknum, unsigned short *cyl, unsigned short *sector, unsigned short *surface)
{
  int temp_cyl;
  int temp_sector;
  int temp_surface;
  
#ifdef DEBUG_BLOCKMAP
  fprintf(stderr,"input block %d to",blocknum);
#endif /* DEBUG_BLOCKMAP */
  disk_get_mapping (MAP_FULL, diskno, blocknum, &temp_cyl, &temp_surface, &temp_sector);
  //  *cyl = blockmap[blocknum / surfaces].cylinder;
  //*sector = blockmap[blocknum / surfaces].sector;
#ifdef DEBUG_BLOCKMAP
  fprintf(stderr," (%d,%d,%d)\n",temp_cyl,temp_sector,temp_surface);
#endif /* DEBUG_BLOCKMAP */

  *cyl = (short)temp_cyl;
  *sector = (short)temp_sector;
  *surface = (short)temp_surface;
  
}

#define CYCLES_PER_MS 550000

inline long long get_time(){

  static unsigned low;
  static unsigned high;
  long long returnval;

  __asm__(".byte 0x0f,0x31" :"=a" (low), "=d" (high)); /* RDTSC */
  
  returnval = high;
  returnval = returnval << 32;
  returnval += low;
  
  return(returnval);
}

/*void calibrate_timer(){
  }*/

void init_free_blocks()
{
  int i,j,k;

  FILE *statfile;

  unsigned short cyl = 0;
  unsigned short sector = 0;
  unsigned short surface = 0;
  
  /*
   *   data space
   */
  
  statfile = fopen(STAT_FILENAME,"r");
  if(statfile == NULL){
    fprintf(stderr,"ERROR:  Statsdef file %s could not be opened for reading\n",STAT_FILENAME);
    exit(-1);
  }

  latency_stat = (statgen **)calloc((int)ceil(sim_length/(float)PERIOD),sizeof(statgen));
  if(latency_stat == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for latency_stat\n");
    exit(-1);
  }

  seek_dist_stat = (statgen **)calloc((int)ceil(sim_length/(float)PERIOD),sizeof(statgen));
  if(seek_dist_stat == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for seek_disk_stat\n");
    exit(-1);
  }

  free_pop_stat = (statgen **)calloc((int)ceil(sim_length/(float)PERIOD),sizeof(statgen));
  if(free_pop_stat == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for free_pop_stat\n");
    exit(-1);
  }
  
  free_cyl_pop_stat = (statgen **)calloc((int)ceil(sim_length/(float)PERIOD),sizeof(statgen));
  if(free_cyl_pop_stat == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for free_cyl_pop_stat\n");
    exit(-1);
  }

  free_cyl_stat = (statgen **)calloc((int)ceil(sim_length/(float)PERIOD),sizeof(statgen));
  if(free_cyl_stat == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for free_cyl_stat\n");
    exit(-1);
  }
  
  free_count_stat = (statgen **)calloc((int)ceil(sim_length/(float)PERIOD),sizeof(statgen));
  if(free_count_stat == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for free_count_stat\n");
    exit(-1);
  }
  
  free_demand_count_stat = (statgen **)calloc((int)ceil(sim_length/(float)PERIOD),sizeof(statgen));
  if(free_count_stat == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for free_demand_count_stat\n");
    exit(-1);
  }
  
  take_free_time_stat = (statgen **)calloc((int)ceil(sim_length/(float)PERIOD),sizeof(statgen));
  if(take_free_time_stat == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for take_free_time_stat\n");
    exit(-1);
  }
  
  total_latency_stat = (statgen *)malloc(sizeof(statgen));
  if(total_latency_stat == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for total_latency_stat\n");
    exit(-1);
  }

  stat_initialize(statfile, LATENCY_STAT, total_latency_stat);

  search_count_stat = (statgen *)malloc(sizeof(statgen));
  if(search_count_stat == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for search_count_stat\n");
    exit(-1);
  }

  stat_initialize(statfile, SEARCH_COUNT_STAT, search_count_stat);

  total_transfer_stat = (statgen *)malloc(sizeof(statgen));
  if(total_transfer_stat == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for total_transfer_stat\n");
    exit(-1);
  }

  stat_initialize(statfile, TRANSFER_STAT, total_transfer_stat);

  total_seek_stat = (statgen *)malloc(sizeof(statgen));
  if(total_seek_stat == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for total_seek_stat\n");
    exit(-1);
  }

  stat_initialize(statfile, SEEK_STAT, total_seek_stat);

  free_bandwidth_stat = (statgen *)malloc(sizeof(statgen));
  if(free_bandwidth_stat == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for free_bandwitdh_stat\n");
    exit(-1);
  }
  
  stat_initialize(statfile, FREE_BANDWIDTH_STAT, free_bandwidth_stat);

  extra_seek_stat = (statgen *)malloc(sizeof(statgen));
  if(extra_seek_stat == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for free_bandwitdh_stat\n");
    exit(-1);
  }
  
  stat_initialize(statfile,EXTRA_SEEK_STAT, extra_seek_stat);  

  total_latency_stat_inst = (statgen *)malloc(sizeof(statgen));
  if(total_latency_stat_inst == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for total_latency_stat_inst\n");
    exit(-1);
  }

  stat_initialize(statfile, LATENCY_STAT, total_latency_stat_inst);

  free_bandwidth_stat_inst = (statgen *)malloc(sizeof(statgen));
  if(free_bandwidth_stat_inst == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for free_bandwitdh_stat_inst\n");
    exit(-1);
  }
  
  stat_initialize(statfile, FREE_BANDWIDTH_STAT, free_bandwidth_stat_inst);

  extra_seek_stat_inst = (statgen *)malloc(sizeof(statgen));
  if(extra_seek_stat_inst == NULL){
    fprintf(stderr,"ERROR:  Could not malloc memory for free_bandwitdh_stat_inst\n");
    exit(-1);
  }
  
  stat_initialize(statfile,EXTRA_SEEK_STAT, extra_seek_stat_inst);  

  for(i=0;i<(int)ceil(sim_length/(float)PERIOD);i++){
    latency_stat[i] = (statgen *)malloc(sizeof(statgen));
    if(latency_stat[i] == NULL){
      fprintf(stderr,"ERROR:  Could not malloc memory for latency_stat\n");
      exit(-1);
    }
    seek_dist_stat[i] = (statgen *)malloc(sizeof(statgen));
    if(seek_dist_stat[i] == NULL){
      fprintf(stderr,"ERROR:  Could not malloc memory for seek_dist_stat\n");
      exit(-1);
    }
    free_pop_stat[i] = (statgen *)malloc(sizeof(statgen));
    if(free_pop_stat[i] == NULL){
      fprintf(stderr,"ERROR:  Could not malloc memory for free_pop_stat\n");
      exit(-1);
    }

    free_cyl_pop_stat[i] = (statgen *)malloc(sizeof(statgen));
    if(free_cyl_pop_stat[i] == NULL){
      fprintf(stderr,"ERROR:  Could not malloc memory for free_cyl_pop_stat\n");
      exit(-1);
    }

    free_cyl_stat[i] = (statgen *)malloc(sizeof(statgen));
    if(free_cyl_stat[i] == NULL){
      fprintf(stderr,"ERROR:  Could not malloc memory for free_cyl_stat\n");
      exit(-1);
    }
    
    free_count_stat[i] = (statgen *)malloc(sizeof(statgen));
    if(free_count_stat[i] == NULL){
      fprintf(stderr,"ERROR:  Could not malloc memory for free_count_stat\n");
      exit(-1);
    }
    
    free_demand_count_stat[i] = (statgen *)malloc(sizeof(statgen));
    if(free_count_stat[i] == NULL){
      fprintf(stderr,"ERROR:  Could not malloc memory for free_demand_count_stat\n");
      exit(-1);
    }
    
    take_free_time_stat[i] = (statgen *)malloc(sizeof(statgen));
    if(take_free_time_stat[i] == NULL){
      fprintf(stderr,"ERROR:  Could not malloc memory for take_free_time_stat\n");
      exit(-1);
    }
    
    stat_initialize(statfile, LATENCY_STAT, latency_stat[i]);
    stat_initialize(statfile, SEEK_DIST_STAT, seek_dist_stat[i]);
    stat_initialize(statfile, FREE_POP_STAT, free_pop_stat[i]);
    stat_initialize(statfile, FREE_CYL_POP_STAT, free_cyl_pop_stat[i]);
    stat_initialize(statfile, FREE_CYL_STAT, free_cyl_stat[i]);
    stat_initialize(statfile, FREE_COUNT_STAT, free_count_stat[i]);
    stat_initialize(statfile, FREE_DEMAND_COUNT_STAT, free_demand_count_stat[i]);
    stat_initialize(statfile, TAKE_FREE_TIME_STAT, take_free_time_stat[i]);
  }

  //  if(algorithm ==  1 || algorithm ==  2 || algorithm ==  3){
  if(1){
    for(i=0;i<numsurfaces;i++){
      for(j=0;j<ANGLE_ENTRIES;j++){
	for(k=0;k<ndrives;k++){
	  free_blocks[k] = 0;
	}
      }
    }
  }
  
#ifdef DEBUG_MEMORY
  fprintf(stderr,"allocated %d bytes\n",6144 * 216 * sizeof(node_t));
  fprintf(stderr,"sizeof(cyl_node_t) = %d sizeof(node_t) = %d\n",sizeof(cyl_node_t),sizeof(node_t));
#endif /* DEBUG_MEMORY */

  //  init_locations();

  for(j=0;j<ndrives;j++) {
    nrequests[j] = 0;
  }
  for(j=0;j<ndrives;j++) {
    for(i=0;i<numblocks;i++) {
      map_blocknum(j,i,&cyl,&sector,&surface);
      free_block_request(cyl, sector,((float)sector/(float)sectorspercyl[cyl] * (float)MAX_ANGLE),surface,j);
    }
  }
}


void free_block_request(unsigned short cyl, unsigned short sector,  float angle, unsigned short surface, int devno){
  BITMAP_SET(devno,cyl,surface,sector);
  free_count[devno][cyl]++;
  free_count_track[devno][cyl][surface]++;
}

static long max_time = 0;
static float max_percent = 0.0;
static int max_cyl = 0;

void pickup_free_blocks(unsigned short cylinder, 
			unsigned short surface,
			int start_sector,
			int finish_sector,
			unsigned short source_cyl,
			unsigned short dest_cyl,
			unsigned short devno){
  int i;
  int count=0;
  unsigned short top_cyl,bottom_cyl;
  float extra_seek=0.0;
  int min_cyl = abs(source_cyl - cylinder);

  if(min_cyl > abs(dest_cyl - cylinder)){
    min_cyl = abs(dest_cyl - cylinder);
  }

  if(min_cyl > max_cyl){
    max_cyl = min_cyl;
  }

  if(source_cyl < dest_cyl){
    bottom_cyl = source_cyl;
    top_cyl = dest_cyl;
  }else{
    bottom_cyl = dest_cyl;
    top_cyl = source_cyl;
  }

  for(i=0;i<sectorspercyl[cylinder];i++){
    if(finish_sector < start_sector){
      if(i >= start_sector || i < finish_sector){
	if(BITMAP_CHECK(devno,cylinder,surface,i)){
	  free_count[devno][cylinder]--;
	  free_count_track[devno][cylinder][surface]--;
	  free_blocks[devno]++;
	  BITMAP_CLEAR(devno,cylinder,surface,i);
	  touchmap[devno][surface][cylinder].max = now;
	  touchmap[devno][surface][cylinder].touch = (now/
						      (float)
						      sectorspercyl[cylinder]);
	  touchmap[devno][surface][cylinder].squared = (now * now /
						      (float)
						      sectorspercyl[cylinder]);
	  count++;
	}
      }
    }else if(finish_sector > start_sector){
      if(i >= start_sector && i < finish_sector){
	if(BITMAP_CHECK(devno,cylinder,surface,i)){
	  free_count[devno][cylinder]--;
	  free_count_track[devno][cylinder][surface]--;
	  free_blocks[devno]++;
	  BITMAP_CLEAR(devno,cylinder,surface,i);
	  touchmap[devno][surface][cylinder].max = now;
	  touchmap[devno][surface][cylinder].touch = (now/
						      (float)
						      sectorspercyl[cylinder]);
	  touchmap[devno][surface][cylinder].squared = (now * now /
						      (float)
						      sectorspercyl[cylinder]);
	  count++;
	}
      }	  
    } else {
      if(BITMAP_CHECK(devno,cylinder,surface,i)){
	free_count[devno][cylinder]--;
	free_count_track[devno][cylinder][surface]--;
	free_blocks[devno]++;
	BITMAP_CLEAR(devno,cylinder,surface,i);
	touchmap[devno][surface][cylinder].max = now;
	touchmap[devno][surface][cylinder].touch = (now/
						    (float)
						    sectorspercyl[cylinder]);
	touchmap[devno][surface][cylinder].squared = (now * now /
						      (float)
						      sectorspercyl[cylinder]);
	count++;
      }
    }
  }
  if(count != 0){
    if(count > 1){
      //      fprintf(stderr,"picked up %d blocks from %d source %d dest %d time %f\n",count,cylinder,source_cyl,dest_cyl,(float)count/(float)sectorspercyl[cylinder]*oneRotation);
      //  fprintf(stderr,"pick finish sec %d start sec %d %d\n",finish_sector,start_sector,sectorspercyl[cylinder]);
    }
    free_bandwidth += ((float)count / (float) sectorspercyl[cylinder] * 
		       oneRotation);
    extra_seek = (seek_angle[abs(source_cyl-cylinder)] +
		  seek_angle[abs(dest_cyl-cylinder)] -
		  seek_angle[abs(source_cyl-dest_cyl)])*oneRotation;
    if(extra_seek < 0.0){
      extra_seek = 0.0;
    }
    stat_update(extra_seek_stat, extra_seek);
    stat_update(extra_seek_stat_inst, extra_seek);
    if(cylinder > top_cyl || cylinder < bottom_cyl){
      outside[devno]++;
    }else if(cylinder == source_cyl){
      sourcecyl[devno]++;
    }else if(cylinder == dest_cyl){
      destcyl[devno]++;
    }else{
      inside[devno]++;	
    }
  }else{
    zero[devno]++;
  }
}

typedef struct alg_args_s{
  float rotate_angle;
  unsigned short devno;
  unsigned short source_cyl;
  float source_angle;
  unsigned short dest_cyl;
  float dest_angle;
  unsigned short source_surface;
  unsigned short dest_surface;
  unsigned short target_cyl;
  unsigned int inputflags;
  unsigned int outputflags;
  unsigned short surface;
  unsigned short start_sector;
  float new_angle;
  unsigned short best_cyl;
  unsigned short finish_sector;
} alg_args_t;

inline void location_info(float extra_time_angle,
			  float source_seek_angle,
			  alg_args_t *args,
			  int *stop_sector,
			  int *finish_sector,
			  int *extra_sectors){
  
  float temp_stop_sector;
  float temp_finish_sector;

  temp_stop_sector = args->source_angle + source_seek_angle;

  while(temp_stop_sector >= 1.0){
    temp_stop_sector -= 1.0;
  } 

  temp_stop_sector *= sectorspercyl[args->target_cyl];

  temp_finish_sector = temp_stop_sector + (extra_time_angle * 
					   sectorspercyl[args->target_cyl]);

  temp_stop_sector = ceil(temp_stop_sector);

  temp_finish_sector = floor(temp_finish_sector);

  *extra_sectors = min_sectors[args->target_cyl][((int)temp_finish_sector -
						  (int)temp_stop_sector)];

  *stop_sector = (min_sectors[args->target_cyl]
		  [(int)temp_stop_sector]);
  
  *finish_sector = (unsigned short)temp_finish_sector;


  while(*finish_sector >= sectorspercyl[args->target_cyl]){
    *finish_sector -= sectorspercyl[args->target_cyl];
  }


  *finish_sector = (min_sectors[args->target_cyl]
		    [*finish_sector]);
}

inline void seek_angles(alg_args_t *args,
			float *source_seek_angle,
			float *dest_seek_angle,
			float *extra_time_angle){

  float orig_seek_angle = (seek_angle[abs(args->source_cyl -
					  args->dest_cyl)]);
  float extra_seek_angle = 0.0;

  if (args->target_cyl == args->source_cyl) {
    *source_seek_angle = 0.0;
  } else {
    if(seek_angle[abs(args->target_cyl - args->source_cyl)] != 0.0){
      *source_seek_angle = (seek_angle[abs(args->target_cyl - 
					     args->source_cyl)]);
			      
    } else {
      fprintf(stderr,"ERROR: seek distance not in structure %d\n",
	      abs(args->target_cyl-args->source_cyl));
      abort();
    }
  }

  if (args->target_cyl == args->dest_cyl) {
    *dest_seek_angle = 0.0;
  } else {
    if(seek_angle[abs(args->target_cyl - args->dest_cyl)] != 0.0){
      *dest_seek_angle = (seek_angle[abs(args->target_cyl - 
					   args->dest_cyl)]);

    } else {
      fprintf(stderr,"ERROR: seek distance not in structure %d\n",
	      abs(args->target_cyl-args->dest_cyl));
      abort();
    }
  }

  extra_seek_angle = ((*source_seek_angle + 
			*dest_seek_angle + 
			 extra_settle_angle + extra_settle_angle) 
			- orig_seek_angle);

  *extra_time_angle = args->rotate_angle - extra_seek_angle;
}

inline void count_blocks(unsigned short stop_sector,
			 unsigned short finish_sector,
			 unsigned short extra_sectors,
			 int surface,
			 alg_args_t *args,
			 unsigned int *temp_count,
			 int *before_found,
			 unsigned int *last_found,
			 unsigned int *last_count){

  unsigned int old_count = 0;
  int j;
  if(extra_sectors > 0){
    *temp_count = 0;
    if(last_count != NULL){
      *last_count = 0;
      *last_found = 0;
    }
    if(before_found != NULL){
      *before_found = 0;
    }
    if(stop_sector > finish_sector){
      for(j=stop_sector;j<sectorspercyl[args->target_cyl];j++){
	(*temp_count) += BITMAP_CHECK(args->devno,args->target_cyl,surface,j);
	if(last_count != NULL){
	  if(*temp_count != old_count){
	    *last_found = j + 1;
	    *last_count = 0;
	  }else{
	    (*last_count)++;
	  }
	  old_count = *temp_count;
	}
	if(before_found != NULL){
	  if(*temp_count == 0){
	    (*before_found)++;
	  }
	}
      }
      for(j=0;j<finish_sector;j++){
	(*temp_count) += BITMAP_CHECK(args->devno,args->target_cyl,surface,j);
	if(last_count != NULL){
	  if(*temp_count != old_count){
	    *last_found = j + 1;
	    *last_count = 0;
	  }else{
	    (*last_count)++;
	  }
	  old_count = *temp_count;
	}
	if(before_found != NULL){
	  if(*temp_count == 0){
	    (*before_found)++;
	  }
	}
      }
    }else if(stop_sector == finish_sector){
      for(j=0;j<sectorspercyl[args->target_cyl];j++){
	(*temp_count) += BITMAP_CHECK(args->devno,args->target_cyl,surface,j);
	if(last_count != NULL){
	  if(*temp_count != old_count){
	    *last_found = j + 1;
	    *last_count = 0;
	  }else{
	    (*last_count)++;
	  }
	  old_count = *temp_count;
	}
	if(before_found != NULL){
	  if(*temp_count == 0){
	    (*before_found)++;
	  }
	}
      }	  
    }else{
      for(j=stop_sector;j<finish_sector;j++){
	(*temp_count) += BITMAP_CHECK(args->devno,args->target_cyl,surface,j);
	if(last_count != NULL){
	  if(*temp_count != old_count){
	    *last_found = j + 1;
	    *last_count = 0;
	  }else{
	    (*last_count)++;
	  }
	  old_count = *temp_count;
	}
	if(before_found != NULL){
	  if(*temp_count == 0){
	    (*before_found)++;
	  }
	}
      }
    }
    if(*temp_count > extra_sectors){
      fprintf(stderr,"ERROR: %d sectors picked up %d possible\n",
	      *temp_count,extra_sectors);
    }
  }
}

#define CYL_RANGE 10

int make_selection_GREEDY_BW_EDGE_COIN(alg_args_t *args){
  /* static vars */
  static float max_bandwidth;
  /* end static vars */

  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  float temp_bandwidth =  0.0;
  unsigned int extra_sectors = 0;
  unsigned int stop_sector = 0;
  int i;
  int temp_count=0;
  float temp_max=0.0;
  int temp_surface=0;
  int temp_finish_sector = 0;
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
  }
  
  if(args->target_cyl < numcyls && (max_bandwidth == 0.0 ||
				    (free_count[args->devno][args->target_cyl]
				     >= sectorspercyl[args->target_cyl] ||
				     sector_angle[args->target_cyl]
				     [free_count[args->devno]
				     [args->target_cyl]] >= 
				     max_bandwidth))){
    
    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);
    
    if(extra_angle > 0.0 && (max_bandwidth == 0.0 || extra_angle
			     >= max_bandwidth)){
      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);
      
      for(i=0;i<numsurfaces;i++){

	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     NULL,
		     NULL,
		     NULL);

	temp_bandwidth = sector_angle[args->target_cyl][temp_count];
	
	if(temp_bandwidth > max_bandwidth || 
	   ((temp_bandwidth == max_bandwidth) && 
	    ((rand() & 0x1) == 1))){
	  max_bandwidth = temp_bandwidth;
	  args->outputflags |= SCHED_OUT_VALID;
	  args->best_cyl = args->target_cyl;
	  args->surface = i;
	  args->new_angle = 0;
	  
	  args->start_sector = stop_sector;
	  args->finish_sector = temp_finish_sector;
	}
	if((args->target_cyl >= (numcyls-CYL_RANGE)) || 
	   (args->target_cyl <= CYL_RANGE)){
	  if(temp_max < temp_bandwidth){
	    temp_max = temp_bandwidth;
	    temp_surface = i;
	  }
	}
      }
    }
  }
  if((args->target_cyl >= (numcyls-CYL_RANGE)) || 
     (args->target_cyl <= CYL_RANGE)){
    if(temp_bandwidth != 0.0){
      args->outputflags |= SCHED_OUT_VALID;
      args->best_cyl = args->target_cyl;
      args->surface = temp_surface;
      args->new_angle = 0;
      
      args->start_sector = stop_sector;
      args->finish_sector = temp_finish_sector;
      return(1);
    }
  }
  return(0);
}

int make_selection_GREEDY_BW_COIN(alg_args_t *args){
  /* static vars */
  static float max_bandwidth;
  /* end static vars */

  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  float temp_bandwidth = 0.0;
  unsigned int extra_sectors = 0;
  int temp_finish_sector = 0;
  unsigned int stop_sector = 0;
  int i;
  int temp_count=0;
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
  }
  
  if(args->target_cyl < numcyls && (max_bandwidth == 0.0 ||
				    (free_count[args->devno][args->target_cyl]
				     >= sectorspercyl[args->target_cyl] ||
				     sector_angle[args->target_cyl]
				     [free_count[args->devno]
				     [args->target_cyl]] >= 
				     max_bandwidth))){
    
    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);
    
    if(extra_angle > 0.0 && (max_bandwidth == 0.0 || extra_angle
			     >= max_bandwidth)){
      
      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);
      
      for(i=0;i<numsurfaces;i++){
	
	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     NULL,
		     NULL,
		     NULL);
	
	temp_bandwidth = sector_angle[args->target_cyl][temp_count];
	
	if(temp_count != 0){
	  if(temp_bandwidth > max_bandwidth || 
	     ((temp_bandwidth == max_bandwidth) && 
	      ((rand() & 0x1) == 1))){
	    max_bandwidth = temp_bandwidth;
	    args->outputflags |= SCHED_OUT_VALID;
	    args->best_cyl = args->target_cyl;
	    args->surface = i;
	    args->new_angle = 0;
	    
	    args->start_sector = stop_sector;
	    args->finish_sector = temp_finish_sector;
	  }
	}
      }
    }
  }
  return(0);
}

int make_selection_GREEDY_BW(alg_args_t *args){
  /* static vars */
  static float max_bandwidth;
  /* end static vars */
  
  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  float temp_bandwidth = 0.0;
  unsigned int extra_sectors = 0;
  int temp_finish_sector = 0;
  unsigned int stop_sector = 0;
  int i;
  int temp_count=0;
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
  }
  
  if(args->target_cyl < numcyls && (max_bandwidth == 0.0 ||
				    (free_count[args->devno][args->target_cyl]
				     >= sectorspercyl[args->target_cyl] ||
				     sector_angle[args->target_cyl]
				     [free_count[args->devno]
				     [args->target_cyl]] >= 
				     max_bandwidth))){
    
    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);
    
    if(extra_angle > 0.0 && (max_bandwidth == 0.0 || extra_angle
			     >= max_bandwidth)){
      
      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);
      
      for(i=0;i<numsurfaces;i++){
	
	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     NULL,
		     NULL,
		     NULL);
	
	temp_bandwidth = sector_angle[args->target_cyl][temp_count];
	
	if(temp_bandwidth > max_bandwidth){
	  max_bandwidth = temp_bandwidth;
	  args->outputflags |= SCHED_OUT_VALID;
	  args->best_cyl = args->target_cyl;
	  args->surface = i;
	  args->new_angle = 0;
	  
	  args->start_sector = stop_sector;
	  args->finish_sector = temp_finish_sector;
	}
      }
    }
  }
  return(0);
}

int make_selection_GREEDY_AREA_FIRST(alg_args_t *args){
  /* static vars */
  static float max_bandwidth;
  /* end static vars */
  
  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  float temp_bandwidth = 0.0;
  unsigned int extra_sectors = 0;
  int temp_finish_sector = 0;
  unsigned int stop_sector = 0;
  int i;
  int temp_count=0;
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
  }
  
  if (((args->target_cyl >= area_firstcyl) && 
       (args->target_cyl <= area_lastcyl))
      && (args->target_cyl < numcyls)){
    
    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);
    
    if(extra_angle > 0.0) {
      
      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);
      for(i=0;i<numsurfaces;i++){
	
	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     NULL,
		     NULL,
		     NULL);
	
	temp_bandwidth = sector_angle[args->target_cyl][temp_count];
	
	if (temp_bandwidth > max_bandwidth) {
	  max_bandwidth = temp_bandwidth;
	  args->outputflags |= SCHED_OUT_VALID;
	  args->best_cyl = args->target_cyl;
	  args->surface = i;
	  args->new_angle = 0;
	  
	  args->start_sector = stop_sector;
	  args->finish_sector = temp_finish_sector;
	}
      }
    }
  }
  return(0);
}

int make_selection_GREEDY_UNIFORM_RATE(alg_args_t *args){
  /* static vars */
  static float max_bandwidth;
  /* end static vars */

  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  float temp_bandwidth = 0.0;
  unsigned int extra_sectors = 0;
  int temp_finish_sector = 0;
  unsigned int stop_sector = 0;
  int i = 0;
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
  }

  if(args->target_cyl < numcyls) { 
    
    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);

    if(extra_angle > 0.0) {  
      
      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);
      
      temp_bandwidth = ((float) free_count[args->devno][args->target_cyl]/
			(float) sectorspercyl[args->target_cyl]); 
      if (temp_bandwidth >= max_bandwidth) {
      max_bandwidth = temp_bandwidth;
      args->outputflags |= SCHED_OUT_VALID;
      args->best_cyl = args->target_cyl;
      args->surface = i;
      args->new_angle = 0;
      
      args->start_sector = stop_sector;
      args->finish_sector = temp_finish_sector;
      }
    }
  }
  return(0);
}

int make_selection_GREEDY_BW_SOURCE(alg_args_t *args){
  /* static vars */
  static float max_bandwidth;
  /* end static vars */

  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  float temp_bandwidth = 0.0;
  unsigned int extra_sectors = 0;
  int temp_finish_sector = 0.0;
  unsigned int stop_sector = 0;
  int i;
  int temp_count=0;
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
  }

  if(args->target_cyl < numcyls && (max_bandwidth == 0.0 ||
				    (free_count[args->devno][args->target_cyl]
				     >= sectorspercyl[args->target_cyl] ||
				     sector_angle[args->target_cyl]
				     [free_count[args->devno]
				     [args->target_cyl]] >= 
				     max_bandwidth))){

    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);

    if(extra_angle > 0.0 && (max_bandwidth == 0.0 || extra_angle
       >= max_bandwidth)){
      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);

      for(i=0;i<numsurfaces;i++){

	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     NULL,
		     NULL,
		     NULL);
	
	temp_bandwidth = sector_angle[args->target_cyl][temp_count];

	if(temp_bandwidth > max_bandwidth || 
	   ((temp_bandwidth == max_bandwidth) && (temp_count != 0) && 
	    (abs(args->target_cyl-args->source_cyl) < 
	     abs(args->best_cyl-args->source_cyl)))){
	  max_bandwidth = temp_bandwidth;
	  args->outputflags |= SCHED_OUT_VALID;
	  args->best_cyl = args->target_cyl;
	  args->surface = i;
	  args->new_angle = 0;	  
	  args->start_sector = stop_sector;
	  args->finish_sector = temp_finish_sector;
	}
      }
    }
  }
  return(0);
}

#define MAX_CYL_DIST 150

int make_selection_GREEDY_BW_SOURCE_DIST(alg_args_t *args){
  /* static vars */
  static float max_bandwidth;
  /* end static vars */

  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  float temp_bandwidth = 0.0;
  float temp_latency = 0.0;
  unsigned int extra_sectors = 0;
  int temp_finish_sector = 0;
  unsigned int stop_sector = 0;
  int i;
  int temp_count=0;
  int last_found=0;
  int last_count=0;
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
  }
  
  if(abs(args->target_cyl - args->source_cyl) > MAX_CYL_DIST && 
     abs(args->target_cyl - args->dest_cyl) > MAX_CYL_DIST && 
     args->outputflags & SCHED_OUT_VALID){
    return(1);
  }

  if(args->target_cyl < numcyls && (max_bandwidth == 0.0 ||
  (free_count[args->devno][args->target_cyl]
				     >= sectorspercyl[args->target_cyl] ||
				     sector_angle[args->target_cyl]
				     [free_count[args->devno]
				     [args->target_cyl]] >= 
				     max_bandwidth))){
    
    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);
    
    if(extra_angle > 0.0 && (max_bandwidth == 0.0 || extra_angle
			     >= max_bandwidth)){
      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);
      
      for(i=0;i<numsurfaces;i++){
	
	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     NULL,
		     &last_found,
		     &last_count);
	
	temp_bandwidth = sector_angle[args->target_cyl][temp_count];
	
	temp_latency = sector_angle[args->target_cyl][last_count];
	
	if(temp_count != 0){
	  if(temp_bandwidth > max_bandwidth || 
	     ((temp_bandwidth == max_bandwidth) && 
	      (abs(args->target_cyl-args->source_cyl) < 
	       abs(args->best_cyl-args->source_cyl)))){
	    max_bandwidth = temp_bandwidth;
	    args->outputflags |= SCHED_OUT_VALID;
	    args->best_cyl = args->target_cyl;
	    args->surface = i;
	    args->new_angle = temp_latency;
	    
	    args->start_sector = stop_sector;
	    args->finish_sector = last_found;
	  }
	}
      }
    }
  }
  return(0);
}

int make_selection_GREEDY_BW_TIME_DIST(alg_args_t *args){
  /* static vars */
  static float max_bandwidth;
  static float max_latency;
  /* end static vars */

  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  unsigned int extra_sectors = 0;
  int temp_finish_sector = 0;
  float temp_latency = 0.0;
  float temp_bandwidth = 0.0;
  unsigned int stop_sector = 0;

  int i;
  int temp_count=0;
  int last_found=0;
  int last_count=0;
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
    max_latency = 0.0;
  }
  
  if(abs(args->target_cyl - args->source_cyl) > MAX_CYL_DIST && 
     abs(args->target_cyl - args->dest_cyl) > MAX_CYL_DIST && 
     args->outputflags & SCHED_OUT_VALID){
    return(1);
  }

  if(args->target_cyl < numcyls && (max_bandwidth == 0.0 ||
				    (free_count[args->devno][args->target_cyl]
				     >= sectorspercyl[args->target_cyl] ||
				     sector_angle[args->target_cyl]
				     [free_count[args->devno]
				     [args->target_cyl]] >= 
				     max_bandwidth))){

    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);

    if(extra_angle > 0.0 && (max_bandwidth == 0.0 || extra_angle
			     >= max_bandwidth)){
      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);

      for(i=0;i<numsurfaces;i++){

	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     NULL,
		     &last_found,
		     &last_count);
	
	temp_bandwidth = sector_angle[args->target_cyl][temp_count];
	
	temp_latency = sector_angle[args->target_cyl][last_count];

	if(temp_count != 0 && (
			       temp_bandwidth*FB_weight > 
			       max_bandwidth ||
			       (!(max_bandwidth*FB_weight > 
				  temp_bandwidth) &&
				temp_latency > max_latency))){
	  max_bandwidth = temp_bandwidth;
	  max_latency = temp_latency;
	  args->outputflags |= SCHED_OUT_VALID;
	  args->best_cyl = args->target_cyl;
	  args->surface = i;
	  args->new_angle = temp_latency;
	  
	  args->start_sector = stop_sector;
	  args->finish_sector = last_found;
	}
      }
    }
  }
  return(0);
}

int make_selection_GREEDY_BW_TRACK(alg_args_t *args){
  /* static vars */
  static float max_bandwidth;
  static float max_latency;
  static int max_track;
  /* end static vars */

  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  unsigned int extra_sectors = 0;
  int temp_finish_sector = 0;
  float temp_latency = 0.0;
  float temp_bandwidth = 0.0;
  unsigned int stop_sector = 0;

  int i;
  int temp_count=0;
  int temp_track=0;
  int last_found=0;
  int last_count=0;
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
    max_latency = 0.0;
    max_track = 0;
  }
  
  /*  if(abs(args->target_cyl - args->source_cyl) > MAX_CYL_DIST && 
      abs(args->target_cyl - args->dest_cyl) > MAX_CYL_DIST && 
      args->outputflags & SCHED_OUT_VALID){
      return(1);
    }*/

  if(args->target_cyl < numcyls && (max_bandwidth == 0.0 ||
				    sector_angle[args->target_cyl]
				    [free_count[args->devno]
				    [args->target_cyl]] >=
				    max_bandwidth*FB_weight)){

    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);

    if(extra_angle > 0.0 && extra_angle >= max_bandwidth*FB_weight){

      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);

      for(i=0;i<numsurfaces;i++){

	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     NULL,
		     &last_found,
		     &last_count);
	
	temp_bandwidth = sector_angle[args->target_cyl][temp_count];
			  
	temp_latency = sector_angle[args->target_cyl][last_count];

	if((free_count_track[args->devno][args->target_cyl][i] - 
	    temp_count) == 0){
	  temp_track = 1;
	}else{
	  temp_track = 0;
	}

	if((temp_bandwidth > max_bandwidth && temp_track == max_track) ||
	   (max_bandwidth < temp_bandwidth*FB_weight && temp_track == 0 &&
	    max_track == 1) ||
	   (max_bandwidth*FB_weight < temp_bandwidth && temp_track == 1 &&
	    max_track == 0)){
	  max_bandwidth = temp_bandwidth;
	  max_latency = temp_latency;
	  max_track = temp_track;
	  args->outputflags |= SCHED_OUT_VALID;
	  args->best_cyl = args->target_cyl;
	  args->surface = i;
	  args->new_angle = temp_latency;
	  
	  args->start_sector = stop_sector;
	  args->finish_sector = last_found;
	}
      }
    }
  }
  return(0);
}

int make_selection_GREEDY_BW_TRACK_DIST(alg_args_t *args){
  /* static vars */
  static float max_bandwidth;
  static float max_latency;
  static int max_track;
  /* end static vars */

  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  unsigned int extra_sectors = 0;
  int temp_finish_sector = 0;
  float temp_latency = 0.0;
  float temp_bandwidth = 0.0;
  unsigned int stop_sector = 0;

  int i;
  int temp_count=0;
  int temp_track=0;
  int last_found=0;
  int last_count=0;
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
    max_latency = 0.0;
    max_track = 0;
  }
  
  if(abs(args->target_cyl - args->source_cyl) > MAX_CYL_DIST && 
     abs(args->target_cyl - args->dest_cyl) > MAX_CYL_DIST && 
     args->outputflags & SCHED_OUT_VALID){
    return(1);
  }

  if(args->target_cyl < numcyls && (max_bandwidth == 0.0 ||
				    sector_angle[args->target_cyl]
				    [free_count[args->devno]
				    [args->target_cyl]] >=
				    max_bandwidth*FB_weight)){

    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);

    if(extra_angle > 0.0 && extra_angle >= max_bandwidth*FB_weight){

      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);

      for(i=0;i<numsurfaces;i++){

	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     NULL,
		     &last_found,
		     &last_count);
	
	temp_bandwidth = sector_angle[args->target_cyl][temp_count];
			  
	temp_latency = sector_angle[args->target_cyl][last_count];

	if(free_count_track[args->devno][args->target_cyl][i] - 
	   temp_count == 0){
	  temp_track = 1;
	}else{
	  temp_track = 0;
	}

	if((temp_bandwidth > max_bandwidth && temp_track == max_track) ||
	   (max_bandwidth < temp_bandwidth*FB_weight && temp_track == 0 &&
	    max_track == 1) ||
	    (max_bandwidth*FB_weight < temp_bandwidth && temp_track == 1 &&
	     max_track == 0)){
	  max_bandwidth = temp_bandwidth;
	  max_latency = temp_latency;
	  max_track = temp_track;
	  args->outputflags |= SCHED_OUT_VALID;
	  args->best_cyl = args->target_cyl;
	  args->surface = i;
	  args->new_angle = temp_latency;	  
	  args->start_sector = stop_sector;
	  args->finish_sector = last_found;
	}
      }
    }
  }
  return(0);
}

int make_selection_GREEDY_BW_MAX_DEST(alg_args_t *args){
  /* static vars */
  static int max_bandwidth;
  /* end static vars */

  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  float temp_bandwidth = 0.0;
  unsigned int extra_sectors = 0;
  int temp_finish_sector = 0.0;
  unsigned int stop_sector = 0;
  int i;
  int temp_count=0;
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
  }
  
  if(args->target_cyl < numcyls && (max_bandwidth == 0.0 ||
				    (free_count[args->devno][args->target_cyl]
				     >= sectorspercyl[args->target_cyl] ||
				     sector_angle[args->target_cyl]
				     [free_count[args->devno]
				     [args->target_cyl]] >= 
				     max_bandwidth))){

    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);
    
    if(extra_angle > 0.0 && (max_bandwidth == 0.0 || extra_angle
			     >= max_bandwidth)){
      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);
      
      for(i=0;i<numsurfaces;i++){
	
	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     NULL,
		     NULL,
		     NULL);
	
	temp_bandwidth = sector_angle[args->target_cyl][temp_count];

	if(temp_count != 0){
	  if(temp_bandwidth > max_bandwidth || 
	     ((temp_bandwidth == max_bandwidth) && 
	      (abs(args->target_cyl-args->dest_cyl) > abs(args->best_cyl-args->dest_cyl)))){
	    max_bandwidth = temp_bandwidth;
	    args->outputflags |= SCHED_OUT_VALID;
	    args->best_cyl = args->target_cyl;
	    args->surface = i;
	    args->new_angle = 0;
	    
	    args->start_sector = stop_sector;
	    args->finish_sector = temp_finish_sector;
	  }
	}
      }
    }
  }
  return(0);
}

int make_selection_GREEDY_BW_SOURCE_WEIGHT(alg_args_t *args){
  /* static vars */
  static int max_bandwidth;
  static int printed = 0;
  /* end static vars */

  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  float temp_bandwidth = 0.0;
  unsigned int extra_sectors = 0;
  int temp_finish_sector = 0;
  unsigned int stop_sector = 0;
  int i,j;
  int temp_count=0;

  if (!printed) {
    fprintf(stderr,"The weight for algorithm %d is %f\n",
	    GREEDY_BW_SOURCE_WEIGHT_ALG,FB_weight);
    printed = 1;
  }
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
  }

  if(args->target_cyl < numcyls && (max_bandwidth == 0.0 || 
			      (float)free_count[args->devno][args->target_cyl]/
			      (float)sectorspercyl[args->target_cyl] >= 
			      max_bandwidth)){

    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);

    if(extra_angle > 0.0 && (max_bandwidth == 0.0 || (extra_angle/oneRotation)
			     >= max_bandwidth)){

      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);

      for(i=0;i<numsurfaces;i++){
	temp_count = 0;
	for(j=0;j<extra_sectors;j++){
	  temp_count += BITMAP_CHECK(args->devno,args->target_cyl,i,(stop_sector + j) % 
				     sectorspercyl[args->target_cyl]);
	}
	temp_bandwidth = (float)((float)(temp_count)/
				 (float)(sectorspercyl[args->target_cyl]));
	if(temp_count != 0){
	  if(temp_bandwidth > max_bandwidth || 
	     ((temp_bandwidth == max_bandwidth) && 
	      (abs(args->target_cyl-args->source_cyl) < 
	       abs(args->best_cyl-args->source_cyl)))){
	    max_bandwidth = temp_bandwidth;
	    args->outputflags |= SCHED_OUT_VALID;
	    args->best_cyl = args->target_cyl;
	    args->surface = i;
	    args->new_angle = 0;
	    
	    args->start_sector = stop_sector;
	    args->finish_sector = temp_finish_sector;
	  }
	}
      }
    }
  }
  return(0);
}

int make_selection_LEAST_LATENCY(alg_args_t *args){
  /* static vars */
  static double latency;
  static float max_bandwidth;
  /* end static vars */

  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  unsigned int extra_sectors = 0;
  int temp_finish_sector = 0;
  unsigned int stop_sector = 0;
  double temp_latency=0.0;
  float temp_bandwidth=0.0;
  int i;
  int temp_count=0;
  int last_found=0;
  int last_count=0;
  
  if(args->inputflags & SCHED_INIT){
    latency = oneRotation * oneRotation;
    max_bandwidth = 0.0;
  }

  if(args->target_cyl < numcyls){

    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);

    if(extra_angle > 0.0){

      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);

      for(i=0;i<numsurfaces;i++){

	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     NULL,
		     &last_found,
		     &last_count);
	
	temp_latency = sector_angle[args->target_cyl][extra_sectors - 
						     temp_count];
	temp_bandwidth = sector_angle[args->target_cyl][temp_count];

	if(temp_count > 0 && 
	   (latency > temp_latency || 
	    (abs(latency - temp_latency) <= 0.01 &&
	     (temp_bandwidth > max_bandwidth)))){
	  latency = temp_latency;
	  max_bandwidth = temp_bandwidth;
	  args->outputflags |= SCHED_OUT_VALID;
	  args->best_cyl = args->target_cyl;
	  args->surface = i;
	  args->new_angle = sector_angle[args->target_cyl][last_count];	  
	  args->start_sector = stop_sector;
	  args->finish_sector = last_found;
	}
      }
    }
  }
  return(0);
}

int make_selection_GREEDY_BW_PROB_COIN(alg_args_t *args){
  /* static vars */
  static float max_bandwidth;
  static float max_weight;
  /* end static vars */

  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  float temp_bandwidth = 0.0;
  unsigned int extra_sectors = 0;

  int temp_finish_sector = 0;
  unsigned int stop_sector = 0;
  int i;
  int temp_count=0;
  float target_weight=0.0;
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
    max_weight = 0.0;
  }

  if(args->target_cyl < numcyls){
    target_weight = probpercyl[args->target_cyl];

    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);

    if(extra_angle > 0.0){

      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);

      for(i=0;i<numsurfaces;i++){

	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     NULL,
		     NULL,
		     NULL);
	
	temp_bandwidth = sector_angle[args->target_cyl][temp_count];

	
	if((max_bandwidth*FB_weight < temp_bandwidth && 
	    max_bandwidth*max_weight < temp_bandwidth * target_weight) ||
	   max_bandwidth < temp_bandwidth*FB_weight){
	    max_bandwidth = temp_bandwidth;
	    max_weight = target_weight;
	    args->outputflags |= SCHED_OUT_VALID;
	    args->best_cyl = args->target_cyl;
	    args->surface = i;
	    args->new_angle = 0;
	    
	    args->start_sector = stop_sector;
	    args->finish_sector = temp_finish_sector;
	}
      }
    }
  }
  return(0);
}

int make_selection_GREEDY_BLOCK_COIN(alg_args_t *args){
  /* static vars */
  static int max_count;
  /* end static vars */

  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  unsigned int extra_sectors = 0;
  int temp_finish_sector = 0;
  unsigned int stop_sector = 0;
  int i;
  int temp_count=0;
  
  if(args->inputflags & SCHED_INIT){
    max_count = 0;
  }

  if(args->target_cyl < numcyls && 
     free_count[args->devno][args->target_cyl] >= max_count){

    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);

    if(extra_angle > 0.0 && (max_count == 0 || extra_angle
			     >= sector_angle[args->best_cyl][max_count])){
      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);

      for(i=0;i<numsurfaces;i++){

	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     NULL,
		     NULL,
		     NULL);
	
	if(temp_count != 0){
	  if(temp_count > max_count || 
	     ((temp_count == max_count) && 
	      ((rand() & 0x1) == 1))){
	    max_count = temp_count;
	    args->outputflags |= SCHED_OUT_VALID;
	    args->best_cyl = args->target_cyl;
	    args->surface = i;
	    args->new_angle = 0;
	    
	    args->start_sector = stop_sector;
	    args->finish_sector = temp_finish_sector;
	  }
	}
      }
    }
  }
  return(0);
}

int make_selection_GREEDY_BLOCK(alg_args_t *args){
  /* static vars */
  static int max_count;
  /* end static vars */

  float extra_angle = 0.0;
  float source_seekangle = 0.0;
  float dest_seekangle = 0.0;
  unsigned int extra_sectors = 0;
  int temp_finish_sector = 0;
  unsigned int stop_sector = 0;
  int i;
  int temp_count=0;
  
  if(args->inputflags & SCHED_INIT){
    max_count = 0;
  }
  
  if(args->target_cyl < numcyls && 
     free_count[args->devno][args->target_cyl] >= max_count){
    
    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);
    
    if(extra_angle > 0.0 && (max_count == 0 ||
			     extra_angle >= 
			     sector_angle[args->best_cyl][max_count])){

      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);
      
      for(i=0;i<numsurfaces;i++){

	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     NULL,
		     NULL,
		     NULL);
	
	if(temp_count > max_count){
	  /*	  if(args->target_cyl != args->source_cyl && args->target_cyl != args->dest_cyl){
		  fprintf(stderr,"stop %d finish %d extra %d count %d\n",stop_sector,temp_finish_sector,extra_sectors,temp_count);
		  fprintf(stderr,"source %d dest %d target %d time %f\n",args->source_cyl,args->dest_cyl,args->target_cyl,args->rotate_angle);
		  }*/
	  max_count = temp_count;
	  args->outputflags |= SCHED_OUT_VALID;
	  args->best_cyl = args->target_cyl;
	  args->surface = i;
	  args->new_angle = 0;
	  
	  args->start_sector = stop_sector;
	  args->finish_sector = temp_finish_sector;
	}
      }
    }
  }
  return(0);
}

int make_selection_SSTF(alg_args_t *args){
#if 0
  int i,j;
  float source_seekangle;
  float dest_seekangle;
  float extra_angle;
  int temp_finish_sector = 0.0;
  unsigned int extra_sectors;
  unsigned int temp_count;
  unsigned int stop_sector;
  int randval;
  int last_found = 0;

  if(args->inputflags & SCHED_INIT){
  }

  if(args->target_cyl < numcyls){

    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);

    if(extra_angle > 0.0){

      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);
      
      randval = args->source_surface;
      for(i=0;i<numsurfaces;i++){
	temp_count = 0;
	for(j=0;j<extra_sectors;j++){
	  temp_count += BITMAP_CHECK(args->devno,args->target_cyl,i,(stop_sector + j) % 
				     sectorspercyl[args->target_cyl]);
	  if(temp_count > 0 && 
	     !BITMAP_CHECK(args->devno,args->target_cyl,i,(stop_sector + j) % 
			  sectorspercyl[args->target_cyl])){
	    last_found = j + 1 + stop_sector;
	    args->surface = (i+randval) % numsurfaces;
	    args->start_sector = stop_sector;
	    args->new_angle = args->rotate_angle - source_seekangle 
	      - ((float)j/(float)sectorspercyl[args->target_cyl] * oneRotation);
	    
	    args->finish_sector = last_found % sectorspercyl[args->target_cyl];
	    args->best_cyl = args->target_cyl;
	    args->outputflags |= SCHED_OUT_VALID;
	    return(1);
	  }
	}
	if(temp_count > 0){
	  last_found = j + 1 + stop_sector;
	  args->surface = (i+randval) % numsurfaces;
	  args->start_sector = stop_sector;
	  args->new_angle = args->rotate_angle - source_seekangle 
	    - ((float)j/(float)sectorspercyl[args->target_cyl] * oneRotation);
	  
	  args->finish_sector = last_found % sectorspercyl[args->target_cyl];
	  args->best_cyl = args->target_cyl;
	  args->outputflags |= SCHED_OUT_VALID;
	  return(1);	  
	}
      }
    }else{
    }
  }else{
  }
#endif
  return(0);
}

#define INIT_SEPTF_POS 999999.0
#define MAX_FAIL 50

int make_selection_SEPTF(alg_args_t *args){
  
  /* static vars */
  static float max_bandwidth;
  static float best_pos;
  static int fail_count;
  /* end static vars */

  int i;
  float source_seekangle;
  float dest_seekangle;
  float extra_angle;
  float temp_bandwidth;
  float temp_latency;
  float temp_pos = INIT_SEPTF_POS;
  int temp_finish_sector = 0.0;
  unsigned int extra_sectors;
  unsigned int temp_count;
  unsigned int last_count;
  unsigned int last_found;
  unsigned int before_found;
  unsigned int stop_sector;
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
    fail_count = 0;
    best_pos = INIT_SEPTF_POS;
  }

  if(args->target_cyl < numcyls){

    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);

    if((dest_seekangle + source_seekangle - 
	seek_angle[abs(args->source_cyl-args->dest_cyl)]) > best_pos){
      fail_count++;
      if(fail_count > MAX_FAIL){
	return(1);
      }
    }

    if(extra_angle > 0.0){

      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);
      
      for(i=0;i<numsurfaces;i++){
	temp_count = 0;
	
	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     &before_found,
		     &last_found,
		     &last_count);
	
	if(temp_count > 0){
	  temp_latency = sector_angle[args->target_cyl][last_count];
	  
	  temp_bandwidth = sector_angle[args->target_cyl][temp_count];
	  
	  temp_pos = ((args->rotate_angle - extra_angle) + 
		      sector_angle[args->target_cyl]
		      [extra_sectors - temp_count - last_count]);
	}
	
	if(temp_count > 0 && (temp_pos < best_pos || (temp_pos == best_pos &&
						      temp_bandwidth >
						      max_bandwidth))){
	  fail_count = 0;
	  args->outputflags |= SCHED_OUT_VALID;
	  best_pos = temp_pos;
	  max_bandwidth = temp_bandwidth;
	  args->best_cyl = args->target_cyl;
	  args->surface = i;
	  args->start_sector = stop_sector;
	  args->new_angle = temp_latency; 
	  args->finish_sector = last_found;
	}
      }
    }else{
    }
  }else{
  }
  return(0);
}

int make_selection_SEPTF_FRAGMENT(alg_args_t *args){
  
  /* static vars */
  static float max_bandwidth;
  static float best_pos;
  static int fail_count;
  /* end static vars */

  int i,j;
  float source_seekangle;
  float dest_seekangle;
  float extra_angle;
  float temp_bandwidth;
  float temp_latency;
  float temp_pos = INIT_SEPTF_POS;
  unsigned int temp_finish_sector = 0;
  unsigned int extra_sectors;
  unsigned int temp_count;
  unsigned int last_count;
  unsigned int last_found;
  unsigned int before_found;
  unsigned int stop_sector;
  int new_finish_sector = 0;
  int new_stop_sector;
  int new_extra_sectors;
  int done;
  int stop_fragment_count=0;
  int finish_fragment_count=0;
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
    fail_count = 0;
    best_pos = INIT_SEPTF_POS;
  }

  if(args->target_cyl < numcyls){

    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);

    if((dest_seekangle + source_seekangle - 
	seek_angle[abs(args->source_cyl-args->dest_cyl)]) > best_pos){
      fail_count++;
      if(fail_count > MAX_FAIL){
	return(1);
      }
    }

    if(extra_angle > 0.0){

      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);
      
      for(i=0;i<numsurfaces;i++){
	temp_count = 0;

	new_extra_sectors = extra_sectors;
	new_stop_sector = stop_sector;
	new_finish_sector = temp_finish_sector;

	if(extra_sectors != sectorspercyl[i]){
	  new_stop_sector = stop_sector - min_fragment_size;
	  while(new_stop_sector < 0){
	    new_stop_sector += sectorspercyl[i];
	  }

	  done = 0;
	  stop_fragment_count = 0;

	  if(new_stop_sector > stop_sector){
	    for(j=new_stop_sector;j<sectorspercyl[i] && done != 1;j++){
	      if(BITMAP_CHECK(args->devno,args->target_cyl,i,j)){
		done = 1;
	      }else{
		stop_fragment_count++;
	      }
	    }
	    for(j=0;j<stop_sector && done != 1;j++){
	      if(BITMAP_CHECK(args->devno,args->target_cyl,i,j)){
		done = 1;
	      }else{
		stop_fragment_count++;
	      }
	    }
	  }else{
	    for(j=new_stop_sector;j<stop_sector && done != 1;j++){
	      if(BITMAP_CHECK(args->devno,args->target_cyl,i,j)){
		done = 1;
	      }else{
		stop_fragment_count++;
	      }
	    }
	  }

	  if(stop_fragment_count != min_fragment_size){
	    new_stop_sector = stop_sector + stop_fragment_count;
	    while(new_stop_sector >= sectorspercyl[i]){
	      new_stop_sector -= sectorspercyl[i];
	    }
	    new_extra_sectors -= stop_fragment_count;
	  }else{
	    new_stop_sector = stop_sector;
	  }
	    
	  done = 0;
	  finish_fragment_count = 0;

	  new_finish_sector = temp_finish_sector + min_fragment_size;
	  while(new_finish_sector >= sectorspercyl[i]){
	    new_finish_sector -= sectorspercyl[i];
	  }

	  if(new_finish_sector < temp_finish_sector){
	    for(j=new_finish_sector;j>0 && done != 1;j--){
	      if(BITMAP_CHECK(args->devno,args->target_cyl,i,(j-1))){
		done = 1;
	      }else{
		finish_fragment_count++;
	      }
	    }
	    for(j=sectorspercyl[i];j>temp_finish_sector && done != 1;j--){
	      if(BITMAP_CHECK(args->devno,args->target_cyl,i,(j-1))){
		done = 1;
	      }else{
		finish_fragment_count++;
	      }
	    }
	  }else{
	    for(j=new_finish_sector;j>temp_finish_sector && done != 1;j--){
	      if(BITMAP_CHECK(args->devno,args->target_cyl,i,(j-1))){
		done = 1;
	      }else{
		finish_fragment_count++;
	      }
	    }
	  }
	  if(finish_fragment_count != min_fragment_size){
	    new_finish_sector = temp_finish_sector - finish_fragment_count;
	    while(new_finish_sector < 0){
	      new_finish_sector += sectorspercyl[i];
	    }
	    new_extra_sectors -= finish_fragment_count;
	  }else{
	    new_finish_sector = temp_finish_sector;
	  }
	  if(new_extra_sectors < 0){
	    new_extra_sectors = 0;
	  }
	}
	
	/*	if(new_stop_sector != stop_sector || (new_finish_sector != 
		temp_finish_sector)){
		fprintf(stderr,"stop %d new_stop %d finish %d new_finish %d extra %d new_extra %d\n",stop_sector,new_stop_sector,temp_finish_sector,new_finish_sector,extra_sectors,new_extra_sectors);
		}else{
		fprintf(stderr,"BLAH\n");
		}*/

	count_blocks(new_stop_sector,
		     new_finish_sector,
		     new_extra_sectors,
		     i,
		     args,
		     &temp_count,
		     &before_found,
		     &last_found,
		     &last_count);
	
	if(temp_count > 0){
	  temp_latency = sector_angle[args->target_cyl][last_count + 
						       finish_fragment_count];
	  
	  temp_bandwidth = sector_angle[args->target_cyl][temp_count];
	  
	  temp_pos = ((args->rotate_angle - extra_angle) + 
		      sector_angle[args->target_cyl]
		      [stop_fragment_count + new_extra_sectors - 
		      temp_count - last_count]);
	}
	
	if(temp_count > 0 && (temp_pos < best_pos || (temp_pos == best_pos &&
						      temp_bandwidth >
						      max_bandwidth))){
	  fail_count = 0;
	  args->outputflags |= SCHED_OUT_VALID;
	  best_pos = temp_pos;
	  max_bandwidth = temp_bandwidth;
	  args->best_cyl = args->target_cyl;
	  args->surface = i;
	  args->start_sector = new_stop_sector;
	  args->new_angle = temp_latency; 
	  args->finish_sector = last_found;
	}
      }
    }else{
    }
  }else{
  }
  return(0);
}

int make_selection_SEPTF_TRACK(alg_args_t *args){
  
  /* static vars */
  static float max_bandwidth;
  static float best_pos;
  static int fail_count;
  static int max_track;
  /* end static vars */

  int i;
  float source_seekangle;
  float dest_seekangle;
  float extra_angle;
  float temp_bandwidth;
  float temp_latency;
  float temp_pos = INIT_SEPTF_POS;
  int temp_finish_sector = 0.0;
  unsigned int extra_sectors;
  unsigned int temp_count;
  unsigned int last_count;
  unsigned int last_found;
  unsigned int before_found;
  unsigned int stop_sector;
  int temp_track=0;
  
  if(args->inputflags & SCHED_INIT){
    max_bandwidth = 0.0;
    fail_count = 0;
    best_pos = INIT_SEPTF_POS;
    max_track = 0;
  }

  if(args->target_cyl < numcyls){

    seek_angles(args,
		&source_seekangle,
		&dest_seekangle,
		&extra_angle);

    if((dest_seekangle + source_seekangle - 
	seek_angle[abs(args->source_cyl-args->dest_cyl)]) > best_pos){
      fail_count++;
      if(fail_count > MAX_FAIL){
	return(1);
      }
    }

    if(extra_angle > 0.0){

      location_info(extra_angle,
		    source_seekangle,
		    args,
		    &stop_sector,
		    &temp_finish_sector,
		    &extra_sectors);
      
      for(i=0;i<numsurfaces;i++){
	temp_count = 0;
	
	count_blocks(stop_sector,
		     temp_finish_sector,
		     extra_sectors,
		     i,
		     args,
		     &temp_count,
		     &before_found,
		     &last_found,
		     &last_count);
	
	if(temp_count > 0){
	  temp_latency = sector_angle[args->target_cyl][last_count];
	  
	  temp_bandwidth = sector_angle[args->target_cyl][temp_count];
	  
	  temp_pos = ((args->rotate_angle - extra_angle) + 
		      sector_angle[args->target_cyl]
		      [extra_sectors - temp_count - last_count]);
	  if((free_count_track[args->devno][args->target_cyl][i] - 
	      temp_count) == 0){
	    temp_track = 1;
	  }else{
	    temp_track = 0;
	  }
	}
	
	if(temp_count > 0 && (((temp_pos < best_pos || (temp_pos == best_pos &&
							temp_bandwidth >
							max_bandwidth)) &&
			       temp_track == max_track) || 
			      ((temp_pos < best_pos*FB_weight || 
				(temp_pos == best_pos*FB_weight &&
				 temp_bandwidth*FB_weight >
				 max_bandwidth)) &&
			       temp_track == 0 && max_track == 1) || 
			      ((temp_pos*FB_weight < best_pos || 
				(temp_pos*FB_weight == best_pos &&
				 temp_bandwidth >
				 max_bandwidth*FB_weight)) &&
			       temp_track == 1 && max_track == 0)
			      )){
	  fail_count = 0;
	  max_track = temp_track;
	  args->outputflags |= SCHED_OUT_VALID;
	  best_pos = temp_pos;
	  max_bandwidth = temp_bandwidth;
	  args->best_cyl = args->target_cyl;
	  args->surface = i;
	  args->start_sector = stop_sector;
	  args->new_angle = temp_latency; 
	  args->finish_sector = last_found;
	}
      }
    }else{
    }
  }else{
  }
  return(0);
}
/****************************************************************
 *                                                              * 
 * PRIMARY API for all the FreeBlock Scheduling Algorithms      *
 *                                                              *
 * Function:  Used to call specific FB algorithm giving by      *
 * setting algorithm upon start up.                             *
 *                                                              *
 * Variables:                                                   *
 *                                                              *
 * Input:                                                       *
 *                                                              *
 * rotate_angle - orignal positioning time                          *
 * devno - device number of disk                                *
 * source_cyl - source cylinder                                 *
 * source_angle - sector at source cylinder                    *
 * dest_cyl - destination cylinder                              *
 * dest_angle - sector at destination cylinder                 *
 * target_cyl - cylinder to be examined for freeblocks          *
 * inputflags - flags used to signal the algorhtms              *
 *                                                              *
 * Output:                                                      *
 *                                                              *
 * outputflags - flags used to return information               *
 * surface - best surface selected for Free Block pickup        *
 * start_angle - best angle to start pickup                     *
 * new_angle - positioning time to destination from best stop    *
 * point                                                        *
 * best_cyl - the best cyl currently found                      *
 * finish_angle - finish angle for pickup of free blocks        *
 *                                                              *
 * Return Codes:                                                *
 *                                                              *
 * 1 - scheduling pass done make selection and start next pass  *
 * if time remains.                                             *
 * 0 - done scanning selected cylinder, keep looking for best.  * 
 *                                                             */
typedef int (*funcptr)(alg_args_t *);

funcptr make_selection[] = {
  NULL,
  &make_selection_GREEDY_BW,
  &make_selection_GREEDY_BLOCK_COIN,
  &make_selection_SSTF,
  &make_selection_SEPTF,
  &make_selection_GREEDY_BW_PROB_COIN,
  &make_selection_GREEDY_BW_EDGE_COIN,
  &make_selection_GREEDY_BW_SOURCE,
  &make_selection_GREEDY_BLOCK,
  &make_selection_GREEDY_BW_SOURCE_WEIGHT,
  &make_selection_LEAST_LATENCY,
  &make_selection_GREEDY_BW_MAX_DEST,
  &make_selection_GREEDY_BW_COIN,
  &make_selection_GREEDY_BW_SOURCE_DIST,
  &make_selection_GREEDY_BW_TIME_DIST,
  &make_selection_GREEDY_UNIFORM_RATE,
  &make_selection_GREEDY_BW_TRACK,
  &make_selection_GREEDY_AREA_FIRST,
  &make_selection_GREEDY_BW_TRACK_DIST,
  &make_selection_SEPTF_TRACK,
  &make_selection_SEPTF_FRAGMENT
};

static int total_touches=0;
static int num_requests=0;

inline int update_cyl_position(alg_args_t *args,
			       int *top_cyl,
			       int *bottom_cyl,
			       int *first_cyl,
			       int *last_cyl,
			       int *bot_inc,
			       int *bot_dec,
			       int *top_inc,
			       int *top_dec,
			       int *last_dec,
			       int *first_inc){

  args->outputflags = 0;
  args->inputflags |= SCHED_INIT;
  if(args->new_angle > 0.0){
    args->rotate_angle = args->new_angle;
    args->rotate_angle -= conservative;
  }else{
    return(1);
  }
  args->source_cyl = args->best_cyl;
  args->source_angle = ((float)args->finish_sector/
			(float)sectorspercyl[args->best_cyl]);
  args->source_surface = args->surface;
  *bot_inc = *top_inc = *bot_dec = *top_dec = 0;
  if(args->best_cyl > args->dest_cyl){
    *top_cyl = args->best_cyl;
    *bottom_cyl = args->dest_cyl;
  }else{
    *top_cyl = args->dest_cyl;
    *bottom_cyl = args->best_cyl;
  }
  *last_cyl = numcyls;
  if(*last_cyl == *top_cyl){
    *last_dec = -1;
  }else{
    *last_dec = 0;
  }
  *first_cyl = 0;
  if(*first_cyl == *bottom_cyl){
    *first_inc = -1;
  }else{
    *first_inc = 0;
  }
  if(*top_cyl == *bottom_cyl){
    *top_dec = -1;
    *bot_inc = -1;
  }else{
    if((*top_cyl - *bottom_cyl) > 1){
      *top_dec = 1;
      *bot_inc = 1;   
    } 
  }
  if(onlyfourpass){
    *first_inc = -1;
    *last_dec = -1;
  }
  return(0);
}

void take_free_blocks(unsigned short devno,
		      unsigned short source_cyl,
		      float source_angle,
		      unsigned short dest_cyl,
		      float dest_angle,
		      float orig_seektime,
		      float orig_rotate, 
		      unsigned short source_surface, 
		      unsigned short dest_surface, 
		      float last_servtime){

  int free_before = free_blocks[devno];
  int free_after;
  /*  unsigned short dest_sector=(int)floor(dest_angle*
					(float)sectorspercyl[source_cyl]);
  unsigned short source_sector = (int)floor(source_angle*
  (float)sectorspercyl[source_cyl]);*/
  //  unsigned short source_sector = min_sectors[temp_source_sector];
#ifndef HAVE_X86
  struct timeval start_t;
  struct timeval current_t;
#endif
  long curroffset = 0;
  int top_cyl = 0;
  int bottom_cyl = 0;
  int last_cyl = 0;
  int last_dec=0;
  int first_cyl=0;
  int first_inc=0;
  int top_inc=0;
  int top_dec=0;
  int bot_inc=0;
  int bot_dec=0;
  long long used_servicetime = 0;
  int requests=0;

  /*from old takeFB*/
#ifdef HAVE_X86
  long long start_mine;
  long long end_mine;
#endif
  struct timeval start_time;
  struct timeval finish_time;

  alg_args_t args;

  if(algorithm < MIN_ALG  || algorithm > MAX_ALG){
    fprintf(stderr,"ERROR:  Invalid scheduling algorithm %d\n",algorithm);
    abort();  
  }

  args.rotate_angle = 0.0;
  args.devno = devno;
  args.source_cyl = 0;
  args.source_angle = 0.0;
  args.dest_cyl = dest_cyl;
  args.dest_angle = dest_angle;
  args.source_surface = 0;
  args.dest_surface = dest_surface;
  args.target_cyl = 0;
  args.inputflags = 0;
  args.outputflags = 0;
  args.surface = source_surface;
  args.start_sector = 0;
  args.new_angle = (orig_rotate/oneRotation);
  args.best_cyl = source_cyl;
  args.finish_sector = (int)rint(source_angle *
				 sectorspercyl[args.source_cyl]);

  total_seeks++;
  cyl_dist_count+=abs(source_cyl - dest_cyl);

  gettimeofday(&start_time,NULL);
  stat_update(total_latency_stat, orig_rotate);
  stat_update(total_seek_stat, orig_seektime);
  stat_update(total_latency_stat_inst, orig_rotate);
  stat_update(latency_stat[(int)floor(now/(float)PERIOD)], orig_rotate);
  stat_update(seek_dist_stat[(int)floor(now/(float)PERIOD)], abs(source_cyl-dest_cyl));

  update_cyl_position(&args,
		      &top_cyl,
		      &bottom_cyl,
		      &first_cyl,
		      &last_cyl,
		      &bot_inc,
		      &bot_dec,
		      &top_inc,
		      &top_dec,
		      &last_dec,
		      &first_inc);

#ifdef HAVE_X86 
  start_mine = get_time();
  curroffset = 0;
#else
  gettimeofday(&start_t,NULL);
  gettimeofday(&current_t,NULL);
  curroffset = current_t.tv_usec - start_t.tv_usec + 1000000 * (current_t.tv_sec - start_t.tv_sec);
#endif
  if(servtime){
    used_servicetime = ((long long)servicetime) * time_scale;
  }else{
    used_servicetime = ((long long)last_servtime) * time_scale;
  }

  while(curroffset <= used_servicetime){
    if(top_inc != -1){
      args.target_cyl = top_cyl + top_inc;
      if(seek_min[abs(args.target_cyl - top_cyl)] > args.rotate_angle){
	top_inc = -1;
      }else{
	if(free_count[devno][args.target_cyl] != 0){
	  alg_count++;
	  if(make_selection[algorithm](&args) == 1){
	    pickup_free_blocks(args.best_cyl, 
			       args.surface,
			       args.start_sector,
			       args.finish_sector,
			       args.source_cyl,
			       args.dest_cyl,
			       args.devno);
	    requests++;
	    if(update_cyl_position(&args,
				   &top_cyl,
				   &bottom_cyl,
				   &first_cyl,
				   &last_cyl,
				   &bot_inc,
				   &bot_dec,
				   &top_inc,
				   &top_dec,
				   &last_dec,
				   &first_inc)){
	      break;
	    }
	    goto next;
	  }
	  if(args.inputflags & SCHED_INIT){
	    args.inputflags -= SCHED_INIT;
	  }
	}
	top_inc++;
	if(onlyfourpass){
	  if((top_cyl + top_inc) == numcyls){
	    top_inc = -1;
	  }
	}else{
	  if((top_cyl + top_inc) == (last_cyl - last_dec)){
	    top_inc = -1;
	  }
	}
      }
    }
    if(bot_dec != -1){
      args.target_cyl = bottom_cyl - bot_dec;
      if(seek_min[abs(args.target_cyl - bottom_cyl)] > args.rotate_angle){
	bot_dec = -1;
      }else{
	if(free_count[devno][args.target_cyl] != 0){
	  alg_count++;
	  if(make_selection[algorithm](&args) == 1){
	    pickup_free_blocks(args.best_cyl, 
			       args.surface,
			       args.start_sector,
			       args.finish_sector,
			       args.source_cyl,
			       args.dest_cyl,
			       args.devno);
	    requests++;
	    if(update_cyl_position(&args,
				   &top_cyl,
				   &bottom_cyl,
				   &first_cyl,
				   &last_cyl,
				   &bot_inc,
				   &bot_dec,
				   &top_inc,
				   &top_dec,
				   &last_dec,
				   &first_inc)){
	      break;
	    }
	    goto next;
	  }
	  if(args.inputflags & SCHED_INIT){
	    args.inputflags -= SCHED_INIT;
	  }
	}
	bot_dec++;
	if(onlyfourpass){
	  if((bottom_cyl - bot_dec) == -1){
	    bot_dec = -1;
	  }
	}else{
	  if((bottom_cyl - bot_dec) == (first_cyl + first_inc)){
	    bot_dec = -1;
	  }
	}
      }
    }
    if(top_dec != -1){
      args.target_cyl = top_cyl - top_dec;
      if(free_count[devno][args.target_cyl] != 0){
	alg_count++;
	if(make_selection[algorithm](&args) == 1){
	  pickup_free_blocks(args.best_cyl, 
			     args.surface,
			     args.start_sector,
			     args.finish_sector,
			     args.source_cyl,
			     args.dest_cyl,
			     args.devno);
	  requests++;
	  if(update_cyl_position(&args,
				 &top_cyl,
				 &bottom_cyl,
				 &first_cyl,
				 &last_cyl,
				 &bot_inc,
				 &bot_dec,
				 &top_inc,
				 &top_dec,
				 &last_dec,
				 &first_inc)){
	    break;
	  }
	  goto next;
	}
	if(args.inputflags & SCHED_INIT){
	  args.inputflags -= SCHED_INIT;
	}
      }
      if((top_cyl - top_dec) == (bottom_cyl + bot_inc)){
	top_dec = -1;
	bot_inc = -1;
      }else{
	top_dec++;
      }
    }
    if(bot_inc != -1){
      args.target_cyl = bottom_cyl + bot_inc;
      if(free_count[devno][args.target_cyl] != 0){
	alg_count++;
	if(make_selection[algorithm](&args) == 1){
	  pickup_free_blocks(args.best_cyl, 
			     args.surface,
			     args.start_sector,
			     args.finish_sector,
			     args.source_cyl,
			     args.dest_cyl,
			     args.devno);
	  requests++;
	  if(update_cyl_position(&args,
				 &top_cyl,
				 &bottom_cyl,
				 &first_cyl,
				 &last_cyl,
				 &bot_inc,
				 &bot_dec,
				 &top_inc,
				 &top_dec,
				 &last_dec,
				 &first_inc)){
	    break;
	  }
	  goto next;
	}
	if(args.inputflags & SCHED_INIT){
	  args.inputflags -= SCHED_INIT;
	}
      }
      if((top_cyl - top_dec) == (bottom_cyl + bot_inc)){
	top_dec = -1;
	bot_inc = -1;
      }else{
	bot_inc++;
      }
    }
    if(!onlyfourpass){
      if(last_dec != -1){
	args.target_cyl = last_cyl - last_dec;
	if(free_count[devno][args.target_cyl] != 0){
	  alg_count++;
	  if(make_selection[algorithm](&args) == 1){
	    pickup_free_blocks(args.best_cyl, 
			       args.surface,
			       args.start_sector,
			       args.finish_sector,
			       args.source_cyl,
			       args.dest_cyl,
			       args.devno);
	    requests++;
	    if(update_cyl_position(&args,
				   &top_cyl,
				   &bottom_cyl,
				   &first_cyl,
				   &last_cyl,
				   &bot_inc,
				   &bot_dec,
				   &top_inc,
				   &top_dec,
				   &last_dec,
				   &first_inc)){
	      break;
	    }
	    goto next;
	  }
	  if(args.inputflags & SCHED_INIT){
	    args.inputflags -= SCHED_INIT;
	  }
	}
	if((top_cyl + top_inc) == (last_cyl - last_dec)){
	  last_dec = -1;
	}
	last_dec++;
      }
      if(first_inc != -1){
	args.target_cyl = first_cyl + first_inc;
	if(free_count[devno][args.target_cyl] != 0){
	  alg_count++;
	  if(make_selection[algorithm](&args) == 1){
	    pickup_free_blocks(args.best_cyl, 
			       args.surface,
			       args.start_sector,
			       args.finish_sector,
			       args.source_cyl,
			       args.dest_cyl,
			       args.devno);
	    requests++;
	    if(update_cyl_position(&args,
				   &top_cyl,
				   &bottom_cyl,
				   &first_cyl,
				   &last_cyl,
				   &bot_inc,
				   &bot_dec,
				   &top_inc,
				   &top_dec,
				   &last_dec,
				   &first_inc)){
	      break;
	    }
	    goto next;
	  }
	  if(args.inputflags & SCHED_INIT){
	    args.inputflags -= SCHED_INIT;
	  }
	}
	if((bottom_cyl - bot_dec) == (first_cyl + first_inc)){
	  first_inc = -1;
	}
	first_inc++;
      }
    }
  next:
    if(top_inc == -1 && top_dec == -1 && bot_inc == -1 
       && bot_dec == -1 && last_dec == -1 && first_inc == -1){
      if(args.outputflags & SCHED_OUT_VALID){
	pickup_free_blocks(args.best_cyl, 
			   args.surface,
			   args.start_sector,
			   args.finish_sector,
			   args.source_cyl,
			   args.dest_cyl,
			   args.devno);
	requests++;
	if(update_cyl_position(&args,
			       &top_cyl,
			       &bottom_cyl,
			       &first_cyl,
			       &last_cyl,
			       &bot_inc,
			       &bot_dec,
			       &top_inc,
			       &top_dec,
			       &last_dec,
			       &first_inc)){
	  break;
	}
      }else{
	if(curroffset > max_time){
	  max_time = curroffset;
	  max_percent = (100.0 * (float)((float)free_blocks[devno] / (float)numblocks));
	}
	break;
      }
    }
#ifdef HAVE_X86
    end_mine = get_time();
    curroffset = end_mine - start_mine;
#else
    gettimeofday(&current_t,NULL);
    curroffset = current_t.tv_usec - start_t.tv_usec + 1000000 * (current_t.tv_sec - start_t.tv_sec);
#endif
    //    fprintf(stderr,"current %lu max %lu\n",curroffset,used_servicetime);
  }
  if(bot_inc != -1 || top_dec != -1){
    //    fprintf(stderr,"Not all found bot %d inc %d top %d dec %d\n",bottom_cyl,bot_inc,top_cyl,top_dec);
  }
  
  if(args.outputflags & SCHED_OUT_VALID){
    requests++;
    pickup_free_blocks(args.best_cyl, 
		       args.surface,
		       args.start_sector,
		       args.finish_sector,
		       args.source_cyl,
		       args.dest_cyl,
		       args.devno);
  }
  if(requests != 0){
    total_touches++;
    num_requests+=requests;
  }
  args.outputflags = 0;

  free_after = free_blocks[devno];
  
  if(free_after-free_before != 0){
  }else{
  }
  /*from old takeFB*/
  gettimeofday(&finish_time,NULL);
  free_after = free_blocks[devno];

  stat_update(free_count_stat[(int)floor(now/(float)PERIOD)], free_after-free_before);
 
  stat_update(take_free_time_stat[(int)floor(now/(float)PERIOD)], (finish_time.tv_usec - start_time.tv_usec) + MICROMULT * (finish_time.tv_sec - start_time.tv_sec));

  
  free_time += (finish_time.tv_usec - start_time.tv_usec) + MICROMULT * (finish_time.tv_sec - start_time.tv_sec);
}

static int last_count = 0;
static float last_time = 0.0;
static float lastinst = 0.0;
static long last_call = 0;

void report_free_blocks(unsigned short devno)
{
  int count;

  int cumulative_count;
  int completed_count;

  static int finished=0;

  count = 0;
  cumulative_count = 0;
  completed_count = 0;
  completed_count = free_blocks[devno];
  
  free_bandwidth_stat_inst->count = total_latency_stat_inst->count;
  extra_seek_stat_inst->count = total_latency_stat_inst->count;
  free_bandwidth_stat->count = total_latency_stat->count;
  extra_seek_stat->count = total_latency_stat->count;

  if (algorithm == GREEDY_AREA_FIRST_ALG) {
    int j,kolik;
     kolik = 0;
    for (j=area_firstcyl;j<=area_lastcyl;j++) {
      kolik += free_count[devno][j];
    }
    if (!finished) {
      /*  fprintf(stderr,"There are %d blocks remaining.\n",kolik); */
      if (!kolik){
	fprintf(stderr,
		"Pickup of blocks between cyls %d and %d finished at %6.4fs\n",
		area_firstcyl,area_lastcyl,now);
	finished = 1;
	free_blocks[0] = numblocks;
	done10 = done20 =  done30 = done40 = done50 = done60 = done70 = 1;
	done80 = done90 =  done95 = 1;
	// now = sim_length;
      }
    }
    else {
      	free_blocks[0] = numblocks;
	// now = sim_length;
    }
  }

  if(done10 == 0 && (100.0 * (float)((float)completed_count / (float)numblocks)) >= 10.0){
    done10 = 1;
    fprintf(stderr,"Max cyl dist = %d average requests %f requests %d total %d\n",max_cyl,
	    (float)num_requests/(float)total_touches,num_requests,total_touches);
    max_cyl = 0;
    total_touches = 0;
    num_requests = 0;
    fprintf(stderr,"Cyl Mean = %f Cyl Dist Mean %f calls %ld\n",cyl_mean,(float)((double)cyl_dist_count/(double)(total_seeks)),alg_count-last_call);
    fprintf(stderr,"At 10 %d blocks in %6.4fs %f latency %f free bandwidth %f extra seek time\n",completed_count,now,stat_get_runval(total_latency_stat)/stat_get_count(total_latency_stat),stat_get_runval(free_bandwidth_stat)/stat_get_count(free_bandwidth_stat),stat_get_runval(extra_seek_stat)/stat_get_count(extra_seek_stat));
    last_call = alg_count;
  }else if(done20 == 0 && (100.0 * (float)((float)completed_count / (float)numblocks)) >= 20.0){
    done20 = 1;
    fprintf(stderr,"Max cyl dist = %d average requests %f requests %d total %d\n",max_cyl,
	    (float)num_requests/(float)total_touches,num_requests,total_touches);
    max_cyl = 0;
    total_touches = 0;
    num_requests = 0;
    fprintf(stderr,"Cyl Mean = %f Cyl Dist Mean %f calls %ld\n",cyl_mean,(float)((double)cyl_dist_count/(double)(total_seeks)),alg_count-last_call);
    fprintf(stderr,"At 20 %d blocks in %6.4fs %f latency %f free bandwidth %f extra seek time\n",completed_count,now,stat_get_runval(total_latency_stat)/stat_get_count(total_latency_stat),stat_get_runval(free_bandwidth_stat)/stat_get_count(free_bandwidth_stat),stat_get_runval(extra_seek_stat)/stat_get_count(extra_seek_stat));
    last_call = alg_count;
  }else if(done30 == 0 && (100.0 * (float)((float)completed_count / (float)numblocks)) >= 30.0){
    done30 = 1;
    fprintf(stderr,"Max cyl dist = %d average requests %f requests %d total %d\n",max_cyl,
	    (float)num_requests/(float)total_touches,num_requests,total_touches);
    max_cyl = 0;
    total_touches = 0;
    num_requests = 0;
    fprintf(stderr,"Cyl Mean = %f Cyl Dist Mean %f calls %ld\n",cyl_mean,(float)((double)cyl_dist_count/(double)(total_seeks)),alg_count-last_call);
    fprintf(stderr,"At 30 %d blocks in %6.4fs %f latency %f free bandwidth %f extra seek time\n",completed_count,now,stat_get_runval(total_latency_stat)/stat_get_count(total_latency_stat),stat_get_runval(free_bandwidth_stat)/stat_get_count(free_bandwidth_stat),stat_get_runval(extra_seek_stat)/stat_get_count(extra_seek_stat));
    last_call = alg_count;
  }else if(done40 == 0 && (100.0 * (float)((float)completed_count / (float)numblocks)) >= 40.0){
    done40 = 1;
    fprintf(stderr,"Max cyl dist = %d average requests %f requests %d total %d\n",max_cyl,
	    (float)num_requests/(float)total_touches,num_requests,total_touches);
    max_cyl = 0;
    total_touches = 0;
    num_requests = 0;
    fprintf(stderr,"Cyl Mean = %f Cyl Dist Mean %f calls %ld\n",cyl_mean,(float)((double)cyl_dist_count/(double)(total_seeks)),alg_count-last_call);
    fprintf(stderr,"At 40 %d blocks in %6.4fs %f latency %f free bandwidth %f extra seek time\n",completed_count,now,stat_get_runval(total_latency_stat)/stat_get_count(total_latency_stat),stat_get_runval(free_bandwidth_stat)/stat_get_count(free_bandwidth_stat),stat_get_runval(extra_seek_stat)/stat_get_count(extra_seek_stat));
    last_call = alg_count;
  }else if(done50 == 0 && (100.0 * (float)((float)completed_count / (float)numblocks)) >= 50.0){
    done50 = 1;
    fprintf(stderr,"Max cyl dist = %d average requests %f requests %d total %d\n",max_cyl,
	    (float)num_requests/(float)total_touches,num_requests,total_touches);
    max_cyl = 0;
    total_touches = 0;
    num_requests = 0;
    fprintf(stderr,"Cyl Mean = %f Cyl Dist Mean %f calls %ld\n",cyl_mean,(float)((double)cyl_dist_count/(double)(total_seeks)),alg_count-last_call);
    fprintf(stderr,"At 50 %d blocks in %6.4fs %f latency %f free bandwidth %f extra seek time\n",completed_count,now,stat_get_runval(total_latency_stat)/stat_get_count(total_latency_stat),stat_get_runval(free_bandwidth_stat)/stat_get_count(free_bandwidth_stat),stat_get_runval(extra_seek_stat)/stat_get_count(extra_seek_stat));
    //    exit(1);
    //exit(1);
    last_call = alg_count;
  }else if(done60 == 0 && (100.0 * (float)((float)completed_count / (float)numblocks)) >= 60.0){
    done60 = 1;
    fprintf(stderr,"Max cyl dist = %d average requests %f requests %d total %d\n",max_cyl,
	    (float)num_requests/(float)total_touches,num_requests,total_touches);
    max_cyl = 0;
    total_touches = 0;
    num_requests = 0;
    fprintf(stderr,"Cyl Mean = %f Cyl Dist Mean %f calls %ld\n",cyl_mean,(float)((double)cyl_dist_count/(double)(total_seeks)),alg_count-last_call);
    fprintf(stderr,"At 60 %d blocks in %6.4fs %f latency %f free bandwidth %f extra seek time\n",completed_count,now,stat_get_runval(total_latency_stat)/stat_get_count(total_latency_stat),stat_get_runval(free_bandwidth_stat)/stat_get_count(free_bandwidth_stat),stat_get_runval(extra_seek_stat)/stat_get_count(extra_seek_stat));
    //exit(1);
    last_call = alg_count;
  }else if(done70 == 0 && (100.0 * (float)((float)completed_count / (float)numblocks)) >= 70.0){
    done70 = 1;
    fprintf(stderr,"Max cyl dist = %d average requests %f requests %d total %d\n",max_cyl,
	    (float)num_requests/(float)total_touches,num_requests,total_touches);
    max_cyl = 0;
    total_touches = 0;
    num_requests = 0;
    fprintf(stderr,"Cyl Mean = %f Cyl Dist Mean %f calls %ld\n",cyl_mean,(float)((double)cyl_dist_count/(double)(total_seeks)),alg_count-last_call);
    fprintf(stderr,"At 70 %d blocks in %6.4fs %f latency %f free bandwidth %f extra seek time\n",completed_count,now,stat_get_runval(total_latency_stat)/stat_get_count(total_latency_stat),stat_get_runval(free_bandwidth_stat)/stat_get_count(free_bandwidth_stat),stat_get_runval(extra_seek_stat)/stat_get_count(extra_seek_stat));
    //exit(1);
    last_call = alg_count;
  }else if(done80 == 0 && (100.0 * (float)((float)completed_count / (float)numblocks)) >= 80.0){
    done80 = 1;
    fprintf(stderr,"Max cyl dist = %d average requests %f requests %d total %d\n",max_cyl,
	    (float)num_requests/(float)total_touches,num_requests,total_touches);
    max_cyl = 0;
    total_touches = 0;
    num_requests = 0;
    fprintf(stderr,"Cyl Mean = %f Cyl Dist Mean %f calls %ld\n",cyl_mean,(float)((double)cyl_dist_count/(double)(total_seeks)),alg_count-last_call);
    fprintf(stderr,"At 80 %d blocks in %6.4fs %f latency %f free bandwidth %f extra seek time\n",completed_count,now,stat_get_runval(total_latency_stat)/stat_get_count(total_latency_stat),stat_get_runval(free_bandwidth_stat)/stat_get_count(free_bandwidth_stat),stat_get_runval(extra_seek_stat)/stat_get_count(extra_seek_stat));
    //    exit(1);
    last_call = alg_count;
  }else if(done90 == 0 && (100.0 * (float)((float)completed_count / (float)numblocks)) >= 90.0){
    done90 = 1;
    fprintf(stderr,"Max cyl dist = %d average requests %f requests %d total %d\n",max_cyl,
	    (float)num_requests/(float)total_touches,num_requests,total_touches);
    max_cyl = 0;
    total_touches = 0;
    num_requests = 0;
    fprintf(stderr,"Cyl Mean = %f Cyl Dist Mean %f calls %ld\n",cyl_mean,(float)((double)cyl_dist_count/(double)(total_seeks)),alg_count-last_call);
    fprintf(stderr,"At 90 %d blocks in %6.4fs %f latency %f free bandwidth %f extra seek time\n",completed_count,now,stat_get_runval(total_latency_stat)/stat_get_count(total_latency_stat),stat_get_runval(free_bandwidth_stat)/stat_get_count(free_bandwidth_stat),stat_get_runval(extra_seek_stat)/stat_get_count(extra_seek_stat));
    //exit(1);
    last_call = alg_count;
  }else if(done95 == 0 && (100.0 * (float)((float)completed_count / (float)numblocks)) >= 95.0){
    done95 = 1;
    fprintf(stderr,"Max cyl dist = %d average requests %f requests %d total %d\n",max_cyl,
	    (float)num_requests/(float)total_touches,num_requests,total_touches);
    max_cyl = 0;
    total_touches = 0;
    num_requests = 0;
    fprintf(stderr,"Cyl Mean = %f Cyl Dist Mean %f calls %ld\n",cyl_mean,(float)((double)cyl_dist_count/(double)(total_seeks)),alg_count-last_call);
    fprintf(stderr,"At 95 %d blocks in %6.4fs %f latency %f free bandwidth %f extra seek time\n",completed_count,now,stat_get_runval(total_latency_stat)/stat_get_count(total_latency_stat),stat_get_runval(free_bandwidth_stat)/stat_get_count(free_bandwidth_stat),stat_get_runval(extra_seek_stat)/stat_get_count(extra_seek_stat));
    //    exit(1);
    last_call = alg_count;
  }else if(done100 == 0 && (100.0 * (float)((float)completed_count / (float)numblocks)) >= 100.0){
    done100 = 1;
    fprintf(stderr,"Max cyl dist = %d average requests %f requests %d total %d\n",max_cyl,
	    (float)num_requests/(float)total_touches,num_requests,total_touches);
    max_cyl = 0;
    total_touches = 0;
    num_requests = 0;
    fprintf(stderr,"Cyl Mean = %f Cyl Dist Mean %f calls %ld\n",cyl_mean,(float)((double)cyl_dist_count/(double)(total_seeks)),alg_count-last_call);
    fprintf(stderr,"At 100 %d blocks in %6.4fs %f latency %f free bandwidth %f extra seek time\n",completed_count,now,stat_get_runval(total_latency_stat)/stat_get_count(total_latency_stat),stat_get_runval(free_bandwidth_stat)/stat_get_count(free_bandwidth_stat),stat_get_runval(extra_seek_stat)/stat_get_count(extra_seek_stat));
    last_call = alg_count;
  }

  if(now >= (lastinst + 50.0)){
    lastinst = now;
    fprintf(stderr,"Cyl Mean = %f Cyl Dist Mean %f calls %ld\n",cyl_mean,(float)((double)cyl_dist_count/(double)(total_seeks)),alg_count);
    fprintf(stderr,"At %f %d blocks in %6.4fs %f latency %f free bandwidth %f extra seek time\n",lastinst,completed_count-last_count,now-last_time,stat_get_runval(total_latency_stat_inst)/stat_get_count(total_latency_stat_inst),stat_get_runval(free_bandwidth_stat_inst)/stat_get_count(free_bandwidth_stat_inst),stat_get_runval(extra_seek_stat_inst)/stat_get_count(extra_seek_stat_inst));
    last_count = completed_count;
    last_time = now;
    stat_reset(free_bandwidth_stat_inst);
    stat_reset(extra_seek_stat_inst);
    stat_reset(total_latency_stat_inst);
    stat_reset(search_count_stat);
  }

#ifdef DEBUG_POSSIBLE
  fprintf(stdout,"possible %7d of %d or %0.2f%%",
	  cumulative_count,numBlocks,100.0 * cumulative_count / numBlocks);
#endif /* DEBUG_POSSIBLE */
  //  if(algorithm ==  1 || algorithm ==  2 || algorithm ==  3){
  if(1){
  }else{
  }

  nrequests[devno]++;
}



double exponential_delay()
{
  /* this result should be in seconds, interarrival is in ms */
  return( ( -1.0 * interarrival * log((double) 1.0 - drand48() ) ) / 1000.0);
}

int exponential_sectorcount()
{
  /* this result should be in seconds, interarrival is in ms */
  return(ceil( -1.0 * (double)ave_sectors / BLOCK2SECTOR * log((double) 1.0 - drand48() )) * BLOCK2SECTOR);
}

Request* new_request(double when, Request* recycle)
{
  Request* r;

  if (NULL == recycle) {
    r = (Request*)malloc(sizeof(Request));
    if (NULL == r) { perror("malloc"); exit(1); }
  } else {
    r = recycle;
  }
  
  r->start = when;
  if (drand48() < readfraction) {
    r->type = 'R';
  } else {
    r->type = 'W';
  }

  if (hot_fraction > 0.0) {
    if (drand48() < hot_fraction) {
      r->devno = 0;
    } else {
      r->devno = (lrand48() % (ndrives - 1)) + 1;
    }
  } else {
    r->devno = lrand48() % ndrives;
  }
  //  r->blkno = BLOCK2SECTOR*(lrand48()%(nsectors/BLOCK2SECTOR));
  r->sectorcount = exponential_sectorcount();
  //  fprintf(stderr,"sector_count = %d\n",r->sectorcount);
  r->background = 0;

#ifdef DEBUG_NEWISSUE
  fprintf(stderr,"new request %d %d at %f\n",r->devno,r->blkno,when);
#endif /* DEBUG_NEWISSUE */

  return(r);
}

/*
 * Schedule next callback at time t.
 * Note that there is only *one* outstanding callback at any given time.
 * The callback is for the earliest event.
 */
void syssim_schedule_callback(void (*f)(void *,double), SysTime t)
{
  next_event = t;
}


/*
 * de-schedule a callback.
 */
void syssim_deschedule_callback(void (*f)())
{
  next_event = -1;
}

static int local_last_cylno[MAXDISKS];
static float local_last_angle[MAXDISKS];
static int local_last_surface[MAXDISKS];
 static float last_servtime = 0;

void syssim_report_completion(SysTime current_time, Request *r)
{
  Request* new;
  Request* req;
  double issue_time;
  float source_angle=0.0;
  //void* check_queue;
  
  int i,j;
  int free_before=0;
  int free_after=0;
  //  double issue;

  unsigned short cyl = 0;
  unsigned short sector = 0;
  unsigned short surface = 0;
 
  unsigned short found_cyl = 0;
  unsigned short found_sector = 0;
  unsigned short found_surface = 0;
  
  int found = 0;
  int count = 0;

  //  ioreq_event *request;

  ioreq_event *return_val = NULL;
  
  //now = current_time;

  /* read out the statistics */
  for(i=0;i<ndrives;i++) {
    for(j=0;j<multi_disk_last[i];j++) {
#ifdef DEBUG_PHYSREQUESTS
      fprintf(stdout,"request on dev %d at cyl %d angle %f surf %d seek %f latency %f\n",
	      i,multi_disk_last_cylno[i][j],multi_disk_last_angle[i][j],multi_disk_last_surface[i][j],
	      multi_disk_last_seektime[i][j],multi_disk_last_latency[i][j]);
#endif /* DEBUG_PHYSREQUESTS */
      if (do_free_blocks) {
	if (local_last_cylno[i] > 0) {
	  //fprintf(stderr,"last lat = %f last xfer = %f last_angle %f\n",multi_disk_last_latency[i][j],multi_disk_last_xfertime[i][j],multi_disk_last_angle[i][j]); 
	  stat_update(total_transfer_stat, multi_disk_last_xfertime[i][j]);
	  source_angle = (multi_disk_last_angle[i][j] - (multi_disk_last_latency[i][j]/oneRotation) - multi_disk_last_seektime[i][j]/oneRotation);
	  while(source_angle < 0.0){
	    source_angle += 1.0;
	  }
	  /*take_free_blocks(i,
	    local_last_cylno[i],local_last_angle[i],
	    multi_disk_last_cylno[i][j],(multi_disk_last_angle[i][j] * MAX_ANGLE),
	    multi_disk_last_seektime[i][j],multi_disk_last_latency[i][j],local_last_surface[i], multi_disk_last_surface[i][j],last_servtime);*/
	  take_free_blocks(i,
			   local_last_cylno[i],
			   source_angle,
			   multi_disk_last_cylno[i][j],
			   multi_disk_last_angle[i][j],
			   multi_disk_last_seektime[i][j],
			   multi_disk_last_latency[i][j],
			   local_last_surface[i], 
			   multi_disk_last_surface[i][j],
			   last_servtime);
	}
      }
      last_servtime = multi_disk_last_seektime[i][j] + multi_disk_last_latency[i][j] + multi_disk_last_xfertime[i][j];
      local_last_cylno[i] = multi_disk_last_cylno[i][j]; 
      local_last_angle[i] = (multi_disk_last_angle[i][j] * MAX_ANGLE);
      //     local_last_angle[i] = (multi_disk_last_angle[i][j] * MAX_ANGLE) + ((multi_disk_last_angle[i][j]/oneRotation) * MAX_ANGLE);
      while(local_last_angle[i] >= MAX_ANGLE){
	local_last_angle[i] -=MAX_ANGLE;
      }
      local_last_surface[i] = multi_disk_last_surface[i][j];
    }
    multi_disk_last[i] = 0;
  }

  for(i=0;i<ndrives;i++) {
    report_free_blocks(i);
  }

#ifdef DEBUG_COMPLETIONS
  fprintf(stdout,"completed %s %d %d at %d %f seek %f latency %f at %f issued %f\n",
	  r->background?"background":"foreground",
	  r->devno,r->blkno,
	  r->last_cylno,r->last_angle,
	  r->last_seek,r->last_latency,now,
	  r->start);
#endif /* DEBUG_COMPLETIONS */

#ifdef DEBUG_QUEUELENGTH
  fprintf(stderr,"base queue %d\n",ioqueue_check_dev_basequeue(0,&check_queue));
#endif /* DEBUG_QUEUELENGTH */

  if (1 == r->background) {
    background_outstanding[r->devno]--;

    /* mark the blocks we got from the 'free' list */
    for(i=0;i<r->sectorcount;i++) {
      map_blocknum(r->devno,r->blkno + i,&cyl,&sector,&surface);
      // surfaces[r->devno][cyl][sector].completed++;
    }

    free(r);

    if (do_background) {
      for(j=0;j<ndrives;j++) {
	if (background_outstanding[j] < (amount_background / 10)) { /* more background work */
	  for(i=0;i<amount_background;i++) {
	    new = new_request(now,NULL);
	    new->start = MAX(now,last_background[j]) + (0.0 / 1000.0);
	    new->type = 'R';
	    seq_blkno[j] += seq_sectors;
	    /*	    while(seq_done[j][seq_blkno[j] / seq_sectors]) {
	      seq_blkno[j] += seq_sectors;
	      }*/
	    /*	    if (seq_blkno[j] > max_sectors) {
		    seq_blkno[j] = BLOCK2SECTOR*(lrand48()%(nsectors/BLOCK2SECTOR));
		    }*/
	    new->devno = j;
	    new->blkno = seq_blkno[j];
	    new->sectorcount = seq_sectors;
	    new->background = 1;
#ifdef DEBUG_ISSUE
	    fprintf(stdout,"background %d %d %f\n",new->devno,new->blkno,new->start);
#endif /* DEBUG_ISSUE */

	    disksim_interface_request_arrive(disksim, new->start, new);
	    last_background[j] = new->start;
	  }
	  fflush(stderr); fflush(stdout);
	  fprintf(stderr,"%d background requests issued at %f to %d after %d foreground\n",
		  amount_background,now,j,st.n);
	  fflush(stderr); fflush(stdout);
	  background_outstanding[j] += amount_background;
	} else {
#ifdef DEBUG_SKIP_ISSUE
	  fprintf(stdout,"queue full, skipping\n");
#endif /* DEBUG_SKIP_ISSUE */
	}
      }
    }
  } else { /* 1 == r->background, since we free r */

    if (0 == r->background) {
      req_count--;
      add_statistics(&st, current_time - r->start);
      if(active_requests != 0){
	active_requests--;
      }
      
      if((active_requests != multi_programming) && !do_input_trace){
	for (i = active_requests; i < multi_programming; i++) {   /* start the foreground work */
	  active_requests++;
	  req = new_request(now,NULL);
	  req->devno = i % ndrives; /* XXX - make sure we spread them out */
	  fprintf(stderr,"Fore2? %d %ldactive requests %d multi_programming %d\n",req->devno,req->blkno,active_requests,multi_programming);
	  disksim_interface_request_arrive(disksim, now, req);
	}
      }
      issue_time = now + exponential_delay(); 
      /* measure delay from last issue, not from this completion */
      /* issue_time = MAX(issue_time,now);*/ /* but don't issue requests in the past */
      
      /* XXX - should we mark the foreground block off the 'free' list? */
      /* let's try it, if given -p on the command-line */
      
      map_blocknum(r->devno,r->blkno + i,&cyl,&sector,&surface);
      //      fore_cyl[r->devno][cyl]++;
      if (pickup_demand_blocks) {
	free_before = free_blocks[r->devno];
	found = 0;
	//	fprintf(stderr,"Forground Begin\n");
	for(i=0;i<r->sectorcount;i++) {
	  map_blocknum(r->devno,r->blkno + i,&cyl,&sector,&surface);
	  if(found == 1){
	    if(cyl == found_cyl && surface == found_surface){
	      count++;
	      if(count == min_sector_pickup){
		pickup_free_blocks(found_cyl, 
				   found_surface,
				   found_sector,
				   found_sector + count,
				   found_cyl,
				   found_cyl,
				   r->devno);
		count = 0;
		found = 0;
	      }
	    }else{
	      pickup_free_blocks(found_cyl, 
				 found_surface,
				 found_sector,
				 found_sector + count,
				 found_cyl,
				 found_cyl,
				 r->devno);
	      count = 0;
	      found = 0;
	    }
	  }
	  if(found != 1 && (sector % min_sector_pickup) == 0){
	    found_sector = sector;
	    found_cyl = cyl;
	    found_surface = surface;
	    found = 1;
	    count = 1;
	    if(count == min_sector_pickup){
	      pickup_free_blocks(found_cyl, 
				 found_surface,
				 found_sector,
				 found_sector + count,
				 found_cyl,
				 found_cyl,
				 r->devno);
	      count = 0;
	      found = 0;
	    }
	  }
	}
	//	fprintf(stderr,"Forground End\n");
	free_after = free_blocks[r->devno];
	//	fprintf(stderr,"forground picked %d request %d\n",free_after-free_before,r->sectorcount);
	stat_update(free_demand_count_stat[(int)floor(now/(float)PERIOD)], free_after-free_before);
      }
      stat_update(free_bandwidth_stat,free_bandwidth);
      stat_update(free_bandwidth_stat_inst,free_bandwidth);
      free_bandwidth = 0.0;	
      
      if(!do_input_trace || return_val){
#ifdef DEBUG_ISSUE
	fprintf(stdout,"foreground %d %f\n",new->blkno,issue_time);
#endif /* DEBUG_ISSUE */
	//disksim_interface_request_arrive(disksim, issue_time, new);
	last_foreground = issue_time;
      }
    }
  }
}

void dump_cyl_count(int devno){
  int i;
  for(i=0;i<numcyls;i++){
    fprintf(stderr,"cyl %d has %d freeblocks\n",i,free_count[devno][i]);
  }
}

#define CPUMHZ 0
#define MINFRAGMENT 1
static struct option long_options[] = {
  {"cpuMHz",required_argument,NULL,CPUMHZ},
  {"minFragment",required_argument,NULL,MINFRAGMENT},
};


int main(int argc, char *argv[])
{
  int i,j,k,l;
  int count;
  int cyl_count;
  int last_index=0;
  struct stat buf;
  Request* r;

  ioreq_event *request;
  ioreq_event *return_val;

  FILE *cyl_count_out = NULL;
  FILE *fore_cyl_out = NULL;
  FILE *stat_out = NULL;
  FILE *map_out = NULL;
  FILE *map_out2 = NULL;

  char* param_filename = NULL;
  char* output_filename = NULL;
  //  Request* background;

  char fname[STRMAX];
  char mapfname[STRMAX];
  char basename[STRMAX];
  char gfname[STRMAX];
  char paramval[STRMAX];
  char algparam[STRMAX];
  int cc;
  int errflag = 0;
  int bflag = 0;
  int iflag = 0;
  int nflag = 0;
  int dflag = 0;
  int mflag = 0;
  int mrflag = 0;
  int fflag = 0;
  int tflag = 0;
  int pflag = 0;
  int hflag = 0;
  int rflag = 0;
  int do_map_dump = 0;
  /*added free blocks*/
  int len = 8192000;
  int temp_free_blocks=0;
  int latency_weight=0;
  int graph_file=0;
  int temp_write;
  int synthetic = 0;
  int max_requests=0;
  int temp_band_count=0;
  float temp_mean=0;
  float curr_min_seek=0.0;
  float temp_seek=0.0;
  double last_time=-1.0;
  double temp_prob1 = 0.0;
  double temp_prob2 = 0.0;
  double temp_sum = 0.0;
  double avg_seek = 0.0;
  //  double extra_seek = 0.0;
  double orig_seek = 0.0;
  struct disk *singledisk;
  struct timeval start_time;
  struct timeval finish_time;

  int curr_track=0;
  unsigned short pickup_count=0;
  
  int option_index;

  /* Parse arguments */
  gettimeofday(&start_time,NULL);
  optarg = NULL;
  while (!errflag && (cc = getopt_long(argc, argv, "bi:n:d:m:ft:ph:r:a:w:sg:v:l:o:q:c:e:jz:k:y:u:",long_options,&option_index)) != EOF) {
    switch (cc) {
    case 0:
      switch(long_options[option_index].val){
      case CPUMHZ:
	time_scale = atoi(optarg)*1000;
	break;
      case MINFRAGMENT:
	min_fragment_size = atoi(optarg);
	break;
      }
      break;
    case 'b':
      do_background++;
      bflag++;
      break;
    case 'i':
      interarrival = atof(optarg);
      iflag++;
      break;
    case 'n':
      amount_background = atoi(optarg);
      nflag++;
      break;
    case 'd':
      ndrives = atoi(optarg);
      dflag++;
      break;
    case 'm':
      multi_programming = atoi(optarg);
      mflag++;
      break;
    case 'f':
      temp_free_blocks++;
      fflag++;
      break;
    case 't':
      sim_length = atof(optarg);
      tflag++;
      break;
    case 'p':
      pickup_demand_blocks++;
      pflag++;
      break;
    case 'a':
      algorithm = atoi(optarg);
      break;
      break;
    case 'v':
      max_requests = atoi(optarg);
      mrflag++;
      break;
    case 'h':
      hot_fraction = atof(optarg) / 100.0;
      hflag++;
      break;
    case 'r':
      do_input_trace = 1;
      strncpy(fname,optarg,STRMAX);
      rflag++;
      break;
    case 'c':
      do_map_dump = 1;
      strncpy(mapfname,optarg,STRMAX);
      break;
    case 's':
      synthetic = 1;
      break;
    case 'j':
      onlyfourpass = 1;
      break;
    case 'g':
      graph_file = 1;
      strncpy(basename,optarg,STRMAX);
      break;
    case 'w':
      latency_weight = 1;
      strncpy(paramval,optarg,STRMAX);
      break;
    case 'e':
      latency_weight = 1;
      strncpy(algparam,optarg,STRMAX);
      break;
    case 'k':
      min_sector_pickup = atoi(optarg);
      break;
    case 'l':
      servtime = 1;
      servicetime = atof(optarg);
      break;
    case 'o':
      off_cyl_mult = atof(optarg);
      break;
    case 'q':
      threshold = atof(optarg);
      break;
    case 'u':
      conservative = atof(optarg);
      break;
    case 'y':
      if (sscanf(optarg,"%d,%d",&area_firstcyl,&area_lastcyl) != 2) {
	fprintf(stderr,"Error in -y arguement!\n");
	exit(1);
      }
      break;
    case 'z':
      FB_weight = atof(optarg);
      break;
    case '?':
    default:
      errflag++;
    }
  }
  
  /* Required filenames */
  if (optind < argc) {
    param_filename = strdup(argv[optind]);
    optind++;
  } else {
    fprintf(stderr,"ERROR: missing required <param file>\n");
    errflag++;
  }
  if (optind < argc) {
    output_filename = strdup(argv[optind]);
    optind++;
  } else {
    fprintf(stderr,"ERROR: missing required <output file>\n");
    errflag++;
  }
  
  if(do_input_trace){
    tracefile = fopen(fname,"r");
    if(tracefile == NULL){
      fprintf(stderr,"ERROR: trace file %s could not be opened for reading\n",fname);
    }
  }

  if(graph_file){
    sprintf(gfname,"%s.cyl_graph",basename);
    cyl_count_out = fopen(gfname,"w");
    if(cyl_count_out == NULL){
      fprintf(stderr,"ERROR: output file %s could not be opened for writting\n",gfname);
    }
    sprintf(gfname,"%s.fore_cyl_graph",basename);
    fore_cyl_out = fopen(gfname,"w");
    if(fore_cyl_out == NULL){
      fprintf(stderr,"ERROR: output file %s could not be opened for writting\n",gfname);
    }
    sprintf(gfname,"%s.stat_output",basename);
    stat_out = fopen(gfname,"w");
    if(stat_out == NULL){
      fprintf(stderr,"ERROR: output file %s could not be opened for writting\n",gfname);
    }
    sprintf(gfname,"%s.free_block_rate",basename);
    free_block_rate = fopen(gfname,"w");
    if(free_block_rate == NULL){
      fprintf(stderr,"ERROR: output file %s could not be opened for writting\n",gfname);
    }
  }

  if (pflag && !fflag) {
    fprintf(stderr,"ERROR: cannot have -p without -f\n");
    //errflag++;
  }

  /* Usage errors */
  if (errflag) {
    fprintf(stderr,"Usage: %s [-b] [-n <number>] [-d <ndisks>] [-f] [-t <time>] [-h <hot fraction>] [-p] [-r] <tracefile> <param file> <output file>\n",argv[0]);
    fprintf(stderr,"  -b   perform background work\n");
    fprintf(stderr,"  -n   add this # of background requests\n");
    fprintf(stderr,"  -d   use this number of disks\n");
    fprintf(stderr,"  -f   do free blocks optimization\n");
    fprintf(stderr,"  -t   run for this long\n");
    fprintf(stderr,"  -p   do 'pickup' optimization and count OLTP blocks\n");
    fprintf(stderr,"  -a   use algorithm n\n");
    fprintf(stderr,"  -r   read 'ascii' format trace file from filename\n");
    exit(1);
  }

  /* Parse rest of command-line */
  for ( ; optind < argc; optind++) {
    fprintf(stderr,"WARNING: Extra argument ignored %s\n", argv[optind]);
  }

  //  nsectors = max_sectors;
  
  if (stat(param_filename, &buf) < 0) {
    panic(param_filename);
  }

  disksim = malloc(len);
  if(disksim == NULL){
    fprintf(stderr,"Malloc failed fpr disksim\n");
    exit(-1);
  }

  disksim = disksim_interface_initialize_latency(disksim, len, param_filename, output_filename, latency_weight, paramval, LATENCY_WEIGHT,synthetic,algparam);
  srand48(1);

  do_free_blocks = temp_free_blocks;

  singledisk = getdisk(0);
  numcyls = singledisk->numcyls;
  numblocks = singledisk->numblocks;
  numsurfaces = singledisk->numsurfaces;
  
  sectorspercyl = (int *) calloc(numcyls, sizeof(int));
  if(sectorspercyl == NULL){
    printf("ERROR:  Calloc failed for sectorspercyl!\n");
    abort();
  }

  cylsperzone = (int *) calloc(numcyls, sizeof(int));
  if(cylsperzone == NULL){
    printf("ERROR:  Calloc failed for cylsperzone!\n");
    abort();
  }

  probpercyl = (double *) calloc(numcyls, sizeof(double));
  if(probpercyl == NULL){
    printf("ERROR:  Calloc failed for probpercyl!\n");
    abort();
  }

  cyl_order = (int *) calloc(numcyls, sizeof(int));
  if(cyl_order == NULL){
    printf("ERROR:  Calloc failed for cyl_order!\n");
    abort();
  }
  for(i=0;i<numcyls;i++){
    cyl_order[i] = i;
  }

  free_blocks = (int *) calloc(ndrives, sizeof(int));
  if(free_blocks == NULL){
    printf("ERROR:  Calloc failed for free_blocks!\n");
    abort();
  }

  free_count = (int **) calloc(ndrives, sizeof(int*));
  if(free_count == NULL){
    printf("ERROR:  Calloc failed for free_count!\n");
    abort();
  }

  free_count_track = (unsigned short ***) calloc(ndrives, 
						 sizeof(unsigned short**));
  if(free_count_track == NULL){
    printf("ERROR:  Calloc failed for free_count_track!\n");
    abort();
  }

  for(i=0;i<ndrives;i++){
    free_count[i] = (int *) calloc(numcyls, sizeof(int));
    if(free_count[i] == NULL){
      printf("ERROR:  Calloc failed for free_count at %d!\n",i);
      abort();
    }
    free_count_track[i] = (unsigned short **) calloc(numcyls, 
						     sizeof(unsigned short *));
    if(free_count_track[i] == NULL){
      printf("ERROR:  Calloc failed for free_count_track at %d!\n",i);
      abort();
    }
    for(j=0;j<numcyls;j++){
      free_count[i][j] = 0;
      free_count_track[i][j] = (unsigned short *) calloc(numsurfaces, 
						      sizeof(unsigned short));
      if(free_count_track[i][j] == NULL){
	printf("ERROR:  Calloc failed for free_count_track at %d %d!\n",i,j);
	abort();
      }
      for(k=0;k<numsurfaces;k++){
	free_count_track[i][j][k] = 0;
      }
    }
  }
  if(do_map_dump){
    sprintf(fname,"%s.avg.map",mapfname);    
    map_out = fopen(fname,"w");
    if(map_out == NULL){
      fprintf(stderr,"ERROR: map_out file %s could not be opened for reading\n",fname);
    }
    fwrite(&numsurfaces,sizeof(int),1,map_out);
    fwrite(&singledisk->numbands,sizeof(int),1,map_out);

    sprintf(fname,"%s.max.map",mapfname);    
    map_out2 = fopen(fname,"w");
    if(map_out2 == NULL){
      fprintf(stderr,"ERROR: map_out file %s could not be opened for reading\n",fname);
    }
    fwrite(&numsurfaces,sizeof(int),1,map_out2);
    fwrite(&singledisk->numbands,sizeof(int),1,map_out2);
  }
  for(i=0;i<singledisk->numbands;i++){
    //    fprintf(stderr,"band = %d start_cyl = %d end_cyl = %d blks = %d\n",i,singledisk->bands[i].startcyl,singledisk->bands[i].endcyl,singledisk->bands[i].blkspertrack);
    if(do_map_dump){
      fwrite(&singledisk->bands[i].startcyl,sizeof(int),1,map_out);
      fwrite(&singledisk->bands[i].endcyl,sizeof(int),1,map_out);
      fwrite(&singledisk->bands[i].blkspertrack,sizeof(int),1,map_out);

      fwrite(&singledisk->bands[i].startcyl,sizeof(int),1,map_out2);
      fwrite(&singledisk->bands[i].endcyl,sizeof(int),1,map_out2);
      fwrite(&singledisk->bands[i].blkspertrack,sizeof(int),1,map_out2);
    }
    temp_band_count = (singledisk->bands[i].endcyl - singledisk->bands[i].startcyl) * 
      singledisk->bands[i].blkspertrack;
    if(temp_band_count > max_band_count){
      max_band_count = temp_band_count;
    }
    //    fprintf(stderr,"sectors %d bitmaps %d\n",singledisk->bands[i].blkspertrack,(int)ceil((float)(singledisk->bands[i].blkspertrack+1)/(float)(sizeof(int)*8)));
    for(j=singledisk->bands[i].startcyl;j<=singledisk->bands[i].endcyl;j++){
      sectorspercyl[j] = singledisk->bands[i].blkspertrack;
      cylsperzone[j] = singledisk->bands[i].endcyl - singledisk->bands[i].startcyl;
      temp_mean += (float)(j * (sectorspercyl[j] * numsurfaces)) / (float)(numblocks);
    }
  }
  /*  for(i=0;i<numcyls;i++){
      if(i <= CYL_DIST_MEAN){
      start_cyl = 0;
      }else{
      start_cyl = i - CYL_DIST_MEAN;
      }
      temp_prob = 0.0;
      for(j=start_cyl;j<numcyls;j++){
      if(j == i + CYL_DIST_MEAN){
      j = numcyls;
      }else{
      temp_prob += (float)(sectorspercyl[j] * numsurfaces) / (float)(numblocks);
      }
      }
      if(temp_prob > max_cyl_prob){
      max_cyl_prob = temp_prob;
      }
      probpercyl[i] = temp_prob;
      }*/

  cyl_mean = temp_mean;
  fprintf(stderr, "E[X] = %f for cylinder location\n",temp_mean);
  //  fprintf(stderr,"numcyls = %d\n",numcyls);
  //abort();

  /*free_buckets = (entry_t ***) calloc(ndrives, sizeof(entry_t **));
    if(free_buckets == NULL){
    printf("ERROR:  Calloc failed for free_buckets!\n");
    abort();
    }*/
  // bitmap = (bitmap_t ****) calloc(ndrives, sizeof(bitmap_t ***));
  bitmap = (unsigned int ****) calloc(ndrives, sizeof(unsigned int***));
  if(bitmap == NULL){
    printf("ERROR:  Calloc failed for bitmap!\n");
    abort();
  }

  touchmap = (touchmap_t ***) calloc(ndrives, sizeof(touchmap_t **));
  if(touchmap == NULL){
    printf("ERROR:  Calloc failed for touchmap!\n");
    abort();
  }

  for(l=0;l<ndrives;l++){
    /*    free_buckets[l] = (entry_t **) calloc(numsurfaces, sizeof(entry_t *));
	  if(free_buckets[l] == NULL){
	  printf("ERROR:  Calloc failed for free_buckets!\n");
	  abort();
	  }*/
    //    bitmap[l] = (bitmap_t ***) calloc(numsurfaces, sizeof(bitmap_t **));
    bitmap[l] = (unsigned int ***) calloc(numsurfaces, sizeof(unsigned int**));
    if(bitmap[l] == NULL){
      printf("ERROR:  Calloc failed for bitmap!\n");
      abort();
    }

    touchmap[l] = (touchmap_t **) calloc(numsurfaces, sizeof(touchmap_t *));
    if(touchmap[l] == NULL){
      printf("ERROR:  Calloc failed for touchmap!\n");
      abort();
    }

    for(i=0;i<numsurfaces;i++){
      //bitmap[l][i] = (bitmap_t **) calloc(numcyls, sizeof(bitmap_t *));
      bitmap[l][i] = (unsigned int **) calloc(numcyls, sizeof(unsigned int*));
      if(bitmap[l][i] == NULL){
	printf("ERROR:  Calloc failed for bitmap at surface %d!\n",i);
	abort();
      }
      touchmap[l][i] = (touchmap_t *) calloc(numcyls, sizeof(touchmap_t));
      if(touchmap[l][i] == NULL){
	printf("ERROR:  Calloc failed for touchmap at surface %d!\n",i);
	abort();
      }
      /*      free_buckets[l][i] = (entry_t *) calloc(ANGLE_ENTRIES, sizeof(entry_t));
	      if(free_buckets[l][i] == NULL){
	      printf("ERROR:  Calloc failed for free_buckets at surface %d!\n",i);
	      abort();
	      }
      */
      for(j=0;j<numcyls;j++){
	//	fprintf(stderr,"sectors %d bitmaps %d\n",sectorspercyl[j],(int)ceil((float)sectorspercyl[j]/(float)(sizeof(int)*8)));
	bitmap[l][i][j] = (unsigned int*) calloc(ceil((float)(sectorspercyl[j])/(float)(sizeof(int)*8)), sizeof(unsigned int));
	//	bitmap[l][i][j] = (bitmap_t *) calloc(sectorspercyl[j], sizeof(bitmap_t));
	if(bitmap[l][i][j] == NULL){
	  printf("ERROR:  Calloc failed for bitmap at cylinder %d!\n",i);
	  abort();
	}

	/*	touchmap[l][i][j] = (touchmap_t *) calloc(sectorspercyl[j], sizeof(touchmap_t));
		if(touchmap[l][i][j] == NULL){
		printf("ERROR:  Calloc failed for touchmap at cylinder %d!\n",i);
		abort();
		}*/
      }
      /*for(j=0;j<ANGLE_ENTRIES;j++){
	free_buckets[l][i][j].free_block_count = (unsigned int *) calloc(numcyls, sizeof(unsigned int));
	}
      */
    }
  }
  //#define BITMAP_CHECK(DEVNO,CYL,SURF,SECT)       ((bitmap[DEVNO][SURF][CYL][SECT >> 5] & (0x1 << (SECT & 0x001F))) >> SECT & 0x001F) 
  /*  for(j = 0;j<334;j++){
    i = (0x0FFFFFFF & (0x1 << (j & 0x001F))) >> (j & 0x001F);
    fprintf(stderr,"check %x\n",i);
    i = (0x1 << (j & 0x001F));
    fprintf(stderr,"set %x\n",i);
    i = (0xFFFFFFFE << (j & 0x001F) | 0xFFFFFFFE >> (0x001F - ((j-1) & 0x001F)));
    fprintf(stderr,"clear %x\n",i);
    }*/
  //exit(1);
  /*  for(i=0;i<numcyls;i++){
    for(j=0;j<numsurfaces;j++){
      for(k=0;k<sectorspercyl[i];k++){
	BITMAP_SET(0,i,j,k);
		if(BITMAP_CHECK(0,i,j,k) != 1){
	  fprintf(stderr,"bad set value at %d %d %d\n",i,j,k);
	  abort();
	}
      }
    }
  }
  for(i=0;i<numcyls;i++){
    for(j=0;j<numsurfaces;j++){
      for(k=0;k<sectorspercyl[i];k++){
	BITMAP_CLEAR(0,i,j,k);
		if(BITMAP_CHECK(0,i,j,k) != 0){
	  fprintf(stderr,"bad unset value at %d %d %d\n",i,j,k);
	  abort();
	}
      }
    }
    }*/
  //  exit(1);
  /*  fprintf(stderr,"locatation %d\n",bitmap[0][0][0][0].bits);
  BITMAP_SET(0,0,0,0);
  fprintf(stderr,"locatation %d\n",bitmap[0][0][0][0].bits);
  BITMAP_SET(0,0,0,1);
  fprintf(stderr,"locatation %d\n",bitmap[0][0][0][0].bits);
  BITMAP_SET(0,0,0,2);
  fprintf(stderr,"locatation %d\n",bitmap[0][0][0][0].bits);
  BITMAP_SET(0,0,0,3);
  fprintf(stderr,"locatation %d\n",bitmap[0][0][0][0].bits);
  BITMAP_SET(0,0,0,4);
  fprintf(stderr,"locatation %d\n",bitmap[0][0][0][0].bits);
  fprintf(stderr,"locatation %d\n",BITMAP_CHECK(0,0,0,0));
  fprintf(stderr,"locatation %d\n",BITMAP_CHECK(0,0,0,1));
  fprintf(stderr,"locatation %d\n",BITMAP_CHECK(0,0,0,2));
  fprintf(stderr,"locatation %d\n",BITMAP_CHECK(0,0,0,3));
  fprintf(stderr,"locatation %d\n",BITMAP_CHECK(0,0,0,4));
  fprintf(stderr,"locatation %d\n",BITMAP_CHECK(0,0,0,5));
  fprintf(stderr,"locatation %d\n",bitmap[0][0][0][0].bits);
  BITMAP_CLEAR(0,0,0,0);
  fprintf(stderr,"locatation %d\n",bitmap[0][0][0][0].bits);
  BITMAP_CLEAR(0,0,0,1);
  fprintf(stderr,"locatation %d\n",bitmap[0][0][0][0].bits);
  BITMAP_CLEAR(0,0,0,2);
  fprintf(stderr,"locatation %d\n",bitmap[0][0][0][0].bits);
  BITMAP_CLEAR(0,0,0,3);
  fprintf(stderr,"locatation %d\n",bitmap[0][0][0][0].bits);
  BITMAP_CLEAR(0,0,0,4);
  fprintf(stderr,"locatation %d\n",bitmap[0][0][0][0].bits);
  fprintf(stderr,"locatation %d\n",BITMAP_CHECK(0,0,0,0));
  fprintf(stderr,"locatation %d\n",BITMAP_CHECK(0,0,0,1));
  fprintf(stderr,"locatation %d\n",BITMAP_CHECK(0,0,0,2));
  fprintf(stderr,"locatation %d\n",BITMAP_CHECK(0,0,0,3));
  fprintf(stderr,"locatation %d\n",BITMAP_CHECK(0,0,0,4));
  fprintf(stderr,"locatation %d\n",BITMAP_CHECK(0,0,0,5));
  fprintf(stderr,"locatation %d\n",bitmap[0][0][0][0].bits);
  i =((0xFFFFFFFE << (5 & 0x001F)) | (0xFFFFFFFE >> (0x001F -(4 & 0x001F))));
  fprintf(stderr,"b %x c %x d %x\n",(0xFFFFFFFE << (5 & 0x001F)),(0xFFFFFFFE >> (0x001F - (4 & 0x001F))),i);
  exit(1);*/
  oneRotation = singledisk->rotatetime;

  conservative = conservative/oneRotation;

  seek_angle = (float *) calloc(numcyls, sizeof(float));
  if(seek_angle == NULL){
    printf("ERROR:  Calloc failed for seek_angle!\n");
    abort();
  }

  seek_min = (float *) calloc(numcyls, sizeof(float));
  if(seek_min == NULL){
    printf("ERROR:  Calloc failed for seek_min!\n");
    abort();
  }

  
  min_sectors = (unsigned short **) calloc(numcyls, sizeof(unsigned short *));
  if(min_sectors == NULL){
    printf("ERROR:  Calloc failed for min_sectors!\n");
    abort();
  }

  sector_angle = (float **) calloc(numcyls, sizeof(float *));
  if(sector_angle == NULL){
    printf("ERROR:  Calloc failed for sector_angle!\n");
    abort();
  }

  curr_track = 0;
  curr_min_seek = 1000000.0 * oneRotation;
  for(i=0;i<numcyls;i++){
    seek_angle[i] = remote_disk_seek_time(i,0,1)/oneRotation;

    temp_seek = remote_disk_seek_time(numcyls-1-i,0,1)/oneRotation;

    if(temp_seek < curr_min_seek){
      curr_min_seek = temp_seek;
    }
    seek_min[numcyls-1-i] = curr_min_seek;
		    
    if(sectorspercyl[i] != curr_track){

      min_sectors[i] = (unsigned short *) calloc(sectorspercyl[i]+1, 
						 sizeof(unsigned short));
      if(min_sectors[i] == NULL){
	fprintf(stderr,"ERROR:  Calloc failed for min_sectors at %d!\n",i);
	abort();
      }

      sector_angle[i] = (float *) calloc(sectorspercyl[i]+1, 
						 sizeof(float));
      if(sector_angle[i] == NULL){
	fprintf(stderr,"ERROR:  Calloc failed for sector_angle at %d!\n",i);
	abort();
      }

      curr_track = sectorspercyl[i];
      pickup_count = 0;
      for(j=0;j<=sectorspercyl[i];j++){
	min_sectors[i][j] = pickup_count;
	if(((j+1) % min_sector_pickup) == 0){
	  pickup_count++;
	}
	sector_angle[i][j] = (float)j/(float)sectorspercyl[i];
      }
    }else{
      if(i != 0){
	min_sectors[i] = min_sectors[i-1];
	sector_angle[i] = sector_angle[i-1];
      }
    }
  }

  for(i=0;i<numcyls;i++){
    temp_sum = 0.0;
    temp_prob1 = ((float)(sectorspercyl[i] * numsurfaces)/
		  (float)(numblocks));
    for(j=0;j<numcyls;j++){
      
      orig_seek = seek_angle[abs(i-j)];   
      temp_prob2 = ((float)(sectorspercyl[j] * numsurfaces)/
		    (float)(numblocks));
      temp_sum +=   (temp_prob2 * orig_seek);
      avg_seek += orig_seek*temp_prob1*temp_prob2;
      //      fprintf(stderr,"temp prob %f orig_seek %f avg_seek %f %f %f\n",temp_prob1*temp_prob2,orig_seek,avg_seek,temp_prob1,temp_prob2);
      /*      for(k=0;k<numcyls;k++){
	      probpercyl[k] += (temp_prob *(seek_curve[abs(k-i)] + 
	      seek_curve[abs(k-j)] - orig_seek));
	      }*/
    }
    probpercyl[i] = 2*temp_sum;
    //fprintf(stderr,"temp_sum %d %f\n",i,temp_sum);
  }
  fprintf(stderr,"avg_seek %f\n",avg_seek);
  for(i=0;i<numcyls;i++){
    probpercyl[i] -= avg_seek;
  }

  /*  for(i=0;i<numcyls;i++){
    temp_prob1 = ((float)(sectorspercyl[i] * numsurfaces)/
		  (float)(numblocks));
    for(j=i;j<numcyls;j++){
      orig_seek = seek_curve[abs(i-j)];   
      temp_prob2 = ((float)(sectorspercyl[j] * numsurfaces)/
		    (float)(numblocks));
      for(k=0;k<numcyls;k++){
	extra_seek = seek_curve[abs(k-j)] + seek_curve[abs(k-i)] - orig_seek;
	if(extra_seek <= oneRotation){
	  probpercyl[k] += ((oneRotation - extra_seek)/2.0) * temp_prob1 * temp_prob2;
	}
      }
    }
    }*/
  max_cyl_prob=0.0;
  //  fprintf(stderr,"\n");
  for(i=0;i<numcyls;i++){
    if(probpercyl[i] > max_cyl_prob){
      max_cyl_prob = probpercyl[i];
    }
    //    fprintf(stderr,"%d %f\n",i,probpercyl[i]);
  }  
  //  fprintf(stderr,"\n");
  for(i=0;i<numcyls;i++){
    probpercyl[i] /= max_cyl_prob;
  }
  /*  for(i=0;i<numcyls;i++){
      seek_curve[i] = 0.0;
      }*/
  /*  for(i=0;i<numcyls;i++){
    for(j=0;j<numsurfaces;j++){
      for(k=0;k<sectorspercyl[i];k++){
	BITMAP_SET(0,i,j,k);
      }
    }
    }*/

  for(l=0;l<ndrives;l++){
    for(i=0;i<numsurfaces;i++){
      for(j=0;j<numcyls;j++){
	touchmap[l][i][j].touch = 0.0;
	touchmap[l][i][j].max = 0.0;
	touchmap[l][i][j].squared = 0.0;
	for(k=0;k<sectorspercyl[j];k++){
	  BITMAP_CLEAR(l,j,i,k);
	}
      }
    }
  }
  //  fprintf(stderr,"before init_free_blocks\n");

  /* need to init regardless of -f for background to work */
  init_free_blocks(); 

  /*for(l=0;l<ndrives;l++){
    for(i=0;i<numsurfaces;i++){
      for(j=0;j<numcyls;j++){	
	for(k=0;k<sectorspercyl[j];k++){
	  if(bitmap[l][i][j][(k >> 5)] != 0xffffffff){
	    fprintf(stderr,"bitmap %x surf %d cyl %d sect %d\n",bitmap[l][i][j][(k >> 5)],i,j,k);
	  }
	}
      }
      }
    }*/
  //dump_cyl_count(0);

  //fprintf(stderr,"after init_free_blocks\n");

  for(j=0;j<MAXDISKS;j++) {
    local_last_cylno[j] = -1;
  }

  if(!do_input_trace){
    for (i = 0; i < multi_programming; i++) {   /* start the foreground work */
      r = new_request(now,NULL);
      r->devno = i % ndrives; /* XXX - make sure we spread them out */
      fprintf(stderr,"Fore? %d %ld\n",r->devno,r->blkno);
      disksim_interface_request_arrive(disksim, now, r);
    }
  }


  
      
  /* if we're doing a trace, fix up the request */
  /*  if (do_input_trace) {
      r = new_request(now,NULL);
      return_val = (ioreq_event *)iotrace_get_ioreq_event (tracefile, ASCII, &request);
      if(return_val){
      r->start = MAX(now,(request->time / 1000.0));
      if (request->flags & READ) {
      r->type = 'R';
      } else {
      r->type = 'W';
      }
      r->devno = request->devno;
      r->blkno = request->blkno;
      r->sectorcount = request->bcount;
      r->background = 0;
      disksim_interface_request_arrive(disksim, r->start, r);
      }
      }*/
  if (do_background) {
    for(j=0;j<ndrives;j++) {
      for (i = 0; i < amount_background; i++) {   /* and the background work */
	r = new_request(now,NULL);
	r->start = MAX(now,last_background[j] + (0.0 / 1000.0));
	r->type = 'R';
	r->devno = 0;
	seq_blkno[j] += seq_sectors;
	/*	if (seq_blkno[j] > max_sectors) {
	  seq_blkno[j] = BLOCK2SECTOR*(lrand48()%(nsectors/BLOCK2SECTOR));
	  }*/
	r->devno = j;
	r->blkno = seq_blkno[j];
	r->sectorcount = seq_sectors;
	r->background = 1;
	disksim_interface_request_arrive(disksim, r->start, r);
	last_background[j] = r->start;
      }
      fflush(stderr); fflush(stdout);
      fprintf(stderr,"%d background requests issued at %f to %d\n",amount_background,now,j);
      fflush(stderr); fflush(stdout);
      background_outstanding[j] = amount_background;
    }
  }

  r = NULL;

  if(graph_file){
    temp_write = numcyls;
    fwrite(&temp_write,sizeof(int),1,cyl_count_out);
    temp_write = (int)ceil(sim_length/(float)PERIOD);
    fwrite(&temp_write,sizeof(int),1,cyl_count_out);
    fwrite(&ndrives,sizeof(int),1,cyl_count_out);
  }

  while(((now < sim_length && mrflag == 0) || (nrequests[0] != max_requests && mrflag > 0)) && (free_blocks[0] != numblocks)) { /* minutes of simulation */
    now = next_event;
    //    fprintf(stderr,"now = %f\n",now);
    if (do_input_trace && multi_programming == 0) {
      /* and issue a new one */
      /*      if(last_request != NULL)
	      fprintf(stderr,"now %f last %f\n",now,last_request->start);*/
      do{
	//fprintf(stderr,"First\n");
	if(r != NULL){
	  if(r->start > now){
	    //fprintf(stderr,"Breaking\n");
	    break;
	  }
	  //fprintf(stderr,"Insert %p %f\n",r,r->start);
	  disksim_interface_request_arrive(disksim, r->start, r);
	  r = NULL;
	}
	request = (ioreq_event *)getfromextraq();
	return_val = (ioreq_event *)iotrace_get_ioreq_event (tracefile, ASCII, request);
	if(return_val){
	  //	  fprintf(stderr,"Return\n");
	  r = new_request(now,NULL);
	  /*iotrace_ascii_get_ioreq_event(stdin,&request);*/
	  r->start = MS_TO_SYSSIMTIME(request->time);
	  if (request->flags & READ) {
	    r->type = 'R';
	  } else {
	    r->type = 'W';
	  }
	  r->devno = request->devno;
	  r->blkno = request->blkno;
	  r->sectorcount = request->bcount;
	  r->background = 0;
	  addtoextraq((event *)request);
	}
      }while(return_val != NULL);
    }else if(do_input_trace){
      while(active_requests != multi_programming){
	request = (ioreq_event *)getfromextraq();
	return_val = (ioreq_event *)iotrace_get_ioreq_event (tracefile, ASCII, request);
	if(return_val){
	  //	  fprintf(stderr,"Return\n");
	  r = new_request(now,NULL);
	  /*iotrace_ascii_get_ioreq_event(stdin,&request);*/
	  //	  r->start = MS_TO_SYSSIMTIME(request->time);
	  if (request->flags & READ) {
	    r->type = 'R';
	  } else {
	    r->type = 'W';
	  }
	  r->devno = request->devno;
	  r->blkno = request->blkno;
	  r->sectorcount = request->bcount;
	  r->background = 0;
	  r->start = now;
	  //fprintf(stderr,"Insert %p %f\n",r,r->start);
	  disksim_interface_request_arrive(disksim, r->start, r);
	  active_requests++;
	  r = NULL;
	  addtoextraq((event *)request);
	}
      }
    }
    if(disksim->intq == NULL && r == NULL){
      fprintf(stderr,"now = %f\n",now);
      break;
    }else if(r != NULL && disksim->intq == NULL && req_count != MAX_REQ){
      //      fprintf(stderr,"Insert2 %p %f\n",r,r->start);
      if(r->start < now){
	r->start = now;
      }
      disksim_interface_request_arrive(disksim, r->start, r);
      req_count++;
      r = NULL;
    }else{
      //      if(now > 732)
      //      fprintf(stderr,"now = %f\n",now);
      disksim_interface_internal_event(disksim,now);
    }
    if(floor(now/(float)PERIOD) != last_index){
      //      if(algorithm ==  1 || algorithm ==  2 || algorithm ==  3){
      if(1){
	if(graph_file){
	  for(i=0;i<ndrives;i++){
	    for(j=0;j<numcyls;j++){
	      //fwrite(&free_cyl[i][j],sizeof(int),1,cyl_count_out);
	      //fwrite(&fore_cyl[i][j],sizeof(int),1,fore_cyl_out);
	    }
	  }
	}
	for(i=0;i<ndrives;i++){
	  for(j=0;j<numsurfaces;j++){
	    for(k=0;k<numcyls;k++){
	      count = 0;
	      cyl_count = 0;
	      for(l=0;l<sectorspercyl[k];l++){
		if(BITMAP_CHECK(i,k,j,l)){
		  //		if(bitmap[i][j][k][l].bit){
		  cyl_count++;
		  count++;
		}else{
		  if(count != 0){
		    stat_update(free_pop_stat[last_index], count);
		  }
		  count = 0;
		}
	      }
	      stat_update(free_cyl_pop_stat[last_index], cyl_count);	  
	    }
	  }
	}
      }
      last_index = floor(now/(float)PERIOD);
    }
  }
  //  dump_cyl_count(0);

  disksim_interface_shutdown(disksim, now);
  
  print_statistics(&st, "response time");
  
  gettimeofday(&finish_time,NULL);
  
  total_timer = (finish_time.tv_usec - start_time.tv_usec) + MICROMULT * (finish_time.tv_sec - start_time.tv_sec);

  //  if(algorithm ==  1 || algorithm ==  2 || algorithm ==  3){
  if(1){
    if(graph_file){
      for(i=0;i<ndrives;i++){
	for(j=0;j<numcyls;j++){
	  //	  fwrite(&free_cyl[i][j],sizeof(int),1,cyl_count_out);
	  // fwrite(&fore_cyl[i][j],sizeof(int),1,fore_cyl_out);
	}
      }
    }
    for(i=0;i<ndrives;i++){
      for(j=0;j<numsurfaces;j++){
	for(k=0;k<numcyls;k++){
	  count = 0;
	  cyl_count = 0;
	  for(l=0;l<sectorspercyl[k];l++){
	    if(BITMAP_CHECK(i,k,j,l)){
	      //	    if(bitmap[i][j][k][l].bit){
	      cyl_count++;
	      if(count != 0){
		stat_update(free_pop_stat[(int)floor(now/(float)PERIOD)], count);
	      }
	      count = 0;
	    }else{
	      count++;
	    }
	  }
	  if(do_map_dump){
	    //	    fprintf(stderr,"%d %d = %f\n",j,k,avg_time);
	    fwrite(&touchmap[i][j][k].touch,sizeof(float),1,map_out);
	    fwrite(&touchmap[i][j][k].max,sizeof(float),1,map_out2);
	  }
	  stat_update(free_cyl_pop_stat[(int)floor(now/(float)PERIOD)], cyl_count);	  
	}
      }
    }
  }
  if(do_map_dump){
    fclose(map_out);
    fclose(map_out2);
  }
  for(i=0;i<ceil(sim_length/(float)PERIOD);i++){
    /*    if (statptr->count > 0) {
	  avg = statptr->runval / (double) statptr->count;
	  runsquares = statptr->runsquares / (double) statptr->count - (avg*avg);
	  runsquares = (runsquares > 0.0) ? sqrt(runsquares) : 0.0;
	  }
	  fprintf(outfile, "%s%s average: \t%f\n", identstr, statdesc, avg);
	  fprintf(outfile, "%s%s std.dev.:\t%f\n", identstr, statdesc, runsquares);
	  if (statptr->maxval == (double) ((int)statptr->maxval)) {
	  fprintf(outfile, "%s%s maximum:\t%d\n", identstr, statdesc, ((int)statptr->maxval));
	  } else {
	  fprintf(outfile, "%s%s maximum:\t%f\n", identstr, statdesc, statptr->maxval);
	  }
	  if (buckets > DISTSIZE) {
	  stat_print_large_dist(&statptr, 1, statptr->count, identstr);
	  return;
	  }
	  fprintf(outfile, "%s%s distribution\n", identstr, statdesc);
	  for (i=(DISTSIZE-buckets); i<(DISTSIZE-1); i++) {
	  if (i >= statptr->equals) {
	  distchar = '<';
	  }
	  if (statptr->scale == 1) {
	  fprintf(outfile, "   %c%3d ", distchar, statptr->distbrks[i]);
	  } else {
	  fprintf(outfile, "  %c%4.1f ", distchar, ((double) statptr->distbrks[i] / scale));
	  }
	  }
	  if (statptr->equals == (DISTSIZE-1)) {
	  intval++;
	  }
	  fprintf(outfile, "   %3d+\n", intval);
	  for (i=(DISTSIZE-buckets); i<DISTSIZE; i++) {
	  count += statptr->smalldistvals[i];
	  fprintf(outfile, " %6d ", statptr->smalldistvals[i]);
	  }
	  fprintf(outfile, "\n");  
	  fprintf(stderr, "From %d to %d\n", i*(int)PERIOD,(i+1)*(int)PERIOD);*/
    /*    stat_print_file(seek_dist_stat[i], SEEK_DIST_STAT,stderr);
    stat_print_file(latency_stat[i], LATENCY_STAT,stderr);
    stat_print_file(free_pop_stat[i], FREE_POP_STAT,stderr);
    stat_print_file(free_cyl_pop_stat[i], FREE_CYL_POP_STAT,stderr);
    stat_print_file(free_cyl_stat[i], FREE_CYL_STAT,stderr);
    stat_print_file(free_count_stat[i], FREE_COUNT_STAT,stderr);
    stat_print_file(free_demand_count_stat[i], FREE_DEMAND_COUNT_STAT,stderr);
    stat_print_file(take_free_time_stat[i], TAKE_FREE_TIME_STAT,stderr);*/
  }
  lastinst = now;
  free_bandwidth_stat_inst->count = total_latency_stat_inst->count;
  extra_seek_stat_inst->count = total_latency_stat_inst->count;
  free_bandwidth_stat->count = total_latency_stat->count;
  extra_seek_stat->count = total_latency_stat->count;
  fprintf(stderr,"At %f %d blocks in %6.4fs %f latency %f free bandwidth %f extra seek time\n",lastinst,free_blocks[0]-last_count,now-last_time,stat_get_runval(total_latency_stat_inst)/stat_get_count(total_latency_stat_inst),stat_get_runval(free_bandwidth_stat_inst)/stat_get_count(free_bandwidth_stat_inst),stat_get_runval(extra_seek_stat_inst)/stat_get_count(extra_seek_stat_inst));
  fprintf(stderr,"At Finish %d blocks in %6.4fs %f latency %f free bandwidth %f extra seek time\n",free_blocks[0],now,stat_get_runval(total_latency_stat)/stat_get_count(total_latency_stat),stat_get_runval(free_bandwidth_stat)/stat_get_count(free_bandwidth_stat),stat_get_runval(extra_seek_stat)/stat_get_count(extra_seek_stat));
  stat_print_file(total_seek_stat, SEEK_STAT,stderr);
  stat_print_file(total_transfer_stat, TRANSFER_STAT,stderr);
  stat_print_file(total_latency_stat, LATENCY_STAT,stderr);
  stat_print_file(free_bandwidth_stat, FREE_BANDWIDTH_STAT,stderr);
  stat_print_file(extra_seek_stat, EXTRA_SEEK_STAT,stderr);
  fprintf(stderr,"Time = %d %d %f\n",(int)free_time, (int)total_timer, (float)free_time/total_timer);
  fprintf(stderr,"Max time = %ld Max Percent = %f\n",max_time,max_percent);
  fprintf(stderr,"Total calls = %ld\n",alg_count);
  /*  for(i=0;i<ndrives;i++){
    fprintf(stderr,"Disk %d Inside = %d Outside = %d Source %d Destination %d zero %d\n", i, inside[i], outside[i],sourcecyl[i],destcyl[i],zero[i]);
    }*/

 return(0);
}
