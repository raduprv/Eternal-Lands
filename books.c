#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include <malloc.h>
#include "global.h"
#include "books.h"

#define _TITLE 0
#define _AUTHOR 1
#define _TEXT 2
#define _IMAGE 3
#define _IMAGE_TEXT 4

book * books=NULL;

/*Memory handling etc.*/

page * add_page(book * b)
{
	page *p;
	
	if(!b) return NULL;
	
	p=(page*)calloc(1,sizeof(page));
	p->lines=(char**)calloc(b->max_lines+1,sizeof(char *));
	p->page_no=b->no_pages+1;
	
	b->pages=(page**)realloc(b->pages,(b->no_pages+2)*sizeof(page*));
	b->pages[b->no_pages++]=p;
	b->pages[b->no_pages]=NULL;

	return p;
}

book * create_book(char * title, int type, int id)
{
	book *b=(book*)calloc(1,sizeof(book));
	
	switch(type){
		case 1:	
			b->max_width=29;
			b->max_lines=20;
			type=1;
			break;
		case 2: 
			b->max_width=20;
			b->max_lines=15;
			type=2;
			break;
	}
	b->type=type;
	b->id=id;
	strncpy(b->title, title, 34);
	
	add_book(b);
	
	return b;
}

_image *create_image(char * file, int x, int y, int w, int h, float u_start, float v_start, float u_end, float v_end)
{
	_image *img=(_image *)calloc(1,sizeof(_image));
	
	img->x=x;
	img->y=y;
	img->w=w;
	img->h=h;
	img->u[0]=u_start;
	img->u[1]=u_end;
	img->v[0]=v_start;
	img->v[1]=v_end;

	img->texture=load_texture_cache(file,0);
	if(!img->texture) {
		free(img);
		img=NULL;
	}

	return img;
}

void free_page(page * p)
{
	char **l=p->lines;
	
	if(p->image) free(p->image);
	for(;*l;l++) free(*l);
	free(p->lines);
}

void free_book(book * b)
{
	int i;
	page **p;
	
	p=b->pages;
	for(i=0;i<b->no_pages;i++,p++) free_page(*p);
	free(b->pages);
	free(b);
}

void free_books()
{
	book *b,*l=NULL;
	for(b=books;b;l=b){
		b=b->next;
		free_book(l);
	}
}

/*Multiple book handling*/

int have_book(int id)
{
	book *b=books;
	if(b){
		for(;b->next;b=b->next){
			if(b->id==id) return 1;
		}
	}
	return 0;
}

book * get_book(int id)
{
	book *b=books;
	if(b){
		for(;b->next;b=b->next){
			if(b->id==id) return b;
		}
	}
	return NULL;
}

void add_book(book *bs)
{
	book *b=books;
	if(b) {
		for(;b->next;b=b->next);
		b->next=bs;
	} else books=bs;
}

/*Book parser*/

page * add_str_to_page(char * str, int type, book *b, page *p)
{
	char ** lines=NULL;
	char ** newlines=NULL;
	int i;

	if(!str) return NULL;
	if(!p)p=add_page(b);
	
	newlines=get_lines(str, b->max_width);
	lines=p->lines;
	
	for(i=0;*lines;i++,lines++);

	if(type==_AUTHOR){
		*lines++=(char*)calloc(1,sizeof(char));
	} else if (type==_TITLE){
		*lines++=(char*)calloc(1,sizeof(char));
	}

	for(;*newlines;i++) {
		if(type==_AUTHOR){
			memmove(*newlines+1,*newlines,strlen(*newlines)+1);
			**newlines=127+c_orange3;
		} else if(type==_TITLE){
			memmove(*newlines+1,*newlines,strlen(*newlines)+1);
			**newlines=127+c_orange4;
		}
		if(i>=b->max_lines){
			*lines=NULL;
			p=add_page(b);
			lines=p->lines;
			i=0;
		}
		*lines++=*newlines++;
	}
	if(i<b->max_lines){
		if(type==_AUTHOR){
			*lines++=(char*)calloc(1,sizeof(char));
		} else if (type==_TITLE){
			*lines++=(char*)calloc(1,sizeof(char));
		}
	}
	*lines=NULL;

	return p;
}

char * wrap_line_around_image(char * line, int w, int x, int max_width, char * last)
{
	int i,j;
	if(last){
		for(i=0;i<max_width;i++){
			if(!*last||(i>=x && i<x+w)) {
				*line++=' ';
			} else {
				for(j=0;j<max_width && last[j] && last[j]!=' ' && last[j]!=0x0a && last[j]!=0x0d;j++);
				if(i+j<x||(i>=x+w && i+j<max_width)){
					for(;j-->=0;line++,last++){
						if(*last==0x0d)last++;
						if(*last==0x0a){
							last++;
							i=max_width;
							break;
						}
						*line=*last;
						i++;
					}
				} else *line++=' ';
			}
		}
		
		*line=0;
	}

	return last;
}

page * add_image_to_page(char * in_text, _image *img, book * b, page * p)
{
	char **line;
	char *last_ptr;
	int i=0,k;
	int max_width=b->max_width;
	int max_lines=b->max_lines;
	int h, w, x, y;

	if(!img||!b) return NULL;
	if(!p || p->image)p=add_page(b);

	h=img->h/18+1;
	y=img->y/18+1;
	w=img->w/12+1;
	x=img->x/12+1;

	if(y+h>max_lines || w+x>max_width) return NULL;
	
	line=p->lines;

	for(;line[i];i++);

	if(i+h>=max_lines) {
		p=add_page(b);
		line=p->lines;
		i=0;
	}

	p->image=img;

	if(in_text){
		k=i;
		line+=i;

		last_ptr=in_text;

		for(i=0;i<h;i++,line++){//
			*line=(char*)malloc((max_width+2)*sizeof(char));
			if(i && (w<(max_width/3*2) && x+w<max_width) && last_ptr && *last_ptr){//If it's more than 2/3rd of the page width, don't work on it
				last_ptr=wrap_line_around_image(*line,w,x,b->max_width,last_ptr);
			} else {
				**line=0;
			}
		}

		if(*last_ptr) p=add_str_to_page(last_ptr,_IMAGE_TEXT,b,p);
	}

	return p;
}

/*XML-parser*/

void add_xml_image_to_page(xmlNode * cur, book * b, page *p)
{
	char *image_path;
	int x,y,w,h;
	float u_start,v_start,u_end,v_end;
	_image *img;
	char *text=NULL;

	x=xmlGetInt(cur,"x");
	y=xmlGetInt(cur,"y");
	w=xmlGetInt(cur,"w");
	h=xmlGetInt(cur,"h");
	
	u_start=xmlGetFloat(cur,"u_start");
	u_end=xmlGetFloat(cur,"u_end");
	if(!u_end) u_end=1;
	
	v_start=xmlGetFloat(cur,"v_start");
	if(!v_start) v_start=1;
	v_end=xmlGetFloat(cur,"v_end");
	
	image_path=xmlGetProp(cur,"src");
	if(!image_path) return;

	img=create_image(image_path, x, y, w, h, u_start, v_start, u_end, v_end);

	if(cur->children && cur->children->content){
		MY_XMLSTRCPY(&text, cur->children->content);
	}
	
	if(add_image_to_page(text, img, b, p)==NULL) free(img);
	
	free(image_path);
	if(text) free(text);
}

void add_xml_str_to_page(xmlNode * cur, int type, book * b, page *p)
{
	char * string=NULL;
	if(cur->children && cur->children->content && MY_XMLSTRCPY(&string, cur->children->content)!=-1){
		add_str_to_page(string, type, b, p);
	} else {
		char str[200];
		sprintf(str,"An error occured when parsing the content of the <%s>-tag on line %d - Check it for letters that cannot be translated into iso8859-1\n",cur->name,cur->line);
		log_error(str);
	}
	free(string);
}

void add_xml_page(xmlNode *cur, book * b)
{
	page *p=add_page(b);
	for(;cur;cur=cur->next){
		if(cur->type == XML_ELEMENT_NODE){
			if (!xmlStrcasecmp(cur->name,"title")){
				add_xml_str_to_page(cur,_TITLE,b,p);
			} else if (!xmlStrcasecmp(cur->name,"author")){
				add_xml_str_to_page(cur,_AUTHOR,b,p);
			} else if (!xmlStrcasecmp(cur->name,"text")){
				add_xml_str_to_page(cur,_TEXT,b,p);
			} else if (!xmlStrcasecmp(cur->name,"image")){
				add_xml_image_to_page(cur, b, p);
			}
		}
	}
}

book * parse_book(xmlNode *in, char * title, int type, int id)
{
	xmlNode * cur;
	book * b=create_book(title, type, id);
	
	for(cur=in;cur;cur=cur->next){
		if(cur->type == XML_ELEMENT_NODE){
			if(!xmlStrcasecmp(cur->name,"page")){
				add_xml_page(cur->children,b);
			}
		}
	}

	return b;
}

book * read_book(char * file)
{
	xmlDoc * doc;
	xmlNode * root=NULL;
	xmlChar *type=NULL,*title=NULL, *idstr=NULL;
	book *b=NULL;
	int id;

	if ((doc = xmlReadFile(file, NULL, 0)) == NULL) {
		char str[200];
		snprintf(str,198,"Couldn't open the book: %s",file);
		log_error(str);
		LOG_TO_CONSOLE(c_red1,str);
	} else if ((root = xmlDocGetRootElement(doc))==NULL) {
		char str[200];
		snprintf(str,198,"Error while parsing: %s",file);
		log_error(str);
	} else if(xmlStrcasecmp(root->name,"book")){
		char str[200];
		snprintf(str,198,"Root element in %s is not <book>",file);
		log_error(str);
	} else if((type=xmlGetProp(root,"type"))==NULL){
		char str[200];
		snprintf(str,198,"Root element in %s does not contain a type=\"<booktype>\" property.",file);
		log_error(str);
	} else if((title=xmlGetProp(root,"title"))==NULL){
		char str[200];
		snprintf(str,198,"Root element in %s does not contain a title=\"<short title>\" property.",file);
		log_error(str);
	} else if((idstr=xmlGetProp(root,"id"))==NULL){
		char str[200];
		snprintf(str,198,"Root element in %s does not contain an id=\"<book id>\" property", file);
		log_error(str);
	} else {
		id=atoi(idstr);
		b=get_book(id);
		if(!b) b=parse_book(root->children, title, atoi(type), id);
	}
	
	if(type)free(type);
	if(title)free(title);
	if(idstr)free(idstr);
	
	xmlFreeDoc(doc);

	return b;
}

/*Network parser*/

void read_local_book(char * data, int len)
{
	char file_name[200];
	book *b;
	if(len>198) return;
	strncpy(file_name,data,len);
	b=read_book(file_name);
	display_book_window(b);//Otherwise there's no point...
}

page * add_image_from_server(char *data, book *b, page *p)
{
	return p;
}

void add_book_from_server(char * data, int len)
{/*
	char buffer[1024];
	book *b;
	page *p;
	_image *i;
	int l=*data++;//Title length
	strncpy(buffer,data,l);
	b=create_book(buffer,*(data+l),*((Uint16)*(data+l+1)));
	data+=l+3;
	do {
		switch(*data){
			case TEXT:
				p=add_str_to_page(buffer,TEXT,b,p);
				break;
			case AUTHOR:
				p=add_str_to_page(buffer,AUTHOR,b,p);
				break;
			case _IMAGE:
				p=add_image_from_server(buffer, b, p);
		}
	} while(len--);
*/}

/*Generic display*/

void display_image(_image *i)
{
	glColor4f(1.0f,1.0f,1.0f,0.5f);
	get_and_set_texture_id(i->texture);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.05f);
	glBegin(GL_QUADS);
		glTexCoord2f(i->u[0],i->v[1]); 	glVertex2i(i->x,i->y+i->h);
		glTexCoord2f(i->u[1],i->v[1]);	glVertex2i(i->x+i->w,i->y+i->h);
		glTexCoord2f(i->u[1],i->v[0]);	glVertex2i(i->x+i->w,i->y);
		glTexCoord2f(i->u[0],i->v[0]);	glVertex2i(i->x,i->y);
	glEnd();
	glDisable(GL_ALPHA_TEST);
}

void display_page(book * b, page * p)
{
	char ** l;
	int i;
	char str[20];
	
	if(!p)return;
	
	set_font(2);
	if(p->image) {
		display_image(p->image);
	}

	for(i=0, l=p->lines; *l; i++,l++){
		glColor3f(0.385f,0.285f, 0.19f);
		draw_string_zoomed(10,i*18*0.9f,*l,0,1.0f);
	}
	
	glColor3f(0.385f,0.285f, 0.19f);
	
	sprintf(str,"%d",p->page_no);
	if(b->type==1)draw_string_zoomed(140,b->max_lines*18*0.9f+2,str,0,1.0);
	else if(b->type==2)draw_string_zoomed(110,b->max_lines*18*0.9f+2,str,0,1.0);
	set_font(0);
}

void display_book(book * b, int type)
{
	page ** p=&b->pages[b->active_page];
	int x=0;
	switch(type){
		case 2:
			glPushMatrix();
			glTranslatef(x,0,0);
			display_page(b,*p);
			glPopMatrix();
			if(b->no_pages<=b->active_page)break;
			p++;
			x+=250;
		case 1:
		default:
			glPushMatrix();
			glTranslatef(x,0,0);
			display_page(b,*p);
			glPopMatrix();
			break;
	}
}

/*Book window*/

int book_win=-1;
int paper_win=-1;
int book_win_x=100;
int book_win_y=100;
int book_win_x_len=400;
int book_win_y_len=300;
int book1_text;

int display_book_handler(window_info *win)
{
	int x=32,i,p;
	char str[20];
	book *b=win->data;
	
	if(!b) {
		toggle_window(book_win);
		return 1;
	}
	switch(b->type){
		case 1: get_and_set_texture_id(paper1_text);
			break;
		case 2: get_and_set_texture_id(book1_text);
			break;
	}
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f,1.0f); glVertex3i(0,0,0);
		glTexCoord2f(0.0f,0.0f); glVertex3i(0,win->len_y-20,0);
		glTexCoord2f(1.0f,0.0f); glVertex3i(win->len_x,win->len_y-20,0);
		glTexCoord2f(1.0f,1.0f); glVertex3i(win->len_x,0,0);
	glEnd();
	glPushMatrix();
	if(b->type==1)glTranslatef(15,25,0);
	else if(b->type==2)glTranslatef(30,15,0);
	display_book(b, b->type);
	glPopMatrix();
	
	glColor3f(0.77f,0.59f, 0.38f);
	
	glPushMatrix();
	glTranslatef(0,win->len_y-18,0);
	x=10;
	draw_string(10,-2,"<-",0);
	draw_string(win->len_x-33,-2,"->",0);
	if(b->type==1) {
		x=50;
		p=b->active_page-5;
		if(p>=0){
			sprintf(str,"%d",p+1);
			draw_string(x,0,str,0);
		}
		x=100;
		p=b->active_page-2;
		if(p>=0){
			sprintf(str,"%d",p+1);
			draw_string(x,0,str,0);
		}
		x=win->len_x-120;
		p=b->active_page+2;
		if(p<b->no_pages){
			sprintf(str,"%d",p+1);
			draw_string(x,0,str,0);
		}
		x=win->len_x-70;
		p=b->active_page+5;
		if(p<b->no_pages){
			sprintf(str,"%d",p+1);
			draw_string(x,0,str,0);
		}
	} else if(b->type==2) {
		x=win->len_x/2-60;
		for(i=1;i<5;i++){
			p=b->active_page-i*b->type;
			if(p>=0){
				sprintf(str,"%d",p+1);
				draw_string(x,0,str,0);
			}
			x-=40;
		}
		x=win->len_x/2+50;
		for(i=1;i<5;i++){
			p=b->active_page+i*b->type;
			if(p<b->no_pages){
				sprintf(str,"%d",p+1);
				draw_string(x,0,str,0);
			}
			x+=40;
		}
	}
	
	draw_string(win->len_x/2-15,0,"[X]",0);
	glPopMatrix();
        return 1;
}

int click_book_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int i,x,p;
	book *b=win->data;
	my-=win->len_y;
	if(my<-2 && my>-18) {
		if(mx>10 && mx < 20){
			if(b->active_page-b->type>=0)b->active_page-=b->type;
		} else if(mx>win->len_x-20 && mx<win->len_x-10){
			if(b->active_page+b->type<b->no_pages)b->active_page+=b->type;
		}
		if(b->type==1){
			x=50;
			p=b->active_page-5;
			if(p>=0 && mx>=x&&mx<x+25){
				b->active_page-=5;
			}
			x=100;
			p=b->active_page-2;
			if(p>=0 && mx>=x&&mx<x+25){
				b->active_page-=2;
			}
			x=140;
			
			if(mx>win->len_x/2-15 && mx < win->len_x/2+15) win->displayed=0;
			
			x=win->len_x-120;
			p=b->active_page+2;
			if(p<b->no_pages && mx>=x && mx<x+25){
				b->active_page+=2;
			}
			x=win->len_x-70;
			p=b->active_page+5;
			if(p<b->no_pages && mx>=x && mx<x+25){
				b->active_page+=5;
			}
		} else if(b->type==2){
			x=win->len_x/2-60;
			for(i=1;i<5;i++){
				p=b->active_page-i*b->type;
				if(mx>=x && mx<x+25 && p>=0){
					b->active_page-=i*b->type;
				}
				x-=40;
			}
			
			if(mx>win->len_x/2-15 && mx < win->len_x/2+15) win->displayed=0;
			
			x=win->len_x/2+50;
			for(i=1;i<5;i++){
				p=b->active_page+i*b->type;
				if(mx>=x && mx<x+25 && p<b->no_pages){
					b->active_page+=i*b->type;
				}
				x+=40;
			}
		}
	}
	return 1;
}

void display_book_window(book *b)
{
	int *p;
	if(!b)return;
	if(b->type==1)p=&paper_win;
	else p=&book_win;
        if(*p<0){
                if(b->type==1)
#ifndef OLD_EVENT_HANDLER
			*p=create_window(b->title, game_win, 0, book_win_x, book_win_y, 320, 400, ELW_TITLE_NAME|ELW_TITLE_BAR|ELW_DRAGGABLE|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW);
#else
			*p=create_window(b->title, -1, 0, book_win_x, book_win_y, 320, 400, ELW_TITLE_NAME|ELW_TITLE_BAR|ELW_DRAGGABLE|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW);
#endif
		else if(b->type==2)
#ifndef OLD_EVENT_HANDLER
			*p=create_window(b->title, game_win, 0, book_win_x, book_win_y, 528, 320, ELW_TITLE_NAME|ELW_TITLE_BAR|ELW_DRAGGABLE|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW); //width/height are different
#else
			*p=create_window(b->title, -1, 0, book_win_x, book_win_y, 528, 320, ELW_TITLE_NAME|ELW_TITLE_BAR|ELW_DRAGGABLE|ELW_USE_BACKGROUND|ELW_USE_BORDER|ELW_SHOW); //width/height are different
#endif
                set_window_handler(*p, ELW_HANDLER_DISPLAY, &display_book_handler);
                set_window_handler(*p, ELW_HANDLER_CLICK, &click_book_handler);
		windows_list.window[*p].data=b;
        } else {
		if((long int)windows_list.window[*p].data!=(long int)b){
			strcpy(windows_list.window[*p].window_name,b->title);
			windows_list.window[*p].data=b;
			if(!windows_list.window[*p].displayed) show_window(*p);
		}else toggle_window(*p);
        }
}

