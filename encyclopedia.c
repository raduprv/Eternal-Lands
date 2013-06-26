#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "encyclopedia.h"
#include "asc.h"
#include "elwindows.h"
#include "errors.h"
#include "init.h"
#include "interface.h"
#include "misc.h"
#include "platform.h"
#include "textures.h"
#include "url.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#ifdef ENCYCL_NAVIGATION
#include "context_menu.h"
#include "gamewin.h"
#include "hud.h"
#include "notepad.h"
#include "tabs.h"
#endif // ENCYCL_NAVIGATION
#include "translate.h"
#include "text.h"


int encyclopedia_win=-1;
int encyclopedia_menu_x=100;
int encyclopedia_menu_y=20;
int encyclopedia_menu_x_len=500;
int encyclopedia_menu_y_len=350;
//int encyclopedia_menu_dragged=0;
int encyclopedia_scroll_id=0;

_Category Category[100];
_Page Page[MAX_ENC_PAGES];
int num_category=0,numpage=-1,numtext,x,y,numimage,id,color,size,ref,currentpage=0,isize,tsize,tid,ssize,mouseover=0,xposupdate,yposupdate,lastextlen=0;
float u,v,uend,vend,xend,yend,r,g,b;
char *s,*ss;

#ifdef ENCYCL_NAVIGATION
static void save_raw_page_link(const char *link, const char *title, size_t from_page_index);
static void find_page(const char *search_title, void *data);
static void encycl_nav_free(void);
static char * last_search = NULL;
static int repeat_search = 0;
static int show_cm_help = 0;
/* move to translate */
static const char* cm_encycl_help_str = "Right-click for search and bookmark options";
#endif // ENCYCL_NAVIGATION

int display_encyclopedia_handler(window_info *win)
{
	_Text *t=Page[currentpage].T.Next;
	_Image *i=Page[currentpage].I.Next;
	int j;

	j=vscrollbar_get_pos(encyclopedia_win, encyclopedia_scroll_id);

	while(t)
	{
		int ylen=(t->size)?18:15;
		int xlen=strlen(t->text)*((t->size)?11:8);

		// Bounds Check the Text
		if((t->y-j > 0) && (t->y-j < encyclopedia_menu_y_len-20 ))
		{
			if(t->ref)
				{
					//draw a line
					glColor3f(0.5,0.5,0.5);
					glDisable(GL_TEXTURE_2D);
					glBegin(GL_LINES);
					glVertex3i(t->x+4,t->y+ylen-j,0);
					glVertex3i(t->x+4+xlen-8,t->y+ylen-j,0);
					glEnd();
					glEnable(GL_TEXTURE_2D);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
				}
			if(t->size)
				{
					if(t->ref && mouse_x>(t->x+win->cur_x) && mouse_x<(t->x+xlen+win->cur_x) && mouse_y>(t->y+win->cur_y-j) && mouse_y<(t->y+ylen+win->cur_y-j))
					glColor3f(0.3,0.6,1.0);
					else
					glColor3f(t->r,t->g,t->b);
					draw_string(t->x,t->y-j,(unsigned char*)t->text,1);
				}
			else
				{
					if(t->ref && mouse_x>(t->x+win->cur_x) && mouse_x<(t->x+xlen+win->cur_x) && mouse_y>(t->y+win->cur_y-j) && mouse_y<(t->y+ylen+win->cur_y-j))
					glColor3f(0.3,0.6,1.0);
					else
					glColor3f(t->r,t->g,t->b);
					draw_string_small(t->x,t->y-j,(unsigned char*)t->text,1);
				}
		}
		// No next line?
		if(!t->Next)
			break;

		t=t->Next;
	}

	glColor3f(1.0f,1.0f,1.0f);
	while(i){
		// Bounds Check the Text
		if((i->y-j > 0) && (i->yend-j < encyclopedia_menu_y_len-40 ))
		{
			if(i->mouseover==1)
			{
				i=i->Next;
				continue;
			}
			if(mouse_x>(i->x+win->cur_x) && mouse_x<(win->cur_x+i->xend) && mouse_y>(i->y+win->cur_y-j) && mouse_y<(win->cur_y-j+i->yend))
			{
				if(i->Next!=NULL)
				{
					if(i->Next->mouseover==1)
						i=i->Next;
				}
			}
#ifdef	NEW_TEXTURES
			bind_texture(i->id);
#else	/* NEW_TEXTURES */
			get_and_set_texture_id(i->id);
#endif	/* NEW_TEXTURES */
			glBegin(GL_QUADS);
			draw_2d_thing(i->u, i->v, i->uend, i->vend,i->x, i->y-j,i->xend,i->yend-j);
			glEnd();
		}
		i=i->Next;

	}

#ifdef ENCYCL_NAVIGATION
	if (repeat_search && last_search != NULL)
	{
		find_page(last_search, NULL);
		repeat_search = 0;
	}
	if (show_cm_help)
	{
		show_help(cm_encycl_help_str, 0, win->len_y+10);
		show_cm_help = 0;
	}
#endif

	return 1;
}

int click_encyclopedia_handler(window_info *win, int mx, int my, Uint32 flags)
{
	_Text *t=Page[currentpage].T.Next;
	
	if(flags&ELW_WHEEL_UP) {
		vscrollbar_scroll_up(encyclopedia_win, encyclopedia_scroll_id);
	} else if(flags&ELW_WHEEL_DOWN) {
		vscrollbar_scroll_down(encyclopedia_win, encyclopedia_scroll_id);
	} else {
		int j = vscrollbar_get_pos(encyclopedia_win, encyclopedia_scroll_id);

		while(t){
			int xlen=strlen(t->text)*((t->size)?11:8),ylen=(t->size)?18:15;
			if(t->ref && mx>(t->x) && mx<(t->x+xlen) && my>(t->y-j) && my<(t->y+ylen-j)){
				// check if its a webpage
				if (!strncasecmp(t->ref, "http://", 7)) {
					open_web_link(t->ref);
				} else {
					//changing page
					int i;
					for(i=0;i<numpage+1;i++){
						if(!xmlStrcasecmp((xmlChar*)Page[i].Name,(xmlChar*)t->ref)){
							currentpage=i;
							vscrollbar_set_pos(encyclopedia_win, encyclopedia_scroll_id, 0);
							vscrollbar_set_bar_len(encyclopedia_win, encyclopedia_scroll_id, Page[currentpage].max_y);
							break;
						}
					}
				}
				break;
			}
			t=t->Next;
		}
	}

	return 1;
}

void GetColorFromName (const xmlChar *t)
{
	if(!xmlStrcasecmp((xmlChar*)"silver",t)){r=192/255.0f; g=192/255.0f; b=192/255.0f;return;}
	if(!xmlStrcasecmp((xmlChar*)"grey",t)){r=128/255.0f; g=128/255.0f; b=128/255.0f;return;}
	if(!xmlStrcasecmp((xmlChar*)"maroon",t)){r=128/255.0f; g=0.0f; b=0.0f;return;}
	if(!xmlStrcasecmp((xmlChar*)"green",t)){r=0.0f; g=128/255.0f; b=0.0f;return;}
	if(!xmlStrcasecmp((xmlChar*)"navy",t)){r=0.0f; g=0.0f; b=128/255.0f;return;}
	if(!xmlStrcasecmp((xmlChar*)"olive",t)){r=128/255.0f; g=128/255.0f; b=0.0f;return;}
	if(!xmlStrcasecmp((xmlChar*)"purple",t)){r=128/255.0f; g=0.0f; b=128/255.0f;return;}
	if(!xmlStrcasecmp((xmlChar*)"teal",t)){r=0.0f; g=128/255.0f; b=128/255.0f;return;}
	if(!xmlStrcasecmp((xmlChar*)"white",t)){r=1.0f; g=1.0f; b=1.0f;return;}
	if(!xmlStrcasecmp((xmlChar*)"black",t)){r=0.0f; g=0.0f; b=0.0f;return;}
	if(!xmlStrcasecmp((xmlChar*)"red",t)){r=1.0f; g=0.0f; b=0.0f;return;}
	if(!xmlStrcasecmp((xmlChar*)"lime",t)){r=0.0f; g=1.0f; b=0.0f;return;}
	if(!xmlStrcasecmp((xmlChar*)"blue",t)){r=0.0f; g=0.0f; b=1.0f;return;}
	if(!xmlStrcasecmp((xmlChar*)"magenta",t)){r=1.0f; g=0.0f; b=1.0f;return;}
	if(!xmlStrcasecmp((xmlChar*)"yellow",t)){r=1.0f; g=1.0f; b=0.0f;return;}
	if(!xmlStrcasecmp((xmlChar*)"cyan",t)){r=0.0f; g=1.0f; b=1.0f;return;}
}


// XML
void ParseLink(xmlAttr *a_node)
{
	xmlAttr *cur_attr=NULL;

	for (cur_attr = a_node; cur_attr; cur_attr = cur_attr->next) {
		if (cur_attr->type==XML_ATTRIBUTE_NODE){
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"ref")){
				ss=(char*)cur_attr->children->content;
			}
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"title")){
				s=(char*)cur_attr->children->content;
			}
			//x=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"x")){
				x=atoi((char*)cur_attr->children->content);
			}
			//y=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"y")){
				y=atoi((char*)cur_attr->children->content);
			}
		}
	}
}

void ParseColor(xmlAttr *a_node)
{
	xmlAttr *cur_attr=NULL;

	for (cur_attr = a_node; cur_attr; cur_attr = cur_attr->next) {
		if (cur_attr->type==XML_ATTRIBUTE_NODE){
			//r=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"r")){
				r=atof((char*)cur_attr->children->content);
			}
			//g=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"g")){
				g=atof((char*)cur_attr->children->content);
			}
			//b=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"b")){
				b=atof((char*)cur_attr->children->content);
			}
		}
	}
}

void ParseImage(xmlAttr *a_node)
{
	xmlAttr *cur_attr=NULL;

	for (cur_attr = a_node; cur_attr; cur_attr = cur_attr->next) {
		if (cur_attr->type==XML_ATTRIBUTE_NODE){
			//u=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"u")){
				u=(float)atof((char*)cur_attr->children->content);
			}
			//v=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"v")){
				v=(float)atof((char*)cur_attr->children->content);
			}
			//uend=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"uend")){
				uend=(float)atof((char*)cur_attr->children->content);
			}
			//vend=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"vend")){
				vend=(float)atof((char*)cur_attr->children->content);
			}
			//xend=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"xlen")){
				xend=(float)atof((char*)cur_attr->children->content);
			}
			//yend=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"ylen")){
				yend=(float)atof((char*)cur_attr->children->content);
			}
			//name=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"name")){
#ifdef	NEW_TEXTURES
				id = load_texture_cached((char*)cur_attr->children->content, tt_gui);
#else	/* NEW_TEXTURES */
				id=load_texture_cache_deferred((char*)cur_attr->children->content,0);
#endif	/* NEW_TEXTURES */
			}
			//x=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"x")){
				x=atoi((char*)cur_attr->children->content);
			}
			//y=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"y")){
				y=atoi((char*)cur_attr->children->content);
			}
			//mouseover=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"mouseover")){
				mouseover=atoi((char*)cur_attr->children->content);
			}
			//xposupdate=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"xposupdate")){
				xposupdate=atoi((char*)cur_attr->children->content);
			}
			//yposupdate=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"yposupdate")){
				yposupdate=atoi((char*)cur_attr->children->content);
			}
		}
	}
}

void ParseSimage(xmlAttr *a_node)
{
	xmlAttr *cur_attr=NULL;

	for (cur_attr = a_node; cur_attr; cur_attr = cur_attr->next) {
		if (cur_attr->type==XML_ATTRIBUTE_NODE){
			//name=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"name")){
#ifdef	NEW_TEXTURES
				id = load_texture_cached((char*)cur_attr->children->content, tt_gui);
#else	/* NEW_TEXTURES */
				id=load_texture_cache_deferred((char*)cur_attr->children->content,0);
#endif	/* NEW_TEXTURES */
			}
			//isize=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"isize")){
				isize=atoi((char*)cur_attr->children->content);
			}
			//tsize=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"tsize")){
				tsize=atoi((char*)cur_attr->children->content);
			}
			//tid=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"tid")){
				tid=atoi((char*)cur_attr->children->content);
			}
			//size=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"size")){
				ssize=atoi((char*)cur_attr->children->content);
			}
			//x=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"x")){
				x=atoi((char*)cur_attr->children->content);
			}
			//y=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"y")){
				y=atoi((char*)cur_attr->children->content);
			}
			//mouseover=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"mouseover")){
				mouseover=atoi((char*)cur_attr->children->content);
			}
			//xposupdate=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"xposupdate")){
				xposupdate=atoi((char*)cur_attr->children->content);
			}
			//yposupdate=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"yposupdate")){
				yposupdate=atoi((char*)cur_attr->children->content);
			}

		}
	}
}

void ParsePos(xmlAttr *a_node)
{
	xmlAttr *cur_attr=NULL;

	for (cur_attr = a_node; cur_attr; cur_attr = cur_attr->next) {
		if (cur_attr->type==XML_ATTRIBUTE_NODE){
			//x=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"x")){
				x=atoi((char*)cur_attr->children->content);
			}
			//y=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"y")){
				y=atoi((char*)cur_attr->children->content);
			}
		}
	}
}

void ParsePage(xmlAttr *a_node)
{
	xmlAttr *cur_attr=NULL;

	for (cur_attr = a_node; cur_attr; cur_attr = cur_attr->next) {
		if (cur_attr->type==XML_ATTRIBUTE_NODE){
			//name=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"name")){
				Page[numpage].Name=NULL;
				MY_XMLSTRCPY(&Page[numpage].Name, (char*)cur_attr->children->content);
			}
		}
	}
}

void ParseText(xmlAttr *a_node)
{
	xmlAttr *cur_attr=NULL;

	for (cur_attr = a_node; cur_attr; cur_attr = cur_attr->next) {
		if (cur_attr->type==XML_ATTRIBUTE_NODE){
			//x=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"x")){
				x=atoi((char*)cur_attr->children->content);
			}
			//y=""
			if(!xmlStrcasecmp(cur_attr->name,(xmlChar*)"y")){
				y=atoi((char*)cur_attr->children->content);
			}

		}
	}
}

void ReadCategoryXML(xmlNode * a_node)
{
	xmlNode *cur_node=NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		if (cur_node->type==XML_ELEMENT_NODE){
			//<Page>
			if(!xmlStrcasecmp(cur_node->name,(xmlChar*)"Page")){
				if(numpage < MAX_ENC_PAGES-1){
					numpage++;
					numtext=0;
					numimage=0;
					x=2;
					y=2;
					ParsePage(cur_node->properties);
				} else {
					LOG_ERROR("Too many Enc Pages, limit of %d hit", MAX_ENC_PAGES);
					return;
				}
			}

			//<Size>
			if(!xmlStrcasecmp(cur_node->name,(xmlChar*)"Size"))
			{
				if (cur_node->children != NULL)
					size = (xmlStrcasecmp ((xmlChar*)"Big", cur_node->children->content) == 0) ? 1 : 0;
			}

			//<Color>
			if(!xmlStrcasecmp(cur_node->name,(xmlChar*)"Color")){
				ParseColor(cur_node->properties);
				if (cur_node->children != NULL)
					GetColorFromName (cur_node->children->content);
			}

			//<Text>
			if(!xmlStrcasecmp(cur_node->name,(xmlChar*)"Text")){
				_Text *T=(_Text*)malloc(sizeof(_Text));
				_Text *t=&Page[numpage].T;
				T->Next=NULL;
				ParseText(cur_node->properties);
				T->x=x;
				T->y=y;
				T->size=size;
				T->r=r; T->g=g; T->b=b;
				T->text=NULL;
				T->ref=NULL;
				lastextlen = 0;
				if (cur_node->children != NULL)
				{
					MY_XMLSTRCPY (&T->text, (char*)cur_node->children->content);
					lastextlen = strlen (T->text) * ((T->size) ? 11 : 8);
					x += lastextlen;
				}
				while (t->Next != NULL)
					t = t->Next;
				t->Next = T;
			}

			//<nl>
			if(!xmlStrcasecmp(cur_node->name,(xmlChar*)"nl")){
				x=2;
				y+=(size)?18:15;
			}
			
			//<nlkx>
			if(!xmlStrcasecmp(cur_node->name,(xmlChar*)"nlkx")){
				y+=(size)?18:15;
				x-=lastextlen;
			}

			//<Image>
			if(!xmlStrcasecmp(cur_node->name,(xmlChar*)"image")){
				_Image *I=(_Image*)malloc(sizeof(_Image));
				_Image *i=&Page[numpage].I;
				xposupdate=1; yposupdate=1;
				ParseImage(cur_node->properties);
				I->mouseover=mouseover;
				mouseover=0;
			
				while(i->Next!=NULL)i=i->Next;
				I->id=id;
				I->Next=NULL;
				if(!I->mouseover){
					I->x=x;
					I->y=y;
					I->xend=x+xend;
					I->yend=y+yend;
					if(xposupdate)
						x+=xend;
					if(yposupdate)
						y+=yend-((size)?18:15);
					
				}else{
					I->x=i->x;
					I->y=i->y;
					I->xend=i->xend;
					I->yend=i->yend;
				}
				I->u=u;
				I->v=v;
				I->uend=uend;
				I->vend=vend;
				i->Next=I;
				numimage++;
			}

			//<sImage>
			if(!xmlStrcasecmp(cur_node->name,(xmlChar*)"simage")){
				_Image *I=(_Image*)malloc(sizeof(_Image));
				_Image *i=&Page[numpage].I;
				int picsperrow,xtile,ytile;
				float ftsize;
				xposupdate=1; yposupdate=1;
				ParseSimage(cur_node->properties);

				picsperrow=isize/tsize;
				xtile=tid%picsperrow;
				ytile=tid/picsperrow;
				ftsize=(float)tsize/isize;
#ifdef	NEW_TEXTURES
				u = ftsize * xtile;
				v = ftsize * ytile;
				uend = u + ftsize;
				vend = v + ftsize;
#else	/* NEW_TEXTURES */
				u=ftsize*xtile;
				v=-ftsize*ytile;
				uend=u+ftsize;
				vend=v-ftsize;
#endif	/* NEW_TEXTURES */
				I->mouseover=mouseover;
				mouseover=0;
				
				while(i->Next!=NULL)i=i->Next;
				I->id=id;
				I->Next=NULL;
				if(!I->mouseover){
					I->x=x;
					I->y=y;
					I->xend=x+(tsize*((float)ssize/100));
					I->yend=y+(tsize*((float)ssize/100));
					if(xposupdate)
						x+=(tsize*((float)ssize/100));
					if(yposupdate)
						y+=(tsize*((float)ssize/100))-((size)?18:15);
				}else{
					I->x=i->x;
					I->y=i->y;
					I->xend=i->xend;
					I->yend=i->yend;
				}
				I->u=u;
				I->v=v;
				I->uend=uend;
				I->vend=vend;
				i->Next=I;
				
				numimage++;
			}

			//<ddsImage>
			if(!xmlStrcasecmp(cur_node->name,(xmlChar*)"ddsimage")){
				_Image *I=(_Image*)malloc(sizeof(_Image));
				_Image *i=&Page[numpage].I;
				int picsperrow,xtile,ytile;
				float ftsize;
				xposupdate=1; yposupdate=1;
				ParseSimage(cur_node->properties);
				if(size==99)
					size=99;

				picsperrow=isize/tsize;
				xtile=tid%picsperrow;
				ytile=tid/picsperrow;
				ftsize=(float)tsize/isize;
				u=ftsize*xtile;
				v=ftsize*ytile;
				uend=u+ftsize;
				vend=v+ftsize;
				I->mouseover=mouseover;
				mouseover=0;

				while(i->Next!=NULL)i=i->Next;
					I->id=id;
				I->Next=NULL;
				if(!I->mouseover){
					I->x=x;
					I->y=y;
					I->xend=x+(tsize*((float)ssize/100));
					I->yend=y+(tsize*((float)ssize/100));
					if(xposupdate)
						x+=(tsize*((float)ssize/100));
					if(yposupdate)
						y+=(tsize*((float)ssize/100))-((size)?18:15);
				}else{
					I->x=i->x;
					I->y=i->y;
					I->xend=i->xend;
					I->yend=i->yend;
				}
				I->u=u;
				I->v=v;
				I->uend=uend;
				I->vend=vend;
				i->Next=I;

				numimage++;
			}

			//<Pos>
			if(!xmlStrcasecmp(cur_node->name,(xmlChar*)"pos")){
				ParsePos(cur_node->properties);
			}

			//<link>
			if(!xmlStrcasecmp(cur_node->name,(xmlChar*)"link")){
				_Text *T=(_Text*)malloc(sizeof(_Text));
				_Text *t=&Page[numpage].T;
				ParseLink(cur_node->properties);
				T->Next=NULL;
				T->x=x;
				T->y=y;
				T->size=size;
				T->r=r; T->g=g; T->b=b;
				T->text=NULL;
				T->ref=NULL;
				MY_XMLSTRCPY(&T->text, s);
				MY_XMLSTRCPY(&T->ref, ss);
				while(t->Next!=NULL)t=t->Next;
				t->Next=T;
				x+=strlen(T->text)*((T->size)?11:8);
				lastextlen=strlen(T->text)*((T->size)?11:8);
#ifdef ENCYCL_NAVIGATION
				save_raw_page_link(T->ref, T->text, numpage);
#endif
			}
			// See if this is the new maximum length.
			if(Page[numpage].max_y < y)
			{
				Page[numpage].max_y = y;
			}
		}

		ReadCategoryXML(cur_node->children);
	}
}

void ReadIndexXML(xmlNode * a_node)
{
	xmlNode *cur_node=NULL;

	for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
		if (cur_node->type==XML_ELEMENT_NODE){
			if(!xmlStrcasecmp(cur_node->name,(xmlChar*)"Category")){
				xmlDocPtr doc;
				char tmp[100];
				Category[num_category].Name=NULL;
				MY_XMLSTRCPY(&Category[num_category].Name, (char*)cur_node->children->content);
				num_category++;

				//we load the category now
				safe_snprintf(tmp,sizeof(tmp),"languages/%s/Encyclopedia/%s.xml",lang,cur_node->children->content);
				doc=xmlReadFile(tmp, NULL, 0);
				if (doc==NULL)
					{
						safe_snprintf(tmp,sizeof(tmp),"languages/en/Encyclopedia/%s.xml",cur_node->children->content);
						doc=xmlReadFile(tmp, NULL, 0);
					}
				if(doc==NULL)
					{
						return;
					}
				ReadCategoryXML(xmlDocGetRootElement(doc));
				xmlFreeDoc(doc);

			}
		}

		ReadIndexXML(cur_node->children);
	}
}

void ReadXML(const char *filename)
{
	int i;
	
	xmlDocPtr doc=xmlReadFile(filename, NULL, 0);
	if (doc==NULL)
		return;

	ReadIndexXML(xmlDocGetRootElement(doc));
	xmlFreeDoc(doc);
	
	// Sanitize all of the page lengths.
	for (i = 0; i < numpage; i++) {
		if(Page[i].max_y > encyclopedia_menu_y_len - ENCYC_OFFSET)
		{
			Page[i].max_y -= encyclopedia_menu_y_len - ENCYC_OFFSET;
		} else {
			Page[i].max_y = 0;
		}
	}
}

void FreeXML()
{
	int i;
	//free categories
	for(i=0;i<num_category;i++)
		free(Category[i].Name);

	//Free Pages
	for(i=0;i<numpage+1;i++){
		_Text *t=Page[i].T.Next;
		_Image *tt=Page[i].I.Next;
		free(Page[i].Name);
		while(t!=NULL){
			_Text *tmp=t;
			if(t->ref)free(t->ref);
			if(t->text)free(t->text);
			t=t->Next;
			free(tmp);
		}
		while(tt!=NULL){
			_Image *tmp=tt;
			tt=tt->Next;
			free(tmp);
		}
	}
#ifdef ENCYCL_NAVIGATION
	encycl_nav_free();
#endif
}


#ifdef ENCYCL_NAVIGATION
/*
 *	Functions to search and navigate to encyclopedia pages.
 *	All links are save into a list with the link text used as the title.
 *	Function are provided via a context menu on the encyclopedia window.
 *
 *	Search is across all the titles, an exact match opens the page
 *	immediately, as does a single partial match.  For multiple matches,
 *	a context menu is opened with the first few matches show.  Clicking
 *	on a line in the menu navigates to that page.
 *
 *	A bookmark is a link to the a page, available from a context menu.
 *	There are options to save a bookmark or remove a bookmark for the
 *	current displayed page.  Bookmarks are no saved at client exit.
 *
 *	pjbroad/bluap Feb 2011.
 *
 * TODO
 *		Move strings to translate.
 * 		Could add window to all three help sections help, skills, encycl
 * 			already have which pages belong to which tabs, we just throw all but encycl away
*/

/* a structure to save links, i.e. page name and title */
struct PAGE_LINK
{
	const char *link;
	const char *title;
	size_t from_page_index;
};

/* move to translate */
static const char* cm_encycl_base_str = "Encyclopedia Index\nSearch Encyclopedia Titles\nRepeat Last Search\nBookmark This Page\nUnbookmark This Page\nClear Bookmarks";
static const char* encycl_search_propmt = "Enter text to find";

#define MAX_SAME_TITLE_LINKS 10
#define MAX_FOUND_LINKS 25
#define MAX_BOOKMARKS 10
enum { CM_ENCYCL_INDEX=0, CM_ENCYCL_SEARCH, CM_ENCYCL_REPSEARCH, CM_ENCYCL_BOOKMARK,
	CM_ENCYCL_UNBOOKMARK, CM_ENCYCL_CLEARBOOKMARKS, CM_ENCYCL_SEP_02, CM_ENCYCL_THEBOOKMARKS};
static size_t cm_encycl = CM_INIT_VALUE;
static size_t cm_encycl_res = CM_INIT_VALUE;
static INPUT_POPUP ipu_encycl;
static struct PAGE_LINK *page_links = NULL;
static struct PAGE_LINK *raw_page_links = NULL;
static char * * gen_titles;
static size_t found_links[MAX_FOUND_LINKS];
static size_t book_marks[MAX_BOOKMARKS];
static size_t num_gen_titles = 0;
static size_t max_gen_titles = 0;
static size_t num_raw_page_links = 0;
static size_t max_raw_page_links = 0;
static size_t num_page_links = 0;
static size_t max_page_links = 0;
static size_t num_found_links = 0;
static size_t num_bookmarks = 0;
enum { ENCYCLT_HELP=0, ENCYCLT_SKILLS, ENCYCLT_ENCYC };
#define NUM_PAGE_INDEX 3
static size_t page_index[NUM_PAGE_INDEX] = { (size_t)-1, (size_t)-1, (size_t)-1 };
static const char * index_name[NUM_PAGE_INDEX] = { "HelpPage", "newskills", "index",};
static void rebuild_cm_encycl(void);


/*	Save a raw link, title and from page - done during parsing of XML files.
*/
static void save_raw_page_link(const char *link, const char *title, size_t from_page_index)
{
	if (link==NULL || title==NULL)
		return;
	if (num_raw_page_links >= max_raw_page_links)
	{
		max_raw_page_links += MAX_ENC_PAGES;
		raw_page_links = (struct PAGE_LINK *)realloc(raw_page_links, sizeof(struct PAGE_LINK) * max_raw_page_links);
	}
	raw_page_links[num_raw_page_links].link = link;
	raw_page_links[num_raw_page_links].title = title;
	raw_page_links[num_raw_page_links].from_page_index = from_page_index; /* the page this link is on */
	num_raw_page_links++;
}


/*	Save a confirmed link with its title and index.
*/
static void save_confirmed_page_link(const char *link, const char *title, size_t from_page_index)
{
	if (link==NULL || title==NULL)
		return;
	if (from_page_index != page_index[ENCYCLT_ENCYC])
		return;
	if (num_page_links >= max_page_links)
	{
		max_page_links += 200;
		page_links = (struct PAGE_LINK *)realloc(page_links, sizeof(struct PAGE_LINK) * max_page_links);
	}
	page_links[num_page_links].title = title;
	page_links[num_page_links].link = link;
	page_links[num_page_links].from_page_index = from_page_index; /* the base page (help, skills or encycl */
	num_page_links++;
	//printf("[%s] [%s] %lu [%s]\n", title, link, (unsigned long)from_page_index, Page[from_page_index].Name);
} 


/*	Free encyclopedia navigation memory before exit.
*/
static void encycl_nav_free(void)
{
	size_t i;
	for (i=0; i<num_gen_titles; i++)
		free(gen_titles[i]);
	free(gen_titles);
	free(page_links);
	if (last_search != NULL)
		free(last_search);
	last_search = NULL;
	if (raw_page_links != NULL)
		free(raw_page_links);
	raw_page_links = NULL;
	max_gen_titles = num_gen_titles = max_page_links = num_page_links = 0;
}


/*	For each raw link, find and set the base index page
*/
static void find_base_pages(void)
{
	size_t i,j,k;

	/* find the index  in the Page list of each of the base pages */
	for (j=0; j<NUM_PAGE_INDEX; j++)
		for (i=0; i<numpage; ++i)
			if(!xmlStrcasecmp((xmlChar*)index_name[j],(xmlChar*)Page[i].Name))
			{
				page_index[j] = i;
				break;
			}

	for (i=0; i<num_raw_page_links; ++i)
	{
		size_t from_page_index = raw_page_links[i].from_page_index;
		int found_base = 0;

		/* limit loops to avoid never exiting */
		for (k=0; k<10 && !found_base; k++)
		{
			/* if the from page is one of the base pages, we're done */
			for (j=0; j<NUM_PAGE_INDEX; j++)
				if (from_page_index == page_index[j])
				{
					found_base = 1;
					raw_page_links[i].from_page_index = page_index[j];
					break;
				}
			if (found_base)
				break;
			/* find the first occurance of from_page_index in the raw list */
			for (j=0; j<num_raw_page_links; j++)
				if (!xmlStrcasecmp((xmlChar*)Page[from_page_index].Name, (xmlChar*)raw_page_links[j].link))
				{
					from_page_index = raw_page_links[j].from_page_index;
					break;
				}
		}
		if (!found_base)
		{
			//printf("No base for link [%s] [%s]\n --%lu [%s]\n", raw_page_links[i].title, raw_page_links[i].link, (unsigned long)raw_page_links[i].from_page_index, Page[raw_page_links[i].from_page_index].Name);
			raw_page_links[i].title = NULL;
		}
	}
}


/*	Process the full set of raw links to find the real ones.
*/
static void process_encycl_links(void)
{
	size_t i;
	struct PAGE_LINK *temp_links = (struct PAGE_LINK *)malloc(sizeof(struct PAGE_LINK) * MAX_SAME_TITLE_LINKS);

	for (i=0; i<num_raw_page_links; i++)
	{
		if ((raw_page_links[i].title != NULL) && (raw_page_links[i].from_page_index == page_index[ENCYCLT_ENCYC]))
		{
			const char * title = raw_page_links[i].title;
			size_t num_temp_links = 0;
			size_t j;
			/* find all the links with the same title */
			for (j=0; j<num_raw_page_links; j++)
			{
				if ((raw_page_links[j].title != NULL) && (raw_page_links[j].from_page_index == page_index[ENCYCLT_ENCYC]))
				{
					if (!xmlStrcasecmp((xmlChar*)title, (xmlChar*)raw_page_links[j].title))
					{
						size_t k;
						const char * link = raw_page_links[j].link;
						size_t from_page_index = raw_page_links[j].from_page_index;
						if (i!=j)
							raw_page_links[j].title = NULL;
						if (num_temp_links>=MAX_SAME_TITLE_LINKS)
							continue;
						for (k=0; k<num_temp_links; k++)
							if (!xmlStrcasecmp((xmlChar*)link, (xmlChar*)temp_links[k].link))
							{
								link = NULL;
								break;
							}
						/* only record a link if it has not been recorded before */
						if (link != NULL)
						{
							temp_links[num_temp_links].title = title;
							temp_links[num_temp_links].link = link;
							temp_links[num_temp_links].from_page_index = from_page_index;
							num_temp_links++;
						}
					}
				}
			}
			raw_page_links[i].title = NULL;
			/* if there are too many links with the same title, ignore all of them */
			if (num_temp_links>=MAX_SAME_TITLE_LINKS)
				continue;
			/* hurrah! a unique title/link pair */
			if (num_temp_links == 1)
				save_confirmed_page_link(temp_links[0].link, title, temp_links[0].from_page_index);
			/* multiple links with the same title but not too many */
			else if (num_temp_links<MAX_SAME_TITLE_LINKS)
			{
				/* append " (page name)" to multiple links with the same title */
				for (j=0; j<num_temp_links; j++)
				{
					size_t new_len = strlen(title) + strlen(temp_links[j].link) + 4;
					if (num_gen_titles>=max_gen_titles)
					{
						max_gen_titles += 50;
						gen_titles = (char * *)realloc(gen_titles, sizeof(char *) * max_gen_titles);
					}
					gen_titles[num_gen_titles] = (char *)malloc(sizeof(char)*new_len);
					safe_snprintf(gen_titles[num_gen_titles], new_len, "%s (%s)", title, temp_links[j].link);
					save_confirmed_page_link(temp_links[j].link, gen_titles[num_gen_titles], temp_links[j].from_page_index);
					num_gen_titles++;
				}
			}
		}
	}

	free(temp_links);
	free(raw_page_links);
	raw_page_links = NULL;
	num_raw_page_links = max_raw_page_links = 0;
}


/*	Find the specified page then open it.
*/
static void open_page(size_t index)
{
	size_t i;
	if (index>=num_page_links)
		return;

	for (i=0; i<numpage; ++i)
		if(!xmlStrcasecmp((xmlChar*)Page[i].Name,(xmlChar*)page_links[index].link))
		{
			currentpage = i;
			vscrollbar_set_pos(encyclopedia_win, encyclopedia_scroll_id, 0);
			vscrollbar_set_bar_len(encyclopedia_win, encyclopedia_scroll_id, Page[currentpage].max_y);
			return;
		}
	fprintf(stderr, "Weird, open_page(%lu) not found. [%s]\n", (unsigned long)index, page_links[index].title);
}


/*	If the current page is bookmarked, return it's bookmark index.
*/
static size_t get_page_bookmark_index()
{
	size_t i;
	size_t index = MAX_ENC_PAGES;
	for (i=0; i<num_page_links; i++)
		if(!xmlStrcasecmp((xmlChar*)Page[currentpage].Name,(xmlChar*)page_links[i].link))
		{
			index = i;
			break;
		}
	if (index >= MAX_ENC_PAGES)
		return MAX_BOOKMARKS;
	for (i=0; i<num_bookmarks; i++)
		if (index == book_marks[i])
			return i;
	return MAX_BOOKMARKS;
}


/*	Called just before the multi-results context menu is displayed.
*/
static void cm_encycl_res_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
	if (cm_win!= NULL)
		cm_win->opaque = 1;
}


/*	Called just before the main contextmenu is displayed, grey options that can't be used.
*/
static void cm_encycl_pre_show_handler(window_info *win, int widget_id, int mx, int my, window_info *cm_win)
{
	int is_bookmarked = (get_page_bookmark_index() < MAX_BOOKMARKS) ?1 :0;
	if (cm_win!= NULL)
		cm_win->opaque = 1;
	cm_grey_line(cm_encycl, CM_ENCYCL_REPSEARCH, (last_search == NULL));
	cm_grey_line(cm_encycl, CM_ENCYCL_BOOKMARK, (is_bookmarked || num_bookmarks == MAX_BOOKMARKS) ?1: 0);
	cm_grey_line(cm_encycl, CM_ENCYCL_UNBOOKMARK, !is_bookmarked);
	cm_grey_line(cm_encycl, CM_ENCYCL_CLEARBOOKMARKS, !num_bookmarks);
}


/*	Open the page selected from the multi-match list.
*/
static int cm_encycl_res_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	if (option>=0 && option<num_found_links)
		open_page(found_links[option]);
	return 1;
}


/*	Search the list of link title for a match
*/
static void find_page(const char *search_title, void *data)
{
	size_t i;

	/* find matches where the search string is a substring of the title */
	num_found_links = 0;
	for (i=0; i<num_page_links && num_found_links<MAX_FOUND_LINKS; ++i)
		if(xmlStrcasestr((xmlChar*)page_links[i].title,(xmlChar*)search_title)!=NULL)
			found_links[num_found_links++] = i;

	/* ignore if no matches, open if one match, or display a list of the first few matches */
	if (num_found_links < 1)
		return;
	else if (num_found_links == 1)
		open_page(found_links[0]);
	else
	{
		if (cm_valid(cm_encycl_res))
			cm_destroy(cm_encycl_res);
		cm_encycl_res = cm_create(page_links[found_links[0]].title, cm_encycl_res_handler);
		cm_set_pre_show_handler(cm_encycl_res, cm_encycl_res_pre_show_handler);
		for (i=1; i<num_found_links; i++)
			cm_add(cm_encycl_res, page_links[found_links[i]].title, NULL);
		cm_show_direct(cm_encycl_res, -1, -1);
	}
}


/*	Search dialog callback function
*/
static void find_page_callback(const char *search_title, void *data)
{
	/* save the search string for possible repeat */
	last_search = (char *)realloc(last_search, strlen(search_title)+1);
	strcpy(last_search, search_title);

	find_page(search_title, data);
}


/*	Handle options selected from the main context menu
*/
static int cm_encycl_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	switch (option)
	{
		case CM_ENCYCL_INDEX:
			currentpage = page_index[ENCYCLT_ENCYC];
			vscrollbar_set_pos(encyclopedia_win, encyclopedia_scroll_id, 0);
			vscrollbar_set_bar_len(encyclopedia_win, encyclopedia_scroll_id, Page[currentpage].max_y);
			break;
		case CM_ENCYCL_SEARCH:
			close_ipu(&ipu_encycl);
			init_ipu(&ipu_encycl, encyclopedia_win, DEFAULT_FONT_X_LEN * 20, -1, 40, 1, NULL, find_page_callback);
			ipu_encycl.x = mx; ipu_encycl.y = my;
			display_popup_win(&ipu_encycl, encycl_search_propmt);
			if (ipu_encycl.popup_win >=0 && ipu_encycl.popup_win<windows_list.num_windows)
				windows_list.window[ipu_encycl.popup_win].opaque = 1;
			break;
		case CM_ENCYCL_REPSEARCH:
			if (last_search != NULL)
				repeat_search = 1;
			break;
		case CM_ENCYCL_BOOKMARK:
			if (num_bookmarks < MAX_BOOKMARKS)
			{
				size_t i;
				for (i=0; i<num_page_links; i++)
					if(!xmlStrcasecmp((xmlChar*)Page[currentpage].Name,(xmlChar*)page_links[i].link))
					{
						book_marks[num_bookmarks++] = i;
						rebuild_cm_encycl();
						break;
					}
			}
			break;
		case CM_ENCYCL_UNBOOKMARK:
			{
				size_t i;
				size_t index = get_page_bookmark_index();
				if (index < MAX_BOOKMARKS)
				{
					for (i=index+1; i<num_bookmarks; i++)
						book_marks[i-1] = book_marks[i];
					num_bookmarks--;
					rebuild_cm_encycl();
				}
				break;
			}
		case CM_ENCYCL_CLEARBOOKMARKS:
			num_bookmarks = 0;
			rebuild_cm_encycl();
			break;
		default:
			open_page(book_marks[option-CM_ENCYCL_THEBOOKMARKS]);
	}
	return 1;
}


/*	Regenerate the main context menu with any bookmarks appended.
*/
static void rebuild_cm_encycl(void)
{
	size_t i;
	cm_set(cm_encycl, cm_encycl_base_str, cm_encycl_handler);
	if (num_bookmarks == 0)
		return;
	cm_add(cm_encycl, "--", NULL);
	for (i=0; i<num_bookmarks; i++)
		cm_add(cm_encycl, page_links[book_marks[i]].title, NULL);
}

/*	Set the flag for the window display handler to show the conetxt menu help string
*/
int mouseover_encyclopedia_handler(window_info *win, int mx, int my)
{
	if (my > 0)
		show_cm_help = 1;
	return 1;
}

/*	Add shortcut keypresses for search
*/
int keypress_encyclopedia_handler(window_info *win, int mx, int my, Uint32 key, Uint32 unikey)
{
	char keychar = tolower(key_to_char(unikey));
	if ((key == K_MARKFILTER) || (keychar=='/'))
	{
		cm_encycl_handler(win, -1, mx, my, CM_ENCYCL_SEARCH);
		return 1;
	}
	return 0;
}
#endif // ENCYCL_NAVIGATION


void fill_encyclopedia_win ()
{
	set_window_handler (encyclopedia_win, ELW_HANDLER_DISPLAY, &display_encyclopedia_handler);
	set_window_handler (encyclopedia_win, ELW_HANDLER_CLICK, &click_encyclopedia_handler);

	encyclopedia_scroll_id = vscrollbar_add_extended(encyclopedia_win, encyclopedia_scroll_id, NULL, encyclopedia_menu_x_len-20, 0, 20, encyclopedia_menu_y_len, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 30, Page[currentpage].max_y);

	if (numpage<=0)
	{
		LOG_TO_CONSOLE(c_red1, cant_load_encycl);
		return;
	}

#ifdef ENCYCL_NAVIGATION
	set_window_handler(encyclopedia_win, ELW_HANDLER_MOUSEOVER, &mouseover_encyclopedia_handler);
	set_window_handler(encyclopedia_win, ELW_HANDLER_KEYPRESS, &keypress_encyclopedia_handler);

	if (!cm_valid(cm_encycl))
	{
		cm_encycl = cm_create(cm_encycl_base_str, cm_encycl_handler);
		cm_set_pre_show_handler(cm_encycl, cm_encycl_pre_show_handler);
		cm_add_window(cm_encycl, encyclopedia_win);
		init_ipu(&ipu_encycl, -1, -1, -1, 1, 1, NULL, NULL);
		find_base_pages();
		process_encycl_links();
	}
#endif
}
