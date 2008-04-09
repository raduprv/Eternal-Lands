#ifndef __textaliases_h__
#define __textaliases_h__


#define NUMERIC_ALIASES_FILENAME "alias.ini"

int init_text_aliases ();
void shutdown_text_aliases ();
int alias_command (char *text, int len);
int unalias_command (char *text, int len);
int aliases_command (char *text, int len);

int process_text_alias (char *text, int len);

#endif
