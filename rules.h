#ifndef __RULES_H__
#define __RULES_H__

struct rules_struct {
	int no;
	struct rule_struct {
		char * short_desc;
		int short_len;
		char * long_desc;
		int long_len;
		int type;
	} rule[40];
};

typedef struct {
	int type;
	int show_long_desc;
	int mouseover;
	int highlight;//Used for moderators
	int x_start;
	int y_start;
	int x_end;
	int y_end;
	char ** short_str;
	char ** long_str;
} rule_string;


extern int have_rules;
extern int countdown;
extern int has_accepted;

int read_rules(void);
void free_rules(rule_string * d);
void draw_rules_interface(void);
void toggle_rules_window(int toggle);
rule_string * get_interface_rules(int chars_per_line);
void init_rules_interface(int next, float text_size, int countdown);
void check_mouse_rules_interface(rule_string * rules, int lenx, int leny, int mx, int my);
int draw_rules(rule_string * rules, int rules_no, int x, int y, int lenx, int leny, float text_size);
void cleanup_rules(void);
void reset_rules(rule_string * r);
void highlight_rule(int type, Uint8 * rules, int no);

#endif
