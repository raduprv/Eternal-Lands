#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef __GNUC__
 #include <dirent.h>
 #include <unistd.h>
#endif
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include "astrology.h"
#include "init.h"
#include "2d_objects.h"
#include "actor_scripts.h"
#include "asc.h"
#include "books.h"
#include "buddy.h"
#include "chat.h"
#include "colors.h"
#include "console.h"
#include "consolewin.h"
#include "counters.h"
#include "cursors.h"
#include "dialogues.h"
#include "draw_scene.h"
#include "e3d.h"
#include "elconfig.h"
#include "elwindows.h"
#include "encyclopedia.h"
#include "errors.h"
#include "filter.h"
#include "framebuffer.h"
#include "gamewin.h"
#include "gl_init.h"
#include "hud.h"
#include "items.h"
#include "item_lists.h"
#include "keys.h"
#include "knowledge.h"
#include "langselwin.h"
#include "lights.h"
#include "loading_win.h"
#include "loginwin.h"
#include "multiplayer.h"
#include "manufacture.h"
#include "astrology.h"
#include "mapwin.h"
#include "missiles.h"
#include "named_colours.h"
#include "new_actors.h"
#include "openingwin.h"
#include "particles.h"
#include "questlog.h"
#include "reflection.h"
#include "rules.h"
#include "servers.h"
#include "sound.h"
#include "spells.h"
#include "storage.h"
#include "tabs.h"
#include "textures.h"
#include "tiles.h"
#include "timers.h"
#include "trade.h"
#include "translate.h"
#include "update.h"
#include "weather.h"
#include "eye_candy_wrapper.h"
#include "minimap.h"
#include "io/elpathwrapper.h"
#include "io/elfilewrapper.h"
#include "io/xmlcallbacks.h"
#ifdef PAWN
#include "pawn/elpawn.h"
#endif // PAWN
#include "sky.h"
#include "mines.h"
#include "popup.h"
#ifdef TEXT_ALIASES
#include "text_aliases.h"
#endif /* TEXT_ALIASES */
#include "user_menus.h"
#include "emotes.h"
#ifdef	NEW_TEXTURES
#include "image_loading.h"
#endif	/* NEW_TEXTURES */
#include "io/fileutil.h"
#ifdef  CUSTOM_UPDATE
#include "custom_update.h"
#endif  //CUSTOM_UPDATE

#define	CFG_VERSION 7	// change this when critical changes to el.cfg are made that will break it

int ini_file_size=0;

int disconnected= 1;
int auto_update= 1;
#ifdef  CUSTOM_UPDATE
int custom_update= 1;
int custom_clothing= 1;
#endif  //CUSTOM_UPDATE

int exit_now=0;
int restart_required=0;
int allow_restart=1;
int poor_man=0;

#ifdef ANTI_ALIAS
int anti_alias=0;
#endif  //ANTI_ALIAS

int special_effects=0;

int isometric=1;
int mouse_limit=15;
int no_adjust_shadows=0;
int clouds_shadows=1;
int item_window_on_drop=1;
int buddy_log_notice=1;
char configdir[256]="./";
#ifdef DATA_DIR
char datadir[256]=DATA_DIR;
#else
char datadir[256]="./";
#endif //DATA_DIR

char lang[10] = "en";
static int no_lang_in_config = 0;

int video_mode_set=0;

#ifdef OSX
int emulate3buttonmouse=0;
#endif

void read_command_line(); //from main.c

#ifndef FASTER_MAP_LOAD
static void load_harvestable_list()
{
	FILE *f = NULL;
	int i = 0;
	char strLine[255];

	memset(harvestable_objects, 0, sizeof(harvestable_objects));
	f = open_file_data("harvestable.lst", "rb");
	if(f == NULL) {
		LOG_ERROR("%s: %s \"harvestable.lst\": %s\n", reg_error_str, cant_open_file, strerror(errno));
		return;
	}
	while(1)
	{
		if (fscanf (f, "%254s", strLine) != 1)
			break;
		my_strncp (harvestable_objects[i], strLine, sizeof (harvestable_objects[i]));

		i++;
		if(!fgets(strLine, sizeof(strLine), f)) {
			break;
		}
	}
	fclose(f);
}

static void load_entrable_list()
{
	FILE *f = NULL;
	int i=0;
	char strLine[255];

	memset(entrable_objects, 0, sizeof(entrable_objects));
	i=0;
	f=open_file_data("entrable.lst", "rb");
	if(f == NULL){
		LOG_ERROR("%s: %s \"entrable.lst\": %s\n", reg_error_str, cant_open_file, strerror(errno));
		return;
	}
	while(1)
		{
			if (fscanf (f, "%254s", strLine) != 1)
				break;
			my_strncp (entrable_objects[i], strLine, sizeof (entrable_objects[i]));

			i++;
			if(!fgets(strLine, sizeof(strLine), f))break;
		}
	fclose(f);
}
#endif // FASTER_MAP_LOAD

void load_knowledge_list()
{
	FILE *f = NULL;
	int i=0;
	char strLine[255];
	char *out;

	memset(knowledge_list, 0, sizeof(knowledge_list));
	i= 0;
	knowledge_count= 0;
	// try the language specific knowledge list
	f=open_file_lang("knowledge.lst", "rb");
	if(f == NULL){
		LOG_ERROR("%s: %s \"knowledge.lst\": %s\n", reg_error_str, cant_open_file, strerror(errno));
		return;
	}
	while(1)
		{
			if(!fgets(strLine, sizeof(strLine), f)) {
				break;
			}
			out = knowledge_list[i].name;
			my_xmlStrncopy(&out, strLine, sizeof(knowledge_list[i].name)-1);
			i++;
		}
	// memorize the count
	knowledge_count= i;
	// close the file
	fclose(f);
}


void read_config()
{
	// Set our configdir
	const char * tcfg = get_path_config();

	my_strncp (configdir, tcfg , sizeof(configdir));

	if ( !read_el_ini () )
	{
		// oops, the file doesn't exist, give up
		const char *err_stg = "Failure reading el.ini";
		fprintf(stderr, "%s\n", err_stg);
		LOG_ERROR(err_stg);
		SDL_Quit ();
		exit (1);
	}

	/* if language is not set, default to "en" but use the language selection window */
	if (strlen(lang) == 0)
	{
		no_lang_in_config = 1;
		safe_strncpy(lang, "en", sizeof(lang));
		LOG_INFO("No language set so defaulting to [%s] and using language selection window", lang );
	}

#ifndef WINDOWS
	if (chdir(datadir) != 0)
	{
		LOG_ERROR("%s() chdir(\"%s\") failed: %s\n", __FUNCTION__, datadir, strerror(errno));
	}
#endif //!WINDOWS

	if(password_str[0])//We have a password
	{
		size_t k;

		for (k=0; k < strlen (password_str); k++)
			display_password_str[k] = '*';
		display_password_str[k] = 0;
	}
	else if (username_str[0]) //We have a username but not a password...
	{
		username_box_selected = 0;
		password_box_selected = 1;
	}
}

void read_bin_cfg()
{
	FILE *f = NULL;
	bin_cfg cfg_mem;
	int i;
	const char *fname = "el.cfg";
	size_t ret;

	f=open_file_config_no_local(fname,"rb");
	if(f == NULL)return;//no config file, use defaults
	memset(&cfg_mem, 0, sizeof(cfg_mem));	// make sure its clean

	ret = fread(&cfg_mem,1,sizeof(cfg_mem),f);
	fclose(f);
	if (ret != sizeof(cfg_mem))
	{
		LOG_ERROR("%s() failed to read %s\n", __FUNCTION__, fname);
		return;
	}

	//verify the version number
	if(cfg_mem.cfg_version_num != CFG_VERSION) return; //oops! ignore the file

	//good, retrive the data
	// TODO: move window save/restore into the window handler
	items_menu_x=cfg_mem.items_menu_x;
	items_menu_y=cfg_mem.items_menu_y;

	ground_items_menu_x=cfg_mem.ground_items_menu_x & 0xFFFF;
	ground_items_menu_y=cfg_mem.ground_items_menu_y & 0xFFFF;
	ground_items_menu_x_len=cfg_mem.ground_items_menu_x >> 16;
	ground_items_menu_y_len=cfg_mem.ground_items_menu_y >> 16;

	ranging_win_x=cfg_mem.ranging_win_x;
	ranging_win_y=cfg_mem.ranging_win_y;

	trade_menu_x=cfg_mem.trade_menu_x;
	trade_menu_y=cfg_mem.trade_menu_y;

	sigil_menu_x=cfg_mem.sigil_menu_x;
	sigil_menu_y=cfg_mem.sigil_menu_y;
	start_mini_spells=cfg_mem.start_mini_spells;
	emotes_menu_x=cfg_mem.emotes_menu_x;
	emotes_menu_y=cfg_mem.emotes_menu_y;

	dialogue_menu_x=cfg_mem.dialogue_menu_x;
	dialogue_menu_y=cfg_mem.dialogue_menu_y;

	manufacture_menu_x=cfg_mem.manufacture_menu_x;
	manufacture_menu_y=cfg_mem.manufacture_menu_y;

	astrology_win_x = cfg_mem.astrology_win_x;
 	astrology_win_y = cfg_mem.astrology_win_y;

	tab_stats_x=cfg_mem.tab_stats_x;
	tab_stats_y=cfg_mem.tab_stats_y;

	elconfig_menu_x=cfg_mem.elconfig_menu_x;
	elconfig_menu_y=cfg_mem.elconfig_menu_y;

	tab_help_x=cfg_mem.tab_help_x;
	tab_help_y=cfg_mem.tab_help_y;

	storage_win_x=cfg_mem.storage_win_x;
	storage_win_y=cfg_mem.storage_win_y;

	buddy_menu_x=cfg_mem.buddy_menu_x;
	buddy_menu_y=cfg_mem.buddy_menu_y;

	questlog_menu_x=cfg_mem.questlog_win_x;
	questlog_menu_y=cfg_mem.questlog_win_y;

	minimap_win_x=cfg_mem.minimap_win_x;
	minimap_win_y=cfg_mem.minimap_win_y;
	minimap_tiles_distance=cfg_mem.minimap_zoom;

	tab_selected=cfg_mem.tab_selected;

	tab_info_x=cfg_mem.tab_info_x;
	tab_info_y=cfg_mem.tab_info_y;

	if(quickbar_relocatable>0)
		{
			if((quickbar_x=cfg_mem.quickbar_x)>window_width||quickbar_x<=0)quickbar_x=34;
			if((quickbar_y=cfg_mem.quickbar_y)>window_height||quickbar_y<=0)quickbar_y=64;
			if((quickbar_dir=cfg_mem.quickbar_flags&0xFF)!=HORIZONTAL)quickbar_dir=VERTICAL;
			if((quickbar_draggable=(cfg_mem.quickbar_flags&0xFF00)>>8)!=1)quickbar_draggable=0;
		}

#if MAX_WATCH_STATS != 5
#error You cannot just go around changing MAX_WATCH_STATS as its used by the el.cfg file!
#endif
	for(i=0;i<MAX_WATCH_STATS;i++){
		watch_this_stats[i]=cfg_mem.watch_this_stats[i];
		if (watch_this_stats[i]<0 || watch_this_stats[i]>=NUM_WATCH_STAT)
			watch_this_stats[i]=0;
	}
	if(watch_this_stats[0]<1 || watch_this_stats[0]>=NUM_WATCH_STAT)
		watch_this_stats[0]=NUM_WATCH_STAT-1;

	has_accepted=cfg_mem.has_accepted_rules;

	rx=cfg_mem.camera_x;
	ry=cfg_mem.camera_y;
	rz=cfg_mem.camera_z;
	new_zoom_level=zoom_level=cfg_mem.zoom_level;

	// Needed for just one release, recover previous settings for player banners.
	if (cfg_mem.unused_01 != 0 || cfg_mem.unused_02 != 0)
	{
		cfg_mem.banner_settings &= 1;
		cfg_mem.banner_settings |= (cfg_mem.unused_01 << 2);
		cfg_mem.banner_settings |= (cfg_mem.unused_02 << 3);
		cfg_mem.unused_01 = cfg_mem.unused_02 = 0;
	}

	view_health_bar=cfg_mem.banner_settings & 1;
	view_ether_bar=(cfg_mem.banner_settings >> 1) & 1;
	view_names=(cfg_mem.banner_settings >> 2) & 1;
	view_hp=(cfg_mem.banner_settings >> 3) & 1;
	view_ether=(cfg_mem.banner_settings >> 4) & 1;

	quantities.selected=cfg_mem.quantity_selected;

	for(i=0;i<ITEM_EDIT_QUANT;i++){
		if(cfg_mem.quantity[i]){
			quantities.quantity[i].val=cfg_mem.quantity[i];
			safe_snprintf(quantities.quantity[i].str, sizeof(quantities.quantity[i].str),"%d", cfg_mem.quantity[i]);
			quantities.quantity[i].len=strlen(quantities.quantity[i].str);
		}
	}

	if(zoom_level != 0.0f) resize_root_window();
	
	have_saved_langsel = cfg_mem.have_saved_langsel;
	
	use_small_items_window = cfg_mem.misc_bool_options & 1;
	manual_size_items_window = (cfg_mem.misc_bool_options >> 1) & 1;
	allow_equip_swap = (cfg_mem.misc_bool_options >> 2) & 1;
	items_mix_but_all = (cfg_mem.misc_bool_options >> 3) & 1;
	items_stoall_nolastrow = (cfg_mem.misc_bool_options >> 4) & 1;
	items_dropall_nolastrow = (cfg_mem.misc_bool_options >> 5) & 1;
	autoclose_storage_dialogue = (cfg_mem.misc_bool_options >> 6) & 1;
	auto_select_storage_option = (cfg_mem.misc_bool_options >> 7) & 1;
	dialogue_copy_excludes_responses = (cfg_mem.misc_bool_options >> 8) & 1;
	items_stoall_nofirstrow = (cfg_mem.misc_bool_options >> 9) & 1;
	items_dropall_nofirstrow = (cfg_mem.misc_bool_options >> 10) & 1;
	items_auto_get_all = (cfg_mem.misc_bool_options >> 11) & 1;
	dialogue_copy_excludes_newlines = (cfg_mem.misc_bool_options >> 12) & 1;
	open_minimap_on_start = (cfg_mem.misc_bool_options >> 13) & 1;
	sort_storage_categories = (cfg_mem.misc_bool_options >> 14) & 1;
	disable_manuwin_keypress = (cfg_mem.misc_bool_options >> 15) & 1;
	always_show_astro_details = (cfg_mem.misc_bool_options >> 16) & 1;
	items_list_on_left = (cfg_mem.misc_bool_options >> 17) & 1;

	set_options_user_menus(cfg_mem.user_menu_win_x, cfg_mem.user_menu_win_y, cfg_mem.user_menu_options);

	floating_counter_flags = cfg_mem.floating_counter_flags;

	set_options_questlog(cfg_mem.questlog_flags);
}

void save_bin_cfg()
{
	FILE *f = NULL;
	bin_cfg cfg_mem;
	int i;

	f=open_file_config("el.cfg","wb");
	if(f == NULL){
		LOG_ERROR("%s: %s \"el.cfg\": %s\n", reg_error_str, cant_open_file, strerror(errno));
		return;//blah, whatever
	}
	memset(&cfg_mem, 0, sizeof(cfg_mem));	// make sure its clean

	cfg_mem.cfg_version_num=CFG_VERSION;	// set the version number
	//good, retrive the data
	/*
	// TODO: move window save/restore into the window handler
	cfg_mem.items_menu_x=items_menu_x;
	cfg_mem.items_menu_y=items_menu_y;

	cfg_mem.ground_items_menu_x=ground_items_menu_x;
	cfg_mem.ground_items_menu_y=ground_items_menu_y;

	cfg_mem.trade_menu_x=trade_menu_x;
	cfg_mem.trade_menu_y=trade_menu_y;

	cfg_mem.sigil_menu_x=sigil_menu_x;
	cfg_mem.sigil_menu_y=sigil_menu_y;

	cfg_mem.dialogue_menu_x=dialogue_menu_x;
	cfg_mem.dialogue_menu_y=dialogue_menu_y;

	cfg_mem.manufacture_menu_x=manufacture_menu_x;
	cfg_mem.manufacture_menu_y=manufacture_menu_y;

	cfg_mem.attrib_menu_x=attrib_menu_x;
	cfg_mem.attrib_menu_y=attrib_menu_y;

	cfg_mem.elconfig_menu_x=elconfig_menu_x;
	cfg_mem.elconfig_menu_y=elconfig_menu_y;

	cfg_mem.knowledge_menu_x=knowledge_menu_x;
	cfg_mem.knowledge_menu_y=knowledge_menu_y;

	cfg_mem.encyclopedia_menu_x=encyclopedia_menu_x;
	cfg_mem.encyclopedia_menu_y=encyclopedia_menu_y;

	cfg_mem.questlog_menu_x=questlog_menu_x;
	cfg_mem.questlog_menu_y=questlog_menu_y;
*/
	if(range_win >= 0) {
		cfg_mem.ranging_win_x=windows_list.window[range_win].cur_x;
		cfg_mem.ranging_win_y=windows_list.window[range_win].cur_y;
	} else {
		cfg_mem.ranging_win_x=ranging_win_x;
		cfg_mem.ranging_win_y=ranging_win_y;
	}

	if(tab_help_win >= 0) {
		cfg_mem.tab_help_x=windows_list.window[tab_help_win].cur_x;
		cfg_mem.tab_help_y=windows_list.window[tab_help_win].cur_y;
	} else {
		cfg_mem.tab_help_x=tab_help_x;
		cfg_mem.tab_help_y=tab_help_y;
	}

	if(items_win >= 0) {
		cfg_mem.items_menu_x=windows_list.window[items_win].cur_x;
		cfg_mem.items_menu_y=windows_list.window[items_win].cur_y;
	} else {
		cfg_mem.items_menu_x=items_menu_x;
		cfg_mem.items_menu_y=items_menu_y;
	}

	if(ground_items_win >= 0) {
		cfg_mem.ground_items_menu_x=windows_list.window[ground_items_win].cur_x;
		cfg_mem.ground_items_menu_y=windows_list.window[ground_items_win].cur_y;
		cfg_mem.ground_items_menu_x |= windows_list.window[ground_items_win].len_x << 16;
		cfg_mem.ground_items_menu_y |= windows_list.window[ground_items_win].len_y << 16;
	} else {
		cfg_mem.ground_items_menu_x=ground_items_menu_x;
		cfg_mem.ground_items_menu_y=ground_items_menu_y;
		cfg_mem.ground_items_menu_x |= ground_items_menu_x_len << 16;
		cfg_mem.ground_items_menu_y |= ground_items_menu_y_len << 16;
	}

	if(trade_win >= 0) {
		cfg_mem.trade_menu_x=windows_list.window[trade_win].cur_x;
		cfg_mem.trade_menu_y=windows_list.window[trade_win].cur_y;
	} else {
		cfg_mem.trade_menu_x=trade_menu_x;
		cfg_mem.trade_menu_y=trade_menu_y;
	}

	cfg_mem.start_mini_spells=start_mini_spells;
	if(sigil_win >= 0) {
		cfg_mem.sigil_menu_x=windows_list.window[sigil_win].cur_x;
		cfg_mem.sigil_menu_y=windows_list.window[sigil_win].cur_y;
	} else {
		cfg_mem.sigil_menu_x=sigil_menu_x;
		cfg_mem.sigil_menu_y=sigil_menu_y;
	}
	if(emotes_win >= 0) {
		cfg_mem.emotes_menu_x=windows_list.window[emotes_win].cur_x;
		cfg_mem.emotes_menu_y=windows_list.window[emotes_win].cur_y;
	} else {
		cfg_mem.emotes_menu_x=emotes_menu_x;
		cfg_mem.emotes_menu_y=emotes_menu_y;
	}
	if(dialogue_win >= 0) {
		cfg_mem.dialogue_menu_x=windows_list.window[dialogue_win].cur_x;
		cfg_mem.dialogue_menu_y=windows_list.window[dialogue_win].cur_y;
	} else {
		cfg_mem.dialogue_menu_x=dialogue_menu_x;
		cfg_mem.dialogue_menu_y=dialogue_menu_y;
	}

	if(manufacture_win >= 0) {
		cfg_mem.manufacture_menu_x=windows_list.window[manufacture_win].cur_x;
		cfg_mem.manufacture_menu_y=windows_list.window[manufacture_win].cur_y;
	} else {
		cfg_mem.manufacture_menu_x=manufacture_menu_x;
		cfg_mem.manufacture_menu_y=manufacture_menu_y;
	}

	if(astrology_win >= 0) {
 		cfg_mem.astrology_win_x=windows_list.window[astrology_win].cur_x;
 		cfg_mem.astrology_win_y=windows_list.window[astrology_win].cur_y;
 	} else {
 		cfg_mem.astrology_win_x=astrology_win_x;
 		cfg_mem.astrology_win_y=astrology_win_y;
 	}

	if(elconfig_win >= 0) {
		cfg_mem.elconfig_menu_x=windows_list.window[elconfig_win].cur_x;
		cfg_mem.elconfig_menu_y=windows_list.window[elconfig_win].cur_y;
	} else {
		cfg_mem.elconfig_menu_x=elconfig_menu_x;
		cfg_mem.elconfig_menu_y=elconfig_menu_y;
	}

	if(storage_win >= 0) {
		cfg_mem.storage_win_x=windows_list.window[storage_win].cur_x;
		cfg_mem.storage_win_y=windows_list.window[storage_win].cur_y;
	} else {
		cfg_mem.storage_win_x=storage_win_x;
		cfg_mem.storage_win_y=storage_win_y;
	}

	if(tab_stats_win >= 0) {
		cfg_mem.tab_stats_x=windows_list.window[tab_stats_win].cur_x;
		cfg_mem.tab_stats_y=windows_list.window[tab_stats_win].cur_y;
	} else {
		cfg_mem.tab_stats_x=tab_stats_x;
		cfg_mem.tab_stats_y=tab_stats_y;
	}

	if(buddy_win >= 0) {
		cfg_mem.buddy_menu_x=windows_list.window[buddy_win].cur_x;
		cfg_mem.buddy_menu_y=windows_list.window[buddy_win].cur_y;
	} else {
		cfg_mem.buddy_menu_x=buddy_menu_x;
		cfg_mem.buddy_menu_y=buddy_menu_y;
	}

	if(questlog_win >= 0) {
		cfg_mem.questlog_win_x=windows_list.window[questlog_win].cur_x;
		cfg_mem.questlog_win_y=windows_list.window[questlog_win].cur_y;
	} else {
		cfg_mem.questlog_win_x=questlog_menu_x;
		cfg_mem.questlog_win_y=questlog_menu_y;
	}

	if(minimap_win >= 0) {
		cfg_mem.minimap_win_x=windows_list.window[minimap_win].cur_x;
		cfg_mem.minimap_win_y=windows_list.window[minimap_win].cur_y;
	} else {
		cfg_mem.minimap_win_x=minimap_win_x;
		cfg_mem.minimap_win_y=minimap_win_y;
	}
	cfg_mem.minimap_zoom=minimap_tiles_distance;

	cfg_mem.tab_selected=get_tab_selected();

	if(tab_info_win >= 0) {
		cfg_mem.tab_info_x=windows_list.window[tab_info_win].cur_x;
		cfg_mem.tab_info_y=windows_list.window[tab_info_win].cur_y;
	} else {
		cfg_mem.tab_info_x=tab_info_x;
		cfg_mem.tab_info_y=tab_info_y;
	}

	cfg_mem.banner_settings = 0;
	cfg_mem.banner_settings |= view_health_bar;
	cfg_mem.banner_settings |= view_ether_bar << 1;
	cfg_mem.banner_settings |= view_names << 2;
	cfg_mem.banner_settings |= view_hp << 3;
	cfg_mem.banner_settings |= view_ether << 4;

	cfg_mem.quantity_selected=(quantities.selected<ITEM_EDIT_QUANT)?quantities.selected :0;

	if(quickbar_relocatable>0)
		{
			if(quickbar_win >= 0){
				cfg_mem.quickbar_x=window_width-windows_list.window[quickbar_win].cur_x;
				cfg_mem.quickbar_y=windows_list.window[quickbar_win].cur_y;
				cfg_mem.quickbar_flags=quickbar_dir|(quickbar_draggable<<8);
			} else {
				cfg_mem.quickbar_x=quickbar_x;
				cfg_mem.quickbar_y=quickbar_y;
				cfg_mem.quickbar_flags=VERTICAL;
			}
		}

	for(i=0;i<MAX_WATCH_STATS;i++){
		cfg_mem.watch_this_stats[i]=watch_this_stats[i];
	}

	cfg_mem.has_accepted_rules=has_accepted;

	cfg_mem.camera_x=rx;
	cfg_mem.camera_y=ry;
	cfg_mem.camera_z=rz;
	cfg_mem.zoom_level=zoom_level;

	for(i=0;i<ITEM_EDIT_QUANT;i++){
		cfg_mem.quantity[i]=quantities.quantity[i].val;
	}
	
	cfg_mem.have_saved_langsel = have_saved_langsel;
	
	cfg_mem.misc_bool_options = 0;
	cfg_mem.misc_bool_options |= use_small_items_window;
	cfg_mem.misc_bool_options |= manual_size_items_window << 1;
	cfg_mem.misc_bool_options |= allow_equip_swap << 2;
	cfg_mem.misc_bool_options |= items_mix_but_all << 3;
	cfg_mem.misc_bool_options |= items_stoall_nolastrow << 4;
	cfg_mem.misc_bool_options |= items_dropall_nolastrow << 5;
 	cfg_mem.misc_bool_options |= autoclose_storage_dialogue << 6;
 	cfg_mem.misc_bool_options |= auto_select_storage_option << 7;
	cfg_mem.misc_bool_options |= dialogue_copy_excludes_responses << 8;
	cfg_mem.misc_bool_options |= items_stoall_nofirstrow << 9;
	cfg_mem.misc_bool_options |= items_dropall_nofirstrow << 10;
	cfg_mem.misc_bool_options |= items_auto_get_all << 11;
	cfg_mem.misc_bool_options |= dialogue_copy_excludes_newlines << 12;
	cfg_mem.misc_bool_options |= open_minimap_on_start << 13;
	cfg_mem.misc_bool_options |= sort_storage_categories << 14;
	cfg_mem.misc_bool_options |= disable_manuwin_keypress << 15;
	cfg_mem.misc_bool_options |= always_show_astro_details << 16;
	cfg_mem.misc_bool_options |= items_list_on_left << 17;

	get_options_user_menus(&cfg_mem.user_menu_win_x, &cfg_mem.user_menu_win_y, &cfg_mem.user_menu_options);

	cfg_mem.floating_counter_flags = floating_counter_flags;

	cfg_mem.questlog_flags = get_options_questlog();

	fwrite(&cfg_mem,sizeof(cfg_mem),1,f);
	fclose(f);

}

#ifndef	NEW_TEXTURES
void init_texture_cache()
{
	memset(texture_cache, 0, sizeof(texture_cache));
}
#endif	/* NEW_TEXTURES */

void init_e3d_cache()
{
	//cache_e3d= cache_init(1000, &destroy_e3d);	//TODO: autofree the name as well
	cache_e3d = cache_init("E3d cache", 1500, NULL);	//no aut- free permitted
	cache_set_compact(cache_e3d, &free_e3d_va);	// to compact, free VA arrays
	cache_set_time_limit(cache_e3d, 5*60*1000);
	cache_set_size_limit(cache_e3d, 8*1024*1024);
}

#ifndef FASTER_MAP_LOAD
void init_2d_obj_cache()
{
	memset(obj_2d_def_cache, 0, sizeof(obj_2d_def_cache));
}
#endif

void init_stuff()
{
	int seed;
	char file_name[250];
	int i;
	char config_location[300];
	const char * cfgdir;

	if (chdir(datadir) != 0)
	{
		LOG_ERROR("%s() chdir(\"%s\") failed: %s\n", __FUNCTION__, datadir, strerror(errno));
	}

	init_crc_tables();
	init_zip_archives();

	// initialize the text buffers - needed early for logging
	init_text_buffers ();

	load_server_list("servers.lst");
	set_server_details();

	// Read the config file
	read_config();

	// Parse command line options
	read_command_line();

	// all options loaded
	options_loaded();

	// Check if our datadir is valid and if not failover to ./
	file_check_datadir();

	// Here you can add zip files, like
	// add_zip_archive(datadir + "data.zip");
	xml_register_el_input_callbacks();

#ifdef WRITE_XML
	load_translatables();//Write to the current working directory - hopefully we'll have write rights here...
#endif
	// XXX FIXME (Grum): actually this should only be done when windowed
	// chat is not used (which we don't know yet at this point), but let's
	// leave it here until we're certain that the chat channel buffers are
	// never used otherwise, then move it down till after the configuration
	// is read.
	init_chat_channels ();

	// load the named colours for the elgl-Colour-() functions
	init_named_colours();

	// initialize the fonts, but don't load the textures yet. Do that here
	// because the messages need the font widths.
	init_fonts();

	//OK, we have the video mode settings...
	setup_video_mode(full_screen,video_mode);
	//now you may set the video mode using the %<foo> in-game
	video_mode_set=1;

	//Good, we should be in the right working directory - load all translatables from their files
	load_translatables();

	//if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE | SDL_INIT_EVENTTHREAD) == -1)	// experimental
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) == -1)
		{
			LOG_ERROR("%s: %s\n", no_sdl_str, SDL_GetError());
			fprintf(stderr, "%s: %s\n", no_sdl_str, SDL_GetError());
			SDL_Quit();
			exit(1);
		}
	init_video();

#ifdef MAP_EDITOR2
	SDL_WM_SetCaption( "Map Editor", "mapeditor" );
#else
	SDL_WM_SetCaption( win_principal, "eternallands" );
#endif

#ifdef OSX
	// don't emulate a 3 button mouse except you still have a 1 button mouse, ALT+leftclick doesn't work with the emulation
	if (!emulate3buttonmouse) SDL_putenv("SDL_HAS3BUTTONMOUSE=1");
#endif

	//Init the caches here, as the loading window needs them
	cache_system_init(MAX_CACHE_SYSTEM);
	init_texture_cache();
	init_e3d_cache();
#ifndef FASTER_MAP_LOAD
	init_2d_obj_cache();
#endif
	//now load the font textures
	if (load_font_textures () != 1)
	{
		LOG_ERROR("%s\n", fatal_data_error);
		fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, fatal_data_error);
		SDL_Quit();
		exit(1);
	}
	CHECK_GL_ERRORS();

	// read the continent map info
	read_mapinfo();

	// now create the root window

	// XXX FIXME (Grum): Maybe we should do this at a later time, after
	// we're logged in?
	create_game_root_window (window_width, window_height);
	create_console_root_window (window_width, window_height);
	create_map_root_window (window_width, window_height);
	create_login_root_window (window_width, window_height);

	//create the loading window
	create_loading_win (window_width, window_height, 0);
	show_window(loading_win);

	update_loading_win(init_opengl_str, 5);
	LOG_DEBUG("Init extensions.");
	init_gl_extensions();
	LOG_DEBUG("Init extensions done");

	// Setup the new eye candy system
	LOG_DEBUG("Init eyecandy");
	ec_init();
	LOG_DEBUG("Init eyecandy done");

#ifdef  CUSTOM_UPDATE
	init_custom_update();
#endif  //CUSTOM_UPDATE

	// check for invalid combinations
	check_options();

	update_loading_win(init_random_str, 4);
	seed= time (NULL);
	srand(seed);

	update_loading_win(load_ignores_str, 1);
	load_ignores();
	update_loading_win(load_filters_str, 2);
	load_filters();
	update_loading_win(load_lists_str, 2);
	load_harvestable_list();
	load_entrable_list();
	load_knowledge_list();
	load_mines_config();
	update_loading_win(load_cursors_str, 5);
	load_cursors();
	build_cursors();
	change_cursor(CURSOR_ARROW);
	update_loading_win(bld_glow_str, 3);
	build_glow_color_table();


	update_loading_win(init_lists_str, 2);
	init_actors_lists();
	update_loading_win("init particles", 4);
	memset(tile_list, 0, sizeof(tile_list));
	memset(lights_list, 0, sizeof(lights_list));
	main_bbox_tree = build_bbox_tree();
	init_particles ();
#ifdef NEW_SOUND
	update_loading_win(init_audio_str, 1);
	initial_sound_init();
	load_sound_config_data(SOUND_CONFIG_PATH);
#endif // NEW_SOUND
	update_loading_win(init_actor_defs_str, 4);
	memset(actors_defs, 0, sizeof(actors_defs));

	LOG_DEBUG("Init actor defs");
	init_actor_defs();
	LOG_DEBUG("Init actor defs done");
	read_emotes_defs("", "emotes.xml");

	missiles_init_defs();

	update_loading_win(load_map_tiles_str, 4);
	load_map_tiles();

	update_loading_win(init_lights_str, 4);
	//lights setup
	build_global_light_table();
	build_sun_pos_table();
	reset_material();

	LOG_DEBUG("Init lights");
	init_lights();
	LOG_DEBUG("Init done");

	disable_local_lights();
	update_loading_win(init_logs_str, 4);
	clear_conn_log();
	update_loading_win(read_config_str, 2);
	read_bin_cfg();
 	update_loading_win(init_weather_str, 3);
	weather_init();
	build_levels_table();//for some HUD stuff

	update_loading_win(load_icons_str, 4);
	//load the necesary textures
#ifdef	NEW_TEXTURES
	icons_text = load_texture_cached("textures/gamebuttons.dds", tt_gui);
	hud_text = load_texture_cached("textures/gamebuttons2.dds", tt_gui);
#else	/* NEW_TEXTURES */
#ifdef	NEW_ALPHA
	icons_text= load_texture_cache("./textures/gamebuttons.bmp", -1);
	hud_text= load_texture_cache("./textures/gamebuttons2.bmp", -1);
#else	//NEW_ALPHA
	icons_text= load_texture_cache("./textures/gamebuttons.bmp",0);
	hud_text= load_texture_cache("./textures/gamebuttons2.bmp",0);
#endif	//NEW_ALPHA
#endif	/* NEW_TEXTURES */
	update_loading_win(load_textures_str, 4);
#ifdef	NEW_TEXTURES
	cons_text = load_texture_cached("textures/console.dds", tt_gui);
#else	/* NEW_TEXTURES */
	cons_text= load_texture_cache("./textures/console.bmp",255);
#endif	/* NEW_TEXTURES */


	update_loading_win("init item textures", 5);

	for(i=0; i<MAX_ITEMS_TEXTURES; i++){
		char	buffer[256];

#ifdef	NEW_TEXTURES
		safe_snprintf(buffer, sizeof(buffer), "textures/items%d.dds", i+1);

		if (check_image_name(buffer, sizeof(buffer), buffer) != 0)
		{
			items_text[i] = load_texture_cached(buffer, tt_gui);
#else	/* NEW_TEXTURES */
		safe_snprintf(buffer, sizeof(buffer), "./textures/items%d.bmp", i+1);
		if(el_custom_file_exists(buffer)){
			items_text[i]= load_texture_cache(buffer, 0);
#endif	/* NEW_TEXTURES */
		}
	}
	update_loading_win("init portraits", 5);

	for(i=0; i<MAX_PORTRAITS_TEXTURES; i++){
		char	buffer[256];

#ifdef	NEW_TEXTURES
		safe_snprintf(buffer, sizeof(buffer), "textures/portraits%d.dds", i+1);

		if (check_image_name(buffer, sizeof(buffer), buffer) != 0)
		{
			portraits_tex[i] = load_texture_cached(buffer, tt_gui);
#else	/* NEW_TEXTURES */
		safe_snprintf(buffer, sizeof(buffer), "./textures/portraits%d.bmp", i+1);
		if(el_custom_file_exists(buffer)){
			portraits_tex[i]= load_texture_cache_deferred(buffer, 0);
#endif	/* NEW_TEXTURES */
		}
	}
	update_loading_win("init textures", 5);

#ifdef NEW_CURSOR
#ifdef	NEW_TEXTURES
	cursors_tex = load_texture_cached("textures/cursors2.dds", tt_gui);
#else	/* NEW_TEXTURES */
	disable_compression();
	cursors_tex = load_texture_cache("./textures/cursors2.bmp",0);
	enable_compression();
#endif	/* NEW_TEXTURES */

	//Emajekral's hi-color & big cursor code
	if (!sdl_cursors) SDL_ShowCursor(0);
#endif // NEW_CURSOR

	//Load the map legend and continent map
#ifdef	NEW_TEXTURES
	legend_text = load_texture_cached("maps/legend.dds", tt_gui);
#else	/* NEW_TEXTURES */
	legend_text= load_texture_cache("./maps/legend.bmp",0);
#endif	/* NEW_TEXTURES */

#ifdef	NEW_TEXTURES
	ground_detail_text = load_texture_cached("textures/ground_detail.dds", tt_gui);
#else	/* NEW_TEXTURES */
	ground_detail_text=load_texture_cache("./textures/ground_detail.bmp",255);
#endif	/* NEW_TEXTURES */
	CHECK_GL_ERRORS();
	init_login_screen ();
	init_spells ();

#ifdef PAWN
	update_loading_win (init_pawn_str, 0);
	initialize_pawn ();
#endif

	update_loading_win(init_network_str, 5);
	if(SDLNet_Init()<0){
		LOG_ERROR("%s: %s\n", failed_sdl_net_init, SDLNet_GetError());
		fprintf(stderr, "%s: %s\n", failed_sdl_net_init, SDLNet_GetError());
		SDLNet_Quit();
		SDL_Quit();
		exit(2);
	}
	update_loading_win(init_timers_str, 5);

	if(SDL_InitSubSystem(SDL_INIT_TIMER)<0){
		LOG_ERROR("%s: %s\n", failed_sdl_timer_init, SDL_GetError());
		fprintf(stderr, "%s: %s\n", failed_sdl_timer_init, SDL_GetError());
		SDL_Quit();
	 	exit(1);
	}
	update_loading_win(load_encyc_str, 5);
	safe_snprintf(file_name, sizeof(file_name), "languages/%s/Encyclopedia/index.xml", lang);
	if (!el_file_exists(file_name))
		safe_snprintf(file_name, sizeof(file_name), "languages/%s/Encyclopedia/index.xml", "en");
	ReadXML(file_name);
	read_key_config();
	init_buddy();
	init_channel_names();
#ifdef	OLC
	olc_finish_init();
#endif	//OLC

	if(auto_update){
		init_update();
	}

#ifdef  CUSTOM_UPDATE
	if (custom_update != 0)
	{
		start_custom_update();
	}
#endif  //CUSTOM_UPDATE

	have_rules=read_rules();
	if(!have_rules){
		LOG_ERROR(rules_not_found);
		fprintf(stderr, "%s\n", rules_not_found);
		SDL_Quit();
		exit(3);
	}

	//initiate function pointers
	init_attribf();

	init_statsinfo_array();

	//Read the books for i.e. the new char window
	init_books();

	update_loading_win(init_display_str, 5);
	if (!disable_gamma_adjust)
		SDL_SetGamma(gamma_var, gamma_var, gamma_var);

	draw_scene_timer= SDL_AddTimer (1000/(18*4), my_timer, NULL);
	misc_timer= SDL_AddTimer (500, check_misc, NULL);

	safe_snprintf(config_location, sizeof(config_location), datadir_location_str, datadir);
	LOG_TO_CONSOLE(c_green4, config_location);
	cfgdir = get_path_config();
	if (cfgdir != NULL) {
		//Realistically, if this failed, then there's not much point in continuing, but oh well...
		safe_snprintf(config_location, sizeof(config_location), config_location_str, cfgdir);
		LOG_TO_CONSOLE(c_green4, config_location);
	}

	update_loading_win(prep_op_win_str, 7);
	create_opening_root_window (window_width, window_height);
	// initialize the chat window
	if (use_windowed_chat == 2) {
		display_chat ();
	}

	init_commands("commands.lst");

#ifdef TEXT_ALIASES
	init_text_aliases();
#endif

#ifdef NEW_SOUND
	// Try to turn the sound on now so we have it for the login window
	if (have_sound_config)
		turn_sound_on();
	else
	{
		sound_on = 0;
		turn_sound_off();
	}
#endif // NEW_SOUND

	// display something
	destroy_loading_win();
	if (!have_saved_langsel || no_lang_in_config)
	{
		display_langsel_win();
	}
	else if (has_accepted)
	{
		show_window (opening_root_win);
		connect_to_server();
	}
	else
	{
		create_rules_root_window (window_width, window_height, opening_root_win, 15);
		show_window (rules_root_win);
	}

	if (use_frame_buffer) make_reflection_framebuffer(window_width, window_height);

	skybox_init_gl();
	popup_init();

	LOG_DEBUG("Init done!");
}
