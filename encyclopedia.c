#include <stdlib.h>
#include <string.h>
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
			get_and_set_texture_id(i->id);
			glBegin(GL_QUADS);
			draw_2d_thing(i->u, i->v, i->uend, i->vend,i->x, i->y-j,i->xend,i->yend-j);
			glEnd();
		}
		i=i->Next;

	}
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
				id=load_texture_cache_deferred((char*)cur_attr->children->content,0);
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
				id=load_texture_cache_deferred((char*)cur_attr->children->content,0);
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
					log_error("Too many Enc Pages, limit of %d hit", MAX_ENC_PAGES);
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
				if(size==99)
					size=99;

				picsperrow=isize/tsize;
				xtile=tid%picsperrow;
				ytile=tid/picsperrow;
				ftsize=(float)tsize/isize;
				u=ftsize*xtile;
				v=-ftsize*ytile;
				uend=u+ftsize;
				vend=v-ftsize;
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
}

void fill_encyclopedia_win ()
{
	set_window_handler (encyclopedia_win, ELW_HANDLER_DISPLAY, &display_encyclopedia_handler);
	set_window_handler (encyclopedia_win, ELW_HANDLER_CLICK, &click_encyclopedia_handler);

	encyclopedia_scroll_id = vscrollbar_add_extended(encyclopedia_win, encyclopedia_scroll_id, NULL, encyclopedia_menu_x_len-20, 0, 20, encyclopedia_menu_y_len, 0, 1.0, 0.77f, 0.57f, 0.39f, 0, 30, Page[currentpage].max_y);
}
