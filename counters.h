#ifdef COUNTERS

#ifndef __COUNTERS_H__
#define __COUNTERS_H___

#ifdef __cplusplus
extern "C" {
#endif

extern int harvesting;
extern char harvest_name[32];
extern int counters_win;

void load_counters();
void flush_counters();
void cleanup_counters();
void fill_counters_win();

void increment_death_counter(actor *a);
void increment_harvest_counter(int quantity);
void decrement_harvest_counter(int quantity);
void increment_alchemy_counter();
void increment_crafting_counter();
void increment_potions_counter();
void increment_manufacturing_counter();
void increment_spell_counter(int spell_id);
void increment_summon_counter(char *string);

void counters_set_product_name(char *name);
void counters_set_spell_name(int spell_id, char *name, int len);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __COUNTERS_H__ */

#endif /* COUNTERS */
