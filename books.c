#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <string.h>
#include "books.h"
#include "asc.h"
#include "draw_scene.h"
#include "errors.h"
#include "elwindows.h"
#include "init.h"
#include "knowledge.h"
#include "multiplayer.h"
#include "new_character.h"
#include "textures.h"
#include "translate.h"
#ifdef OPENGL_TRACE
#include "gl_init.h"
#endif
#include "io/elfilewrapper.h"

#ifdef BSD
 #include <stdlib.h>
#else
 #ifdef OSX
  #include <sys/malloc.h>
 #else
  #ifndef alloca         // newer versions of SDL have their own alloca!
   #include <malloc.h>
  #endif   //alloca
 #endif   //OSX
#endif   //BSD

/* NOTE: This file contains implementations of the following, currently unused, and commented functions:
 *          Look at the end of the file.
 *
 * int have_book(int);
 */

#define _TITLE 0
#define _AUTHOR 1
#define _TEXT 2
#define _IMAGE 3
#define _IMAGE_TEXT 4
#define _PAGE 6

#define LOCAL 0
#define SERVER 1

#define KNOWLEDGE_BOOK_OFFSET 10000

int book_opened=-1;//The ID of the book opened

static int paper1_text = -1; // Index in the texture cache of the paper texture
static int book1_text = -1;  // Index in the texture cache of the book texture

typedef struct {
	char file[200];
	
	int x;
	int y;

	int w;
	int h;
	
	int texture;

	int u[2];
	int v[2];
} _image;

typedef struct {
	char ** lines;
	_image * image;
	int page_no;
} page;

typedef struct _book {
	char title[35];
	int id;
	
	int type;
	
	int no_pages;
	page ** pages;
	int max_width;
	int max_lines;
	
	int server_pages;
	int have_server_pages;
	int pages_to_scroll;

	int active_page;

	struct _book * next;
} book;

book * books=NULL;

void add_book(book *bs);
void display_book_window(book *b);

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

book *create_book (const char* title, int type, int id)
{
	book *b=(book*)calloc(1,sizeof(book));
	
	switch(type){
		case 2: 
			b->max_width=20;
			b->max_lines=15;
			type=2;
			break;
		case 1:	
		default:
			b->max_width=29;
			b->max_lines=20;
			type=1;
			break;
	}
	b->type=type;
	b->id=id;
	safe_snprintf(b->title, sizeof(b->title), "%s", title);
	
	add_book(b);
	
	return b;
}

_image *create_image (const char* file, int x, int y, int w, int h, float u_start, float v_start, float u_end, float v_end)
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

#ifdef	NEW_TEXTURES
	img->texture = load_texture_cached(file, tt_image);
#else	/* NEW_TEXTURES */
	img->texture=load_texture_cache(file,0);
#endif	/* NEW_TEXTURES */
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
	free(p);
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

/*Multiple book handling*/

book * get_book(int id)
{
	book *b;
	
	for(b=books;b;b=b->next){
		if(b->id==id) break;
	}
	
	return b;
}

void add_book(book *bs)
{
	book *b=books;
	if(b) {
		for(;b->next;b=b->next);
		b->next=bs;
	} else {
		books=bs;
	}
}

/*Book parser*/

page * add_str_to_page(char * str, int type, book *b, page *p)
{
	char ** lines=NULL;
	char ** newlines=NULL;
	char ** newlines_ptr;
	int i;

	if(!str) return NULL;
	if(!p)p=add_page(b);
	
	newlines=get_lines(str, b->max_width);
	newlines_ptr = newlines;
	lines=p->lines;
	
	for(i=0;*lines;i++,lines++);

	if(type==_AUTHOR){
		*lines++=(char*)calloc(1,sizeof(char));
	} else if (type==_TITLE){
		*lines++=(char*)calloc(1,sizeof(char));
	}

	for(;newlines && *newlines;i++) {
		if(type==_AUTHOR){
			memmove(*newlines+1,*newlines,strlen(*newlines)+1);
			**newlines = to_color_char (c_orange3);
		} else if(type==_TITLE){
			memmove(*newlines+1,*newlines,strlen(*newlines)+1);
			**newlines = to_color_char (c_orange4);
		}
		if(i>=b->max_lines){
			*lines=NULL;
			p=add_page(b);
			lines=p->lines;
			i=0;
		}
		*lines++=*newlines++;
		// Grum: don't free *newlines, it's the pointer to actual line.
		// It's this pointer that is copied to p->lines, not the data
		// that it points to.
		//free(*newlines);
	}
	// This is a temporary array that holds the pointers to the lines. It 
	// can safely be freed.
	free(newlines_ptr);

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
				for(j=0;j<max_width && last[j] && last[j]!=' ' && last[j]!='\n' && last[j]!='\r';j++);
				if(i+j<x||(i>=x+w && i+j<max_width)){
					for(;j-->=0;line++,last++){
						if(*last=='\r')
							last++;
						if(*last=='\n'){
							last++;
							i=max_width;
							break;
						}
						*line=*last;
						i++;
						if(!*last)
							break;
					}
				} else {
					*line++=' ';
				}
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
	int i=0;
	int max_width;
	int max_lines;
	int h, w, x, y;

	if (img == NULL || b == NULL) return NULL;
	if(!p || p->image)p=add_page(b);
	
	max_width=b->max_width;
	max_lines=b->max_lines;

	h=img->h/16+1;
	y=img->y/16+1;
	w=img->w/9;
	x=img->x/10;

	if(y+h>max_lines || w+x>max_width) return NULL;
	
	line=p->lines;

	for(;line[i];i++);

	if(i+h>=max_lines||y<=i) {
		p=add_page(b);
		line=p->lines;
		i=0;
	}

	p->image=img;

	if(in_text){
		line+=i;

		last_ptr=in_text;

		for(i=0;i<h;i++,line++){//
			*line=(char*)malloc((max_width+2)*sizeof(char));
			if(i && (w<(max_width/3*2) && x+w<max_width) && last_ptr && *last_ptr){//If it's more than 2/3rd of the page width, don't work on it
				last_ptr=wrap_line_around_image(*line,w,x,b->max_width+1,last_ptr);
			} else {
				**line=0;
			}
		}

		if(*last_ptr)
			p=add_str_to_page(last_ptr,_IMAGE_TEXT,b,p);
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

	x=xmlGetInt(cur,(xmlChar*)"x");
	y=xmlGetInt(cur,(xmlChar*)"y");
	w=xmlGetInt(cur,(xmlChar*)"w");
	h=xmlGetInt(cur,(xmlChar*)"h");
	
	u_start=xmlGetFloat(cur,(xmlChar*)"u_start");
	u_end=xmlGetFloat(cur,(xmlChar*)"u_end");
	if(!u_end)
		u_end=1;
	
	v_start=xmlGetFloat(cur,(xmlChar*)"v_start");
	if(!v_start)
		v_start=1;
	v_end=xmlGetFloat(cur,(xmlChar*)"v_end");
	
	image_path=(char*)xmlGetProp(cur,(xmlChar*)"src");
	if(!image_path) return;

	img=create_image(image_path, x, y, w, h, u_start, v_start, u_end, v_end);

	if(cur->children && cur->children->content){
		MY_XMLSTRCPY(&text, (char*)cur->children->content);
	}
	
	if(add_image_to_page(text, img, b, p)==NULL)
		free(img);
	
	xmlFree(image_path);
	if(text)
		free(text);
}

void add_xml_str_to_page(xmlNode * cur, int type, book * b, page *p)
{
	char * string=NULL;
	if(cur->children && cur->children->content && MY_XMLSTRCPY(&string, (char*)cur->children->content)!=-1){
		add_str_to_page(string, type, b, p);
	} else {
#ifndef OSX
		LOG_ERROR("An error occured when parsing the content of the <%s>-tag on line %d - Check it for letters that cannot be translated into iso8859-1\n", cur->name, cur->line);
#else
		LOG_ERROR("An error occured when parsing the content of the <%s>-tag - Check it for letters that cannot be translated into iso8859-1\n", cur->name);
#endif
	}
	free(string);
}

void add_xml_page(xmlNode *cur, book * b)
{
	page *p=add_page(b);
	for(;cur;cur=cur->next){
		if(cur->type == XML_ELEMENT_NODE){
			if (!xmlStrcasecmp(cur->name,(xmlChar*)"title")){
				add_xml_str_to_page(cur,_TITLE,b,p);
			} else if (!xmlStrcasecmp(cur->name,(xmlChar*)"author")){
				add_xml_str_to_page(cur,_AUTHOR,b,p);
			} else if (!xmlStrcasecmp(cur->name,(xmlChar*)"text")){
				add_xml_str_to_page(cur,_TEXT,b,p);
			} else if (!xmlStrcasecmp(cur->name,(xmlChar*)"image")){
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
			if(!xmlStrcasecmp(cur->name,(xmlChar*)"page")){
				add_xml_page(cur->children,b);
			}
		}
	}

	return b;
}

book * read_book(char * file, int type, int id)
{
	xmlDoc * doc;
	xmlNode * root=NULL;
	xmlChar *title=NULL;
	book *b=NULL;
	char path[1024];

	safe_snprintf(path, sizeof(path), "languages/%s/%s", lang, file);

	if ((doc = xmlReadFile(path, NULL, 0)) == NULL) {
		safe_snprintf(path, sizeof(path), "languages/en/%s", file);
	}
	if(doc == NULL && ((doc = xmlReadFile(path, NULL, 0)) == NULL)) {
		char str[200];

		safe_snprintf(str, sizeof(str), book_open_err_str, path);
		LOG_ERROR(str);
		LOG_TO_CONSOLE(c_red1,str);
	} else if ((root = xmlDocGetRootElement(doc))==NULL) {
		LOG_ERROR("Error while parsing: %s", path);
	} else if(xmlStrcasecmp(root->name,(xmlChar*)"book")){
		LOG_ERROR("Root element in %s is not <book>", path);
	} else if((title=xmlGetProp(root,(xmlChar*)"title"))==NULL){
		LOG_ERROR("Root element in %s does not contain a title=\"<short title>\" property.", path);
	} else {
		b=parse_book(root->children, (char*)title, type, id);
	}
	
	if(title) {
		xmlFree(title);
	}
	
	xmlFreeDoc(doc);

	return b;
}

void parse_knowledge_item(xmlNode *in)
{
	xmlNode * cur;
	int id = -1;
	char * strID=NULL;
	char * string=NULL;

	for(cur=in;cur;cur=cur->next){
		if(cur->type == XML_ELEMENT_NODE){
			if(!xmlStrcasecmp(cur->name,(xmlChar*)"Knowledge")){
				if ((strID=(char*)xmlGetProp(cur,(xmlChar*)"ID"))==NULL){
					LOG_ERROR("Knowledge Item does not contain an ID property.");
				} else {
					id = atoi(strID);
					if(cur->children && cur->children->content && MY_XMLSTRCPY(&string, (char*)cur->children->content)!=-1){
						if (read_book(string, 2, id + KNOWLEDGE_BOOK_OFFSET) != NULL) {
							knowledge_list[id].has_book = 1;
						}
					} else {
#ifndef OSX
						LOG_ERROR("An error occured when parsing the content of the <%s>-tag on line %d - Check it for letters that cannot be translated into iso8859-1\n", cur->name, cur->line);
#else
						LOG_ERROR("An error occured when parsing the content of the <%s>-tag - Check it for letters that cannot be translated into iso8859-1\n", cur->name);
#endif
					}
					xmlFree(strID);
					free(string);
					string = NULL;
				}
			}
		}
	}

	return;
}

void read_knowledge_book_index()
{
	xmlDoc * doc;
	xmlNode * root=NULL;
	char path[1024];
	
	if ((doc = xmlReadFile("knowledge.xml", NULL, 0)) == NULL) {
			LOG_TO_CONSOLE(c_red1, "Can't open knowledge book index");
	} else if ((root = xmlDocGetRootElement(doc))==NULL) {
		LOG_ERROR("Error while parsing: %s", path);
	} else if(xmlStrcasecmp(root->name,(xmlChar*)"Knowledge_Books")){
		LOG_ERROR("Root element in %s is not <Knowledge_Books>", path);
	} else {
		parse_knowledge_item(root->children);
	}
	
	xmlFreeDoc(doc);

	return;
}

void init_books()
{
#ifdef	NEW_TEXTURES
	paper1_text = load_texture_cached ("textures/paper1.dds", tt_image);
	book1_text = load_texture_cached ("textures/book1.dds", tt_image);
#else	/* NEW_TEXTURES */
	paper1_text = load_texture_cache_deferred ("./textures/paper1.bmp", 0);
	book1_text = load_texture_cache_deferred ("./textures/book1.bmp", 0);
#endif	/* NEW_TEXTURES */

	read_book("books/races/human.xml", 2, book_human);
	read_book("books/races/dwarf.xml", 2, book_dwarf);
	read_book("books/races/elf.xml", 2, book_elf);
	read_book("books/races/gnome.xml", 2, book_gnome);
	read_book("books/races/orchan.xml", 2, book_orchan);
	read_book("books/races/draegoni.xml", 2, book_draegoni);

	read_knowledge_book_index();
}

/*Network parser*/

void open_book(int id)
{
	book *b=get_book(id);

	if(!b) {
		char str[5];
		
		str[0]=SEND_BOOK;
		*((Uint16*)(str+1))=SDL_SwapLE16((Uint16)id);
		*((Uint16*)(str+3))=SDL_SwapLE16(0);

		my_tcp_send(my_socket, (Uint8*)str, 5);
	} else {
		display_book_window(b);
	}
}

void read_local_book (const char *data, int len)
{
	char file_name[200];
	book *b;

	safe_snprintf (file_name, sizeof(file_name), "%.*s", len-3, data+3);
	
	b = get_book (SDL_SwapLE16 (*((Uint16*)(data+1))));
	if (b == NULL)
	{
		b = read_book (file_name, data[0], SDL_SwapLE16 (*((Uint16*)(data+1))));
		if (b == NULL)
		{
			char str[200];
			safe_snprintf (str, sizeof(str), book_open_err_str, file_name);
			LOG_TO_CONSOLE(c_red1, str);
			return;
		}
	}
	
	display_book_window (b); // Otherwise there's no point...
}

page * add_image_from_server(char *data, book *b, page *p)
{
	int x, y;
	int w, h;
	int u_start, u_end;
	int v_start, v_end;
	char image_path[256];
	char text[512];
	int l=SDL_SwapLE16(*((Uint16*)(data)));
	_image *img;

	if(l>254)l=254;
	memcpy(image_path, data+2, l);
	image_path[l]=0;
	
	data+=l+2;

	l=SDL_SwapLE16(*((Uint16*)(data)));
	if(l>510)
		l=510;
	memcpy(text, data+2, l);
	text[l]=0;

	data+=l+2;

	x=SDL_SwapLE16(*((Uint16*)(data)));
	y=SDL_SwapLE16(*((Uint16*)(data+2)));
	w=SDL_SwapLE16(*((Uint16*)(data+4)));
	h=SDL_SwapLE16(*((Uint16*)(data+6)));

	u_start=data[8];
	u_end=data[9];
	v_start=data[10];
	v_end=data[11];

	img=create_image(image_path, x, y, w, h, u_start, v_start, u_end, v_end);
	if(add_image_to_page(text, img, b, p)==NULL) free(img);
	
	return p;
}

void read_server_book (const char *data, int len)
{
	char buffer[8192];
	book *b;
	page *p;
	int l = SDL_SwapLE16(*((Uint16*)(data+4)));
	int idx;

	if ( l >= sizeof (buffer) ) // Safer
		l = sizeof (buffer) - 1;
	memcpy (buffer, data+6, l);
	buffer[l] = '\0';
	
	b = get_book (SDL_SwapLE16 (*((Uint16*)(data+1))));
	if (b == NULL)
		b = create_book (buffer, data[0], SDL_SwapLE16 (*((Uint16*)(data+1))));

	b->server_pages = data[3];
	b->have_server_pages++;

	p=add_page(b);//Will create a page if pages is not found.

	idx = l + 6;
	while (idx <= len)
	{
		l = SDL_SwapLE16 (*((Uint16*)(&data[idx+1])));
		if ( l >= sizeof (buffer) ) // Safer.
			l = sizeof (buffer) - 1;
		memcpy (buffer, &data[idx+3], l);
		buffer[l]=0;

		switch (data[idx])
		{
			case _TEXT:
				p=add_str_to_page(buffer,_TEXT,b,p);
				break;
			case _AUTHOR:
				p=add_str_to_page(buffer,_AUTHOR,b,p);
				break;
			case _TITLE:
				p=add_str_to_page(buffer,_TITLE,b,p);
				break;
			case _IMAGE:
				p=add_image_from_server(buffer, b, p);
				break;
			case _PAGE:
				//p=add_page(b);
				break;
		}
		idx += l + 3;
	}

	b->active_page += b->pages_to_scroll;
	b->pages_to_scroll = 0;
	
	if (b) display_book_window (b); // Otherwise there's no point...
}


void read_network_book (const char *in_data, int data_length)
{
	switch (*in_data)
	{
		case LOCAL:
			read_local_book (&in_data[1], data_length-1);
			break;
		case SERVER:
			read_server_book (&in_data[1], data_length-1);
			break;
	}
}


/*Generic display*/

void display_image(_image *i)
{
	glColor4f(1.0f,1.0f,1.0f,0.5f);
#ifdef	NEW_TEXTURES
	bind_texture(i->texture);
#else	/* NEW_TEXTURES */
	get_and_set_texture_id(i->texture);
#endif	/* NEW_TEXTURES */
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.05f);
	glBegin(GL_QUADS);
		glTexCoord2f(i->u[0],i->v[1]); 	glVertex2i(i->x,i->y+i->h);
		glTexCoord2f(i->u[1],i->v[1]);	glVertex2i(i->x+i->w,i->y+i->h);
		glTexCoord2f(i->u[1],i->v[0]);	glVertex2i(i->x+i->w,i->y);
		glTexCoord2f(i->u[0],i->v[0]);	glVertex2i(i->x,i->y);
	glEnd();
	glDisable(GL_ALPHA_TEST);
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

void display_page(book * b, page * p)
{
	char ** l;
	int i;
	char str[20];
	
	if(!p)
		return;
	
	set_font(2);
	if(p->image) {
		display_image(p->image);
	}

	for(i=0, l=p->lines; *l; i++,l++){
		glColor3f(0.34f,0.25f, 0.16f);
		draw_string_zoomed(10,i*18*0.9f,(unsigned char*)*l,0,1.0f);
	}
	
	glColor3f(0.385f,0.285f, 0.19f);
	
	safe_snprintf(str,sizeof(str),"%d",p->page_no);
	if(b->type==1)
		draw_string_zoomed(140,b->max_lines*18*0.9f+2,(unsigned char*)str,0,1.0);
	else if(b->type==2)
		draw_string_zoomed(110,b->max_lines*18*0.9f+2,(unsigned char*)str,0,1.0);
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
			if(b->no_pages<=b->active_page)
				break;
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
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
}

/*Book window*/

int book_win=-1;
int paper_win=-1;
int book_win_x=100;
int book_win_y=100;
int book_win_x_len=400;
int book_win_y_len=300;

int book_mouse_x=0;
int book_mouse_y=0;

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
		case 1:
#ifdef	NEW_TEXTURES
			bind_texture(paper1_text);
#else	/* NEW_TEXTURES */
			get_and_set_texture_id(paper1_text);
#endif	/* NEW_TEXTURES */
			break;
		case 2:
#ifdef	NEW_TEXTURES
			bind_texture(book1_text);
#else	/* NEW_TEXTURES */
			get_and_set_texture_id(book1_text);
#endif	/* NEW_TEXTURES */
			break;
	}
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f,1.0f); glVertex3i(0,0,0);
		glTexCoord2f(0.0f,0.0f); glVertex3i(0,win->len_y-20,0);
		glTexCoord2f(1.0f,0.0f); glVertex3i(win->len_x,win->len_y-20,0);
		glTexCoord2f(1.0f,1.0f); glVertex3i(win->len_x,0,0);
	glEnd();
	glPushMatrix();
	if(b->type==1)
		glTranslatef(15,25,0);
	else if(b->type==2)
		glTranslatef(30,15,0);
	display_book(b, b->type);
	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(0,win->len_y-18,0);
	book_mouse_y-=(win->len_y-18);
	x=10;
	if(book_mouse_y>0 && book_mouse_y<18 && book_mouse_x>10 && book_mouse_x<(get_string_width((unsigned char*)"<-")*11.0f/12.0f)){
		glColor3f(0.95f, 0.76f, 0.52f);
		draw_string(10,-2,(unsigned char*)"<-",0);
		
		glColor3f(0.77f,0.59f, 0.38f);
		draw_string(win->len_x-33,-2,(unsigned char*)"->",0);
	} else if(book_mouse_y>0 && book_mouse_y<18 && book_mouse_x>win->len_x-33 && book_mouse_x<win->len_x-33+(get_string_width((unsigned char*)"->")*11.0f/12.0f)){
		glColor3f(0.95f, 0.76f, 0.52f);
		draw_string(win->len_x-33,-2,(unsigned char*)"->",0);
		
		glColor3f(0.77f,0.59f, 0.38f);
		draw_string(10,-2,(unsigned char*)"<-",0);
	} else {
		glColor3f(0.77f,0.59f, 0.38f);
		draw_string(10,-2,(unsigned char*)"<-",0);
		draw_string(win->len_x-33,-2,(unsigned char*)"->",0);
	}
	if(b->type==1) {
		x=50;
		p=b->active_page-5;
		if(p>=0){
			safe_snprintf(str,sizeof(str),"%d",p+1);

			if(book_mouse_y>0 && book_mouse_y<18 && book_mouse_x>x && book_mouse_x<x+(get_string_width((unsigned char*)str)*11.0f/12.0f)){
				glColor3f(0.95f, 0.76f, 0.52f);
				draw_string(x,0,(unsigned char*)str,0);
				glColor3f(0.77f,0.59f, 0.38f);
			} else 
				draw_string(x,0,(unsigned char*)str,0);
		}
		x=100;
		p=b->active_page-2;
		if(p>=0){
			safe_snprintf(str,sizeof(str),"%d",p+1);
			
			if(book_mouse_y>0 && book_mouse_y<18 && book_mouse_x>x && book_mouse_x<x+(get_string_width((unsigned char*)str)*11.0f/12.0f)){
				glColor3f(0.95f, 0.76f, 0.52f);
				draw_string(x,0,(unsigned char*)str,0);
				glColor3f(0.77f,0.59f, 0.38f);
			} else 
				draw_string(x,0,(unsigned char*)str,0);
		}
		x=win->len_x-120;
		p=b->active_page+2;
		if(p<b->no_pages){
			safe_snprintf(str,sizeof(str),"%d",p+1);
			
			if(book_mouse_y>0 && book_mouse_y<18 && book_mouse_x>x && book_mouse_x<x+(get_string_width((unsigned char*)str)*11.0f/12.0f)){
				glColor3f(0.95f, 0.76f, 0.52f);
				draw_string(x,0,(unsigned char*)str,0);
				glColor3f(0.77f,0.59f, 0.38f);
			} else 
				draw_string(x,0,(unsigned char*)str,0);
		}
		x=win->len_x-70;
		p=b->active_page+5;
		if(p<b->no_pages){
			safe_snprintf(str,sizeof(str),"%d",p+1);
			
			if(book_mouse_y>0 && book_mouse_y<18 && book_mouse_x>x && book_mouse_x<x+(get_string_width((unsigned char*)str)*11.0f/12.0f)){
				glColor3f(0.95f, 0.76f, 0.52f);
				draw_string(x,0,(unsigned char*)str,0);
				glColor3f(0.77f,0.59f, 0.38f);
			} else 
				draw_string(x,0,(unsigned char*)str,0);
		}
	} else if(b->type==2) {
		x=win->len_x/2-60;
		for(i=1;i<5;i++){
			p=b->active_page-i*b->type;
			if(p>=0){
				safe_snprintf(str,sizeof(str),"%d",p+1);
				
				if(book_mouse_y>0 && book_mouse_y<18 && book_mouse_x>x && book_mouse_x<x+(get_string_width((unsigned char*)str)*11.0f/12.0f)){
					glColor3f(0.95f, 0.76f, 0.52f);
					draw_string(x,0,(unsigned char*)str,0);
					glColor3f(0.77f,0.59f, 0.38f);
				} else
					draw_string(x,0,(unsigned char*)str,0);
			}
			x-=40;
		}
		x=win->len_x/2+50;
		for(i=1;i<5;i++){
			p=b->active_page+i*b->type;
			if(p<b->no_pages){
				safe_snprintf(str,sizeof(str),"%d",p+1);
				
				if(book_mouse_y>0 && book_mouse_y<18 && book_mouse_x>x && book_mouse_x<x+(get_string_width((unsigned char*)str)*11.0f/12.0f)){
					glColor3f(0.95f, 0.76f, 0.52f);
					draw_string(x,0,(unsigned char*)str,0);
					glColor3f(0.77f,0.59f, 0.38f);
				} else
					draw_string(x,0,(unsigned char*)str,0);
			}
			x+=40;
		}
	}
	
	if(book_mouse_y>0 && book_mouse_y<18 && book_mouse_x>win->len_x/2-15 && book_mouse_x<win->len_x/2+15)
		glColor3f(0.95f, 0.76f, 0.52f);
	
	draw_string(win->len_x/2-15,0,(unsigned char*)"[X]",0);
	glPopMatrix();
#ifdef OPENGL_TRACE
CHECK_GL_ERRORS();
#endif //OPENGL_TRACE
	return 1;
}

int click_book_handler(window_info *win, int mx, int my, Uint32 flags)
{
	int i,x,p;	
	book *b=win->data;

	// only handle mouse button clicks, not scroll wheels moves
	if ( (flags & ELW_MOUSE_BUTTON) == 0) return 0;
	
	my -= win->len_y;
	if(my<-2 && my>-18) {
		if(mx>10 && mx < 20){
			if(b->have_server_pages<b->server_pages) {
				if(b->active_page-b->type >= 0)
					b->active_page -= b->type;
				//We'll always have the first pages, you can't advance from 0-20 in 1 jump but must get them all.
				//TODO: Make it possible to jump that many pages.
			} else if(b->active_page-b->type >= 0) {
				b->active_page-=b->type;
			}
		} else if(mx>win->len_x-20 && mx<win->len_x-10){
			if(b->have_server_pages<b->server_pages){
				//Get a 2 new pages...
				char str[5];
				int id=b->id;
				int pages=b->have_server_pages;
		
				str[0]=SEND_BOOK;
				*((Uint16*)(str+1))=SDL_SwapLE16(id);
				*((Uint16*)(str+3))=SDL_SwapLE16(pages);
				my_tcp_send(my_socket, (Uint8*)str, 5);

				if(b->active_page+b->type<b->no_pages)
					b->active_page+=b->type;
				else
					b->pages_to_scroll=b->type;
			} else if(b->active_page+b->type<b->no_pages) {
				b->active_page+=b->type;
			}
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
			
			if(mx>win->len_x/2-15 && mx < win->len_x/2+15) {
//				char str[5];
//				int id=b->id;

//				// Lachesis: Please either fix this branching condition or remove it.
//				if (10000 > id > 11000) {
//					str[0]=SEND_BOOK;
//					*((Uint16*)(str+1))=SDL_SwapLE16(id);
//					*((Uint16*)(str+3))=SDL_SwapLE16(0xFFFF); // Swap not actually necessary.. But it's cleaner.
//					my_tcp_send(my_socket, str, 5);
//				}
				
				hide_window(win->window_id);
				book_opened=-1;
			}
			
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
			
			if(mx>win->len_x/2-15 && mx < win->len_x/2+15) {
//				char str[5];
//				int id=b->id;
		
//				// Lachesis: Please either fix this branching condition or remove it.
//				if (10000 > id > 11000) {
//					str[0]=SEND_BOOK;
//					*((Uint16*)(str+1))=SDL_SwapLE16(id);
//					*((Uint16*)(str+3))=SDL_SwapLE16(0xFFFF);
//					my_tcp_send(my_socket, str, 5);
//				}
				
				hide_window(win->window_id);
				book_opened=-1;
			}
			
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

int mouseover_book_handler(window_info * win, int mx, int my)
{
	//Save for later
	book_mouse_x=mx;
	book_mouse_y=my;
	
	return 0;
}


void display_book_window(book *b)
{
	int *p;

	if(!b)
		return;

	if(b->type==1){
		p=&paper_win;
		if(book_win!=-1)
			hide_window(book_win);
	} else {
		p=&book_win;
		if(paper_win!=-1)
			hide_window(paper_win);
	}
	book_opened = b->id;
	if(*p<0){
		if(b->type==1)
			*p=create_window(b->title, -1, 0, book_win_x, book_win_y, 320, 400, ELW_WIN_DEFAULT^ELW_CLOSE_BOX);
		else if(b->type==2)
			*p=create_window(b->title, -1, 0, book_win_x, book_win_y, 528, 320, ELW_WIN_DEFAULT^ELW_CLOSE_BOX); //width/height are different

		set_window_handler(*p, ELW_HANDLER_DISPLAY, &display_book_handler);
		set_window_handler(*p, ELW_HANDLER_MOUSEOVER, &mouseover_book_handler);
		set_window_handler(*p, ELW_HANDLER_CLICK, &click_book_handler);
		windows_list.window[*p].data=b;
	} else {
		if((point)windows_list.window[*p].data!=(point)b) {
			safe_snprintf(windows_list.window[*p].window_name, sizeof(windows_list.window[*p].window_name), "%s", b->title);
			windows_list.window[*p].data=b;
			if(!get_show_window(*p))
				show_window(*p);
			select_window(*p);
		} else if(!get_show_window(*p)) {
			show_window(*p);
			select_window(*p);
		}
	}
}

void close_book(int book_id)
{
	book *b=get_book(book_id);

	if(!b)
		return;
	if(book_win!=-1) {
		if((point)windows_list.window[book_win].data==(point)b) {
			hide_window(book_win);
		}
	}
	if(paper_win!=-1) {
		if((point)windows_list.window[paper_win].data == (point)b) {
			hide_window(paper_win);
		}
	}

	book_opened=-1;
}

void free_books()
{
	book *b,*l=NULL;
	for(b=books;b;l=b){
		b=b->next;
		if (l)
			free_book(l);
	}
}

/* currently UNUSED
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
*/
