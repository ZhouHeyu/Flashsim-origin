
#define STACK_MAX 32
{
    int c;
    int needed = 0;
    int param_stack[STACK_MAX];
    int stack_ptr = 0;  // index of first free slot
    BITVECTOR(paramvec, DISKSIM_DISK_MAX_PARAM);
    bit_zero(paramvec, DISKSIM_DISK_MAX_PARAM);

    for(c = 0; c < b->params_len; c++) {
	int pnum;
	int i = 0; 
	double d = 0.0; 
	char *s = 0; 
	struct lp_block *blk = 0;
	struct lp_list *l = 0;
	
	if(!b->params[c]) continue;

TOP:
	pnum = lp_param_name(lp_mod_name("disksim_disk"), b->params[c]->name);

        // don't initialize things more than once
	if(BIT_TEST(paramvec, pnum)) continue;


	if(stack_ptr > 0) {
	    for(c = 0; c < b->params_len; c++) {
	        if(lp_param_name(lp_mod_name("disksim_disk"), b->params[c]->name) == needed)
		    goto FOUND;
            }
	    break;
	}
      FOUND:

	switch(PTYPE(b->params[c])) {
	    case I:  i   = IVAL(b->params[c]); break;
	    case D:  d   = DVAL(b->params[c]); break;
	    case S:  s   = SVAL(b->params[c]); break;
	    case LIST: l = LVAL(b->params[c]); break;
	  default: blk = BVAL(b->params[c]); break;
	}


	switch(pnum) {
case DISKSIM_DISK_MODEL:
{
if(!(result->model = dm_disk_loadparams(blk, 0))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_DISK_SCHEDULER:
{
if(!((result->queue = (struct ioq *)disksim_ioqueue_loadparams(blk, device_printqueuestats, device_printcritstats, device_printidlestats, device_printintarrstats, device_printsizestats)))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_DISK_MAX_QUEUE_LENGTH:
{
if(!(i >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->maxqlen = i;
}
} break;
case DISKSIM_DISK_BULK_SECTOR_TRANSFER_TIME:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->blktranstime = d;
}
} break;
case DISKSIM_DISK_SEGMENT_SIZE_:
{
if(!(RANGE(i, 1, result->model->dm_sectors))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->segsize = i;
}
} break;
case DISKSIM_DISK_NUMBER_OF_BUFFER_SEGMENTS:
{
if(!(i >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->numsegs = i;
}
} break;
case DISKSIM_DISK_PRINT_STATS:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->printstats = i;
}
} break;
case DISKSIM_DISK_PER_REQUEST_OVERHEAD_TIME:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead = d;
}
} break;
case DISKSIM_DISK_TIME_SCALE_FOR_OVERHEADS:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->timescale = d;
}
} break;
case DISKSIM_DISK_HOLD_BUS_ENTIRE_READ_XFER:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->hold_bus_for_whole_read_xfer = i;
}
} break;
case DISKSIM_DISK_HOLD_BUS_ENTIRE_WRITE_XFER:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->hold_bus_for_whole_write_xfer = i;
}
} break;
case DISKSIM_DISK_ALLOW_ALMOST_READ_HITS:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->almostreadhits = i;
}
} break;
case DISKSIM_DISK_ALLOW_SNEAKY_FULL_READ_HITS:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->sneakyfullreadhits = i;
}
} break;
case DISKSIM_DISK_ALLOW_SNEAKY_PARTIAL_READ_HITS:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->sneakypartialreadhits = i;
}
} break;
case DISKSIM_DISK_ALLOW_SNEAKY_INTERMEDIATE_READ_HITS:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->sneakyintermediatereadhits = i;
}
} break;
case DISKSIM_DISK_ALLOW_READ_HITS_ON_WRITE_DATA:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->readhitsonwritedata = i;
}
} break;
case DISKSIM_DISK_ALLOW_WRITE_PREBUFFERING:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->writeprebuffering = i;
}
} break;
case DISKSIM_DISK_PRESEEKING_LEVEL:
{
if(!(RANGE(i,0,2))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->preseeking = i;
}
} break;
case DISKSIM_DISK_NEVER_DISCONNECT:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->neverdisconnect = i;
}
} break;
case DISKSIM_DISK_AVG_SECTORS_PER_CYLINDER:
{
if(!(RANGE(i,1,result->model->dm_sectors))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->sectpercyl = i;
}
} break;
case DISKSIM_DISK_MAXIMUM_NUMBER_OF_WRITE_SEGMENTS:
{
if(!BIT_TEST(paramvec, DISKSIM_DISK_NUMBER_OF_BUFFER_SEGMENTS)) {   param_stack[stack_ptr++] = c;   needed = DISKSIM_DISK_NUMBER_OF_BUFFER_SEGMENTS; continue;  }if(!(RANGE(i,1,result->numsegs))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->numwritesegs = i;
}
} break;
case DISKSIM_DISK_USE_SEPARATE_WRITE_SEGMENT:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->dedicatedwriteseg = (segment *)i;
}
} break;
case DISKSIM_DISK_LOW__WATER_MARK:
{
if(!(RANGE(d,0.0,1.0))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->writewater = d;
}
} break;
case DISKSIM_DISK_HIGH__WATER_MARK:
{
if(!(RANGE(d,0.0,1.0))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->readwater = d;
}
} break;
case DISKSIM_DISK_SET_WATERMARK_BY_REQSIZE:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->reqwater = i;
}
} break;
case DISKSIM_DISK_CALC_SECTOR_BY_SECTOR:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->sectbysect = i;
}
} break;
case DISKSIM_DISK_ENABLE_CACHING_IN_BUFFER:
{
if(!(RANGE(i,0,2))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->enablecache = i;
}
} break;
case DISKSIM_DISK_BUFFER_CONTINUOUS_READ:
{
if(!(RANGE(i,0,4))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->contread = i;
}
} break;
case DISKSIM_DISK_MINIMUM_READ_AHEAD_:
{
if(!(RANGE(i,0,result->segsize))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->minreadahead = i;
}
} break;
case DISKSIM_DISK_MAXIMUM_READ_AHEAD_:
{
if(!(RANGE(i,0,result->segsize))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->maxreadahead = i;
}
} break;
case DISKSIM_DISK_READ_AHEAD_OVER_REQUESTED:
{
if(!(RANGE(i,-1,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->keeprequestdata = i;
}
} break;
case DISKSIM_DISK_READ_AHEAD_ON_IDLE_HIT:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->readaheadifidle = i;
}
} break;
case DISKSIM_DISK_READ_ANY_FREE_BLOCKS:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->readanyfreeblocks = i;
}
} break;
case DISKSIM_DISK_FAST_WRITE_LEVEL:
{
if(!(RANGE(i,0,2))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->fastwrites = i;
}
} break;
case DISKSIM_DISK_COMBINE_SEQ_WRITES:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->writecomb = i;
}
} break;
case DISKSIM_DISK_STOP_PREFETCH_IN_SECTOR:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->stopinsector = i;
}
} break;
case DISKSIM_DISK_DISCONNECT_WRITE_IF_SEEK:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->disconnectinseek = i;
}
} break;
case DISKSIM_DISK_WRITE_HIT_STOP_PREFETCH:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->write_hit_stop_readahead = i;
}
} break;
case DISKSIM_DISK_READ_DIRECTLY_TO_BUFFER:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->read_direct_to_buffer = i;
}
} break;
case DISKSIM_DISK_IMMED_TRANSFER_PARTIAL_HIT:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->immedtrans_any_readhit = i;
}
} break;
case DISKSIM_DISK_READ_HIT_OVER_AFTER_READ:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_command_readhit_afterread = d;
}
} break;
case DISKSIM_DISK_READ_HIT_OVER_AFTER_WRITE:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_command_readhit_afterwrite = d;
}
} break;
case DISKSIM_DISK_READ_MISS_OVER_AFTER_READ:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_command_readmiss_afterread = d;
}
} break;
case DISKSIM_DISK_READ_MISS_OVER_AFTER_WRITE:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_command_readmiss_afterwrite = d;
}
} break;
case DISKSIM_DISK_WRITE_HIT_OVER_AFTER_READ:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_command_writehit_afterread = d;
}
} break;
case DISKSIM_DISK_WRITE_HIT_OVER_AFTER_WRITE:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_command_writehit_afterwrite = d;
}
} break;
case DISKSIM_DISK_WRITE_MISS_OVER_AFTER_READ:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_command_writemiss_afterread = d;
}
} break;
case DISKSIM_DISK_WRITE_MISS_OVER_AFTER_WRITE:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_command_writemiss_afterwrite = d;
}
} break;
case DISKSIM_DISK_READ_COMPLETION_OVERHEAD:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_complete_read = d;
}
} break;
case DISKSIM_DISK_WRITE_COMPLETION_OVERHEAD:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_complete_write = d;
}
} break;
case DISKSIM_DISK_DATA_PREPARATION_OVERHEAD:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_data_prep = d;
}
} break;
case DISKSIM_DISK_FIRST_RESELECT_OVERHEAD:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_reselect_first = d;
}
} break;
case DISKSIM_DISK_OTHER_RESELECT_OVERHEAD:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_reselect_other = d;
}
} break;
case DISKSIM_DISK_READ_DISCONNECT_AFTERREAD:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_disconnect_read_afterread = d;
}
} break;
case DISKSIM_DISK_READ_DISCONNECT_AFTERWRITE:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_disconnect_read_afterwrite = d;
}
} break;
case DISKSIM_DISK_WRITE_DISCONNECT_OVERHEAD:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->overhead_disconnect_write = d;
}
} break;
case DISKSIM_DISK_EXTRA_WRITE_DISCONNECT:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->extra_write_disconnect = i;
}
} break;
case DISKSIM_DISK_EXTRADISC_COMMAND_OVERHEAD:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->extradisc_command = d;
}
} break;
case DISKSIM_DISK_EXTRADISC_DISCONNECT_OVERHEAD:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->extradisc_disconnect1 = d;
}
} break;
case DISKSIM_DISK_EXTRADISC_INTER_DISCONNECT_DELAY:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->extradisc_inter_disconnect = d;
}
} break;
case DISKSIM_DISK_EXTRADISC_2ND_DISCONNECT_OVERHEAD:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->extradisc_disconnect2 = d;
}
} break;
case DISKSIM_DISK_EXTRADISC_SEEK_DELTA:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->extradisc_seekdelta = d;
}
} break;
case DISKSIM_DISK_MINIMUM_SEEK_DELAY:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->minimum_seek_delay = d;
}
} break;
case DISKSIM_DISK_IMMEDIATE_BUFFER_READ:
{
if(!(RANGE(i,0,2))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->immedread = i;
}
} break;
case DISKSIM_DISK_IMMEDIATE_BUFFER_WRITE:
{
if(!(RANGE(i,0,2))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->immedwrite = i;
}
} break;
    default: assert(0); break; 
    } /* end of switch */ 
    BIT_SET(paramvec, pnum);
    if(stack_ptr > 0) { c = param_stack[--stack_ptr]; goto TOP; }    } /* end of outer for loop */ 

  for(c = 0; c <= DISKSIM_DISK_MAX; c++) {
    if(disksim_disk_params[c].req && !BIT_TEST(paramvec,c)) {
      fprintf(stderr, "*** error: in DISKSIM_DISK spec -- missing required parameter \"%s\"
", disksim_disk_params[c].name);
      return 0;
    }
  }
} /* end of scope */ 

 