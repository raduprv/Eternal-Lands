#ifndef __TRADE_H__
#define __TRADE_H__

extern int trade_win;

void display_trade_menu();
//int check_trade_interface();
void get_trade_partner_name(Uint8 *player_name,int len);
void get_your_trade_objects(Uint8 *data);
void put_item_on_trade(Uint8 *data);
void remove_item_from_trade(Uint8 *data);

#endif
