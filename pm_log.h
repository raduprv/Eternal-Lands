#ifndef __PM_LOG_H__
#define __PM_LOG_H__

typedef struct
{
        int msgs;
        char * name;
        char ** messages;
} afk_struct;

struct pm_struct
{
        int msgs;
        int ppl;
        afk_struct * afk_msgs;
};

struct pm_struct pm_log;

extern int afk;
extern int last_action_time;
extern int afk_time;
extern char afk_message[160];

void free_pm_log(void);
void go_afk(void);
void go_ifk(void);
void add_message_to_pm_log(char * msg, int len);
void add_name_to_pm_log(char *name, int len);
void send_afk_message(char * server_msg, int type);
void print_return_message(void);
void print_message(int no);
int have_name(char *name, int len);
int is_talking_about_me(Uint8 * server_msg, int len);
void print_return_message(void);
void print_message(int no);

#endif
