/*!
 * \file
 * \brief Trading related functions
 * \internal Check groups!
 */
/*!
 * \defgroup    trade   Trading related stuff
 * \ingroup     misc
 */
#ifndef __TRADE_H__
#define __TRADE_H__

extern int trade_win;   /*!< trade_win */

/*!
 * \ingroup trade
 * \brief display_trade_menu
 *
 *      TODO: display_trade_menu
 *
 * \param   None
 * \return  None
 */
void display_trade_menu();

//int check_trade_interface();

/*!
 * \ingroup trade
 * \brief get_trade_partner_name
 *
 *      TODO: get_trade_partner_name
 *
 * \param   player_name
 * \param   len
 * \return  None
 */
void get_trade_partner_name(Uint8 *player_name,int len);

/*!
 * \ingroup trade
 * \brief get_your_trade_objects
 *
 *      TODO: get_your_trade_objects
 *
 * \param   data
 * \return  None
 */
void get_your_trade_objects(Uint8 *data);

/*!
 * \ingroup trade
 * \brief put_item_on_trade
 *
 *      TODO: put_item_on_trade
 *
 * \param   data
 * \return  None
 */
void put_item_on_trade(Uint8 *data);

/*!
 * \ingroup trade
 * \brief remove_item_from_trade
 *
 *      TODO: remove_item_from_trade
 *
 * \param   data
 * \return  None
 */
void remove_item_from_trade(Uint8 *data);

#endif
