
#define STACK_MAX 32
{
    int c;
    int needed = 0;
    int param_stack[STACK_MAX];
    int stack_ptr = 0;  // index of first free slot
    BITVECTOR(paramvec, DISKSIM_IODRIVER_MAX_PARAM);
    bit_zero(paramvec, DISKSIM_IODRIVER_MAX_PARAM);

    for(c = 0; c < b->params_len; c++) {
	int pnum;
	int i = 0; 
	double d = 0.0; 
	char *s = 0; 
	struct lp_block *blk = 0;
	struct lp_list *l = 0;
	
	if(!b->params[c]) continue;

TOP:
	pnum = lp_param_name(lp_mod_name("disksim_iodriver"), b->params[c]->name);

        // don't initialize things more than once
	if(BIT_TEST(paramvec, pnum)) continue;


	if(stack_ptr > 0) {
	    for(c = 0; c < b->params_len; c++) {
	        if(lp_param_name(lp_mod_name("disksim_iodriver"), b->params[c]->name) == needed)
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
case DISKSIM_IODRIVER_TYPE:
{
if(!(i >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->type = i;
}
} break;
case DISKSIM_IODRIVER_CONSTANT_ACCESS_TIME:
{
if(!((d >= 0.0) || (d == -3.0) || (d == -2.0) || (d == -1.0))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->consttime = d;
}
} break;
case DISKSIM_IODRIVER_USE_QUEUEING_IN_SUBSYSTEM:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->usequeue = i;
}
} break;
case DISKSIM_IODRIVER_SCHEDULER:
{
if(!(result->queue = (struct ioq *)disksim_ioqueue_loadparams(blk, drv_printqueuestats, drv_printcritstats, drv_printidlestats, drv_printintarrstats, drv_printsizestats))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
    default: assert(0); break; 
    } /* end of switch */ 
    BIT_SET(paramvec, pnum);
    if(stack_ptr > 0) { c = param_stack[--stack_ptr]; goto TOP; }    } /* end of outer for loop */ 

  for(c = 0; c <= DISKSIM_IODRIVER_MAX; c++) {
    if(disksim_iodriver_params[c].req && !BIT_TEST(paramvec,c)) {
      fprintf(stderr, "*** error: in DISKSIM_IODRIVER spec -- missing required parameter \"%s\"
", disksim_iodriver_params[c].name);
      return 0;
    }
  }
} /* end of scope */ 

 