#ifndef __CALC__
#define __CALC__


#define CALCERR_OK 0
#define CALCERR_SYNTAX 1
#define CALCERR_DIVIDE 2
#define CALCERR_MEM 3
#define CALCERR_LOPSYNTAX 4
#define CALCERR_XOPSYNTAX 5
#define CALCERR_EOPSYNTAX 6
#define CALCERR_NOPSYNTAX 7
#define CALCERR_ZOPSYNTAX 8
#define CALCERR_QOPSYNTAX 9

double calc_exp(const char* str, int *err);

#endif
