#ifndef __BOOKS_H__
#define __BOOKS_H__

typedef struct {
	char file[200];
	
	int x;//Characters
	int y;//Lines
	int w;//Width
	int h;//Height
	
	GLuint texture;
	int u[2];
	int v[2];
} _image;

typedef struct {
	char ** lines;
	_image * image;
	int page_no;
} page;

typedef struct {
	char title[35];	
	int id;//The book ID
	
	int type;
	
	int no_pages;
	page ** pages;
	int max_width;//The page width in characters
	int max_lines;
	
	int active_page;
} book;

struct _books {
	book *b;
	struct _books *n;
};

extern int book1_text;

book * create_book(char * title, int type, int id);
page * add_page(book * b);
_image *create_image(char * file, int x, int y, int w, int h, float u_start, float v_start, float u_end, float v_end);
void free_page(page * p);
void free_book(book * b);
void free_books(struct _books *b);

int have_book(int id);
book * get_book(int id);
void add_book(book *bs, struct _books **b);

char * wrap_line_around_image(char * line, int w, int x, int max_width, char * last);
int add_image_to_page(char * text, _image *img, book * b, page *p);
void add_str_to_page(char * str, int type, book *b, page *p);
void add_xml_image_to_page(xmlNode * cur, book * b, page *p);
void add_xml_str_to_page(xmlNode * cur, char * in, int type, book * b, page *p);
void add_xml_page(xmlNode *cur, book * b);
book * parse_book(xmlNode *in, char * title, int type, int id);
book * read_book(char * file);
void read_local_book(char * data, int len);

void display_book_window(book *b);

#endif
