
#define STACK_MAX 32
{
    int c;
    int needed = 0;
    int param_stack[STACK_MAX];
    int stack_ptr = 0;  // index of first free slot
    BITVECTOR(paramvec, DISKSIM_SYNTHGEN_MAX_PARAM);
    bit_zero(paramvec, DISKSIM_SYNTHGEN_MAX_PARAM);

    for(c = 0; c < b->params_len; c++) {
	int pnum;
	int i = 0; 
	double d = 0.0; 
	char *s = 0; 
	struct lp_block *blk = 0;
	struct lp_list *l = 0;
	
	if(!b->params[c]) continue;

TOP:
	pnum = lp_param_name(lp_mod_name("disksim_synthgen"), b->params[c]->name);

        // don't initialize things more than once
	if(BIT_TEST(paramvec, pnum)) continue;


	if(stack_ptr > 0) {
	    for(c = 0; c < b->params_len; c++) {
	        if(lp_param_name(lp_mod_name("disksim_synthgen"), b->params[c]->name) == needed)
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
case DISKSIM_SYNTHGEN_STORAGE_CAPACITY_PER_DEVICE:
{
if(!(i > 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 (*result)->sectsperdisk = i;
}
} break;
case DISKSIM_SYNTHGEN_DEVICES:
{
if(!(!loadsynthdevs((*result), l))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_SYNTHGEN_BLOCKING_FACTOR:
{
if(!(i > 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 (*result)->blocksize = i;
}
} break;
case DISKSIM_SYNTHGEN_PROBABILITY_OF_SEQUENTIAL_ACCESS:
{
if(!(RANGE(d,0.0,1.0))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 (*result)->probseq = d;
}
} break;
case DISKSIM_SYNTHGEN_PROBABILITY_OF_LOCAL_ACCESS:
{
if(!(RANGE(d,0.0,1.0))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 (*result)->probloc = d;
}
} break;
case DISKSIM_SYNTHGEN_PROBABILITY_OF_READ_ACCESS:
{
if(!(RANGE(d,0.0,1.0))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 (*result)->probread = d;
}
} break;
case DISKSIM_SYNTHGEN_PROBABILITY_OF_TIME_CRITICAL_REQUEST:
{
if(!(RANGE(d,0.0,1.0))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 (*result)->probtmcrit = d;
}
} break;
case DISKSIM_SYNTHGEN_PROBABILITY_OF_TIME_LIMITED_REQUEST:
{
if(!(RANGE(d,0.0,1.0))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 (*result)->probtmlim = d;
}
} break;
case DISKSIM_SYNTHGEN_TIME_LIMITED_THINK_TIMES:
{
if(!(!loaddistn(l, &(*result)->tmlimit))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_SYNTHGEN_GENERAL_INTER_ARRIVAL_TIMES:
{
if(!(!loaddistn(l, &(*result)->genintr))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_SYNTHGEN_SEQUENTIAL_INTER_ARRIVAL_TIMES:
{
if(!(!loaddistn(l, &(*result)->seqintr))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_SYNTHGEN_LOCAL_INTER_ARRIVAL_TIMES:
{
if(!(!loaddistn(l, &(*result)->locintr))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_SYNTHGEN_LOCAL_DISTANCES:
{
if(!(!loaddistn(l, &(*result)->locdist))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_SYNTHGEN_SIZES:
{
if(!(!loaddistn(l, &(*result)->sizedist))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
    default: assert(0); break; 
    } /* end of switch */ 
    BIT_SET(paramvec, pnum);
    if(stack_ptr > 0) { c = param_stack[--stack_ptr]; goto TOP; }    } /* end of outer for loop */ 

  for(c = 0; c <= DISKSIM_SYNTHGEN_MAX; c++) {
    if(disksim_synthgen_params[c].req && !BIT_TEST(paramvec,c)) {
      fprintf(stderr, "*** error: in DISKSIM_SYNTHGEN spec -- missing required parameter \"%s\"
", disksim_synthgen_params[c].name);
      return 0;
    }
  }
} /* end of scope */ 

 