#ifndef __MISC_H__
#define __MISC_H__

void reset_under_the_mouse();
int anything_under_the_mouse(int object_id, int object_type);
void find_last_url(char * source_string, int len);
int go_to_url(void *dummy);

#endif
