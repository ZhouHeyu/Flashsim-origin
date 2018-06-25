
#define STACK_MAX 32
{
    int c;
    int needed = 0;
    int param_stack[STACK_MAX];
    int stack_ptr = 0;  // index of first free slot
    BITVECTOR(paramvec, DISKSIM_SIMPLEFLASH_MAX_PARAM);
    bit_zero(paramvec, DISKSIM_SIMPLEFLASH_MAX_PARAM);

    for(c = 0; c < b->params_len; c++) {
	int pnum;
	int i = 0; 
	double d = 0.0; 
	char *s = 0; 
	struct lp_block *blk = 0;
	struct lp_list *l = 0;
	
	if(!b->params[c]) continue;

TOP:
	pnum = lp_param_name(lp_mod_name("disksim_simpleflash"), b->params[c]->name);

        // don't initialize things more than once
	if(BIT_TEST(paramvec, pnum)) continue;


	if(stack_ptr > 0) {
	    for(c = 0; c < b->params_len; c++) {
	        if(lp_param_name(lp_mod_name("disksim_simpleflash"), b->params[c]->name) == needed)
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
case DISKSIM_SIMPLEFLASH_SCHEDULER:
{
if(!(result->queue = (struct ioq *)disksim_ioqueue_loadparams(blk, device_printqueuestats, device_printcritstats, device_printidlestats, device_printintarrstats, device_printsizestats))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_SIMPLEFLASH_MAX_QUEUE_LENGTH:
{
if(!(i >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->maxqlen = i;
}
} break;
case DISKSIM_SIMPLEFLASH_BLOCK_COUNT:
{
if(!(i >= 1)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->numblocks = i;
}
} break;
case DISKSIM_SIMPLEFLASH_EXTRA_BLOCK_COUNT:
{
if(!(i >= 1)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->extrblocks = i;
}
} break;
case DISKSIM_SIMPLEFLASH_FTL_TYPE:
{
if(!(i >= 1)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->type = i;
}
} break;
case DISKSIM_SIMPLEFLASH_BUS_TRANSACTION_LATENCY:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->bus_transaction_latency = d;
}
} break;
case DISKSIM_SIMPLEFLASH_BULK_SECTOR_TRANSFER_TIME:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->blktranstime = d;
}
} break;
case DISKSIM_SIMPLEFLASH_NEVER_DISCONNECT:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->neverdisconnect = i;
}
} break;
case DISKSIM_SIMPLEFLASH_PRINT_STATS:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->printstats = i;
}
} break;
    default: assert(0); break; 
    } /* end of switch */ 
    BIT_SET(paramvec, pnum);
    if(stack_ptr > 0) { c = param_stack[--stack_ptr]; goto TOP; }    } /* end of outer for loop */ 

  for(c = 0; c <= DISKSIM_SIMPLEFLASH_MAX; c++) {
    if(disksim_simpleflash_params[c].req && !BIT_TEST(paramvec,c)) {
      fprintf(stderr, "*** error: in DISKSIM_SIMPLEFLASH spec -- missing required parameter \"%s\"
", disksim_simpleflash_params[c].name);
      return 0;
    }
  }
} /* end of scope */ 

 