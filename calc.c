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
#include "stats.h"


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
#define CALCTOK_EOP 12
#define CALCTOK_NOP 13
#define CALCTOK_ZOP 14
#define CALCTOK_QOP 15

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
	CalcTok stack[CALCSTACKMAX];
	int pos;
	int error;
} CalcStack;

static int reduce_stack(CalcStack* cs);
static void next_calctoken(const char* str, int *spos, CalcStack *cs, CalcTok *ct);
static CalcStack* init_calcstack();
static void done_calcstack(CalcStack* cs);
static void calcpop(CalcStack* cs, int n);
static CalcTok* calcinspect(CalcStack* cs, int pos);
static void calcpush(CalcStack* cs, CalcTok *ct);

static int si_from_str(const char *str);

double last_res=0;


//XP table
//using exp_lev from hud.c
#define XPT_MAX (MAX_EXP_LEVEL - 1)
#define XPLDIFF(a,b) (exp_lev[b]-exp_lev[a])
#define XPL(a) (exp_lev[a])

//Parsing functions

double calc_exp(const char* str, int *err)
{
	CalcStack* cs;
	CalcTok ct;
	int pos=0;
	double res=0;

	cs = init_calcstack();
	cs->error = CALCERR_OK;
	next_calctoken(str, &pos, cs, &ct);
	while (ct.type != CALCTOK_END && cs->error == CALCERR_OK)
	{
		calcpush(cs, &ct);
		while (reduce_stack(cs))
			/* do nothing */;
		next_calctoken(str, &pos, cs, &ct);
	}
	calcpush(cs, &ct);

	if (cs->error == CALCERR_OK)
	{
		CalcTok *ctr;
		while (reduce_stack(cs))
			/* do nothing */ ;
		ctr = calcinspect(cs, 0);
		if (cs->pos != 1 || ctr == NULL || ctr->type != CALCTOK_NUM)
			cs->error = CALCERR_SYNTAX;
		else
			last_res = res = ctr->value;
	}

	if (err)
		*err = cs->error;

	done_calcstack(cs);

	return res;
}


static int reduce_stack(CalcStack* cs)
{
	CalcTok *cs1,*cs2,*cs3,*cs4, nt;
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
	if (t1 == CALCTOK_NUM && t2 == CALCTOK_LOP)
	{
		int lvl;
		calcpop(cs, 2);
		if (cs1->value >= 0 && cs1->value <= XPT_MAX)
		{
			nt.type = CALCTOK_NUM;
			lvl = (int)trunc(cs1->value);
			nt.value = XPL(lvl) + (cs1->value-lvl) * XPLDIFF(lvl, lvl+1);
			calcpush(cs, &nt);
		}
		else
		{
			cs->error=CALCERR_LOPSYNTAX;
		}
		return 1;
	}
	//X operator
	if (t1 == CALCTOK_NUM && t2 == CALCTOK_XOP)
	{
		int i;
		calcpop(cs, 2);
		if (cs1->value >= 0 && cs1->value <= XPL(XPT_MAX))
		{
			nt.type = CALCTOK_NUM;
			for (i = 0; i <= XPT_MAX; i++)
			{
				if (XPL(i) >= cs1->value)
					break;
			}
			i--;
			nt.value = i + (cs1->value-XPL(i)) / XPLDIFF(i, i+1);
			calcpush(cs, &nt);
		}
		else
		{
			cs->error=CALCERR_XOPSYNTAX;
		}
		return 1;
	}
	//E operator
	if(t1 == CALCTOK_EOP){
		calcpop(cs, 1);
		if(cs1->value < NUM_SKILLS){
			nt.type = CALCTOK_NUM;
			nt.value = *statsinfo[(int)cs1->value].exp;
			calcpush(cs, &nt);
		}
		else
		{
			cs->error=CALCERR_EOPSYNTAX;
		}
		return 1;
	}
	//N operator
	if(t1==CALCTOK_NOP){
		calcpop(cs, 1);
		if(cs1->value < NUM_SKILLS){
			nt.type = CALCTOK_NUM;
			nt.value = *statsinfo[(int)cs1->value].next_lev-*statsinfo[(int)cs1->value].exp;
			calcpush(cs, &nt);
		}
		else
		{
			cs->error = CALCERR_NOPSYNTAX;
		}
		return 1;
	}
	//Z operator
	if(t1==CALCTOK_ZOP){
		calcpop(cs, 1);
		if(cs1->value < NUM_SKILLS){
			nt.type = CALCTOK_NUM;
			nt.value = last_exp[(int)cs1->value];
			calcpush(cs, &nt);
		}
		else
		{
			cs->error = CALCERR_ZOPSYNTAX;
		}
		return 1;
	}
	//Q operator
	if(t1 == CALCTOK_NUM && t2 == CALCTOK_QOP){
		calcpop(cs, 2);
		if(cs1->value <= exp_lev[XPT_MAX]){
			int i=0;
			while ((i <= XPT_MAX) && (cs1->value >= exp_lev[i])) i++;
			nt.type = CALCTOK_NUM;
			nt.value = i-1;
			calcpush(cs, &nt);
		}
		else
		{
			cs->error = CALCERR_QOPSYNTAX;
		}
		return 1;
	}
	//mul
	if (t1 == CALCTOK_NUM && t2 == CALCTOK_MUL && t3 == CALCTOK_NUM)
	{
		calcpop(cs, 3);
		nt.type = CALCTOK_NUM;
		nt.value = cs1->value * cs3->value;
		calcpush(cs, &nt);
		return 1;
	}
	//div
	if (t1 == CALCTOK_NUM && t2 == CALCTOK_DIV && t3 == CALCTOK_NUM)
	{
		calcpop(cs, 3);
		if (cs1->value != 0)
		{
			nt.type = CALCTOK_NUM;
			nt.value = cs3->value / cs1->value;
			calcpush(cs, &nt);
		}
		else
		{
			cs->error = CALCERR_DIVIDE;
		}
		return 1;
	}
	//plus&minus
	if (
	   (t1 == CALCTOK_PLUS || t1 == CALCTOK_MINUS || t1 == CALCTOK_END || t1 == CALCTOK_CPAR) &&
	   t2 == CALCTOK_NUM &&
	   (t3 == CALCTOK_PLUS || t3 == CALCTOK_MINUS) &&
	   t4 == CALCTOK_NUM)
	{
		CalcTok nt1 = *cs1;
		calcpop(cs, 4);
		nt.type = CALCTOK_NUM;
		if (t3==CALCTOK_MINUS)
			nt.value = cs4->value - cs2->value;
		else
			nt.value = cs4->value + cs2->value;
		calcpush(cs, &nt);
		calcpush(cs, &nt1);
		return 1;
	}
	//modulo
	if (t1 == CALCTOK_NUM && t2 == CALCTOK_MOD && t3 == CALCTOK_NUM)
	{
		calcpop(cs, 3);
		if (cs1->value!=0)
		{
			nt.type=CALCTOK_NUM;
			nt.value = fmod(cs3->value, cs1->value);
			calcpush(cs, &nt);
		}
		else
		{
			cs->error = CALCERR_DIVIDE;
		}
		return 1;
	}
	//pars
	if (t1 == CALCTOK_CPAR && t2 == CALCTOK_NUM && t3 == CALCTOK_OPAR)
	{
		CalcTok nt = *cs2;
		calcpop(cs, 3);
		calcpush(cs, &nt);
		return 1;
	}
	//done
	if (t1 == CALCTOK_END && t2 == CALCTOK_NUM && t3 == 0)
	{
		CalcTok nt = *cs2;
		calcpop(cs, 2);
		calcpush(cs, &nt);
		return 1;
	}

	return 0;
}

static void next_calctoken(const char *str, int *spos, CalcStack *cs, CalcTok *ct)
{
	int startpos = 0;
	int pos = *spos;
	char *pp;

	ct->type = CALCTOK_END;
	ct->value = 0;

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
		case 'E':
		case 'e':
			ct->type = CALCTOK_EOP;pos++;
			ct->value = si_from_str(&str[pos]);
			if (ct->value < NUM_SKILLS)
				pos += strlen((char *)statsinfo[(int)ct->value].skillnames->shortname);
			break;
		case 'N':
		case 'n':
			ct->type = CALCTOK_NOP;pos++;
			ct->value = si_from_str(&str[pos]);
			if (ct->value < NUM_SKILLS)
				pos += strlen((char *)statsinfo[(int)ct->value].skillnames->shortname);
			break;
		case 'Z':
		case 'z':
			ct->type = CALCTOK_ZOP;pos++;
			ct->value = si_from_str(&str[pos]);
			if (ct->value < NUM_SKILLS)
				pos += strlen((char *)statsinfo[(int)ct->value].skillnames->shortname);
			break;
		case 'Q':
		case 'q':
			ct->type = CALCTOK_QOP;pos++;
			break;
		case '\0':
			ct->type=CALCTOK_END;
			break;
		default:
			ct->type = CALCTOK_END;
			cs->error = CALCERR_SYNTAX;
			break;
	}
	*spos=pos;
}

int si_from_str(const char *str){
	int i;
	char *shortname;
	for(i = 0; i < NUM_SKILLS; i++) {
		shortname = (char *)statsinfo[i].skillnames->shortname;
		if (0==strncasecmp(str, shortname, strlen(shortname)))
			return i;
	}
	return NUM_SKILLS;
}

//Token stack
static CalcStack* init_calcstack()
{
	CalcStack* cs = malloc(sizeof(CalcStack));
	cs->pos=0;
	return cs;
}

static void done_calcstack(CalcStack* cs)
{
	free(cs);
}

static void calcpush(CalcStack* cs, CalcTok* ct)
{
	if (cs->pos >= CALCSTACKMAX)
		cs->error=CALCERR_MEM;
	else
		cs->stack[cs->pos++] = *ct;
}


static void calcpop(CalcStack *cs, int n)
{
	if (cs->pos < n)
		cs->error = CALCERR_MEM;
	else
		cs->pos -= n;
}

static CalcTok* calcinspect(CalcStack *cs, int p)
{
	if (cs->pos + p - 1 < 0)
		return NULL;
	return cs->stack + (cs->pos + p - 1);
}


/*
int main(int args, char**argv){

	char* test="L0";
	int error;
	double res=calc_exp(test, &error);
	if (trunc(res)==res)	printf("%.0f-----------%i\n",res,error);
	else printf("%.2f-----------%i\n",res,error);
}
*/
