#include <string.h>
#include "global.h"
#include "elwindows.h"

int encyclopedia_win=0;
int encyclopedia_menu_x=100;
int encyclopedia_menu_y=20;
int encyclopedia_menu_x_len=500;
int encyclopedia_menu_y_len=350;
//int encyclopedia_menu_dragged=0;

_Category Category[100];
_Page Page[500];
int num_category=0,numpage=-1,numtext,x,y,numimage,id,color,size,ref,currentpage=0,isize,tsize,tid,ssize,mouseover=0,xposupdate,yposupdate,lastextlen=0;
float u,v,uend,vend,xend,yend,r,g,b;
char *s,*ss;

int display_encyclopedia_handler(window_info *win)
{
	_Text *t=Page[currentpage].T.Next;
	_Image *i=Page[currentpage].I.Next;

	while(t){
		int ylen=(t->size)?18:15;
		int xlen=strlen(t->text)*((t->size)?11:8);

		if(t->ref)
			{
				//draw a line
				glColor3f(0.5,0.5,0.5);
				glDisable(GL_TEXTURE_2D);
				glBegin(GL_LINES);
				glVertex3i(t->x+4,t->y+ylen,0);
				glVertex3i(t->x+4+xlen-8,t->y+ylen,0);
				glEnd();
				glEnable(GL_TEXTURE_2D);
			}
		if(t->size)
			{
				if(t->ref && mouse_x>(t->x+win->cur_x) && mouse_x<(t->x+xlen+win->cur_x) && mouse_y>(t->y+win->cur_y) && mouse_y<(t->y+ylen+win->cur_y))
				glColor3f(0.3,0.6,1.0);
				else
				glColor3f(t->r,t->g,t->b);
				draw_string(t->x,t->y,t->text,1);
			}
		else
			{
				if(t->ref && mouse_x>(t->x+win->cur_x) && mouse_x<(t->x+xlen+win->cur_x) && mouse_y>(t->y+win->cur_y) && mouse_y<(t->y+ylen+win->cur_y))
				glColor3f(0.3,0.6,1.0);
				else
				glColor3f(t->r,t->g,t->b);
				draw_string_small(t->x,t->y,t->text,1);
			}
		t=t->Next;
	}

	glColor3f(1.0f,1.0f,1.0f);
	while(i){
		if(i->mouseover==1){i=i->Next;continue;}
		if(mouse_x>(i->x+win->cur_x) && mouse_x<(win->cur_x+i->xend) && mouse_y>(i->y+win->cur_y) && mouse_y<(win->cur_y+i->yend)){
			if(i->Next!=NULL){
				if(i->Next->mouseover==1)
					i=i->Next;
			}
		}
		get_and_set_texture_id(i->id);
		glBegin(GL_QUADS);
		draw_2d_thing(i->u, i->v, i->uend, i->vend,i->x, i->y,i->xend,i->yend);
		glEnd();
		i=i->Next;
	}
	return 1;
}

int click_encyclopedia_handler(window_info *win, int mx, int my, Uint32 flags)
{
	_Text *t=Page[currentpage].T.Next;

	while(t){
		int xlen=strlen(t->text)*((t->size)?11:8),ylen=(t->size)?18:15;
		if(t->ref && mx>(t->x) && mx<(t->x+xlen) && my>(t->y) && my<(t->y+ylen)){
				//changing page
				int i;
				for(i=0;i<numpage+1;i++){
					if(!xmlStrcasecmp(Page[i].Name,t->ref)){
						currentpage=i;
						break;
					}
				}

			break;
		}
		t=t->Next;
	}

	return 1;
}

void GetColor(char *t)
{
	if(!xmlStrcasecmp("silver",t)){r=192/255.0f; g=192/255.0f; b=192/255.0f;return;}
	if(!xmlStrcasecmp("grey",t)){r=128/255.0f; g=128/255.0f; b=128/255.0f;return;}
	if(!xmlStrcasecmp("maroon",t)){r=128/255.0f; g=0.0f; b=0.0f;return;}
	if(!xmlStrcasecmp("green",t)){r=0.0f; g=128/255.0f; b=0.0f;return;}
	if(!xmlStrcasecmp("navy",t)){r=0.0f; g=0.0f; b=128/255.0f;return;}
	if(!xmlStrcasecmp("olive",t)){r=128/255.0f; g=128/255.0f; b=0.0f;return;}
	if(!xmlStrcasecmp("purple",t)){r=128/255.0f; g=0.0f; b=128/255.0f;return;}
	if(!xmlStrcasecmp("teal",t)){r=0.0f; g=128/255.0f; b=128/255.0f;return;}
	if(!xmlStrcasecmp("white",t)){r=1.0f; g=1.0f; b=1.0f;return;}
	if(!xmlStrcasecmp("black",t)){r=0.0f; g=0.0f; b=0.0f;return;}
	if(!xmlStrcasecmp("red",t)){r=1.0f; g=0.0f; b=0.0f;return;}
	if(!xmlStrcasecmp("lime",t)){r=0.0f; g=1.0f; b=0.0f;return;}
	if(!xmlStrcasecmp("blue",t)){r=0.0f; g=0.0f; b=1.0f;return;}
	if(!xmlStrcasecmp("magenta",t)){r=1.0f; g=0.0f; b=1.0f;return;}
	if(!xmlStrcasecmp("yellow",t)){r=1.0f; g=1.0f; b=0.0f;return;}
	if(!xmlStrcasecmp("cyan",t)){r=0.0f; g=1.0f; b=1.0f;return;}

}


// XML
void ParseLink(xmlAttr *a_node)
{
	xmlAttr *cur_attr=NULL;

    for (cur_attr = a_node; cur_attr; cur_attr = cur_attr->next) {
        if (cur_attr->type==XML_ATTRIBUTE_NODE){
			if(!xmlStrcasecmp(cur_attr->name,"ref")){
				ss=cur_attr->children->content;
			}
			if(!xmlStrcasecmp(cur_attr->name,"title")){
				s=cur_attr->children->content;
			}
			//x=""
			if(!xmlStrcasecmp(cur_attr->name,"x")){
				x=atoi(cur_attr->children->content);
			}
			//y=""
			if(!xmlStrcasecmp(cur_attr->name,"y")){
				y=atoi(cur_attr->children->content);
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
			if(!xmlStrcasecmp(cur_attr->name,"r")){
				r=atof(cur_attr->children->content);
			}
			//g=""
			if(!xmlStrcasecmp(cur_attr->name,"g")){
				g=atof(cur_attr->children->content);
			}
			//b=""
			if(!xmlStrcasecmp(cur_attr->name,"b")){
				b=atof(cur_attr->children->content);
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
			if(!xmlStrcasecmp(cur_attr->name,"u")){
				u=(float)atof(cur_attr->children->content);
			}
			//v=""
			if(!xmlStrcasecmp(cur_attr->name,"v")){
				v=(float)atof(cur_attr->children->content);
			}
			//uend=""
			if(!xmlStrcasecmp(cur_attr->name,"uend")){
				uend=(float)atof(cur_attr->children->content);
			}
			//vend=""
			if(!xmlStrcasecmp(cur_attr->name,"vend")){
				vend=(float)atof(cur_attr->children->content);
			}
			//xend=""
			if(!xmlStrcasecmp(cur_attr->name,"xlen")){
				xend=(float)atof(cur_attr->children->content);
			}
			//yend=""
			if(!xmlStrcasecmp(cur_attr->name,"ylen")){
				yend=(float)atof(cur_attr->children->content);
			}
			//name=""
			if(!xmlStrcasecmp(cur_attr->name,"name")){
				id=load_texture_cache(cur_attr->children->content,0);
			}
			//x=""
			if(!xmlStrcasecmp(cur_attr->name,"x")){
				x=atoi(cur_attr->children->content);
			}
			//y=""
			if(!xmlStrcasecmp(cur_attr->name,"y")){
				y=atoi(cur_attr->children->content);
			}
			//mouseover=""
			if(!xmlStrcasecmp(cur_attr->name,"mouseover")){
				mouseover=atoi(cur_attr->children->content);
			}
			//xposupdate=""
			if(!xmlStrcasecmp(cur_attr->name,"xposupdate")){
				xposupdate=atoi(cur_attr->children->content);
			}
			//yposupdate=""
			if(!xmlStrcasecmp(cur_attr->name,"yposupdate")){
				yposupdate=atoi(cur_attr->children->content);
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
			if(!xmlStrcasecmp(cur_attr->name,"name")){
				id=load_texture_cache(cur_attr->children->content,0);
			}
			//isize=""
			if(!xmlStrcasecmp(cur_attr->name,"isize")){
				isize=atoi(cur_attr->children->content);
			}
			//tsize=""
			if(!xmlStrcasecmp(cur_attr->name,"tsize")){
				tsize=atoi(cur_attr->children->content);
			}
			//tid=""
			if(!xmlStrcasecmp(cur_attr->name,"tid")){
				tid=atoi(cur_attr->children->content);
			}
			//size=""
			if(!xmlStrcasecmp(cur_attr->name,"size")){
				ssize=atoi(cur_attr->children->content);
			}
			//x=""
			if(!xmlStrcasecmp(cur_attr->name,"x")){
				x=atoi(cur_attr->children->content);
			}
			//y=""
			if(!xmlStrcasecmp(cur_attr->name,"y")){
				y=atoi(cur_attr->children->content);
			}
			//mouseover=""
			if(!xmlStrcasecmp(cur_attr->name,"mouseover")){
				mouseover=atoi(cur_attr->children->content);
			}
			//xposupdate=""
			if(!xmlStrcasecmp(cur_attr->name,"xposupdate")){
				xposupdate=atoi(cur_attr->children->content);
			}
			//yposupdate=""
			if(!xmlStrcasecmp(cur_attr->name,"yposupdate")){
				yposupdate=atoi(cur_attr->children->content);
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
			if(!xmlStrcasecmp(cur_attr->name,"x")){
				x=atoi(cur_attr->children->content);
			}
			//y=""
			if(!xmlStrcasecmp(cur_attr->name,"y")){
				y=atoi(cur_attr->children->content);
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
			if(!xmlStrcasecmp(cur_attr->name,"name")){
				int l=strlen(cur_attr->children->content);
				Page[numpage].Name=(char*)malloc(l+1);
				UTF8Toisolat1(Page[numpage].Name, &l, cur_attr->children->content, &l);
				Page[numpage].Name[l]=0;
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
			if(!xmlStrcasecmp(cur_attr->name,"x")){
				x=atoi(cur_attr->children->content);
			}
			//y=""
			if(!xmlStrcasecmp(cur_attr->name,"y")){
				y=atoi(cur_attr->children->content);
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
			if(!xmlStrcasecmp(cur_node->name,"Page")){

				numpage++;
				numtext=0;
				numimage=0;
				x=2;
				y=2;
				ParsePage(cur_node->properties);
			}

			//<Size>
			if(!xmlStrcasecmp(cur_node->name,"Size")){
				size=(!xmlStrcasecmp("Big",cur_node->children->content))?1:0;
			}

			//<Color>
			if(!xmlStrcasecmp(cur_node->name,"Color")){
				ParseColor(cur_node->properties);
				if(cur_node->children)
					GetColor(cur_node->children->content);
			}

			//<Text>
			if(!xmlStrcasecmp(cur_node->name,"Text")){
				_Text *T=(_Text*)malloc(sizeof(_Text));
				_Text *t=&Page[numpage].T;
				int l=strlen(cur_node->children->content);
				T->Next=NULL;
				ParseText(cur_node->properties);
				T->x=x;
				T->y=y;
				T->size=size;
				T->r=r; T->g=g; T->b=b;
				T->text=(char*)malloc(l+1);
				T->ref=NULL;
				UTF8Toisolat1(T->text, &l, cur_node->children->content, &l); 
				T->text[l]=0;
				while(t->Next!=NULL)t=t->Next;
				t->Next=T;
				x+=strlen(T->text)*((T->size)?11:8);
				lastextlen=strlen(T->text)*((T->size)?11:8);
			}

			//<nl>
			if(!xmlStrcasecmp(cur_node->name,"nl")){
				x=2;
				y+=(size)?18:15;
			}
			
			//<nlkx>
			if(!xmlStrcasecmp(cur_node->name,"nlkx")){
				y+=(size)?18:15;
				x-=lastextlen;
			}

			//<Image>
			if(!xmlStrcasecmp(cur_node->name,"image")){
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
			if(!xmlStrcasecmp(cur_node->name,"simage")){
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
			if(!xmlStrcasecmp(cur_node->name,"pos")){
				ParsePos(cur_node->properties);
			}

			//<link>
			if(!xmlStrcasecmp(cur_node->name,"link")){
				_Text *T=(_Text*)malloc(sizeof(_Text));
				_Text *t=&Page[numpage].T;
				int ls;
				int lss;
				ParseLink(cur_node->properties);
				lss=strlen(ss);
				ls=strlen(s);
				T->Next=NULL;
				T->x=x;
				T->y=y;
				T->size=size;
				T->r=r; T->g=g; T->b=b;
				T->text=(char*)malloc(ls+1);
				T->ref=(char*)malloc(lss+1);
				UTF8Toisolat1(T->text, &ls, s, &ls);
				T->text[ls]=0;
				UTF8Toisolat1(T->ref, &lss, ss, &lss);
				T->ref[lss]=0;
				while(t->Next!=NULL)t=t->Next;
				t->Next=T;
				x+=strlen(T->text)*((T->size)?11:8);
				lastextlen=strlen(T->text)*((T->size)?11:8);
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
			if(!xmlStrcasecmp(cur_node->name,"Category")){
				xmlDocPtr doc;
				char tmp[100];
				int l=strlen(cur_node->children->content);
				Category[num_category].Name=(char*)malloc(l+1);
				UTF8Toisolat1(Category[num_category].Name, &l, cur_node->children->content, &l);
				Category[num_category++].Name[l]=0;

				//we load the category now
				sprintf(tmp,"languages/%s/Encyclopedia/%s.xml",lang,cur_node->children->content);
				doc=xmlReadFile(tmp, NULL, 0);
				if (doc==NULL)
					{
						sprintf(tmp,"languages/en/Encyclopedia/%s.xml",cur_node->children->content);
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
    xmlDocPtr doc=xmlReadFile(filename, NULL, 0);
    if (doc==NULL)
		return;

    ReadIndexXML(xmlDocGetRootElement(doc));
    xmlFreeDoc(doc);
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

void display_encyclopedia()
{
	if(encyclopedia_win <= 0)
		{
			encyclopedia_win= create_window("Encyclopedia", 0, 0, encyclopedia_menu_x, encyclopedia_menu_y, encyclopedia_menu_x_len, encyclopedia_menu_y_len, ELW_WIN_DEFAULT);
			set_window_handler(encyclopedia_win, ELW_HANDLER_DISPLAY, &display_encyclopedia_handler );
			set_window_handler(encyclopedia_win, ELW_HANDLER_CLICK, &click_encyclopedia_handler );
		}
	else
		{
			show_window(encyclopedia_win);
			select_window(encyclopedia_win);
		}
}

