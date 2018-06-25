/* 
 * Contributors: Youngjae Kim (youkim@cse.psu.edu)
 *               Aayush Gupta (axg354@cse.psu.edu)
 * 
 * In case if you have any doubts or questions, kindly write to: youkim@cse.psu.edu 
 *   
 * Description: This is a header file for pagemap.c.
 * 
 * Acknowledgement: We thank Jeong Uk Kang by sharing the initial version 
 * of sector-level FTL source code. 
 * 
 */

#include "type.h"

#define MAP_INVALID 1 
#define MAP_REAL 2
#define MAP_GHOST 3

#define CACHE_INVALID 0
#define CACHE_VALID 1

int cache_hit;
int flash_hit;
int disk_hit;
int read_cache_hit;
int write_cache_hit;
int evict;
int delay_flash_update;

struct ftl_operation * pm_setup();

struct pm_entry {
  _u32 free  : 1;
  _u32 ppn   : 31;
  int  cache_status;
  int  cache_age;
  int  map_status;
  int  map_age;
  int  flash_valid;   // 1 flash 0 disk
};

struct map_dir{
  unsigned int ppn;
};

#define MAP_ENTRIES_PER_PAGE 512

int TOTAL_MAP_ENTRIES; 
int MAP_REAL_NUM_ENTRIES;
int MAP_GHOST_NUM_ENTRIES;
int CACHE_NUM_ENTRIES;
static int SYNC_NUM;

sect_t pagemap_num;
struct pm_entry *pagemap;
