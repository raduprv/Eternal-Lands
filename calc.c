#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifndef _MSC_VER
#include <strings.h>
#endif
#include "calc.h"
#include "hud.h"
#include "platform.h"


#define CALCSTACKMAX 256
#define CALCTOK_OPAR 1
#define CALCTOK_CPAR 2
#define CALCTOK_PLUS 3
#define CALCTOK_MINUS 4
#define CALCTOK_MUL 5
#define CALCTOK_DIV 6
#define CALCTOK_NUM 7
#define CALCTOK_END 8
#define CALCTOK_XOP 9
#define CALCTOK_LOP 10
#define CALCTOK_MOD 11

/*Implementation of #calc command

#calc expr
expr can be any combination of positive (no unary minus) real numbers, [+/-*()] operators and L
Ln (n is a positive real number) returns the xp for lvl n (fractional too: L1.5 is halfway between L1 and L2)
*/

typedef struct caltoken {
	int type;
	double value;
} CalcTok;

typedef struct calstack {
	int pos;
	CalcTok **stack;
	
} CalcStack;

int reduce_stack(CalcStack* cs);
CalcTok* next_calctoken(char* str, int *spos);
CalcStack* init_calcstack();
void done_calcstack(CalcStack* cs);
CalcTok* calcpop(CalcStack* cs);
CalcTok* calcinspect(CalcStack* cs,int pos);
void calcpush(CalcStack* cs, CalcTok* ct);

int calc_error=CALCERR_OK;
double last_res=0;


//XP table
//using exp_lev from hud.c
#define XPT_MAX 179
#define XPLDIFF(a,b) (exp_lev[b]-exp_lev[a])
#define XPL(a) (exp_lev[a])



//Parsing functions

int calc_geterror(){
	int t_err=calc_error;
	calc_error=CALCERR_OK;
	return t_err;
}

double calc_exp(char* str){
	CalcStack* cs;
	CalcTok* ct;
	int pos=0;
	double res=0;
	
	calc_error=CALCERR_OK;
	cs= init_calcstack();
	ct=next_calctoken(str,&pos);
	while(ct->type!=CALCTOK_END && calc_error==CALCERR_OK){
		calcpush(cs,ct);
		while(reduce_stack(cs));
		ct=next_calctoken(str,&pos);
	}
	calcpush(cs,ct);
	
	if (calc_error==CALCERR_OK) {
		while(reduce_stack(cs));
		ct = calcinspect(cs,0);
		if (cs->pos!=1||ct==NULL) calc_error=CALCERR_SYNTAX;
		else 
			if (ct->type==CALCTOK_NUM) last_res = res = ct->value;
			else calc_error=CALCERR_SYNTAX;
	}

	done_calcstack(cs);
	return res;
}


int reduce_stack(CalcStack* cs){
	CalcTok *cs1,*cs2,*cs3,*cs4,*nt;
	int t1,t2,t3,t4;
	
	cs1=calcinspect(cs,0);
	cs2=calcinspect(cs,-1);
	cs3=calcinspect(cs,-2);
	cs4=calcinspect(cs,-3);
	
	t1=(cs1) ? (cs1->type):(0);
	t2=(cs2) ? (cs2->type):(0);
	t3=(cs3) ? (cs3->type):(0);
	t4=(cs4) ? (cs4->type):(0);
	
	//L operator
	if(t1==CALCTOK_NUM&&t2==CALCTOK_LOP){
		int lvl;
		calcpop(cs);calcpop(cs);
		if(cs1->value>=0&&cs1->value<=XPT_MAX){
			nt=(CalcTok*)malloc(sizeof(CalcTok));
			nt->type=CALCTOK_NUM;
			lvl=(int)trunc(cs1->value);
			nt->value=XPL(lvl)+(cs1->value-lvl)*XPLDIFF(lvl,lvl+1);			
			calcpush(cs,nt);
		} else calc_error=CALCERR_LOPSYNTAX;
		free(cs1);free(cs2);
		return 1;
	}	
	//X operator
	if(t1==CALCTOK_NUM&&t2==CALCTOK_XOP){
		int i;
		calcpop(cs);calcpop(cs);
		if(cs1->value>=0&&cs1->value<=XPL(XPT_MAX)){
			nt=(CalcTok*)malloc(sizeof(CalcTok));
			nt->type=CALCTOK_NUM;
			for(i=0;i<=XPT_MAX;i++) {
				if(XPL(i)>=cs1->value) break;
			}
			i--;
			nt->value=i+(cs1->value-XPL(i))/(XPLDIFF(i,i+1));
			calcpush(cs,nt);
		} else calc_error=CALCERR_XOPSYNTAX;
		free(cs1);free(cs2);
		return 1;
	}
	//mul
	if(t1==CALCTOK_NUM&&t2==CALCTOK_MUL&&t3==CALCTOK_NUM){
		calcpop(cs);calcpop(cs);calcpop(cs);
		nt=(CalcTok*)malloc(sizeof(CalcTok));
		nt->type=CALCTOK_NUM;
		nt->value=cs1->value*cs3->value;
		calcpush(cs,nt);
		free(cs1);free(cs2);free(cs3);
		return 1;
	}
	//div
	if(t1==CALCTOK_NUM&&t2==CALCTOK_DIV&&t3==CALCTOK_NUM){
		calcpop(cs);calcpop(cs);calcpop(cs);
		nt=(CalcTok*)malloc(sizeof(CalcTok));
		if(cs1->value!=0){
			nt->type=CALCTOK_NUM;
			nt->value=cs3->value/cs1->value;
		} else calc_error=CALCERR_DIVIDE;
		calcpush(cs,nt);
		free(cs1);free(cs2);free(cs3);
		return 1;
	}
	//plus&minus
	if(
	   (t1==CALCTOK_PLUS||t1==CALCTOK_MINUS||t1==CALCTOK_END||t1==CALCTOK_CPAR)&&
	   t2==CALCTOK_NUM&&
	   (t3==CALCTOK_PLUS||t3==CALCTOK_MINUS)&&
	   t4==CALCTOK_NUM
	   ){
		calcpop(cs);calcpop(cs);calcpop(cs);calcpop(cs);
		nt=(CalcTok*)malloc(sizeof(CalcTok));
		nt->type=CALCTOK_NUM;
		if(t3==CALCTOK_MINUS) nt->value=cs4->value-cs2->value;
		else nt->value=cs4->value+cs2->value;
		calcpush(cs,nt);
		calcpush(cs,cs1);
		free(cs2);free(cs3);free(cs4);
		return 1;
	}
	//modulo
	if(t1==CALCTOK_NUM&&t2==CALCTOK_MOD&&t3==CALCTOK_NUM){
		calcpop(cs);calcpop(cs);calcpop(cs);
		nt=(CalcTok*)malloc(sizeof(CalcTok));
		if(cs1->value!=0){
			nt->type=CALCTOK_NUM;
			nt->value=fmod(cs3->value, cs1->value);
		} else calc_error=CALCERR_DIVIDE;
		calcpush(cs,nt);
		free(cs1);free(cs2);free(cs3);
		return 1;
	}
	//pars
	if(t1==CALCTOK_CPAR&&t2==CALCTOK_NUM&&t3==CALCTOK_OPAR){
		calcpop(cs);calcpop(cs);calcpop(cs);
		calcpush(cs,cs2);
		free(cs1);free(cs3);
		return 1;
	}
	//done
	if(t1==CALCTOK_END&&t2==CALCTOK_NUM&&t3==0){
		calcpop(cs);calcpop(cs);
		calcpush(cs,cs2);
		free(cs1);
		return 1;
	}
	
	return 0;
}



CalcTok* next_calctoken(char *str, int *spos){
	CalcTok* ct = (CalcTok*) malloc(sizeof(CalcTok));
	int startpos = 0;
	int pos = *spos;
	char *pp;
	
	ct->type=CALCTOK_END;
	ct->value=0;
	
	while(str[pos]==' ') pos++;
	startpos=pos;
	switch (str[pos]){
		case '(':
		ct->type=CALCTOK_OPAR;pos++;
		break;
		case ')':
		ct->type=CALCTOK_CPAR;pos++;
		break;
		case '+':
		ct->type=CALCTOK_PLUS;pos++;
		break;
		case '-':
		ct->type=CALCTOK_MINUS;pos++;
		break;
		case '*':
		ct->type=CALCTOK_MUL;pos++;
		break;
		case '/':
		ct->type=CALCTOK_DIV;pos++;
		break;
		case '%':
		ct->type=CALCTOK_MOD;pos++;
		break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			ct->type=CALCTOK_NUM;
			ct->value=strtod(str+startpos,&pp);
			pos=pp-str;
			if (str[pos]=='K'||str[pos]=='k'){
				pos++;
				ct->value*=1000;
			}
			break;
		case 'M':
		case 'm':
			ct->type=CALCTOK_NUM;ct->value=last_res;pos++;
			break;
		case 'X':
		case 'x':
			ct->type=CALCTOK_XOP;pos++;
			break;
		case 'L':
		case 'l':
			ct->type=CALCTOK_LOP;pos++;
			break;
		case '\0':
			ct->type=CALCTOK_END;
			break;
		default:
			ct->type=CALCTOK_END;calc_error=CALCERR_SYNTAX;
			break;
			
	}
	*spos=pos;
	return ct;
}


//Token stack
CalcStack* init_calcstack(){
	CalcStack* cs = (CalcStack*) malloc(sizeof(CalcStack));
	cs->pos=0;
	cs->stack = (CalcTok**)malloc(CALCSTACKMAX*sizeof(CalcTok*));
	return cs;
}

void done_calcstack(CalcStack* cs){
	int i;
	for (i=0;i<cs->pos;i++) free(cs->stack[i]);
	free(cs->stack);
	free(cs);
}

void calcpush(CalcStack* cs, CalcTok* ct){
	if (cs->pos+1>CALCSTACKMAX) {
		calc_error=CALCERR_MEM;
		return;
	}
	cs->stack[cs->pos]=ct;
	cs->pos++;
}


CalcTok* calcpop(CalcStack *cs){
	if (cs->pos<=0) {
		calc_error=CALCERR_MEM;
		return NULL;
	}
	cs->pos--;
	return cs->stack[cs->pos+1];
}

CalcTok* calcinspect(CalcStack *cs, int p){
	if (cs->pos+p-1<0) return NULL;
	return cs->stack[cs->pos+p-1];
}


/*
int main(int args, char**argv){

	char* test="L0";
	double res=calc_exp(test);
	if (trunc(res)==res)	printf("%.0f-----------%i\n",res,calc_error);
	else printf("%.2f-----------%i\n",res,calc_error);
}
*/
