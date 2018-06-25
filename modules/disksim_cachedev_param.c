
#define STACK_MAX 32
{
    int c;
    int needed = 0;
    int param_stack[STACK_MAX];
    int stack_ptr = 0;  // index of first free slot
    BITVECTOR(paramvec, DISKSIM_CACHEDEV_MAX_PARAM);
    bit_zero(paramvec, DISKSIM_CACHEDEV_MAX_PARAM);

    for(c = 0; c < b->params_len; c++) {
	int pnum;
	int i = 0; 
	double d = 0.0; 
	char *s = 0; 
	struct lp_block *blk = 0;
	struct lp_list *l = 0;
	
	if(!b->params[c]) continue;

TOP:
	pnum = lp_param_name(lp_mod_name("disksim_cachedev"), b->params[c]->name);

        // don't initialize things more than once
	if(BIT_TEST(paramvec, pnum)) continue;


	if(stack_ptr > 0) {
	    for(c = 0; c < b->params_len; c++) {
	        if(lp_param_name(lp_mod_name("disksim_cachedev"), b->params[c]->name) == needed)
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
case DISKSIM_CACHEDEV_CACHE_SIZE:
{
if(!(i >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 if(result->validmap || result->dirtymap) { fprintf(stderr, "*** ignoring repeat cache size definition.\n"); } else { result->size = i; result->validmap = (bitstr_t *) DISKSIM_malloc(bitstr_size(result->size)); result->dirtymap = (bitstr_t *) DISKSIM_malloc(bitstr_size(result->size)); ASSERT ((result->validmap != NULL) && (result->dirtymap != NULL)); }
}
} break;
case DISKSIM_CACHEDEV_MAX_REQUEST_SIZE:
{
if(!(i >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->maxreqsize = i;
}
} break;
case DISKSIM_CACHEDEV_WRITE_SCHEME:
{
if(!(RANGE(i,CACHE_WRITE_MIN,CACHE_WRITE_MAX))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->writescheme = i;
}
} break;
case DISKSIM_CACHEDEV_FLUSH_POLICY:
{
if(!(RANGE(i,CACHE_FLUSH_MIN,CACHE_FLUSH_MAX))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->flush_policy = i;
}
} break;
case DISKSIM_CACHEDEV_FLUSH_PERIOD:
{
if(!(d >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->flush_period = d;
}
} break;
case DISKSIM_CACHEDEV_FLUSH_IDLE_DELAY:
{
if(!((d >= 0) || (d == -1.0))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->flush_idledelay = d;
}
} break;
case DISKSIM_CACHEDEV_CACHE_DEVICE:
{
if(!(getdevbyname(s, &result->cache_devno, 0, 0))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_CACHEDEV_CACHED_DEVICE:
{
if(!(getdevbyname(s, &result->real_devno, 0, 0))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
    default: assert(0); break; 
    } /* end of switch */ 
    BIT_SET(paramvec, pnum);
    if(stack_ptr > 0) { c = param_stack[--stack_ptr]; goto TOP; }    } /* end of outer for loop */ 

  for(c = 0; c <= DISKSIM_CACHEDEV_MAX; c++) {
    if(disksim_cachedev_params[c].req && !BIT_TEST(paramvec,c)) {
      fprintf(stderr, "*** error: in DISKSIM_CACHEDEV spec -- missing required parameter \"%s\"
", disksim_cachedev_params[c].name);
      return 0;
    }
  }
} /* end of scope */ 

 