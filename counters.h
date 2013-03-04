
#ifndef __COUNTERS_H__
#define __COUNTERS_H___

#include "actors.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int harvesting;
extern char harvest_name[32];
extern int counters_win;
extern Uint32 disconnect_time;
extern char last_spell_name[60];
extern unsigned int floating_counter_flags;
extern int floating_session_counters;

void load_counters();
void flush_counters();
void cleanup_counters();
void fill_counters_win();
void reset_session_counters();
void print_session_counters(const char *category);

void increment_death_counter(actor *a);
void increment_critfail_counter(char *name);
void increment_harvest_counter(int quantity);
void decrement_harvest_counter(int quantity);
void increment_alchemy_counter();
void increment_crafting_counter();
void increment_engineering_counter();
void increment_tailoring_counter();
void increment_potions_counter();
void increment_manufacturing_counter();
void increment_spell_counter(int spell_id);
void increment_summon_manu_counter();
void increment_summon_counter(char *string);
int chat_to_counters_command(const char *text, int len);
void catch_counters_text(const char* text);

void counters_set_product_info(char *name, int count);
void counters_set_spell_name(int spell_id, char *name, int len);
int is_death_message (const char * RawText);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __COUNTERS_H__ */

