#include "global.h"

void finish_trade(int player_id)
{
	int i;
	int the_other_player;
	Uint8 str[200];
	inventory_item palyer_1_items[36];
	inventory_item palyer_2_items[36];
	int player_with_problems;//store which player has a problem with the items (can't carry them)


	the_other_player=players[player_id].trading_with;
	if(the_other_player==-1)return;//shrug. This player is trading with no one...

	//mark them both as free (they are not trading with anyone)
	players[player_id].trading_with=-1;
	players[the_other_player].trading_with=-1;
	players[player_id].trade_accepted=0;
	players[the_other_player].trade_accepted=0;

	str[0]=GET_TRADE_EXIT;
	*((Uint16 *)(str+1))=1;
	MY_SDLNet_TCP_Send(players[player_id].sock, str, 3);
	MY_SDLNet_TCP_Send(players[the_other_player].sock, str, 3);


	//ok, start to copy the inventory in a temp structure

    memcpy(palyer_1_items, players[player_id].player_data.actor_items, sizeof( inventory_item)*36);
    memcpy(palyer_2_items, players[the_other_player].player_data.actor_items, sizeof( inventory_item)*36);


	//excellent, now let's put eachother's stuff where it belongs
	for(i=0;i<16;i++)
		{
			if(players[player_id].trade_items[i].quantity)
				{
					if(add_item_to_player(the_other_player,players[player_id].trade_items[i].item_category,
					players[player_id].trade_items[i].quantity)!=1)
						{
							player_with_problems=0;
							goto restore_original;
						}
				}
		}

	for(i=0;i<16;i++)
		{
			if(players[the_other_player].trade_items[i].quantity)
				{
					if(add_item_to_player(player_id,players[the_other_player].trade_items[i].item_category,
					players[the_other_player].trade_items[i].quantity)!=1)
						{
							player_with_problems=1;
							goto restore_original;
						}
				}
		}

	goto end_of_trade;

	restore_original:

    memcpy( players[player_id].player_data.actor_items, palyer_1_items, sizeof( inventory_item)*36);
    memcpy( players[the_other_player].player_data.actor_items, palyer_2_items, sizeof( inventory_item) *36);

	for(i=0;i<16;i++)
		{
			if(players[player_id].trade_items[i].quantity)
				{
					add_item_to_player(player_id,players[player_id].trade_items[i].item_category,
					players[player_id].trade_items[i].quantity);
					players[player_id].trade_items[i].quantity=0;
				}
		}

	//put the other's player objects back.
	for(i=0;i<16;i++)
		{
			if(players[the_other_player].trade_items[i].quantity)
				{
					add_item_to_player(the_other_player,players[the_other_player].trade_items[i].item_category,
					players[the_other_player].trade_items[i].quantity);
					players[the_other_player].trade_items[i].quantity=0;
				}
		}

	my_text[0]=127+c_red1;
	sprintf(&my_text[1],TRADE_FAILED_PLAYER_CANT_CARRY_ITEMS,player_with_problems ? players[player_id].player_data.player_name : players[the_other_player].player_data.player_name);
	send_text_to_player(the_other_player);
	send_text_to_player(player_id);

	//people might think they have their stuff duplicated, so let's send them both their inventories
	send_inventory_items(player_id,0);
	send_inventory_items(the_other_player,0);

	end_of_trade:
	//now, clear the players on trade items...
	for(i=0;i<16;i++)
		{
			if(players[player_id].trade_items[i].quantity)players[player_id].trade_items[i].quantity=0;
		}

	for(i=0;i<16;i++)
		{
			if(players[the_other_player].trade_items[i].quantity)players[the_other_player].trade_items[i].quantity=0;
		}


}

void complete_initiate_trade(int player_1, int player_2)
{
	Uint8 str[32];
	int len;

	//make it so that players are trading with eachother
	players[player_1].trading_with=player_2;
	players[player_2].trading_with=player_1;

	//after the trade is completed, they shouldn't be pending trade with anyone
	players[player_1].pending_trading_with=-1;
	players[player_2].pending_trading_with=-1;

	//now, send a message to both players that the trading mode is engaged
	send_inventory_items(player_1,1);
	send_inventory_items(player_2,1);

	//now, let's send the name of the players to eachother
	len=strlen(players[player_2].player_data.player_name);
	str[0]=GET_TRADE_PARTNER_NAME;
	*((Uint16 *)(str+1))=len+1;
	my_strcp(&str[3],players[player_2].player_data.player_name);
	MY_SDLNet_TCP_Send(players[player_1].sock, str, len+3);

	len=strlen(players[player_1].player_data.player_name);
	str[0]=GET_TRADE_PARTNER_NAME;
	*((Uint16 *)(str+1))=len+1;
	my_strcp(&str[3],players[player_1].player_data.player_name);
	MY_SDLNet_TCP_Send(players[player_2].sock, str, len+3);
}

void initiate_trade(int player_id, int target_player)
{
	Uint8 str[80];
	float src_x;
	float src_y;
	float dst_x;
	float dst_y;

	if(players[player_id].trading_with==target_player)return;//we are already trading with that player, WTF!

	//see if the target_player is a valid entity
	if(target_player<0 || target_player>=max_players+max_ai)
		{
			disconnect_player(player_id);//the motherfucker tried to crash the server
			sprintf(str,"[%s] tried to look at an out of range actor\n",players[player_id].player_data.player_name);
			log_violation(str);
			return;
		}

	if(!players[target_player].logged_in)return;//maybe that player logged off?

	//see if the other player is an existing human
		if(target_player>=max_players)
		{
			my_text[0]=127+c_red1;
			my_strcp(&my_text[1],CANT_TRADE_WITH_NPCS);
			send_text_to_player(player_id);
			return;
		}
	//see if the other player is self...
		if(target_player==player_id)
		{
			my_text[0]=127+c_red1;
			my_strcp(&my_text[1],CANT_TRADE_WITH_YOURSELF);
			send_text_to_player(player_id);
			return;
		}

	if(players[player_id].player_data.map_id!=players[target_player].player_data.map_id)
		{
			my_text[0]=127+c_red1;
			my_strcp(&my_text[1],CANT_TRADE_NOT_IN_SAME_MAP);
    		send_text_to_player(player_id);
    		return;
		}
	//see if the players are close to eachother
	src_x=players[player_id].player_data.x_pos;
	src_y=players[player_id].player_data.y_pos;
	dst_x=players[target_player].player_data.x_pos;
	dst_y=players[target_player].player_data.y_pos;
	if((dst_x-src_x)*(dst_x-src_x)+(dst_y-src_y)*(dst_y-src_y)>3*3)
		{
			my_text[0]=127+c_red1;
			my_strcp(&my_text[1],TOO_FAR_AWAY);
    		send_text_to_player(player_id);
    		return;
		}

	//see if the current player is trading already with someone
	if(players[player_id].trading_with!=-1)
		{
			my_text[0]=127+c_red1;
			my_strcp(&my_text[1],ALREADY_TRADING);
			send_text_to_player(player_id);
			return;
		}

	//see if the other player is already trading with someone else
		if(players[target_player].trading_with!=-1)
		{
			my_text[0]=127+c_red1;
			sprintf(&my_text[1],TARGET_ALREADY_ON_TRADE,players[target_player].player_data.player_name);
			send_text_to_player(player_id);
			return;
		}

	//see if the other player is fighting with someone
		if(players[target_player].number_of_opponents)
		{
			my_text[0]=127+c_red1;
			sprintf(&my_text[1],TRADE_TARGET_IS_FIGHTING,players[target_player].player_data.player_name);
			send_text_to_player(player_id);
			return;
		}

	//see if we are fighting with someone
		if(players[player_id].number_of_opponents)
		{
			my_text[0]=127+c_red1;
			my_strcp(&my_text[1],CANT_TRADE_WHEN_IN_COMBAT);
			send_text_to_player(player_id);
			return;
		}

		//see if that player is pending a trade with us
		if(players[target_player].pending_trading_with==player_id)
			{
				complete_initiate_trade(player_id,target_player);
				return;
			}
		else
			{
				if(players[player_id].pending_trading_with!=target_player)
					{
						//notify the other player that someone else wants to trade with him
						my_text[0]=127+c_yellow2;
						sprintf(&my_text[1],INCOMING_TRADE_REQUEST,players[player_id].player_data.player_name);
						send_text_to_player(target_player);

						//notify ourselves
						my_text[0]=127+c_yellow2;
						sprintf(&my_text[1],OUTGOING_TRADE_REQUEST,players[target_player].player_data.player_name);
						send_text_to_player(player_id);

					}

				players[player_id].pending_trading_with=target_player;
				return;
			}

}

void accept_trade(int player_id)
{
	Uint8 str[20];

	if(players[player_id].trading_with==-1)return;//shrug. This player is trading with no one...

	players[player_id].trade_accepted=1;

	str[0]=GET_TRADE_ACCEPT;
	*((Uint16 *)(str+1))=2;
	str[3]=0;
	MY_SDLNet_TCP_Send(players[player_id].sock, str, 4);

	str[0]=GET_TRADE_ACCEPT;
	*((Uint16 *)(str+1))=2;
	str[3]=1;
	MY_SDLNet_TCP_Send(players[players[player_id].trading_with].sock, str, 4);

	//check if both accepted, and if so, complete the trade
	if(players[players[player_id].trading_with].trade_accepted)
	finish_trade(player_id);
}

void reject_trade(int player_id)
{
	Uint8 str[20];

	if(players[player_id].trading_with==-1)return;//shrug. This player is trading with no one...

	players[player_id].trade_accepted=0;

	str[0]=GET_TRADE_REJECT;
	*((Uint16 *)(str+1))=2;
	str[3]=0;
	MY_SDLNet_TCP_Send(players[player_id].sock, str, 4);

	str[0]=GET_TRADE_REJECT;
	*((Uint16 *)(str+1))=2;
	str[3]=1;
	MY_SDLNet_TCP_Send(players[players[player_id].trading_with].sock, str, 4);

}

void put_object_on_trade(int player_id, int pos, int quantity)
{
	int quantity_to_put;
	int i;
	Uint8 str[100];
	int item_category;
	int free_trade_slot=-1;

	if(players[player_id].trading_with==-1)return;//shrug. This player is trading with no one...

	if(pos>=36)return;//this player wants to trade an object s/he wears or has in bank



	reject_trade(player_id);


	if(!players[player_id].player_data.actor_items[pos].quantity)return;

	if(players[player_id].player_data.actor_items[pos].quantity<quantity)
	quantity_to_put=players[player_id].player_data.actor_items[pos].quantity;
	else quantity_to_put=quantity;
	item_category=players[player_id].player_data.actor_items[pos].item_category;

	//see if this guy has more than 0xffffffff of that item on trade
	for(i=0;i<16;i++)
		{
			int quantity_count=0;
			if(players[player_id].trade_items[i].quantity && players[player_id].trade_items[i].item_category==item_category)
				{
					quantity_count+=players[player_id].trade_items[i].quantity;
					if(quantity_count+quantity_to_put>0xffffffff)return;//no more than 0xffffffff at the same time
				}
		}

	//get the first free position
	for(i=0;i<16;i++)
		{
			if(!players[player_id].trade_items[i].quantity || (players[player_id].trade_items[i].quantity && players[player_id].trade_items[i].item_category==item_category))
				{
					free_trade_slot=i;
					break;
				}
		}
	if(free_trade_slot==-1)return;//no free trade slots

	//ok, now fill that slot in...
	players[player_id].trade_items[free_trade_slot].quantity+=quantity_to_put;
	players[player_id].trade_items[free_trade_slot].item_category=item_category;

	//now, substract that item from the players main inventory
	remove_item_from_player(player_id,item_category,quantity_to_put);

	str[0]=GET_TRADE_OBJECT;
	*((Uint16 *)(str+1))=9;
	*((Uint16 *)(str+3))=items[item_category].image_id;
	*((Uint32 *)(str+5))=quantity_to_put;
	str[9]=free_trade_slot;
	str[10]=0;//send item to self
	MY_SDLNet_TCP_Send(players[player_id].sock, str, 11);
	str[10]=1;//send item to other
	MY_SDLNet_TCP_Send(players[players[player_id].trading_with].sock, str, 11);


}

void look_at_your_trade_item(int player_id, int pos)
{
	Uint8 str[128];
	int cur_item;
	int len;


	if(pos>15)return;
	if(players[player_id].trading_with==-1)return;
	if(!players[player_id].trade_items[pos].quantity)return;

	cur_item=players[player_id].trade_items[pos].item_category;
	sprintf(&str[4],INVENTORY_ITEM_LOOK,items[cur_item].name,items[cur_item].description,items[cur_item].weight);
	len=strlen(&str[4]);
	len+=2;
	str[0]=INVENTORY_ITEM_TEXT;
	*((Uint16 *)(str+1))=len;
	str[3]=my_text[0]=127+c_green2;
	MY_SDLNet_TCP_Send(players[player_id].sock, str, len+2);
	return;
}

void look_at_their_trade_item(int player_id, int pos)
{
	Uint8 str[128];
	int cur_item;
	int len;
	int target_id;

	target_id=players[player_id].trading_with;
	if(pos>15)return;
	if(target_id==-1)return;
	if(!players[target_id].trade_items[pos].quantity)return;

	cur_item=players[target_id].trade_items[pos].item_category;
	sprintf(&str[4],INVENTORY_ITEM_LOOK,items[cur_item].name,items[cur_item].description,items[cur_item].weight);
	len=strlen(&str[4]);
	len+=2;
	str[0]=INVENTORY_ITEM_TEXT;
	*((Uint16 *)(str+1))=len;
	str[3]=my_text[0]=127+c_green2;
	MY_SDLNet_TCP_Send(players[player_id].sock, str, len+2);
	return;
}

void remove_object_from_trade(int player_id, int pos, int quantity)
{
	int quantity_to_remove;
	int item_category;
	Uint8 str[100];

	if(players[player_id].trading_with==-1)return;//shrug. This player is trading with no one...
	//check to see if we have a valid position
	if(pos>=16)return;
	if(!players[player_id].trade_items[pos].quantity)return;

	reject_trade(players[player_id].trading_with);

	if(players[player_id].trade_items[pos].quantity<quantity)
	quantity_to_remove=players[player_id].trade_items[pos].quantity;
	else quantity_to_remove=quantity;

	players[player_id].trade_items[pos].quantity-=quantity_to_remove;
	//give back the item to that player
	item_category=players[player_id].trade_items[pos].item_category;
	add_item_to_player(player_id,item_category,quantity_to_remove,0,0);
	//now, notify both players about this
	str[0]=REMOVE_TRADE_OBJECT;
	*((Uint16 *)(str+1))=5;
	*((Uint16 *)(str+3))=quantity_to_remove;
	str[5]=pos;
	str[6]=0;//send item to self
	MY_SDLNet_TCP_Send(players[player_id].sock, str, 7);
	str[6]=1;//send item to other
	MY_SDLNet_TCP_Send(players[players[player_id].trading_with].sock, str, 7);

}



/*
This means one of the players aborted the trade so, just put their objects back
altho we do no checks to ensure if there is enough space in their inventory, there should be enough space
since the users are not allowed to do anything that would cause them to geain any new items while they trade
*/
void exit_trade(int player_id)
{
	int i;
	int the_other_player;
	Uint8 str[200];


	the_other_player=players[player_id].trading_with;
	if(the_other_player==-1)return;//shouldn't happen, but let's be sure

	//set them as trading with no one
	players[player_id].trading_with=-1;
	players[the_other_player].trading_with=-1;
	players[player_id].trade_accepted=0;
	players[the_other_player].trade_accepted=0;


	//mark them both as free (they are not trading with anyone)
	players[player_id].trading_with=-1;
	players[the_other_player].trading_with=-1;

	if(the_other_player==-1)return;//shrug. This player is trading with no one...

	//put the first player's objects back
	for(i=0;i<16;i++)
		{
			if(players[player_id].trade_items[i].quantity)
				{
					add_item_to_player(player_id,players[player_id].trade_items[i].item_category,
					players[player_id].trade_items[i].quantity);
					players[player_id].trade_items[i].quantity=0;
				}
		}

	//put the other's player objects back.
	for(i=0;i<16;i++)
		{
			if(players[the_other_player].trade_items[i].quantity)
				{
					add_item_to_player(the_other_player,players[the_other_player].trade_items[i].item_category,
					players[the_other_player].trade_items[i].quantity);
					players[the_other_player].trade_items[i].quantity=0;
				}
		}

	//tell both players to close the trade
	str[0]=GET_TRADE_EXIT;
	*((Uint16 *)(str+1))=1;
	MY_SDLNet_TCP_Send(players[player_id].sock, str, 3);
	MY_SDLNet_TCP_Send(players[the_other_player].sock, str, 3);

	my_text[0]=127+c_red1;
	my_strcp(&my_text[1],YOU_ABORTED_TRADE);
	send_text_to_player(player_id);

	my_text[0]=127+c_red1;
	sprintf(&my_text[1],PARTNER_ABORTED_TRADE,players[player_id].player_data.player_name);
	send_text_to_player(the_other_player);
}


