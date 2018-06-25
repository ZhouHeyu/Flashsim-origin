
#define STACK_MAX 32
{
    int c;
    int needed = 0;
    int param_stack[STACK_MAX];
    int stack_ptr = 0;  // index of first free slot
    BITVECTOR(paramvec, DISKSIM_IOQUEUE_MAX_PARAM);
    bit_zero(paramvec, DISKSIM_IOQUEUE_MAX_PARAM);

    for(c = 0; c < b->params_len; c++) {
	int pnum;
	int i = 0; 
	double d = 0.0; 
	char *s = 0; 
	struct lp_block *blk = 0;
	struct lp_list *l = 0;
	
	if(!b->params[c]) continue;

TOP:
	pnum = lp_param_name(lp_mod_name("disksim_ioqueue"), b->params[c]->name);

        // don't initialize things more than once
	if(BIT_TEST(paramvec, pnum)) continue;


	if(stack_ptr > 0) {
	    for(c = 0; c < b->params_len; c++) {
	        if(lp_param_name(lp_mod_name("disksim_ioqueue"), b->params[c]->name) == needed)
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
case DISKSIM_IOQUEUE_SCHEDULING_POLICY:
{
if(!(RANGE(i,1,21))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->base.sched_alg = i;
}
} break;
case DISKSIM_IOQUEUE_CYLINDER_MAPPING_STRATEGY:
{
if(!(RANGE(i,0,6))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->cylmaptype = i;
}
} break;
case DISKSIM_IOQUEUE_WRITE_INITIATION_DELAY:
{
if(!(d >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->writedelay = d;
}
} break;
case DISKSIM_IOQUEUE_READ_INITIATION_DELAY:
{
if(!(d >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->readdelay = d;
}
} break;
case DISKSIM_IOQUEUE_SEQUENTIAL_STREAM_SCHEME:
{
 result->seqscheme = i;
} break;
case DISKSIM_IOQUEUE_MAXIMUM_CONCAT_SIZE:
{
if(!(i >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->concatmax = i;
}
} break;
case DISKSIM_IOQUEUE_OVERLAPPING_REQUEST_SCHEME:
{
if(!(RANGE(i,0,2))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->comboverlaps = i;
}
} break;
case DISKSIM_IOQUEUE_SEQUENTIAL_STREAM_DIFF_MAXIMUM:
{
if(!(RANGE(i,0,2))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->seqstreamdiff = i;
}
} break;
case DISKSIM_IOQUEUE_SCHEDULING_TIMEOUT_SCHEME:
{
 if(disksim->verbosity > 0) fprintf(stderr, "** warning: ignoring \"Scheduling timeout scheme\" \n");
} break;
case DISKSIM_IOQUEUE_TIMEOUT_TIMEWEIGHT:
{
if(!(i >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->to_time = i;
}
} break;
case DISKSIM_IOQUEUE_TIMEOUT_SCHEDULING:
{
if(!(RANGE(i,0,21))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->timeout.sched_alg = i;
}
} break;
case DISKSIM_IOQUEUE_SCHEDULING_PRIORITY_SCHEME:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->pri_scheme = i;
}
} break;
case DISKSIM_IOQUEUE_PRIORITY_SCHEDULING:
{
if(!(RANGE(i,0,21))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->priority.sched_alg = i;
}
} break;
    default: assert(0); break; 
    } /* end of switch */ 
    BIT_SET(paramvec, pnum);
    if(stack_ptr > 0) { c = param_stack[--stack_ptr]; goto TOP; }    } /* end of outer for loop */ 

  for(c = 0; c <= DISKSIM_IOQUEUE_MAX; c++) {
    if(disksim_ioqueue_params[c].req && !BIT_TEST(paramvec,c)) {
      fprintf(stderr, "*** error: in DISKSIM_IOQUEUE spec -- missing required parameter \"%s\"
", disksim_ioqueue_params[c].name);
      return 0;
    }
  }
} /* end of scope */ 

 