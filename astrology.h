#ifndef __ASTROLOGY_H__
#define __ASTROLOGY_H__

#include "items.h"
#include "elwindows.h"

#ifdef __cplusplus
extern "C" {
#endif

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

extern int astrology_win_x;
extern int astrology_win_y;
extern int astrology_win_x_len;
extern int astrology_win_y_len;

extern int astrology_win;

float calculate_width_coefficeint(int amplitude,int value,int invert);
void display_astrology_window();
int display_astrology_handler(window_info *win);
int is_astrology_message (const char * RawText);

extern float load_bar_colors[12];

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__ASTROLOGY_H__
