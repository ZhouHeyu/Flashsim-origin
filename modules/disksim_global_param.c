
#define STACK_MAX 32
{
    int c;
    int needed = 0;
    int param_stack[STACK_MAX];
    int stack_ptr = 0;  // index of first free slot
    BITVECTOR(paramvec, DISKSIM_GLOBAL_MAX_PARAM);
    bit_zero(paramvec, DISKSIM_GLOBAL_MAX_PARAM);

    for(c = 0; c < b->params_len; c++) {
	int pnum;
	int i = 0; 
	double d = 0.0; 
	char *s = 0; 
	struct lp_block *blk = 0;
	struct lp_list *l = 0;
	
	if(!b->params[c]) continue;

TOP:
	pnum = lp_param_name(lp_mod_name("disksim_global"), b->params[c]->name);

        // don't initialize things more than once
	if(BIT_TEST(paramvec, pnum)) continue;


	if(stack_ptr > 0) {
	    for(c = 0; c < b->params_len; c++) {
	        if(lp_param_name(lp_mod_name("disksim_global"), b->params[c]->name) == needed)
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
case DISKSIM_GLOBAL_INIT_SEED:
{
 disksim->seedval = i;
 DISKSIM_srand48(disksim->seedval);
} break;
case DISKSIM_GLOBAL_INIT_SEED_WITH_TIME:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 if(i) { disksim->seedval = DISKSIM_time(); }
}
} break;
case DISKSIM_GLOBAL_REAL_SEED:
{
 disksim->seedval = i;
} break;
case DISKSIM_GLOBAL_REAL_SEED_WITH_TIME:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 disksim->seedval = DISKSIM_time();
}
} break;
case DISKSIM_GLOBAL_STATISTIC_WARM_UP_TIME:
{
if(!((d >= 0))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 disksim->warmup_event = (timer_event *) getfromextraq();
}
 disksim->warmup_event->type = TIMER_EXPIRED;
 disksim->warmup_event->time = d * (double) 1000.0;
 disksim->warmup_event->func = &disksim->timerfunc_disksim;
} break;
case DISKSIM_GLOBAL_STATISTIC_WARM_UP_IOS:
{
if(!((i >= 0))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 disksim->warmup_iocnt = i;
}
} break;
case DISKSIM_GLOBAL_STAT_DEFINITION_FILE:
{
 statdeffile = fopen(s, "r");
 ddbg_assert2(statdeffile != 0, "failed to open statdefs file!");
} break;
case DISKSIM_GLOBAL_OUTPUT_FILE_FOR_TRACE_OF_IO_REQUESTS_SIMULATED:
{
if(!((outios = fopen(s, "w")) != NULL)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 strcpy(disksim->outiosfilename, s);
}
} break;
    default: assert(0); break; 
    } /* end of switch */ 
    BIT_SET(paramvec, pnum);
    if(stack_ptr > 0) { c = param_stack[--stack_ptr]; goto TOP; }    } /* end of outer for loop */ 

  for(c = 0; c <= DISKSIM_GLOBAL_MAX; c++) {
    if(disksim_global_params[c].req && !BIT_TEST(paramvec,c)) {
      fprintf(stderr, "*** error: in DISKSIM_GLOBAL spec -- missing required parameter \"%s\"
", disksim_global_params[c].name);
      return 0;
    }
  }
} /* end of scope */ 

 