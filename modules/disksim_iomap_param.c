
#define STACK_MAX 32
{
    int c;
    int needed = 0;
    int param_stack[STACK_MAX];
    int stack_ptr = 0;  // index of first free slot
    BITVECTOR(paramvec, DISKSIM_IOMAP_MAX_PARAM);
    bit_zero(paramvec, DISKSIM_IOMAP_MAX_PARAM);

    for(c = 0; c < b->params_len; c++) {
	int pnum;
	int i = 0; 
	double d = 0.0; 
	char *s = 0; 
	struct lp_block *blk = 0;
	struct lp_list *l = 0;
	
	if(!b->params[c]) continue;

TOP:
	pnum = lp_param_name(lp_mod_name("disksim_iomap"), b->params[c]->name);

        // don't initialize things more than once
	if(BIT_TEST(paramvec, pnum)) continue;


	if(stack_ptr > 0) {
	    for(c = 0; c < b->params_len; c++) {
	        if(lp_param_name(lp_mod_name("disksim_iomap"), b->params[c]->name) == needed)
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
case DISKSIM_IOMAP_TRACEDEV:
{
 tracemap[n] = i;
} break;
case DISKSIM_IOMAP_SIMDEV:
{
if(!(getdevbyname(s, &tracemap1[n], 0, 0))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_IOMAP_LOCSCALE:
{
if(!((i > 0) && (!(512 % i) || !(i % 512)))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 tracemap2[n] = i;
}
} break;
case DISKSIM_IOMAP_SIZESCALE:
{
if(!(i > 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 tracemap3[n] = i;
}
} break;
case DISKSIM_IOMAP_OFFSET:
{
if(!(i >= 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 tracemap4[n] = i;
}
} break;
    default: assert(0); break; 
    } /* end of switch */ 
    BIT_SET(paramvec, pnum);
    if(stack_ptr > 0) { c = param_stack[--stack_ptr]; goto TOP; }    } /* end of outer for loop */ 

  for(c = 0; c <= DISKSIM_IOMAP_MAX; c++) {
    if(disksim_iomap_params[c].req && !BIT_TEST(paramvec,c)) {
      fprintf(stderr, "*** error: in DISKSIM_IOMAP spec -- missing required parameter \"%s\"
", disksim_iomap_params[c].name);
      return 0;
    }
  }
} /* end of scope */ 

 