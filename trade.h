/*!
 * \file
 * \ingroup	network_trade
 * \brief 	Trading related functions
 */
#ifndef __TRADE_H__
#define __TRADE_H__

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int trade_you_accepted; /*!< flag, indicating whether you have accepted the trade or not */
extern int trade_other_accepted; /*!< flag, indicating whether the trade partner has accepted the trade or not */

extern int trade_menu_x;
extern int trade_menu_y;

typedef struct {
	int type;
	int image_id;
	int quantity;
	Uint16 id;
} trade_item;

/*!
 * \name windows handlers
 */
/*! @{ */
extern int trade_win;   /*!< trade windows handler */
/*! @} */

/*!
 * \ingroup 	trade_window
 * \brief 	Displays the trade window.
 *
 * 		Displays the trade window (initiates the window if it hasn't been done before).
 *
 * \callgraph
 */
void display_trade_menu();

//int check_trade_interface();

/*!
 * \ingroup 	trade
 * \brief 	Gets the name of the trade partner.
 *
 *      	Gets the name of the trade partner from network data.
 *
 * \param   	player_name A char * to the network data
 * \param   	len The length of the network data
 */
void get_trade_partner_name (const Uint8 *player_name, int len);

/*!
 * \ingroup 	trade
 * \brief 	Resets the trade objects and gets them from the data.
 *
 *      	Resets the trade, gets your current items from the data and hides other windows that it shouldn't have opened. Is i.e. called when a new trade session is started.
 *
 * \param   	data The network data.
 *
 * \callgraph
 */
void get_your_trade_objects (const Uint8 *data);

/*!
 * \ingroup 	trade
 * \brief 	Puts n items on the trade
 *
 *      	The function puts n items on trade from the network data. If data[7]==0 it's your own items, if it's 1 it's the trade partners items.
 *
 * \param   	data The network data.
 */
void put_item_on_trade (const Uint8 *data);

/*!
 * \ingroup 	trade
 * \brief 	Removes n items from the trade
 *
 *      	Removes n items from the given position in the trade window. If data[3]==0 it's your own items, if it's 1 it's your trade partners items.
 *
 * \param   	data The trade data
 */
void remove_item_from_trade (const Uint8 *data);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
