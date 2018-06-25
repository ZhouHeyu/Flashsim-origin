
#define STACK_MAX 32
{
    int c;
    int needed = 0;
    int param_stack[STACK_MAX];
    int stack_ptr = 0;  // index of first free slot
    BITVECTOR(paramvec, DISKSIM_CTLR_MAX_PARAM);
    bit_zero(paramvec, DISKSIM_CTLR_MAX_PARAM);

    for(c = 0; c < b->params_len; c++) {
	int pnum;
	int i = 0; 
	double d = 0.0; 
	char *s = 0; 
	struct lp_block *blk = 0;
	struct lp_list *l = 0;
	
	if(!b->params[c]) continue;

TOP:
	pnum = lp_param_name(lp_mod_name("disksim_ctlr"), b->params[c]->name);

        // don't initialize things more than once
	if(BIT_TEST(paramvec, pnum)) continue;


	if(stack_ptr > 0) {
	    for(c = 0; c < b->params_len; c++) {
	        if(lp_param_name(lp_mod_name("disksim_ctlr"), b->params[c]->name) == needed)
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
case DISKSIM_CTLR_TYPE:
{
if(!(RANGE(i,1,3))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->type = i;
}
} break;
case DISKSIM_CTLR_SCALE_FOR_DELAYS:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->timescale = d;
}
} break;
case DISKSIM_CTLR_BULK_SECTOR_TRANSFER_TIME:
{
if(!((d >= 0.0))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->blktranstime = d;
}
} break;
case DISKSIM_CTLR_MAXIMUM_QUEUE_LENGTH:
{
if(!((i >= 0))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->maxoutstanding = i + 1;
}
} break;
case DISKSIM_CTLR_PRINT_STATS:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->printstats = i;
}
} break;
case DISKSIM_CTLR_SCHEDULER:
{
if(!(result->queue = disksim_ioqueue_loadparams(blk, disksim->ctlrinfo->ctl_printqueuestats, disksim->ctlrinfo->ctl_printcritstats, disksim->ctlrinfo->ctl_printidlestats, disksim->ctlrinfo->ctl_printintarrstats,disksim->ctlrinfo->ctl_printsizestats))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_CTLR_CACHE:
{
if(!(result->cache = disksim_cache_loadparams(blk))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_CTLR_MAX_PER_DISK_PENDING_COUNT:
{
if(!(i >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->maxdiskqsize = i;
}
} break;
    default: assert(0); break; 
    } /* end of switch */ 
    BIT_SET(paramvec, pnum);
    if(stack_ptr > 0) { c = param_stack[--stack_ptr]; goto TOP; }    } /* end of outer for loop */ 

  for(c = 0; c <= DISKSIM_CTLR_MAX; c++) {
    if(disksim_ctlr_params[c].req && !BIT_TEST(paramvec,c)) {
      fprintf(stderr, "*** error: in DISKSIM_CTLR spec -- missing required parameter \"%s\"
", disksim_ctlr_params[c].name);
      return 0;
    }
  }
} /* end of scope */ 

 