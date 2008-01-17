#ifndef __CALC__
#define __CALC__


#define CALCERR_OK 0
#define CALCERR_SYNTAX 1
#define CALCERR_DIVIDE 2
#define CALCERR_MEM 3
#define CALCERR_LOPSYNTAX 4
#define CALCERR_XOPSYNTAX 5


double calc_exp(char* str);
int calc_geterror();

#endif
