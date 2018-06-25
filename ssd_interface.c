/* 
 * Contributors: Youngjae Kim (youkim@cse.psu.edu)
 *               Aayush Gupta (axg354@cse.psu.edu_
 *   
 * In case if you have any doubts or questions, kindly write to: youkim@cse.psu.edu 
 *
 * This source plays a role as bridiging disksim and flash simulator. 
 * 
 * Request processing flow: 
 *
 *  1. Request is sent to the simple flash device module. 
 *  2. This interface determines FTL type. Then, it sends the request
 *     to the lower layer according to the FTL type. 
 *  3. It returns total time taken for processing the request in the flash. 
 *
 */

#include "ssd_interface.h"
#include "disksim_global.h"
#include "dftl.h"

extern int merge_switch_num;
extern int merge_partial_num;
extern int merge_full_num;
int old_merge_switch_num = 0;
int old_merge_partial_num = 0;
int old_merge_full_num= 0;
int old_flash_gc_read_num = 0;
int old_flash_erase_num = 0;
int req_count_num = 1;
int cache_hit, rqst_cnt;
int flag1 = 1;
int count = 0;

int page_num_for_2nd_map_table;

#define MAP_REAL_MAX_ENTRIES 6552// real map table size in bytes
#define MAP_GHOST_MAX_ENTRIES 1640// ghost_num is no of entries chk if this is ok

#define CACHE_MAX_ENTRIES 300
int ghost_arr[MAP_GHOST_MAX_ENTRIES];
int real_arr[MAP_REAL_MAX_ENTRIES];
int cache_arr[CACHE_MAX_ENTRIES];

/***********************************************************************
  Variables for statistics    
 ***********************************************************************/
unsigned int cnt_read = 0;
unsigned int cnt_write = 0;
unsigned int cnt_delete = 0;
unsigned int cnt_evict_from_flash = 0;
unsigned int cnt_evict_into_disk = 0;
unsigned int cnt_fetch_miss_from_disk = 0;
unsigned int cnt_fetch_miss_into_flash = 0;

double sum_of_queue_time = 0.0;
double sum_of_service_time = 0.0;
double sum_of_response_time = 0.0;
unsigned int total_num_of_req = 0;

/***********************************************************************
  Mapping table
 ***********************************************************************/
int real_min = -1;
int real_max = 0;

/***********************************************************************
  Cache
 ***********************************************************************/
int cache_min = -1;
int cache_max = 0;

// Interface between disksim & fsim 

void reset_flash_stat()
{
  flash_read_num = 0;
  flash_write_num = 0;
  flash_gc_read_num = 0;
  flash_gc_write_num = 0; 
  flash_erase_num = 0;
  flash_oob_read_num = 0;
  flash_oob_write_num = 0; 
}

FILE *fp_flash_stat;
FILE *fp_gc;
FILE *fp_gc_timeseries;
double gc_di =0 ,gc_ti=0;


double calculate_delay_flash()
{
  double delay;
  double read_delay, write_delay;
  double erase_delay;
  double gc_read_delay, gc_write_delay;
  double oob_write_delay, oob_read_delay;

  oob_read_delay  = (double)OOB_READ_DELAY  * flash_oob_read_num;
  oob_write_delay = (double)OOB_WRITE_DELAY * flash_oob_write_num;

  read_delay     = (double)READ_DELAY  * flash_read_num; 
  write_delay    = (double)WRITE_DELAY * flash_write_num; 
  erase_delay    = (double)ERASE_DELAY * flash_erase_num; 

  gc_read_delay  = (double)GC_READ_DELAY  * flash_gc_read_num; 
  gc_write_delay = (double)GC_WRITE_DELAY * flash_gc_write_num; 


  delay = read_delay + write_delay + erase_delay + gc_read_delay + gc_write_delay + 
    oob_read_delay + oob_write_delay;

  if( flash_gc_read_num > 0 || flash_gc_write_num > 0 || flash_erase_num > 0 ) {
    gc_ti += delay;
  }
  else {
    gc_di += delay;
  }

  if(warm_done == 1){
    fprintf(fp_gc_timeseries, "%d\t%d\t%d\t%d\t%d\t%d\n", 
      req_count_num, merge_switch_num - old_merge_switch_num, 
      merge_partial_num - old_merge_partial_num, 
      merge_full_num - old_merge_full_num, 
      flash_gc_read_num,
      flash_erase_num);

    old_merge_switch_num = merge_switch_num;
    old_merge_partial_num = merge_partial_num;
    old_merge_full_num = merge_full_num;
    req_count_num++;
  }

  reset_flash_stat();

  return delay;
}


/***********************************************************************
  Initialize Flash Drive 
  ***********************************************************************/

void initFlash()
{
  blk_t total_blk_num;
  blk_t total_util_blk_num;
  blk_t total_extr_blk_num;

  // total number of sectors    
  total_util_sect_num  = flash_numblocks;
  total_extra_sect_num = flash_extrblocks;
  total_sect_num = total_util_sect_num + total_extra_sect_num; 

  // total number of blocks 
  total_blk_num      = total_sect_num / SECT_NUM_PER_BLK;     // total block number
  total_util_blk_num = total_util_sect_num / SECT_NUM_PER_BLK;    // total unique block number

  global_total_blk_num = total_util_blk_num;

  total_extr_blk_num = total_blk_num - total_util_blk_num;        // total extra block number

  ASSERT(total_extr_blk_num != 0);

  if (nand_init(total_blk_num, 3) < 0) {
    EXIT(-4); 
  }

  switch(ftl_type){

    // pagemap
    case 1: ftl_op = pm_setup(); break;
    // blockmap
    //case 2: ftl_op = bm_setup(); break;
    // o-pagemap 
    case 3: ftl_op = opm_setup(); break;
    // fast
    case 4: ftl_op = lm_setup(); break;

    default: break;
  }

  ftl_op->init(total_util_blk_num, total_extr_blk_num);

  nand_stat_reset();
}

void printWearout()
{
  int i;
  FILE *fp = fopen("wearout", "w");
  
  for(i = 0; i<nand_blk_num; i++)
  {
    fprintf(fp, "%d %d\n", i, nand_blk[i].state.ec); 
  }

  fclose(fp);
}


void endFlash()
{
  nand_stat_print(outputfile);
  ftl_op->end;
  nand_end();
}  

/***********************************************************************
  Send request (lsn, sector_cnt, operation flag)
  ***********************************************************************/

void send_flash_request(int start_blk_no, int block_cnt, int operation, int mapdir_flag)
{
	int size;
	//size_t (*op_func)(sect_t lsn, size_t size);
	size_t (*op_func)(sect_t lsn, size_t size, int mapdir_flag);

        if((start_blk_no + block_cnt) >= total_util_sect_num){
          printf("start_blk_no: %d, block_cnt: %d, total_util_sect_num: %d\n", 
              start_blk_no, block_cnt, total_util_sect_num);
          exit(0);
        }

	switch(operation){

	//write
	case 0:

		op_func = ftl_op->write;
		while (block_cnt> 0) {
			size = op_func(start_blk_no, block_cnt, mapdir_flag);
			start_blk_no += size;
			block_cnt-=size;
		}
		break;
	//read
	case 1:


		op_func = ftl_op->read;
		while (block_cnt> 0) {
			size = op_func(start_blk_no, block_cnt, mapdir_flag);
			start_blk_no += size;
			block_cnt-=size;
		}
		break;

	default: 
		break;
	}
}

void find_real_max()
{
  int i; 

  for(i=0;i < MAP_REAL_MAX_ENTRIES; i++) {
      if(opagemap[real_arr[i]].map_age > opagemap[real_max].map_age) {
          real_max = real_arr[i];
      }
  }
}

void find_real_min()
{
  
  int i,index; 
  int temp = 99999999;

  for(i=0; i < MAP_REAL_MAX_ENTRIES; i++) {
        if(opagemap[real_arr[i]].map_age <= temp) {
            real_min = real_arr[i];
            temp = opagemap[real_arr[i]].map_age;
            index = i;
        }
  }    
}

int find_min_ghost_entry()
{
  int i; 

  int ghost_min = 0;
  int temp = 99999999; 

  for(i=0; i < MAP_GHOST_MAX_ENTRIES; i++) {
    if( opagemap[ghost_arr[i]].map_age <= temp) {
      ghost_min = ghost_arr[i];
      temp = opagemap[ghost_arr[i]].map_age;
    }
  }
  return ghost_min;
}

void init_arr()
{

  int i;
  for( i = 0; i < MAP_REAL_MAX_ENTRIES; i++) {
      real_arr[i] = -1;
  }
  for( i = 0; i < MAP_GHOST_MAX_ENTRIES; i++) {
      ghost_arr[i] = -1;
  }
  for( i = 0; i < CACHE_MAX_ENTRIES; i++) {
      cache_arr[i] = -1;
  }

}

int search_table(int *arr, int size, int val) 
{
    int i;
    for(i =0 ; i < size; i++) {
        if(arr[i] == val) {
            return i;
        }
    }

    printf("shouldnt come here for search_table()=%d,%d",val,size);
    for( i = 0; i < size; i++) {
      if(arr[i] != -1) {
        printf("arr[%d]=%d ",i,arr[i]);
      }
    }
    exit(1);
    return -1;
}

int find_free_pos( int *arr, int size)
{
    int i;
    for(i = 0 ; i < size; i++) {
        if(arr[i] == -1) {
            return i;
        }
    } 
    printf("shouldnt come here for find_free_pos()");
    exit(1);
    return -1;
}

void find_min_cache()
{
  int i; 
  int temp = 999999;

  for(i=0; i < CACHE_MAX_ENTRIES ;i++) {
      if(opagemap[cache_arr[i]].cache_age <= temp ) {
          cache_min = cache_arr[i];
          temp = opagemap[cache_arr[i]].cache_age;
      }
  }
}

int youkim_flag1=0;

double callFsim(unsigned int secno, int scount, int operation)
{
  double delay; 
  int bcount;
  unsigned int blkno; // pageno for page based FTL
  int cnt,z; int min_ghost;

  int pos=-1,pos_real=-1,pos_ghost=-1;

  if(ftl_type == 1){ }

  if(ftl_type == 3) {
      page_num_for_2nd_map_table = (opagemap_num / MAP_ENTRIES_PER_PAGE);
    
      if(youkim_flag1 == 0 ) {
        youkim_flag1 = 1;
        init_arr();
      }

      if((opagemap_num % MAP_ENTRIES_PER_PAGE) != 0){
        page_num_for_2nd_map_table++;
      }
  }
      
  // page based FTL 
  if(ftl_type == 1 ) { 
    blkno = secno / 4;
    bcount = (secno + scount -1)/4 - (secno)/4 + 1;
  }  
  // block based FTL 
  else if(ftl_type == 2){
    blkno = secno/4;
    bcount = (secno + scount -1)/4 - (secno)/4 + 1;
  }
  // o-pagemap scheme
  else if(ftl_type == 3 ) { 
    blkno = secno / 4;
    blkno += page_num_for_2nd_map_table;
    bcount = (secno + scount -1)/4 - (secno)/4 + 1;
  }  
  // FAST scheme
  else if(ftl_type == 4){
    blkno = secno/4;
    bcount = (secno + scount -1)/4 - (secno)/4 + 1;
  }

  cnt = bcount;

  switch(operation)
  {
    //write/read
    case 0:
    case 1:

    while(cnt > 0)
    {
          cnt--;

        // page based FTL
        if(ftl_type == 1){
          send_flash_request(blkno*4, 4, operation, 1); 
          blkno++;
        }

        // blck based FTL
        else if(ftl_type == 2){
          send_flash_request(blkno*4, 4, operation, 1); 
          blkno++;
        }

        // opagemap ftl scheme
        else if(ftl_type == 3)
        {

          /************************************************
            primary map table 
          *************************************************/
          //1. pagemap in SRAM 

            rqst_cnt++;
          if((opagemap[blkno].map_status == MAP_REAL) || (opagemap[blkno].map_status == MAP_GHOST))
          {
            cache_hit++;

            opagemap[blkno].map_age++;

            if(opagemap[blkno].map_status == MAP_GHOST){

              if ( real_min == -1 ) {
                real_min = 0;
                find_real_min();
              }    
              if(opagemap[real_min].map_age <= opagemap[blkno].map_age) 
              {
                find_real_min();  // probably the blkno is the new real_min alwaz
                opagemap[blkno].map_status = MAP_REAL;
                opagemap[real_min].map_status = MAP_GHOST;

                pos_ghost = search_table(ghost_arr,MAP_GHOST_MAX_ENTRIES,blkno);
                ghost_arr[pos_ghost] = -1;
                
                pos_real = search_table(real_arr,MAP_REAL_MAX_ENTRIES,real_min);
                real_arr[pos_real] = -1;

                real_arr[pos_real]   = blkno; 
                ghost_arr[pos_ghost] = real_min; 
              }
            }
            else if(opagemap[blkno].map_status == MAP_REAL) 
            {
              if ( real_max == -1 ) {
                real_max = 0;
                find_real_max();
                printf("Never happend\n");
              }

              if(opagemap[real_max].map_age <= opagemap[blkno].map_age)
              {
                real_max = blkno;
              }  
            }
            else {
              printf("forbidden/shouldnt happen real =%d , ghost =%d\n",MAP_REAL,MAP_GHOST);
            }
          }

          //2. opagemap not in SRAM 
          else
          {
            //if map table in SRAM is full
            if((MAP_REAL_MAX_ENTRIES - MAP_REAL_NUM_ENTRIES) == 0)
            {
              if((MAP_GHOST_MAX_ENTRIES - MAP_GHOST_NUM_ENTRIES) == 0)
              { //evict one entry from ghost cache to DRAM or Disk, delay = DRAM or disk write, 1 oob write for invalidation 
                min_ghost = find_min_ghost_entry();
                  evict++;

                if(opagemap[min_ghost].update == 1) {
                  update_reqd++;
                  opagemap[min_ghost].update = 0;
                  send_flash_request(((min_ghost-page_num_for_2nd_map_table)/MAP_ENTRIES_PER_PAGE)*4, 4, 1, 2);   // read from 2nd mapping table then update it

                  send_flash_request(((min_ghost-page_num_for_2nd_map_table)/MAP_ENTRIES_PER_PAGE)*4, 4, 0, 2);   // write into 2nd mapping table 
                } 
                opagemap[min_ghost].map_status = MAP_INVALID;

                MAP_GHOST_NUM_ENTRIES--;

                //evict one entry from real cache to ghost cache 
                MAP_REAL_NUM_ENTRIES--;
                MAP_GHOST_NUM_ENTRIES++;
                find_real_min();
                opagemap[real_min].map_status = MAP_GHOST;

                pos = search_table(ghost_arr,MAP_GHOST_MAX_ENTRIES,min_ghost);
                ghost_arr[pos]=-1;

                
                ghost_arr[pos]= real_min;
                
                pos = search_table(real_arr,MAP_REAL_MAX_ENTRIES,real_min);
                real_arr[pos]=-1;
              }
              else{
                //evict one entry from real cache to ghost cache 
                MAP_REAL_NUM_ENTRIES--;
                find_real_min();
                opagemap[real_min].map_status = MAP_GHOST;
               
                pos = search_table(real_arr,MAP_REAL_MAX_ENTRIES,real_min);
                real_arr[pos]=-1;

                pos = find_free_pos(ghost_arr,MAP_GHOST_MAX_ENTRIES);
                ghost_arr[pos]=real_min;
                
                MAP_GHOST_NUM_ENTRIES++;
              }
            }

            flash_hit++;
            send_flash_request(((blkno-page_num_for_2nd_map_table)/MAP_ENTRIES_PER_PAGE)*4, 4, 1, 2);   // read from 2nd mapping table

            opagemap[blkno].map_status = MAP_REAL;

            opagemap[blkno].map_age = opagemap[real_max].map_age + 1;
            real_max = blkno;
            MAP_REAL_NUM_ENTRIES++;
            
            pos = find_free_pos(real_arr,MAP_REAL_MAX_ENTRIES);
            real_arr[pos] = blkno;
          }

         //comment out the next line when using cache
          if(operation==0){
            write_count++;
            opagemap[blkno].update = 1;
          }
          else
             read_count++;

          send_flash_request(blkno*4, 4, operation, 1); 
          blkno++;
        }

        // FAST scheme  
        else if(ftl_type == 4){ 

          if(operation == 0){
            write_count++;
          }
          else read_count++;

          send_flash_request(blkno*4, 4, operation, 1); //cache_min is a page for page baseed FTL
          blkno++;
        }
    }
    break;
  }

  delay = calculate_delay_flash();

  return delay;
}

