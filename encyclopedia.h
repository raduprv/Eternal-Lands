#ifndef __ENCYCLOPEDIA_H__
#define __ENCYCLOPEDIA_H__

#include <libxml/parser.h>
#include <libxml/tree.h>

typedef struct
{
	char *Name;
}_Category;

typedef struct _Texts
{
	int x,y,size;
	float r,g,b;
	char *text,*ref;
	struct _Texts *Next;
}_Text;

typedef struct _Images
{
	int id,x,y,xend,yend;
	Uint8 mouseover;
	float u,v,uend,vend;
	struct _Images *Next;
}_Image;

typedef struct
{
	char *Name;
	_Text T;
	_Image I;

}_Page;

extern int encyclopedia_win;
extern int encyclopedia_menu_x;
extern int encyclopedia_menu_y;
extern int encyclopedia_menu_x_len;
extern int encyclopedia_menu_y_len;
//extern int encyclopedia_menu_dragged;


void display_encyclopedia();
int encyclopedia_mouse_over();

void ReadCategoryXML(xmlNode * a_node);
void ReadIndexXML(xmlNode * a_node);
void ReadXML(const char *filename);
void FreeXML();
#endif

