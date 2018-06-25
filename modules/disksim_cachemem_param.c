
#define STACK_MAX 32
{
    int c;
    int needed = 0;
    int param_stack[STACK_MAX];
    int stack_ptr = 0;  // index of first free slot
    BITVECTOR(paramvec, DISKSIM_CACHEMEM_MAX_PARAM);
    bit_zero(paramvec, DISKSIM_CACHEMEM_MAX_PARAM);

    for(c = 0; c < b->params_len; c++) {
	int pnum;
	int i = 0; 
	double d = 0.0; 
	char *s = 0; 
	struct lp_block *blk = 0;
	struct lp_list *l = 0;
	
	if(!b->params[c]) continue;

TOP:
	pnum = lp_param_name(lp_mod_name("disksim_cachemem"), b->params[c]->name);

        // don't initialize things more than once
	if(BIT_TEST(paramvec, pnum)) continue;


	if(stack_ptr > 0) {
	    for(c = 0; c < b->params_len; c++) {
	        if(lp_param_name(lp_mod_name("disksim_cachemem"), b->params[c]->name) == needed)
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
case DISKSIM_CACHEMEM_CACHE_SIZE:
{
if(!(i >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 if(result->map) { fprintf(stderr, "*** ignoring repeat cache size definition.\n"); } else { int c2; result->size = i; result->mapmask = 0; result->map = DISKSIM_malloc((result->mapmask+1)*sizeof(cache_mapentry)); result->map[0].freelist = NULL; for (c2=0; c2<CACHE_MAXSEGMENTS; c2++) { result->map[0].maxactive[c2] = result->size; result->map[0].lru[c2] = NULL; } }
}
} break;
case DISKSIM_CACHEMEM_SLRU_SEGMENTS:
{
 cachemem_setup_segs(result, l);
} break;
case DISKSIM_CACHEMEM_LINE_SIZE:
{
if(!(i >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->linesize = i;
}
} break;
case DISKSIM_CACHEMEM_BIT_GRANULARITY:
{
if(!(i > 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->atomsperbit = i;
}
} break;
case DISKSIM_CACHEMEM_LOCK_GRANULARITY:
{
if(!(i > 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->lockgran = i;
}
} break;
case DISKSIM_CACHEMEM_SHARED_READ_LOCKS:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->sharedreadlocks = i;
}
} break;
case DISKSIM_CACHEMEM_MAX_REQUEST_SIZE:
{
if(!(i >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->maxreqsize = i;
}
} break;
case DISKSIM_CACHEMEM_REPLACEMENT_POLICY:
{
if(!(RANGE(i,CACHE_REPLACE_MIN,CACHE_REPLACE_MAX))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->replacepolicy = i;
}
} break;
case DISKSIM_CACHEMEM_ALLOCATION_POLICY:
{
if(!(RANGE(i,CACHE_ALLOCATE_MIN,CACHE_ALLOCATE_MAX))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->allocatepolicy = i;
}
} break;
case DISKSIM_CACHEMEM_WRITE_SCHEME:
{
if(!(RANGE(i,CACHE_WRITE_MIN,CACHE_WRITE_MAX))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->writescheme = i;
}
} break;
case DISKSIM_CACHEMEM_FLUSH_POLICY:
{
if(!(RANGE(i,CACHE_FLUSH_MIN,CACHE_FLUSH_MAX))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->flush_policy = i;
}
} break;
case DISKSIM_CACHEMEM_FLUSH_PERIOD:
{
if(!(d >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->flush_period = d;
}
} break;
case DISKSIM_CACHEMEM_FLUSH_IDLE_DELAY:
{
if(!((d >= 0) || (d == -1.0))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->flush_idledelay = d;
}
} break;
case DISKSIM_CACHEMEM_FLUSH_MAX_LINE_CLUSTER:
{
if(!(i >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->flush_maxlinecluster = i;
}
} break;
case DISKSIM_CACHEMEM_READ_PREFETCH_TYPE:
{
if(!(RANGE(i,CACHE_PREFETCH_MIN,CACHE_PREFETCH_MAX))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->read_prefetch_type = i;
}
} break;
case DISKSIM_CACHEMEM_WRITE_PREFETCH_TYPE:
{
if(!(RANGE(i,CACHE_PREFETCH_MIN,CACHE_PREFETCH_MAX))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->writefill_prefetch_type = i;
}
} break;
case DISKSIM_CACHEMEM_LINE_BY_LINE_FETCHES:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->read_line_by_line = i; result->write_line_by_line = i;
}
} break;
case DISKSIM_CACHEMEM_MAX_GATHER:
{
if(!(i >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->maxscatgath = i;
}
} break;
    default: assert(0); break; 
    } /* end of switch */ 
    BIT_SET(paramvec, pnum);
    if(stack_ptr > 0) { c = param_stack[--stack_ptr]; goto TOP; }    } /* end of outer for loop */ 

  for(c = 0; c <= DISKSIM_CACHEMEM_MAX; c++) {
    if(disksim_cachemem_params[c].req && !BIT_TEST(paramvec,c)) {
      fprintf(stderr, "*** error: in DISKSIM_CACHEMEM spec -- missing required parameter \"%s\"
", disksim_cachemem_params[c].name);
      return 0;
    }
  }
} /* end of scope */ 

 