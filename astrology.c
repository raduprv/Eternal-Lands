#include "astrology.h"
#include "context_menu.h"
#include "elwindows.h"
#include "errors.h"
#include "gamewin.h"
#include "font.h"
#include "textures.h"
#include "stats.h"
#include "asc.h"
#include "translate.h"

typedef enum
{
	atAttDefIndicator = 0,
	atCriticalsIndicator,
	atAccMagicIndicator,
	atHarvDegrIndicator,
	atRareFailIndicator,
	atAttPredictor,
	atDefPredictor,
	atHitPredictor,
	atDamagePredictor,
	atAccPredictor,
	atMagicPredictor,
	atHarvPredictor,
	atDegradePredictor,
	atRarePredictor,
	atFailPredictor
} ASTROLOGY_TYPES;

typedef enum
{
	adtTwoProgressBars = 0,
	adtThreeProgressBars
}ASTROLOGY_DISPLAY_TYPES;

const float positive_bar_colors[12] =
{
	//red    green   blue
	0.086f, 0.988f, 0.659f, // topleft
	0.086f, 0.988f, 0.659f, // topright
	0.294f, 0.690f, 0.173f, // bottomright
	0.294f, 0.690f, 0.173f  // bottomleft
};

const float negative_bar_colors[12] =
{
	//red    green   blue
	0.988f, 0.659f, 0.086f, // topleft
	0.988f, 0.659f, 0.086f, // topright
	0.690f, 0.173f, 0.294f, // bottomright
	0.690f, 0.173f, 0.294f  // bottomleft
};

int always_show_astro_details = 0;

static int astrology_win_x_len = 0;
static int astrology_win_y_len = 0;
static int bar_left_x = 0;
static int bar_top_1 = 0;
static int bar_top_2 = 0;
static int bar_top_3 = 0;
static int astro_border = 0;
static int progress_bar_width = 0;
static int progress_bar_height = 0;
static int bar_space_y = 0;
static int ok_button_id = 103;

static char stone_name[50];
static int value1,value2,value3;
static char text_item1[50],text_item2[50],text_item3[50];
static ASTROLOGY_DISPLAY_TYPES astrology_display_type;
static ASTROLOGY_TYPES astrology_type;
static int capping_already_reported = 0;
static char *last_astro_message = NULL;
static size_t last_astro_message_len = 0;

// forward declaration
static int display_astrology_handler (window_info *win);

int is_astrology_message (const char * RawText)
{
	char * tmp1,*tmp2;
	char str[50];

//check for predictors
	tmp1 = strstr (RawText,"20 Minutes: ");
	if(tmp1)
	{
		astrology_display_type = adtThreeProgressBars;
		memset(stone_name,0,50);
		RawText += 1;
//parse predictors
		tmp2 = strchr (RawText, ',');
		safe_strncpy2 (str, RawText, sizeof (str), tmp2 - RawText);
		safe_snprintf (stone_name, sizeof(stone_name), "%s predictor", str);

		if(strstr(stone_name,"Attack bonus"))
			astrology_type = atAttPredictor;
		else if(strstr(stone_name,"Defense bonus"))
			astrology_type = atDefPredictor;
		else if(strstr(stone_name,"To hit bonus"))
			astrology_type = atHitPredictor;
		else if(strstr(stone_name,"To damage bonus"))
			astrology_type = atDamagePredictor;
		else if(strstr(stone_name,"Accuracy bonus"))
			astrology_type = atAccPredictor;
		else if(strstr(stone_name,"Magic bonus"))
			astrology_type = atMagicPredictor;
		else if(strstr(stone_name,"Harvest events increase"))
			astrology_type = atHarvPredictor;
		else if(strstr(stone_name,"Items Degrade"))
			astrology_type = atDegradePredictor;
		else if(strstr(stone_name,"Make rare"))
			astrology_type = atRarePredictor;
		else if(strstr(stone_name,"Failure"))
			astrology_type = atFailPredictor;

		tmp1 += strlen("20 Minutes: ");

		value1 = atoi(tmp1);
		safe_snprintf(text_item1,sizeof(text_item1),"20 Minutes: %d",value1);

		tmp1 = strstr(RawText,"40 Minutes: ") + strlen("40 Minutes: ");
		value2 = atoi(tmp1);
		safe_snprintf(text_item2,sizeof(text_item2),"40 Minutes: %d",value2);
		tmp1 = strstr(RawText,"60 Minutes: ") + strlen("60 Minutes: ");
		value3 = atoi(tmp1);
		safe_snprintf(text_item3,sizeof(text_item3),"60 Minutes: %d",value3);

		display_astrology_window(RawText);
		return 1;
	}

//check for att/def indicator
	tmp1 = strstr(RawText,"Att: ");
	if(tmp1)
	{
		//parse att/def indicator
		astrology_display_type = adtTwoProgressBars;
		astrology_type = atAttDefIndicator;

		safe_snprintf(stone_name,sizeof(stone_name),"Att/Def indicator");

		tmp1 += strlen("Att: ");
		value1 = atoi(tmp1);
		safe_snprintf(text_item1,sizeof(text_item1),"Attack: %d",value1);

		tmp1 = strstr(tmp1,"Def: ");
		if(tmp1)
		{
			tmp1 += strlen("Def: ");
			value2 = atoi(tmp1);
			safe_snprintf(text_item2,sizeof(text_item2),"Defense: %d",value2);
		}

		display_astrology_window(RawText);
		return 1;
	}

//check for criticals indicator
	tmp1 = strstr(RawText,"To hit: ");
	if(tmp1)
	{
		//parse criticals indicator
		astrology_display_type = adtTwoProgressBars;
		astrology_type = atCriticalsIndicator;
		safe_snprintf(stone_name,sizeof(stone_name),"Criticals indicator");
		tmp1 += strlen("To hit: ");
		value1 = atoi(tmp1);
		safe_snprintf(text_item1,sizeof(text_item1),"To hit: %d",value1);
		tmp1 = strstr(tmp1,"To damage: ");
		if(tmp1)
		{
			tmp1 += strlen("To damage: ");
			value2 = atoi(tmp1);
			safe_snprintf(text_item2,sizeof(text_item2),"To damage: %d",value2);
		}

		display_astrology_window(RawText);
		return 1;
	}

//check for acc/magic indicator
	tmp1 = strstr(RawText,"Acc: ");
	if(tmp1)
	{
		//parse acc/magic indicator
		astrology_display_type = adtTwoProgressBars;
		astrology_type = atAccMagicIndicator;
		safe_snprintf(stone_name,sizeof(stone_name),"Acc/magic indicator");
		tmp1 += strlen("Acc: ");
		value1 = atoi(tmp1);
		safe_snprintf(text_item1,sizeof(text_item1),"Accuracy: %d",value1);

		tmp1 = strstr(tmp1,"Magic: ");
		if(tmp1)
		{
			tmp1 += strlen("Magic: ");
			value2 = atoi(tmp1);
			safe_snprintf(text_item2,sizeof(text_item2),"Magic: %d",value2);
		}

		display_astrology_window(RawText);
		return 1;
	}

//check for harv/degrade indicator
	tmp1 = strstr(RawText,"Harvest Events Increase: ");
	if(tmp1)
	{
		//parse harv/degrade indicator
		astrology_display_type = adtTwoProgressBars;
		astrology_type = atHarvDegrIndicator;
		safe_snprintf(stone_name,sizeof(stone_name),"Harvest events inc./degrade indicator");
		tmp1 += strlen("Harvest Events Increase: ");
		value1 = atoi(tmp1);
		safe_snprintf(text_item1,sizeof(text_item1),"Harvest events increase: %d",value1);

		tmp1 = strstr(tmp1,"Degrade: ");
		if(tmp1)
		{
			tmp1 += strlen("Degrade: ");
			value2 = atoi(tmp1);
			safe_snprintf(text_item2,sizeof(text_item2),"Degrade: %d",value2);
		}

		display_astrology_window(RawText);
		return 1;
	}

//check for rare/failure indicator
	tmp1 = strstr(RawText,"Make rare: ");
	if(tmp1)
	{
		//parse rare/failure indicator
		astrology_display_type = adtTwoProgressBars;
		astrology_type = atRareFailIndicator;
		safe_snprintf(stone_name,sizeof(stone_name),"Make rare/failure indicator");
		tmp1 += strlen("Make rare: ");
		value1 = atoi(tmp1);
		safe_snprintf(text_item1,sizeof(text_item1),"Make rare: %d",value1);

		tmp1 = strstr(tmp1,"Failure: ");
		if(tmp1)
		{
			tmp1 += strlen("Failure: ");
			value2 = atoi(tmp1);
			safe_snprintf(text_item2,sizeof(text_item2),"Failure: %d",value2);
		}

		display_astrology_window(RawText);
		return 1;
	}

	return 0;
}

static int ok_handler()
{
	hide_window_MW(MW_ASTRO);
	return 1;
}

//adjusts the astrology window size/widgets position, depending on what it displays (predictor or indicator)
static void adjust_astrology_window()
{
	int astrology_win = get_id_MW(MW_ASTRO);
	int button_x = (astrology_win_x_len - widget_get_width(astrology_win, ok_button_id)) / 2;
	int button_y = astrology_win_y_len - widget_get_height(astrology_win, ok_button_id) - 2 * astro_border;

	switch(astrology_display_type)
	{
		case adtTwoProgressBars:
		{
			resize_window(astrology_win, astrology_win_x_len,astrology_win_y_len - bar_space_y);
			widget_move(astrology_win,ok_button_id, button_x, button_y - bar_space_y);
		}break;
		case adtThreeProgressBars:
		{
			resize_window(astrology_win, astrology_win_x_len, astrology_win_y_len);
			widget_move(astrology_win, ok_button_id, button_x, button_y);
		}break;
	}
}

static void show_astro_details(void)
{
	if (last_astro_message_len && last_astro_message!=NULL)
	{
		LOG_TO_CONSOLE(c_green2, stone_name);
		LOG_TO_CONSOLE(c_grey1, last_astro_message);
	}
}

static int cm_astro_handler(window_info *win, int widget_id, int mx, int my, int option)
{
	if (option<ELW_CM_MENU_LEN)
		return cm_title_handler(win, widget_id, mx, my, option);
	if (option == ELW_CM_MENU_LEN+1)
		show_astro_details();
	return 1;
}

static int ui_scale_astrology_handler(window_info *win)
{
	astro_border = (int)(0.5 + win->current_scale * 5);
	bar_left_x = (int)(0.5 + win->current_scale * 30);
	progress_bar_width = (int)(0.5 + win->current_scale *  135);
	progress_bar_height = (int)(0.5 + win->current_scale *  10);
	bar_space_y = 2 * win->small_font_len_y + progress_bar_height;
	bar_top_1 = 2 * astro_border + 2 * win->small_font_len_y;
	bar_top_2 = bar_top_1 + bar_space_y;
	bar_top_3 = bar_top_2 + bar_space_y;

	button_resize(win->window_id, ok_button_id, 0, 0, win->current_scale);

	astrology_win_x_len = bar_left_x * 2 + progress_bar_width * 2;
	astrology_win_y_len = bar_top_3 + progress_bar_height + 4 * astro_border + widget_get_height(win->window_id, ok_button_id);

	adjust_astrology_window();

	return 1;
}

void display_astrology_window(const char * raw_text)
{
	int astrology_win = get_id_MW(MW_ASTRO);
	if(astrology_win < 0)
	{
		astrology_win = create_window(win_astrology, (not_on_top_now(MW_ASTRO) ?game_root_win : -1), 0, get_pos_x_MW(MW_ASTRO), get_pos_y_MW(MW_ASTRO), 0, 0,
			(ELW_USE_UISCALE|ELW_WIN_DEFAULT) ^ ELW_CLOSE_BOX);
		set_id_MW(MW_ASTRO, astrology_win);

		set_window_handler(astrology_win, ELW_HANDLER_DISPLAY, &display_astrology_handler );
		set_window_handler(astrology_win, ELW_HANDLER_UI_SCALE, &ui_scale_astrology_handler );

		ok_button_id=button_add_extended(astrology_win, ok_button_id, NULL, 0, 0, 0, 0, 0, 1.0f, "Ok");
		widget_set_OnClick(astrology_win, ok_button_id, ok_handler);

		if (astrology_win >= 0 && astrology_win < windows_list.num_windows)
			ui_scale_astrology_handler(&windows_list.window[astrology_win]);
		check_proportional_move(MW_ASTRO);

		cm_add(windows_list.window[astrology_win].cm_id, cm_astro_menu_str, cm_astro_handler);
		cm_bool_line(windows_list.window[astrology_win].cm_id, ELW_CM_MENU_LEN+2, &always_show_astro_details, NULL );
	}
	else
	{
		show_window(astrology_win);
		select_window(astrology_win);
	}
	adjust_astrology_window();
	capping_already_reported = 0;
	if (last_astro_message_len <= strlen(raw_text))
	{
		last_astro_message_len = strlen(raw_text) + 1;
		if ((last_astro_message = (char *)realloc(last_astro_message, last_astro_message_len)) == NULL)
			last_astro_message_len = 0;
	}
	safe_strncpy(last_astro_message, raw_text, last_astro_message_len);
	if (always_show_astro_details)
		show_astro_details();
}

void free_astro_buffer()
{
	if (last_astro_message != NULL)
	{
	 	free(last_astro_message);
		last_astro_message = NULL;
		last_astro_message_len = 0;
	}
}

static float calculate_width_coefficient(int amplitude,int value,int invert)
{
	int capped_result = 1;
	float Result = ((float)value / (float)amplitude);

	if (Result < -1.0)
		Result = -1.0;
	else if (Result > 1.0)
		Result = 1.0;
	else
		capped_result = 0;

	if (capped_result && ! capping_already_reported)
	{
		LOG_WARNING("Capped astro for %s: amp=%d value=%d invert=%d\n", stone_name, amplitude, value, invert );
		capping_already_reported = 1;
	}

	if(!invert)
		return Result;
	else
		return - Result;
}

static int display_astrology_handler(window_info *win)
{
	float coefficient1 = 0.0,
		  coefficient2 = 0.0,
		  coefficient3 = 0.0;
	GLfloat right_colors[6];
	int i;

	glColor3f(0.77f,0.57f,0.39f);

	switch(astrology_display_type)
	{
		case adtTwoProgressBars:
		{
			//calculate the width coeffitient of the progresses
			switch(astrology_type)
			{
				case atAttDefIndicator:
				{
					coefficient1 = calculate_width_coefficient(your_info.attack_skill.base/20,value1, 0);
					coefficient2 = calculate_width_coefficient(your_info.defense_skill.base/20,value2,0);
				}break;
				case atCriticalsIndicator:
				{
					coefficient1 = calculate_width_coefficient(8,value1,0);
					coefficient2 = calculate_width_coefficient(8,value2,0);

				}break;
				case atAccMagicIndicator:
				{
					coefficient1 = calculate_width_coefficient(your_info.attack_skill.base/20,value1,0);
					coefficient2 = calculate_width_coefficient(9,value2,0);
				}break;
				case atHarvDegrIndicator:
				{
					coefficient1 = calculate_width_coefficient(90,value1,0);
					coefficient2 = calculate_width_coefficient(900,value2,0);
				}break;
				case atRareFailIndicator:
				{
					coefficient1 = calculate_width_coefficient(800,value1,1);
					coefficient2 = calculate_width_coefficient(9,value2,1);
				}break;
				default:
					/* nothing */ ;
			}

			//draw the name of the stone
			draw_string_small_zoomed_centered(win->len_x / 2, astro_border, (const unsigned char*)stone_name, 1, win->current_scale);

			//draw the first indicator item
			draw_string_small_zoomed (bar_left_x, bar_top_1 - win->small_font_len_y, (const unsigned char*)text_item1, 1, win->current_scale);
			//draw the second indicator item
			draw_string_small_zoomed (bar_left_x, bar_top_2 - win->small_font_len_y, (const unsigned char*)text_item2, 1, win->current_scale);

			//draw the plus/minus
			draw_string_small_zoomed_right(bar_left_x - astro_border, bar_top_1 + (progress_bar_height - win->small_font_len_y) / 2, (const unsigned char*)"-", 1, win->current_scale);
			draw_string_small_zoomed_right(bar_left_x - astro_border, bar_top_2 + (progress_bar_height - win->small_font_len_y) / 2, (const unsigned char*)"-", 1, win->current_scale);

			draw_string_small_zoomed (bar_left_x + 2 * progress_bar_width + astro_border, bar_top_1 + (progress_bar_height - win->small_font_len_y) / 2, (const unsigned char*)"+", 1, win->current_scale);
			draw_string_small_zoomed (bar_left_x + 2 * progress_bar_width + astro_border, bar_top_2 + (progress_bar_height - win->small_font_len_y) / 2, (const unsigned char*)"+", 1, win->current_scale);
		}break;
		case adtThreeProgressBars:
		{
			//calculate the width coeffitient of the progresses
			switch(astrology_type)
			{
				case atAccPredictor:
				case atAttPredictor:
				{
					coefficient1 = calculate_width_coefficient(your_info.attack_skill.base/20,value1,0);
					coefficient2 = calculate_width_coefficient(your_info.attack_skill.base/20,value2,0);
					coefficient3 = calculate_width_coefficient(your_info.attack_skill.base/20,value3,0);
				}break;
				case atDefPredictor:
				{
					coefficient1 = calculate_width_coefficient(your_info.defense_skill.base/20,value1,0);
					coefficient2 = calculate_width_coefficient(your_info.defense_skill.base/20,value2,0);
					coefficient3 = calculate_width_coefficient(your_info.defense_skill.base/20,value3,0);
				}break;
				case atHitPredictor:
				{
					coefficient1 = calculate_width_coefficient(8,value1,0);
					coefficient2 = calculate_width_coefficient(8,value2,0);
					coefficient3 = calculate_width_coefficient(8,value3,0);
				}break;
				case atDamagePredictor:
				{
					coefficient1 = calculate_width_coefficient(8,value1,0);
					coefficient2 = calculate_width_coefficient(8,value2,0);
					coefficient3 = calculate_width_coefficient(8,value3,0);
				}break;
				case atMagicPredictor:
				{
					coefficient1 = calculate_width_coefficient(9,value1,0);
					coefficient2 = calculate_width_coefficient(9,value2,0);
					coefficient3 = calculate_width_coefficient(9,value3,0);
				}break;
				case atHarvPredictor:
				{
					coefficient1 = calculate_width_coefficient(90,value1,0);
					coefficient2 = calculate_width_coefficient(90,value2,0);
					coefficient3 = calculate_width_coefficient(90,value3,0);
				}break;
				case atDegradePredictor:
				{
					coefficient1 = calculate_width_coefficient(900,value1,0);
					coefficient2 = calculate_width_coefficient(900,value2,0);
					coefficient3 = calculate_width_coefficient(900,value3,0);
				}break;
				case atRarePredictor:
				{
					coefficient1 = calculate_width_coefficient(800,value1,1);
					coefficient2 = calculate_width_coefficient(800,value2,1);
					coefficient3 = calculate_width_coefficient(800,value3,1);
				}break;
				case atFailPredictor:
				{
					coefficient1 = calculate_width_coefficient(9,value1,1);
					coefficient2 = calculate_width_coefficient(9,value2,1);
					coefficient3 = calculate_width_coefficient(9,value3,1);
				}break;
				default:
					/* nothing */ ;
			}

			//draw the name of the predictor
			draw_string_small_zoomed_centered(win->len_x / 2, 5, (const unsigned char*)stone_name, 1, win->current_scale);

			//draw the prediction for 20 mins
			draw_string_small_zoomed (bar_left_x, bar_top_1 - win->small_font_len_y, (const unsigned char*)text_item1, 1, win->current_scale);
			//draw the prediction for 40 mins
			draw_string_small_zoomed (bar_left_x, bar_top_2 - win->small_font_len_y, (const unsigned char*)text_item2, 1, win->current_scale);
			//draw the prediction for 60 mins
			draw_string_small_zoomed (bar_left_x, bar_top_3 - win->small_font_len_y, (const unsigned char*)text_item3, 1, win->current_scale);

			//draw the plus/minus
			draw_string_small_zoomed_right(bar_left_x - astro_border, bar_top_1 + (progress_bar_height - win->small_font_len_y) / 2, (const unsigned char*)"-", 1, win->current_scale);
			draw_string_small_zoomed_right(bar_left_x - astro_border, bar_top_2 + (progress_bar_height - win->small_font_len_y) / 2, (const unsigned char*)"-", 1, win->current_scale);
			draw_string_small_zoomed_right(bar_left_x - astro_border, bar_top_3 + (progress_bar_height - win->small_font_len_y) / 2, (const unsigned char*)"-", 1, win->current_scale);

			draw_string_small_zoomed (bar_left_x + 2 * progress_bar_width + astro_border, bar_top_1 + (progress_bar_height - win->small_font_len_y) / 2, (const unsigned char*)"+", 1, win->current_scale);
			draw_string_small_zoomed (bar_left_x + 2 * progress_bar_width + astro_border, bar_top_2 + (progress_bar_height - win->small_font_len_y) / 2, (const unsigned char*)"+", 1, win->current_scale);
			draw_string_small_zoomed (bar_left_x + 2 * progress_bar_width + astro_border, bar_top_3 + (progress_bar_height - win->small_font_len_y) / 2, (const unsigned char*)"+", 1, win->current_scale);
		}break;
	}

	glDisable(GL_TEXTURE_2D);
//	glColor3f(0.77f,0.57f,0.39f);

	//draw progress borders
	glLineWidth (2.0f);
	glBegin(GL_LINES);
	//negative progress 1
		glVertex3i(bar_left_x, bar_top_1,0);
		glVertex3i(bar_left_x + progress_bar_width, bar_top_1,0);
		glVertex3i(bar_left_x, bar_top_1 + progress_bar_height,0);
		glVertex3i(bar_left_x + progress_bar_width, bar_top_1 + progress_bar_height,0);

		glVertex3i(bar_left_x, bar_top_1,0);
		glVertex3i(bar_left_x, bar_top_1 + progress_bar_height,0);
		glVertex3i(bar_left_x + progress_bar_width, bar_top_1,0);
		glVertex3i(bar_left_x + progress_bar_width, bar_top_1 + progress_bar_height,0);
	//positive progress 1
		glVertex3i(bar_left_x + progress_bar_width, bar_top_1,0);
		glVertex3i(bar_left_x + (progress_bar_width << 1), bar_top_1,0);
		glVertex3i(bar_left_x + progress_bar_width, bar_top_1 + progress_bar_height,0);
		glVertex3i(bar_left_x + (progress_bar_width << 1), bar_top_1 + progress_bar_height,0);

		glVertex3i(bar_left_x + progress_bar_width, bar_top_1,0);
		glVertex3i(bar_left_x + progress_bar_width, bar_top_1 + progress_bar_height,0);
		glVertex3i(bar_left_x + (progress_bar_width << 1), bar_top_1,0);
		glVertex3i(bar_left_x + (progress_bar_width << 1), bar_top_1 + progress_bar_height,0);
	//negative progress 2
		glVertex3i(bar_left_x, bar_top_2,0);
		glVertex3i(bar_left_x + progress_bar_width, bar_top_2,0);
		glVertex3i(bar_left_x, bar_top_2 + progress_bar_height,0);
		glVertex3i(bar_left_x + progress_bar_width, bar_top_2 + progress_bar_height,0);

		glVertex3i(bar_left_x, bar_top_2,0);
		glVertex3i(bar_left_x, bar_top_2 + progress_bar_height,0);
		glVertex3i(bar_left_x + progress_bar_width, bar_top_2,0);
		glVertex3i(bar_left_x + progress_bar_width, bar_top_2 + progress_bar_height,0);
	//positive progress 2
		glVertex3i(bar_left_x + progress_bar_width, bar_top_2,0);
		glVertex3i(bar_left_x + (progress_bar_width << 1), bar_top_2,0);
		glVertex3i(bar_left_x + progress_bar_width, bar_top_2 + progress_bar_height,0);
		glVertex3i(bar_left_x + (progress_bar_width << 1), bar_top_2 + progress_bar_height,0);

		glVertex3i(bar_left_x + progress_bar_width, bar_top_2,0);
		glVertex3i(bar_left_x + progress_bar_width, bar_top_2 + progress_bar_height,0);
		glVertex3i(bar_left_x + (progress_bar_width << 1), bar_top_2,0);
		glVertex3i(bar_left_x + (progress_bar_width << 1), bar_top_2 + progress_bar_height,0);
		if(astrology_display_type == adtThreeProgressBars)
		{
		//negative progress 3
			glVertex3i(bar_left_x, bar_top_3,0);
			glVertex3i(bar_left_x + progress_bar_width, bar_top_3,0);
			glVertex3i(bar_left_x, bar_top_3 + progress_bar_height,0);
			glVertex3i(bar_left_x + progress_bar_width, bar_top_3 + progress_bar_height,0);

			glVertex3i(bar_left_x, bar_top_3,0);
			glVertex3i(bar_left_x, bar_top_3 + progress_bar_height,0);
			glVertex3i(bar_left_x + progress_bar_width, bar_top_3,0);
			glVertex3i(bar_left_x + progress_bar_width, bar_top_3 + progress_bar_height,0);
		//positive progress 3
			glVertex3i(bar_left_x + progress_bar_width, bar_top_3,0);
			glVertex3i(bar_left_x + (progress_bar_width << 1), bar_top_3,0);
			glVertex3i(bar_left_x + progress_bar_width, bar_top_3 + progress_bar_height,0);
			glVertex3i(bar_left_x + (progress_bar_width << 1), bar_top_3 + progress_bar_height,0);

			glVertex3i(bar_left_x + progress_bar_width, bar_top_3,0);
			glVertex3i(bar_left_x + progress_bar_width, bar_top_3 + progress_bar_height,0);
			glVertex3i(bar_left_x + (progress_bar_width << 1), bar_top_3,0);
			glVertex3i(bar_left_x + (progress_bar_width << 1), bar_top_3 + progress_bar_height,0);
		}
	glEnd();
	glLineWidth (1.0f);

	glBegin(GL_QUADS);
	//progress 1
		if(coefficient1 < 0)
		{
			for (i=0; i<3; i++)
			{
				right_colors[i+0] = negative_bar_colors[i+3];
				right_colors[i+3] = negative_bar_colors[i+6];
			}
			glColor3fv(&negative_bar_colors[0]);
			glVertex3i(bar_left_x + 1 + (int)(progress_bar_width * (1.0f + coefficient1)), bar_top_1, 0);
			glColor3fv(&right_colors[0]);
			glVertex3i(bar_left_x - 1 + progress_bar_width,bar_top_1,0);
			glColor3fv(&right_colors[3]);
			glVertex3i(bar_left_x - 1 + progress_bar_width, bar_top_1 + progress_bar_height, 0);
			glColor3fv(&negative_bar_colors[9]);
			glVertex3i(bar_left_x + 1 + (int)(progress_bar_width * (1 + coefficient1)), bar_top_1 + progress_bar_height, 0);
		}
		else if(coefficient1 > 0)
		{
			for (i=0; i<3; i++)
			{
				right_colors[i+0] = positive_bar_colors[i+3];
				right_colors[i+3] = positive_bar_colors[i+6];
			}
			glColor3fv(&positive_bar_colors[0]);
			glVertex3i(bar_left_x + 1 + progress_bar_width, bar_top_1, 0);
			glColor3fv(&right_colors[0]);
			glVertex3i(bar_left_x - 1 + progress_bar_width + (int)(progress_bar_width * coefficient1), bar_top_1,0);
			glColor3fv(&right_colors[3]);
			glVertex3i(bar_left_x - 1 + progress_bar_width + (int)(progress_bar_width * coefficient1), bar_top_1 + progress_bar_height, 0);
			glColor3fv(&positive_bar_colors[9]);
			glVertex3i(bar_left_x + 1 + progress_bar_width, bar_top_1 + progress_bar_height, 0);
		}
	//progress 2
		if(coefficient2 < 0)
		{
			for (i=0; i<3; i++)
			{
				right_colors[i+0] = negative_bar_colors[i+3];
				right_colors[i+3] = negative_bar_colors[i+6];
			}
			glColor3fv(&negative_bar_colors[0]);
			glVertex3i(bar_left_x + 1 + (int)(progress_bar_width * (1 + coefficient2)), bar_top_2, 0);
			glColor3fv(&right_colors[0]);
			glVertex3i(bar_left_x - 1 + progress_bar_width,bar_top_2,0);
			glColor3fv(&right_colors[3]);
			glVertex3i(bar_left_x - 1  + progress_bar_width, bar_top_2 + progress_bar_height, 0);
			glColor3fv(&negative_bar_colors[9]);
			glVertex3i(bar_left_x + 1 + (int)(progress_bar_width * (1 + coefficient2)), bar_top_2 + progress_bar_height, 0);
		}
		else if(coefficient2 > 0)
		{
			for (i=0; i<3; i++)
			{
				right_colors[i+0] = positive_bar_colors[i+3];
				right_colors[i+3] = positive_bar_colors[i+6];
			}
			glColor3fv(&positive_bar_colors[0]);
			glVertex3i(bar_left_x + 1 + progress_bar_width, bar_top_2, 0);
			glColor3fv(&right_colors[0]);
			glVertex3i(bar_left_x - 1 + progress_bar_width + (int)(progress_bar_width * coefficient2), bar_top_2,0);
			glColor3fv(&right_colors[3]);
			glVertex3i(bar_left_x - 1 + progress_bar_width + (int)(progress_bar_width * coefficient2), bar_top_2 + progress_bar_height, 0);
			glColor3fv(&positive_bar_colors[9]);
			glVertex3i(bar_left_x + 1 + progress_bar_width, bar_top_2 + progress_bar_height, 0);
		}
		if(astrology_display_type == adtThreeProgressBars)
		{
	//progress 3
			if(coefficient3 < 0)
			{
				for (i=0; i<3; i++)
				{
					right_colors[i+0] = negative_bar_colors[i+3];
					right_colors[i+3] = negative_bar_colors[i+6];
				}
				glColor3fv(&negative_bar_colors[0]);
				glVertex3i(bar_left_x + 1 + (int)(progress_bar_width * (1 + coefficient3)), bar_top_3, 0);
				glColor3fv(&right_colors[0]);
				glVertex3i(bar_left_x - 1 + progress_bar_width,bar_top_3,0);
				glColor3fv(&right_colors[3]);
				glVertex3i(bar_left_x - 1 + progress_bar_width, bar_top_3 + progress_bar_height, 0);
				glColor3fv(&negative_bar_colors[9]);
				glVertex3i(bar_left_x + 1 + (int)(progress_bar_width * (1 + coefficient3)), bar_top_3 + progress_bar_height, 0);
			}
			else if(coefficient3 > 0)
			{
				for (i=0; i<3; i++)
				{
					right_colors[i+0] = positive_bar_colors[i+3];
					right_colors[i+3] = positive_bar_colors[i+6];
				}
				glColor3fv(&positive_bar_colors[0]);
				glVertex3i(bar_left_x + 1 + progress_bar_width, bar_top_3, 0);
				glColor3fv(&right_colors[0]);
				glVertex3i(bar_left_x - 1 + progress_bar_width + (int)(progress_bar_width * coefficient3), bar_top_3,0);
				glColor3fv(&right_colors[3]);
				glVertex3i(bar_left_x - 1 + progress_bar_width + (int)(progress_bar_width * coefficient3), bar_top_3 + progress_bar_height, 0);
				glColor3fv(&positive_bar_colors[9]);
				glVertex3i(bar_left_x + 1 + progress_bar_width, bar_top_3 + progress_bar_height, 0);
			}
		}
	glEnd();

	return 1;
}
