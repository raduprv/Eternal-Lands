#include "global.h"

int view_encyclopedia=1;
int encyclopedia_menu_x=100;
int encyclopedia_menu_y=20;
int encyclopedia_menu_x_len=500;
int encyclopedia_menu_y_len=350;
int encyclopedia_menu_dragged=0;

_Category Category[100];
_Page Page[100];
int num_category=0,numpage=-1,numtext,x,y,numimage,id,color,size,ref,currentpage=0;
float u,v,uend,vend,xend,yend;
char *s,*ss;

void display_encyclopedia()
{
	_Text *t=Page[currentpage].T.Next;
	_Image *i=Page[currentpage].I.Next;

	//title bar
	draw_menu_title_bar(encyclopedia_menu_x,encyclopedia_menu_y-16,encyclopedia_menu_x_len);
	// window drawing
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(0.0f,0.0f,0.0f,0.5f);
	glVertex3i(encyclopedia_menu_x,encyclopedia_menu_y+encyclopedia_menu_y_len,0);
	glVertex3i(encyclopedia_menu_x,encyclopedia_menu_y,0);
	glVertex3i(encyclopedia_menu_x+encyclopedia_menu_x_len,encyclopedia_menu_y,0);
	glVertex3i(encyclopedia_menu_x+encyclopedia_menu_x_len,encyclopedia_menu_y+encyclopedia_menu_y_len,0);
	glEnd();
	glDisable(GL_BLEND);
	glColor3f(0.77f,0.57f,0.39f);
	glBegin(GL_LINES);
	glVertex3i(encyclopedia_menu_x,encyclopedia_menu_y,0);
	glVertex3i(encyclopedia_menu_x+encyclopedia_menu_x_len,encyclopedia_menu_y,0);
	glVertex3i(encyclopedia_menu_x+encyclopedia_menu_x_len,encyclopedia_menu_y,0);
	glVertex3i(encyclopedia_menu_x+encyclopedia_menu_x_len,encyclopedia_menu_y+encyclopedia_menu_y_len,0);
	glVertex3i(encyclopedia_menu_x+encyclopedia_menu_x_len,encyclopedia_menu_y+encyclopedia_menu_y_len,0);
	glVertex3i(encyclopedia_menu_x,encyclopedia_menu_y+encyclopedia_menu_y_len,0);
	glVertex3i(encyclopedia_menu_x,encyclopedia_menu_y+encyclopedia_menu_y_len,0);
	glVertex3i(encyclopedia_menu_x,encyclopedia_menu_y,0);
	// X corner
	glVertex3i(encyclopedia_menu_x+encyclopedia_menu_x_len,encyclopedia_menu_y+20,0);
	glVertex3i(encyclopedia_menu_x+encyclopedia_menu_x_len-20,encyclopedia_menu_y+20,0);
	glVertex3i(encyclopedia_menu_x+encyclopedia_menu_x_len-20,encyclopedia_menu_y+20,0);
	glVertex3i(encyclopedia_menu_x+encyclopedia_menu_x_len-20,encyclopedia_menu_y,0);
	glEnd();
	glEnable(GL_TEXTURE_2D);
	draw_string(encyclopedia_menu_x+encyclopedia_menu_x_len-16,encyclopedia_menu_y+2,"X",1);

	while(t){
		glColor3f((float)colors_list[t->color].r1/255.0f,(float)colors_list[t->color].g1/255.0f,(float)colors_list[t->color].b1/255.0f);
		if(t->size)
			draw_string(t->x+encyclopedia_menu_x,t->y+encyclopedia_menu_y,t->text,1);
		else
			draw_string_small(t->x+encyclopedia_menu_x,t->y+encyclopedia_menu_y,t->text,1);
		t=t->Next;
	}
	
	while(i){
		get_and_set_texture_id(i->id);
		glBegin(GL_QUADS);
		draw_2d_thing(i->u, i->v, i->uend, i->vend,i->x+encyclopedia_menu_x, i->y+encyclopedia_menu_y,encyclopedia_menu_x+i->xend,encyclopedia_menu_y+i->yend);
		glEnd();
		i=i->Next;
	}
	
}

int encyclopedia_mouse_over()
{
	return 1;
}

int check_encyclopedia_interface()
{
	_Text *t=Page[currentpage].T.Next;
	if(!view_encyclopedia || mouse_x>encyclopedia_menu_x+encyclopedia_menu_x_len || mouse_x<encyclopedia_menu_x
	   || mouse_y<encyclopedia_menu_y || mouse_y>encyclopedia_menu_y+encyclopedia_menu_y_len)return 0;

	while(t){
		int xlen=strlen(t->text)*((t->size)?11:8),ylen=(t->size)?18:15;
		if(t->ref && mouse_x>(t->x+encyclopedia_menu_x) && mouse_x<(t->x+xlen+encyclopedia_menu_x) && mouse_y>(t->y+encyclopedia_menu_y) && mouse_y<(t->y+ylen+encyclopedia_menu_y)){
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

int GetColor(char *t)
{
	if(!xmlStrcasecmp("red1",t))return 0;
	if(!xmlStrcasecmp("red2",t))return 1;
	if(!xmlStrcasecmp("red3",t))return 2;
	if(!xmlStrcasecmp("red4",t))return 3;
	if(!xmlStrcasecmp("orange1",t))return 4;
	if(!xmlStrcasecmp("orange2",t))return 5;
	if(!xmlStrcasecmp("orange3",t))return 6;
	if(!xmlStrcasecmp("orange4",t))return 7;
	if(!xmlStrcasecmp("yellow1",t))return 8;
	if(!xmlStrcasecmp("yellow2",t))return 9;
	if(!xmlStrcasecmp("yellow3",t))return 10;
	if(!xmlStrcasecmp("yellow4",t))return 11;
	if(!xmlStrcasecmp("green1",t))return 12;
	if(!xmlStrcasecmp("green2",t))return 13;
	if(!xmlStrcasecmp("green3",t))return 14;
	if(!xmlStrcasecmp("green4",t))return 15;
	if(!xmlStrcasecmp("blue1",t))return 16;
	if(!xmlStrcasecmp("blue2",t))return 17;
	if(!xmlStrcasecmp("blue3",t))return 18;
	if(!xmlStrcasecmp("blue4",t))return 19;
	if(!xmlStrcasecmp("purple1",t))return 20;
	if(!xmlStrcasecmp("purple2",t))return 21;
	if(!xmlStrcasecmp("purple3",t))return 22;
	if(!xmlStrcasecmp("purple4",t))return 23;
	if(!xmlStrcasecmp("grey1",t))return 24;
	if(!xmlStrcasecmp("grey2",t))return 25;
	if(!xmlStrcasecmp("grey3",t))return 26;
	if(!xmlStrcasecmp("grey4",t))return 27;

	return -1;
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
			}

			//<Name>
			if(!xmlStrcasecmp(cur_node->name,"Name")){
				Page[numpage].Name=(char*)malloc(strlen(cur_node->children->content)+1);
				strcpy(Page[numpage].Name,cur_node->children->content);
			}

			//<Size>
			if(!xmlStrcasecmp(cur_node->name,"Size")){
				size=(!xmlStrcasecmp("Big",cur_node->children->content))?1:0;
			}
			
			//<Color>
			if(!xmlStrcasecmp(cur_node->name,"Color")){
				color=GetColor(cur_node->children->content);
			}
			
			//<Text>
			if(!xmlStrcasecmp(cur_node->name,"Text")){
				_Text *T=(_Text*)malloc(sizeof(_Text));
				_Text *t=&Page[numpage].T;
				T->Next=NULL;
				T->x=x;
				T->y=y;
				T->size=size;
				T->color=color;
				T->text=(char*)malloc(strlen(cur_node->children->content)+1);
				T->ref=NULL;
				strcpy(T->text,cur_node->children->content);
				while(t->Next!=NULL)t=t->Next;
				t->Next=T;
				x+=strlen(T->text)*((T->size)?11:8);
			}

			//<nl>
			if(!xmlStrcasecmp(cur_node->name,"nl")){
				x=2;
				y+=(size)?18:15;
			}
			
			//<Image>
			if(!xmlStrcasecmp(cur_node->name,"image")){
				_Image *I=(_Image*)malloc(sizeof(_Image));
				_Image *i=&Page[numpage].I;
				ParseImage(cur_node->properties);
				I->Next=NULL;
				I->id=id;
				I->u=u;
				I->v=v;
				I->uend=uend;
				I->vend=vend;
				I->xend=x+xend;
				I->yend=y+yend;
				I->x=x;
				I->y=y;
				while(i->Next!=NULL)i=i->Next;
				i->Next=I;
				x+=xend;
				y+=yend-((size)?18:15);
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
				ParseLink(cur_node->properties);
				T->Next=NULL;
				T->x=x;
				T->y=y;
				T->size=size;
				T->color=color;
				T->text=(char*)malloc(strlen(s)+1);
				T->ref=(char*)malloc(strlen(ss)+1);
				strcpy(T->text,s);
				strcpy(T->ref,ss);
				while(t->Next!=NULL)t=t->Next;
				t->Next=T;
				x+=strlen(T->text)*((T->size)?11:8);
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
				Category[num_category].Name=(char*)malloc(strlen(cur_node->children->content)+1);
				strcpy(Category[num_category++].Name,cur_node->children->content);
				
				//we load the category now
				sprintf(tmp,"Encyclopedia/%s.xml",cur_node->children->content);
				doc=xmlReadFile(tmp, NULL, 0);
				if (doc==NULL)
					return;
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
