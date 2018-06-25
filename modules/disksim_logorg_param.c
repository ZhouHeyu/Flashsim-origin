
#define STACK_MAX 32
{
    int c;
    int needed = 0;
    int param_stack[STACK_MAX];
    int stack_ptr = 0;  // index of first free slot
    BITVECTOR(paramvec, DISKSIM_LOGORG_MAX_PARAM);
    bit_zero(paramvec, DISKSIM_LOGORG_MAX_PARAM);

    for(c = 0; c < b->params_len; c++) {
	int pnum;
	int i = 0; 
	double d = 0.0; 
	char *s = 0; 
	struct lp_block *blk = 0;
	struct lp_list *l = 0;
	
	if(!b->params[c]) continue;

TOP:
	pnum = lp_param_name(lp_mod_name("disksim_logorg"), b->params[c]->name);

        // don't initialize things more than once
	if(BIT_TEST(paramvec, pnum)) continue;


	if(stack_ptr > 0) {
	    for(c = 0; c < b->params_len; c++) {
	        if(lp_param_name(lp_mod_name("disksim_logorg"), b->params[c]->name) == needed)
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
case DISKSIM_LOGORG_ADDRESSING_MODE:
{
if(!(!logorg_addr(result, s))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_LOGORG_DISTRIBUTION_SCHEME:
{
if(!(!logorg_distn(result, s))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_LOGORG_REDUNDANCY_SCHEME:
{
if(!(!logorg_redun(result, s))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_LOGORG_COMPONENTS:
{
} break;
case DISKSIM_LOGORG_DEVICES:
{
if(!(!getlogorgdevs(result,l))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
case DISKSIM_LOGORG_STRIPE_UNIT:
{
if(!(i > 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->stripeunit = i;
}
} break;
case DISKSIM_LOGORG_SYNCH_WRITES_FOR_SAFETY:
{
if(!(RANGE(i,0,1))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->writesync = i;
}
} break;
case DISKSIM_LOGORG_NUMBER_OF_COPIES:
{
if(!(i > 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->copies = i;
}
} break;
case DISKSIM_LOGORG_COPY_CHOICE_ON_READ:
{
if(!(RANGE(i,1,6))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->copychoice = i;
}
} break;
case DISKSIM_LOGORG_RMW_VS_RECONSTRUCT:
{
if(!(RANGE(d,0.0,1.0))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 rmwpoint = d;
}
} break;
case DISKSIM_LOGORG_PARITY_STRIPE_UNIT:
{
if(!(i > 0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->parityunit = i;
}
} break;
case DISKSIM_LOGORG_PARITY_ROTATION_TYPE:
{
if(!(RANGE(i,1,4))) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->rottype = i;
}
} break;
case DISKSIM_LOGORG_TIME_STAMP_INTERVAL:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->stampstart = d;
}
} break;
case DISKSIM_LOGORG_TIME_STAMP_START_TIME:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->stampstart = d;
}
} break;
case DISKSIM_LOGORG_TIME_STAMP_STOP_TIME:
{
if(!(d >= 0.0)) { BADVALMSG(b->params[c]->name); return 0; }
else {
 result->stampstop = d;
}
} break;
case DISKSIM_LOGORG_TIME_STAMP_FILE_NAME:
{
if(!((result->stampfile = fopen(s, "w")))) { BADVALMSG(b->params[c]->name); return 0; }
} break;
    default: assert(0); break; 
    } /* end of switch */ 
    BIT_SET(paramvec, pnum);
    if(stack_ptr > 0) { c = param_stack[--stack_ptr]; goto TOP; }    } /* end of outer for loop */ 

  for(c = 0; c <= DISKSIM_LOGORG_MAX; c++) {
    if(disksim_logorg_params[c].req && !BIT_TEST(paramvec,c)) {
      fprintf(stderr, "*** error: in DISKSIM_LOGORG spec -- missing required parameter \"%s\"
", disksim_logorg_params[c].name);
      return 0;
    }
  }
} /* end of scope */ 

 