#include "global.h"
int add_item_to_bank(int player_id,int item_type, int quantity);
int add_item_to_player(int player_id,int item_type, int quantity);




void send_inventory_item(int player_id, int pos)
{
	Uint8 str[32];

	str[0]=GET_NEW_INVENTORY_ITEM;
	*((Uint16 *)(str+1))=9;
	*((Uint16 *)(str+3))=items[players[player_id].player_data.actor_items[pos].item_category].image_id;
	*((Uint32 *)(str+5))=players[player_id].player_data.actor_items[pos].quantity;
	str[9]=pos;
	str[10]=items[players[player_id].player_data.actor_items[pos].item_category].flags;
	MY_SDLNet_TCP_Send(players[player_id].sock, str, 11);
}

void read_items_def()
{
	int cur_item=0;
	int f_size;
	FILE *f = NULL;
	Uint8 * asc_file_mem;
	Uint8 * asc_file_mem_start;

  f = fopen("items_def.txt", "rb");
  if(!f)return;
  fseek(f,0,SEEK_END);
  f_size = ftell(f);
  //ok, allocate memory for it
  asc_file_mem=(Uint8 *)calloc(f_size+200, 1);
  asc_file_mem_start=asc_file_mem;
  fseek (f, 0, SEEK_SET);
  fread (asc_file_mem, 1, f_size, f);
  fclose (f);

	while(cur_item<255)
		{
			int i;
			int size_to_search;

			i=get_string_occurance("[item]",asc_file_mem,f_size-(asc_file_mem-asc_file_mem_start),0);
			if(i==-1)break;//no more items

			asc_file_mem+=i;//go to the next item, if any
			//see where the item ends

			size_to_search=get_string_occurance("[/item]",asc_file_mem,f_size-(asc_file_mem-asc_file_mem_start),0);
			if(size_to_search==-1)
				{
					log_error("An item has no end tag...");
					break;
				}

			//read the names/info/etc.
 			 i=get_string_occurance("item_name:",asc_file_mem,f_size-(asc_file_mem-asc_file_mem_start),0);

			if(i!=-1)
				{
					Uint8 *temp=asc_file_mem;
					asc_file_mem+=i;
					str_copy_grave(items[cur_item].name,asc_file_mem,64);
					asc_file_mem=temp;//we don't know the order of the strings...
				}

 			 i=get_string_occurance("item_description:",asc_file_mem,f_size-(asc_file_mem-asc_file_mem_start),0);

			if(i!=-1)
				{
					Uint8 *temp=asc_file_mem;
					asc_file_mem+=i;
					str_copy_grave(items[cur_item].description,asc_file_mem,200);
					asc_file_mem=temp;//we don't know the order of the strings...
				}

			items[cur_item].give_back_item=
			get_integer_after_string("give_back_item:",asc_file_mem,size_to_search);

			items[cur_item].image_id=
			get_integer_after_string("image_id:",asc_file_mem,size_to_search);

			//read the flags
			items[cur_item].flags=0;

			if(get_integer_after_string("is_reagent:",asc_file_mem,size_to_search)==1)
			items[cur_item].flags|=ITEM_REAGENT;

			if(get_integer_after_string("is_resource:",asc_file_mem,size_to_search)==1)
			items[cur_item].flags|=ITEM_RESOURCE;

			if(get_integer_after_string("is_stackable:",asc_file_mem,size_to_search)==1)
			items[cur_item].flags|=ITEM_STACKABLE;

			if(get_integer_after_string("is_turnable:",asc_file_mem,size_to_search)==1)
			items[cur_item].flags|=ITEM_ON_OFF;

			if(get_integer_after_string("is_inventory_usable:",asc_file_mem,size_to_search)==1)
			items[cur_item].flags|=ITEM_INVENTORY_USABLE;

			if(get_integer_after_string("is_tile_usable:",asc_file_mem,size_to_search)==1)
			items[cur_item].flags|=ITEM_TILE_USABLE;

			if(get_integer_after_string("is_player_usable:",asc_file_mem,size_to_search)==1)
			items[cur_item].flags|=ITEM_PLAYER_USABLE;

			if(get_integer_after_string("is_object_usable:",asc_file_mem,size_to_search)==1)
			items[cur_item].flags|=ITEM_OBJECT_USABLE;

			//read the rest of the things
			items[cur_item].wearable=get_integer_after_string("is_wearable:",asc_file_mem,size_to_search);
			if(items[cur_item].wearable==-1)items[cur_item].wearable=0;

			items[cur_item].base_cost=
			get_integer_after_string("base_cost:",asc_file_mem,size_to_search);

			items[cur_item].base_quality=
			get_integer_after_string("base_quality:",asc_file_mem,size_to_search);

			items[cur_item].base_quantity=
			get_integer_after_string("base_quantity:",asc_file_mem,size_to_search);

			items[cur_item].weight=
			get_integer_after_string("weight:",asc_file_mem,size_to_search);

			//teleport stuff
			items[cur_item].teleport_x=
			get_integer_after_string("teleport_x:",asc_file_mem,size_to_search);
			items[cur_item].teleport_y=
			get_integer_after_string("teleport_y:",asc_file_mem,size_to_search);
			items[cur_item].teleport_map=
			get_integer_after_string("teleport_map:",asc_file_mem,size_to_search);

			/////////////////on use stuff//////////////////////////////////////////////////

			items[cur_item].increase_attack_on_use=
			get_integer_after_string("increase_attack_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_attack_on_use==-1)items[cur_item].increase_attack_on_use=0;

			items[cur_item].increase_defense_on_use=
			get_integer_after_string("increase_defense_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_defense_on_use==-1)items[cur_item].increase_defense_on_use=0;

			items[cur_item].increase_combat_on_use=
			get_integer_after_string("increase_combat_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_combat_on_use==-1)items[cur_item].increase_combat_on_use=0;

			items[cur_item].increase_manufacturing_on_use=
			get_integer_after_string("increase_manufacturing_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_manufacturing_on_use==-1)items[cur_item].increase_manufacturing_on_use=0;

			items[cur_item].increase_harvesting_on_use=
			get_integer_after_string("increase_harvesting_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_harvesting_on_use==-1)items[cur_item].increase_harvesting_on_use=0;

			items[cur_item].increase_alchemy_on_use=
			get_integer_after_string("increase_alchemy_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_alchemy_on_use==-1)items[cur_item].increase_alchemy_on_use=0;

			items[cur_item].increase_magic_on_use=
			get_integer_after_string("increase_magic_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_magic_on_use==-1)items[cur_item].increase_magic_on_use=0;

			items[cur_item].increase_potion_on_use=
			get_integer_after_string("increase_potion_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_potion_on_use==-1)items[cur_item].increase_potion_on_use=0;

			items[cur_item].increase_summoning_on_use=
			get_integer_after_string("increase_summoning_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_summoning_on_use==-1)items[cur_item].increase_summoning_on_use=0;

			items[cur_item].increase_armor_on_use=
			get_integer_after_string("increase_armor_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_armor_on_use==-1)items[cur_item].increase_armor_on_use=0;

			items[cur_item].increase_physique_on_use=
			get_integer_after_string("increase_physique_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_physique_on_use==-1)items[cur_item].increase_physique_on_use=0;

			items[cur_item].increase_coordination_on_use=
			get_integer_after_string("increase_coordination_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_coordination_on_use==-1)items[cur_item].increase_coordination_on_use=0;

			items[cur_item].increase_reasoning_on_use=
			get_integer_after_string("increase_reasoning_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_reasoning_on_use==-1)items[cur_item].increase_reasoning_on_use=0;

			items[cur_item].increase_will_on_use=
			get_integer_after_string("increase_will_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_will_on_use==-1)items[cur_item].increase_will_on_use=0;

			items[cur_item].increase_instinct_on_use=
			get_integer_after_string("increase_instinct_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_instinct_on_use==-1)items[cur_item].increase_instinct_on_use=0;

			items[cur_item].increase_vitality_on_use=
			get_integer_after_string("increase_vitality_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_vitality_on_use==-1)items[cur_item].increase_vitality_on_use=0;

			items[cur_item].increase_human_nexus_on_use=
			get_integer_after_string("increase_human_nexus_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_human_nexus_on_use==-1)items[cur_item].increase_human_nexus_on_use=0;

			items[cur_item].increase_animal_nexus_on_use=
			get_integer_after_string("increase_animal_nexus_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_animal_nexus_on_use==-1)items[cur_item].increase_animal_nexus_on_use=0;

			items[cur_item].increase_vegetal_nexus_on_use=
			get_integer_after_string("increase_vegetal_nexus_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_vegetal_nexus_on_use==-1)items[cur_item].increase_vegetal_nexus_on_use=0;

			items[cur_item].increase_inorganic_nexus_on_use=
			get_integer_after_string("increase_inorganic_nexus_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_inorganic_nexus_on_use==-1)items[cur_item].increase_inorganic_nexus_on_use=0;

			items[cur_item].increase_artificial_nexus_on_use=
			get_integer_after_string("increase_artificial_nexus_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_artificial_nexus_on_use==-1)items[cur_item].increase_artificial_nexus_on_use=0;

			items[cur_item].increase_magic_nexus_on_use=
			get_integer_after_string("increase_magic_nexus_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_magic_nexus_on_use==-1)items[cur_item].increase_magic_nexus_on_use=0;

			items[cur_item].increase_material_points_on_use=
			get_integer_after_string("increase_material_points_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_material_points_on_use==-1)items[cur_item].increase_material_points_on_use=0;

			items[cur_item].increase_ethereal_points_on_use=
			get_integer_after_string("increase_ethereal_points_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_ethereal_points_on_use==-1)items[cur_item].increase_ethereal_points_on_use=0;

			items[cur_item].increase_karma_on_use=
			get_integer_after_string("increase_karma_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_karma_on_use==-1)items[cur_item].increase_karma_on_use=0;

			items[cur_item].increase_food_on_use=
			get_integer_after_string("increase_food_on_use:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_food_on_use==-1)items[cur_item].increase_food_on_use=0;
			//////////////////////////on wear stuff
			items[cur_item].increase_armor_on_wear=
			get_integer_after_string("increase_armor_on_wear:",asc_file_mem,size_to_search);
			if(items[cur_item].increase_armor_on_wear==-1)items[cur_item].increase_armor_on_wear=0;

			items[cur_item].damage=
			get_integer_after_string("damage:",asc_file_mem,size_to_search);
			if(items[cur_item].damage==-1)items[cur_item].damage=0;

			items[cur_item].accuracy=
			get_integer_after_string("accuracy:",asc_file_mem,size_to_search);
			if(items[cur_item].accuracy==-1)items[cur_item].accuracy=0;

			items[cur_item].what_kind_wearable=
			get_integer_after_string("what_kind_wearable:",asc_file_mem,size_to_search);

			items[cur_item].what_wearable_id=
			get_integer_after_string("what_wearable_id:",asc_file_mem,size_to_search);

			items[cur_item].is_quest_item=
			get_integer_after_string("is_quest_item:",asc_file_mem,size_to_search);
			if(items[cur_item].is_quest_item==-1)items[cur_item].is_quest_item=0;

			cur_item++;
		}

}

//!!!!!!!!!!!!!!!!TODO: send a int instead of short for quantity, and a short for image id
void send_inventory_items(int player_id,int for_trade)
{
	Uint8 str[500];
	int i;
	int items_no=0;

	if(!for_trade)
	str[0]=HERE_YOUR_INVENTORY;
	else
	str[0]=GET_YOUR_TRADEOBJECTS;

	for(i=0;i<36+8;i++)
		{
			if(players[player_id].player_data.actor_items[i].quantity)
				{
					Uint8 flags;
					*((Uint16 *)(str+4+items_no*8))=items[players[player_id].player_data.actor_items[i].item_category].image_id;
					*((Uint32 *)(str+4+items_no*8+2))=players[player_id].player_data.actor_items[i].quantity;
					str[4+items_no*8+6]=i;
					str[4+items_no*8+7]=items[players[player_id].player_data.actor_items[i].item_category].flags;
					items_no++;
				}
		}
	*((Uint16 *)(str+1))=2+items_no*8;
	str[3]=items_no;
	MY_SDLNet_TCP_Send(players[player_id].sock, &str, 4+items_no*8);
}


void send_inventory_item_info(int player_id,Uint8 pos)
{
	Uint8 str[300];
	int len;

	if(pos>=36+8)return;//fuck you

			if(players[player_id].player_data.actor_items[pos].quantity)
				{
					sprintf(&str[4],INVENTORY_ITEM_LOOK,items[players[player_id].player_data.actor_items[pos].item_category].name,
					items[players[player_id].player_data.actor_items[pos].item_category].description,items[players[player_id].player_data.actor_items[pos].item_category].weight);
					len=strlen(&str[4]);
					len+=2;
					str[0]=INVENTORY_ITEM_TEXT;
					*((Uint16 *)(str+1))=len;
					str[3]=my_text[0]=127+c_green2;
					MY_SDLNet_TCP_Send(players[player_id].sock, str, len+2);
				}

}

void move_inventory_item(int old_pos,int new_pos,int player_id)
{
	int k;
	char str[300];
	int len;
	int item_category;

	if(players[player_id].trading_with!=-1)
		{
			my_text[0]=127+c_red1;
			my_strcp(&my_text[1],CANT_DO_WHILE_TRADING);
			send_text_to_player(player_id);
			return;
		}


	if(old_pos>36+8 || new_pos>36+8)
		{
			sprintf(&str[4],"U R teh sux, if u think u can send stupid info and foul the server!");
			len=strlen(&str[4]);
			len+=2;
			str[0]=INVENTORY_ITEM_TEXT;
			*((Uint16 *)(str+1))=len;
			str[3]=my_text[0]=127+c_red2;
			MY_SDLNet_TCP_Send(players[player_id].sock, str, len+2);
			return;
		}




	if(players[player_id].player_data.actor_items[old_pos].quantity)
		{
			//see if the targeted position is free
			if(players[player_id].player_data.actor_items[new_pos].quantity)
				{
					sprintf(&str[4],"U R teh sux, this position is already occupied! Stop sending bogous input to the server!");
					len=strlen(&str[4]);
					len+=2;
					str[0]=INVENTORY_ITEM_TEXT;
					*((Uint16 *)(str+1))=len;
					str[3]=my_text[0]=127+c_red2;
					MY_SDLNet_TCP_Send(players[player_id].sock, str, len+2);
					return;
				}

				if(new_pos>=36)
					{
						int kind_of_wear;
						int cur_kind_of_wear;

						item_category=players[player_id].player_data.actor_items[old_pos].item_category;

						kind_of_wear=items[players[player_id].player_data.actor_items[old_pos].item_category].wearable;
						if(!kind_of_wear)
							{
								sprintf(&str[4],CANT_WEAR_ITEM);
								len=strlen(&str[4]);
								len+=2;
								str[0]=INVENTORY_ITEM_TEXT;
								*((Uint16 *)(str+1))=len;
								str[3]=my_text[0]=127+c_red2;
								MY_SDLNet_TCP_Send(players[player_id].sock, str, len+2);
								return;
							}
						for(k=36;k<36+8;k++)
							{
								if(players[player_id].player_data.actor_items[k].quantity)
									{
										cur_kind_of_wear=items[players[player_id].player_data.actor_items[k].item_category].wearable;
										if(kind_of_wear==cur_kind_of_wear || ((kind_of_wear==LEFT_HAND || kind_of_wear==RIGHT_HAND)&& cur_kind_of_wear==BOTH_HANDS)
											|| ((cur_kind_of_wear==LEFT_HAND || cur_kind_of_wear==RIGHT_HAND)&& kind_of_wear==BOTH_HANDS))
											{
												sprintf(&str[4],"Can't wear, a similar item is already worn!");
												len=strlen(&str[4]);
												len+=2;
												str[0]=INVENTORY_ITEM_TEXT;
												*((Uint16 *)(str+1))=len;
												str[3]=my_text[0]=127+c_red2;
												MY_SDLNet_TCP_Send(players[player_id].sock, str, len+2);
												return;
											}
									}
							}

					}
			//if we are here, we have a valid move
			str[0]=REMOVE_ITEM_FROM_INVENTORY;
			*((Uint16 *)(str+1))=2;
			str[3]=old_pos;
			MY_SDLNet_TCP_Send(players[player_id].sock, str, 4);

			players[player_id].player_data.actor_items[new_pos].item_category=
			players[player_id].player_data.actor_items[old_pos].item_category;
			players[player_id].player_data.actor_items[new_pos].quantity=
			players[player_id].player_data.actor_items[old_pos].quantity;

			send_inventory_item(player_id,new_pos);

			players[player_id].player_data.actor_items[old_pos].item_category=0;
			players[player_id].player_data.actor_items[old_pos].quantity=0;

			if(old_pos<36 && new_pos>=36)wear_item(player_id, new_pos);//wear the item
			if(old_pos>=36 && new_pos<36)unwear_item(player_id, new_pos);//wear the item

		}


}

/*
Returns:
1 =Everything OK
0 =Not enough space
-1=Too heavy
*/

int add_item_to_player(int player_id,int item_type, int quantity)
{
	int i;
	int total_carry_items=0;
	char free_list[36]={0};
	int is_stackable;
	Uint8 str[50];
	int player_max_carry;
	int player_cur_carry;

	//see if this player is not too loaded
	player_cur_carry=get_carried_weight(player_id);
	player_max_carry=(players[player_id].player_data.physique.base+players[player_id].player_data.coordination.base)/2*20;
	if(items[item_type].weight*quantity+player_cur_carry>player_max_carry)return -1;
	//see if the item is stackable
	is_stackable=items[item_type].flags&ITEM_STACKABLE;

	for(i=0;i<36;i++)
		{
			if(players[player_id].player_data.actor_items[i].quantity)
				{
					if(is_stackable && players[player_id].player_data.actor_items[i].item_category==item_type)
						{
							//see if the quantity doesn't go over 0xffffffff
							if(players[player_id].player_data.actor_items[i].quantity+quantity<0xffffffff)
								{
									players[player_id].player_data.actor_items[i].quantity+=quantity;
									send_inventory_item(player_id,i);
									send_partial_stat_to_player(player_id,CARRY_WGHT_CUR);
									return 1;
								}
						}
					//mark this item as occupied in the free_list
					free_list[i]=1;
					total_carry_items++;
				}
		}
	if(total_carry_items>35)return 0;
	//if the item is not stackable, make sure we have enoug free slots to add all of it
	if(!is_stackable && total_carry_items+quantity>36)return 0;
	//find a free space in the inventory...
	if(is_stackable)
	for(i=0;i<36;i++)
		{
			if(!free_list[i])
				{
					//ok, we found a free position in the inventory
					players[player_id].player_data.actor_items[i].quantity=quantity;
					players[player_id].player_data.actor_items[i].item_category=item_type;
					send_inventory_item(player_id,i);
					send_partial_stat_to_player(player_id,CARRY_WGHT_CUR);
					return 1;
				}
		}
	else//not stackable
	for(i=0;i<36;i++)
		{
			if(!free_list[i])
				{
					//ok, we found a free position in the inventory
					players[player_id].player_data.actor_items[i].quantity=1;
					players[player_id].player_data.actor_items[i].item_category=item_type;
					send_inventory_item(player_id,i);
					quantity--;
					if(!quantity)
						{
							send_partial_stat_to_player(player_id,CARRY_WGHT_CUR);
							return 1;//ok, we are done with them
						}
					break;
				}
		}

	return 0;

}

void open_bag(int player_id)
{
	Uint8 str[500];
	int i;
	int bags_no=0;
	int map_id;
	int which_bag;

	which_bag=players[player_id].opened_bag;
	map_id=players[player_id].player_data.map_id;
	if(which_bag==-1)return;

	//first of all get the number of items this bag carries
	str[0]=HERE_YOUR_GROUND_ITEMS;
	for(i=0;i<50;i++)
		{
			if(map_list[map_id].bags[which_bag].items[i].quantity)
				{
					Uint8 flags;
					*((Uint16 *)(str+4+bags_no*7))=items[map_list[map_id].bags[which_bag].items[i].item_category].image_id;
					*((Uint32 *)(str+4+bags_no*7+2))=map_list[map_id].bags[which_bag].items[i].quantity;
					str[4+bags_no*7+6]=i;
					bags_no++;
				}
		}
	*((Uint16 *)(str+1))=2+bags_no*7;
	str[3]=bags_no;
	MY_SDLNet_TCP_Send(players[player_id].sock, &str, 4+bags_no*7);
}

void destroy_bag(int map_id, int bag_id)
{
	int i;
	Uint8 str[8];

	if(!map_list[map_id].bags[bag_id].in_use)return;
	//first, find out if there is any player that has that bag open
	for(i=0;i<max_players;i++)
     {
        if(players[i].sock)
        if(players[i].logged_in)
        if(players[i].player_data.map_id==map_id)
        if(players[i].opened_bag==bag_id)
        	{
				send_1_octet_command_to_player(CLOSE_BAG, i);
				players[i].opened_bag=-1;
				break;
			}
	 }

	//now, send a message to the entire map to destroy that bag
	str[0]=DESTROY_BAG;
	*((Uint16 *)(str+1))=2;
	*((Uint8 *)(str+3))=bag_id;
	send_message_to_map(map_id,str,4);

	//make it not in use
	map_list[map_id].bags[bag_id].in_use=0;
	//decrease the number of bags on this map
	map_list[map_id].bags_no--;
}

void remove_item_from_player(int player_id, int item_type,int quantity)
{
	int i;
	Uint8 str[32];

	for(i=0;i<36;i++)
		{
			if(players[player_id].player_data.actor_items[i].quantity)
				{
					if(players[player_id].player_data.actor_items[i].item_category==item_type)
						{
							if(players[player_id].player_data.actor_items[i].quantity>=quantity)
								{
									players[player_id].player_data.actor_items[i].quantity-=quantity;
									quantity=0;
								}
							else
								{
									quantity-=players[player_id].player_data.actor_items[i].quantity;
									players[player_id].player_data.actor_items[i].quantity=0;
								}
							if(!players[player_id].player_data.actor_items[i].quantity)
								{
									str[0]=REMOVE_ITEM_FROM_INVENTORY;
									*((Uint16 *)(str+1))=2;
									str[3]=i;
									MY_SDLNet_TCP_Send(players[player_id].sock, str, 4);
								}
							else
							send_inventory_item(player_id,i);

							if(!quantity)
								{
									send_partial_stat_to_player(player_id,CARRY_WGHT_CUR);
									return;//finally, we are done with it
								}
						}

				}
		}
}

void drop_item(int player_id, int pos,int quantity)
{
	int i;
	int map_id;
	int item_exists=0;
	int bag_at_the_feet=0;
	int my_x,my_y;
	int which_bag=0;//to avoid warnings
	int item_category;
	Uint8 str[200];
	Uint32 min_time_stamp=0xffffffff;
	int min_time_stamp_entry;
	int all_bags_taken=1;
	int first_time;

	if(players[player_id].trading_with!=-1)
		{
			my_text[0]=127+c_red1;
			my_strcp(&my_text[1],CANT_DO_WHILE_TRADING);
			send_text_to_player(player_id);
			return;
		}

	if(pos>=36)
		{
			my_text[0]=127+c_red1;
			my_strcp(&my_text[1],CANT_DROP_WORN_ITEM);
			send_text_to_player(player_id);
			return;
		}

	//first see if that item really exists
	if(players[player_id].player_data.actor_items[pos].quantity)
	item_exists=1;

	if(!item_exists)return;//*shrug* that wasn't supposed to happen
	//check to see if the quantity of the item is at least equal with the one we try to take
	if(players[player_id].player_data.actor_items[pos].quantity<quantity)return;
	item_category=players[player_id].player_data.actor_items[pos].item_category;

	//ok, now that the item exists, see if there is a bag at our feet...
	map_id=players[player_id].player_data.map_id;
	my_x=players[player_id].player_data.x_pos;
	my_y=players[player_id].player_data.y_pos;

	if(map_list[map_id].bags_no)
	for(i=0;i<MAX_GROUND_BAGS;i++)
		{
			if(map_list[map_id].bags[i].in_use)
			if(map_list[map_id].bags[i].x==my_x)
			if(map_list[map_id].bags[i].y==my_y)
				{
					bag_at_the_feet=1;
					which_bag=i;
					if(players[player_id].opened_bag==i)
						{
							first_time=0;
						}
					else
						{
							players[player_id].opened_bag=i;
							first_time=1;
						}
					if(first_time)open_bag(player_id);
					break;
				}
		}

	if(bag_at_the_feet)
		{
			//see if there is already an item with the same id
			for(i=0;i<50;i++)
				{
					if(map_list[map_id].bags[which_bag].items[i].quantity)
					if(map_list[map_id].bags[which_bag].items[i].item_category==item_category)
					goto space_found;
				}

			//see if there is enough room in this bag
			for(i=0;i<50;i++)
				{
					if(!map_list[map_id].bags[which_bag].items[i].quantity)
					break;
				}
			space_found:
					if(i<50)
						{
							//we found a free spot, now take the item from that guy and put it in the bag
							players[player_id].player_data.actor_items[pos].quantity-=quantity;
							if(!players[player_id].player_data.actor_items[pos].quantity)
								{
									str[0]=REMOVE_ITEM_FROM_INVENTORY;
									*((Uint16 *)(str+1))=2;
									str[3]=pos;
									MY_SDLNet_TCP_Send(players[player_id].sock, str, 4);
								}
							else
							send_inventory_item(player_id,pos);

							map_list[map_id].bags[which_bag].items[i].quantity+=quantity;
							map_list[map_id].bags[which_bag].items[i].item_category=players[player_id].player_data.actor_items[pos].item_category;
							//now, tell the client that he has a new item, on the ground
							str[0]=GET_NEW_GROUND_ITEM;
							*((Uint16 *)(str+1))=9;
							*((Uint16 *)(str+3))=items[item_category].image_id;
							*((Uint32 *)(str+5))=map_list[map_id].bags[which_bag].items[i].quantity;
							str[9]=i;
							str[10]=items[item_category].flags;
							MY_SDLNet_TCP_Send(players[player_id].sock, str, 11);
							//update the last accessed time stamp
							map_list[map_id].bags[which_bag].time_stamp=time_stamp;
							return;
						}
					else//not enough room, damn it!
						{
							int len;
							sprintf(&str[4],NO_ROOM_TO_DROP_ITEM);
							len=strlen(&str[4]);
							len+=2;
							str[0]=INVENTORY_ITEM_TEXT;
							*((Uint16 *)(str+1))=len;
							str[3]=my_text[0]=127+c_red2;
							MY_SDLNet_TCP_Send(players[player_id].sock, str, len+2);
							return;
						}
		}//ok, there is no bag at our feet, create one


		for(i=0;i<MAX_GROUND_BAGS;i++)
		{
			if(!map_list[map_id].bags[i].in_use)
				{
					all_bags_taken=0;
					which_bag=i;
					break;
				}
		}

		if(all_bags_taken)//no free bag spot, must delete an old one...
		for(i=0;i<MAX_GROUND_BAGS;i++)
			{
				if(map_list[map_id].bags[which_bag].in_use)
				if(map_list[map_id].bags[i].time_stamp<min_time_stamp)
					{
						min_time_stamp=map_list[map_id].bags[i].time_stamp;
						which_bag=i;
					}
			}

		if(all_bags_taken)destroy_bag(map_id,which_bag);

		//ok, now add a new bag
		map_list[map_id].bags[which_bag].time_stamp=time_stamp;
		map_list[map_id].bags[which_bag].in_use=1;
		map_list[map_id].bags[which_bag].x=my_x;
		map_list[map_id].bags[which_bag].y=my_y;

		//increase the number of bags on this map
		map_list[map_id].bags_no++;

		//now, notify everyone we have a winner (in fact, a new bag)
		str[0]=GET_NEW_BAG;
		*((Uint16 *)(str+1))=6;
		*((Uint16 *)(str+3))=my_x;
		*((Uint16 *)(str+5))=my_y;
		*((Uint8 *)(str+7))=which_bag;
		send_message_to_map(map_id,str,8);


		//clear the bag content
		for(i=0;i<50;i++)
			{
				map_list[map_id].bags[which_bag].items[i].quantity=0;
			}

		i=0;//we are using the first item in the bag, since it is a new bag

		players[player_id].opened_bag=which_bag;
		//send the "hey, open the ground inventory" message to the client
		open_bag(player_id);


		players[player_id].player_data.actor_items[pos].quantity-=quantity;
		if(!players[player_id].player_data.actor_items[pos].quantity)
			{
				str[0]=REMOVE_ITEM_FROM_INVENTORY;
				*((Uint16 *)(str+1))=2;
				str[3]=pos;
				MY_SDLNet_TCP_Send(players[player_id].sock, str, 4);
			}
		else
		send_inventory_item(player_id,pos);

		map_list[map_id].bags[which_bag].items[i].quantity+=quantity;
		map_list[map_id].bags[which_bag].items[i].item_category=players[player_id].player_data.actor_items[pos].item_category;
		//now, tell the client that he has a new item, on the ground
		str[0]=GET_NEW_GROUND_ITEM;
		*((Uint16 *)(str+1))=9;
		*((Uint16 *)(str+3))=items[item_category].image_id;
		*((Uint32 *)(str+5))=map_list[map_id].bags[which_bag].items[i].quantity;
		str[9]=i;
		str[10]=items[item_category].flags;
		MY_SDLNet_TCP_Send(players[player_id].sock, str, 11);
		return;


}

void send_bags_list(int player_id)
{
	Uint8 str[500];
	int i;
	int bags_no=0;
	int map_id;

	map_id=players[player_id].player_data.map_id;

	//first of all get the number of items this person carries
	str[0]=GET_BAGS_LIST;
	for(i=0;i<MAX_GROUND_BAGS;i++)
		{
			if(map_list[map_id].bags[i].in_use)
				{
					*((Uint16 *)(str+4+bags_no*5))=map_list[map_id].bags[i].x;
					*((Uint16 *)(str+4+bags_no*5+2))=map_list[map_id].bags[i].y;
					*((Uint8 *)(str+4+bags_no*5+4))=i;//the bag id
					bags_no++;
				}
		}
	*((Uint16 *)(str+1))=2+bags_no*5;
	str[3]=bags_no;
	if(bags_no)MY_SDLNet_TCP_Send(players[player_id].sock, &str, 4+bags_no*5);
}


void send_ground_item_info(int player_id,Uint8 pos)
{
	Uint8 str[300];
	int len,cur_item,cur_bag,cur_map;

	//check to see if the bag exists
	cur_bag=players[player_id].opened_bag;
	if(cur_bag==-1)return;//*shrug* there is no bag here

	//see if the user tried to feed us with an invalid position...
	if(pos>49)
		{
			disconnect_player(player_id);//the motherfucker tried to crash the server
			sprintf(str,"%s tried to look at object no %i in a bag.\n",players[player_id].player_data.player_name,pos);
			log_violation(str);
			return;
		}

	cur_map=players[player_id].player_data.map_id;
	cur_item=map_list[cur_map].bags[cur_bag].items[pos].item_category;
	if(map_list[cur_map].bags[cur_bag].items[pos].quantity)
		{
			sprintf(&str[4],INVENTORY_ITEM_LOOK,items[cur_item].name,items[cur_item].description,items[cur_item].weight);
			len=strlen(&str[4]);
			len+=2;
			str[0]=INVENTORY_ITEM_TEXT;
			*((Uint16 *)(str+1))=len;
			str[3]=my_text[0]=127+c_green2;
			MY_SDLNet_TCP_Send(players[player_id].sock, str, len+2);
			return;
		}

}




void get_item_from_bag(int player_id,Uint8 pos,int quantity)
{
	Uint8 str[300];
	int len,cur_item,cur_bag,cur_map;
	int i;
	int res;

	if(players[player_id].trading_with!=-1)
		{
			my_text[0]=127+c_red1;
			my_strcp(&my_text[1],CANT_DO_WHILE_TRADING);
			send_text_to_player(player_id);
			return;
		}


	//check to see if the bag exists
	cur_bag=players[player_id].opened_bag;
	if(cur_bag==-1)return;//*shrug* there is no bag here

	cur_map=players[player_id].player_data.map_id;
	cur_item=map_list[cur_map].bags[cur_bag].items[pos].item_category;
	//do we have any item at that position?
	//see if the user tried to feed us with an invalid position...
	if(pos>49)
		{
			disconnect_player(player_id);//the motherfucker tried to crash the server
			sprintf(str,"%s tried to take object no %i from a bag.\n",players[player_id].player_data.player_name,pos);
			log_violation(str);
			return;
		}

	if(!map_list[cur_map].bags[cur_bag].items[pos].quantity)return;
	//test if someone wants to take more than [s]he diserves
	if(map_list[cur_map].bags[cur_bag].items[pos].quantity-quantity>=0)
		{
			//check if the player has enough room in the inventory for that item
			res=add_item_to_player(player_id,cur_item,quantity);
			if(res==0)
				{
					sprintf(&str[4],INVENTORY_FULL);
					len=strlen(&str[4]);
					len+=2;
					str[0]=INVENTORY_ITEM_TEXT;
					*((Uint16 *)(str+1))=len;
					str[3]=my_text[0]=127+c_red2;
					MY_SDLNet_TCP_Send(players[player_id].sock, str, len+2);
					return;
				}
			if(res==-1)
				{
					sprintf(&str[4],YOU_ARE_OVERLOADED);
					len=strlen(&str[4]);
					len+=2;
					str[0]=INVENTORY_ITEM_TEXT;
					*((Uint16 *)(str+1))=len;
					str[3]=my_text[0]=127+c_red2;
					MY_SDLNet_TCP_Send(players[player_id].sock, str, len+2);
					return;
				}

			//now, decrease the quantity of that item, from the bag
			map_list[cur_map].bags[cur_bag].items[pos].quantity-=quantity;
			//see if there is anything left from that item
			if(!map_list[cur_map].bags[cur_bag].items[pos].quantity)
				{
					int counter=0;
					//see if there are other items left in this bag
					for(i=0;i<50;i++)
						{
							if(map_list[cur_map].bags[cur_bag].items[i].quantity)
								{
									counter=1;
									break;
								}
						}
					//if the bag is empty now, destroy it
					if(!counter)destroy_bag(cur_map,cur_bag);

					str[0]=REMOVE_ITEM_FROM_GROUND;
					*((Uint16 *)(str+1))=2;
					str[3]=pos;
					MY_SDLNet_TCP_Send(players[player_id].sock, str, 4);
				}//there is still some of that item left there
			else
			{
				//now, tell the client that he has a new item, on the ground
				str[0]=GET_NEW_GROUND_ITEM;
				*((Uint16 *)(str+1))=9;
				*((Uint16 *)(str+3))=items[cur_item].image_id;
				*((Uint32 *)(str+5))=map_list[cur_map].bags[cur_bag].items[pos].quantity;
				str[9]=pos;
				str[10]=items[cur_item].flags;
				MY_SDLNet_TCP_Send(players[player_id].sock, str, 11);
			}


			return;

		}

}


void bags_cleanup()
{
	int i,j;
	int bags_no;

	for(i=0;i<=maps_number;i++)
		{
			bags_no=map_list[i].bags_no;
			if(!bags_no)continue;//no maps here
			for(j=0;j<MAX_GROUND_BAGS;j++)
				{
					if(map_list[i].bags[j].in_use)
					if(map_list[i].bags[j].time_stamp+60*1000*12<time_stamp)
					destroy_bag(i,j);
				}
		}
}

void inspect_bag(int player_id,int bag_id,int direct_from_client)
{
	int map_id;


	if(bag_id>=MAX_GROUND_BAGS)
		{
			char str[300];
			disconnect_player(player_id);//the motherfucker tried to crash the server
			sprintf(str,"%s tried to pick up an invalid bag.\n",(char *)&players[player_id].player_data.player_name);
			log_violation(str);
			return;
		}

	map_id=players[player_id].player_data.map_id;
	//see if the player is right on that bag
	if(!map_list[map_id].bags[bag_id].in_use)return;
	if(map_list[map_id].bags[bag_id].x!=players[player_id].player_data.x_pos ||
	map_list[map_id].bags[bag_id].y!=players[player_id].player_data.y_pos)
		{
			if(!direct_from_client)return;
			players[player_id].final_action=PICK_BAG_ACTION;
			players[player_id].final_object=bag_id;
			find_path(player_id,map_list[map_id].bags[bag_id].x,map_list[map_id].bags[bag_id].y);
			return;
		}
	//fine, every prerequisites are completed
	players[player_id].opened_bag=bag_id;
	open_bag(player_id);

}

void drop_item_when_die(int player_id, int which_item,int quantity)
{
	int i;
	int map_id;
	int bag_at_the_feet=0;
	int my_x,my_y;
	int which_bag=0;//to avoid warnings
	int item_category;
	Uint8 str[200];
	Uint32 min_time_stamp=0xffffffff;
	int min_time_stamp_entry;
	int all_bags_taken=1;


	item_category=players[player_id].player_data.actor_items[which_item].item_category;

	//see if there is a bag at our feet...
	map_id=players[player_id].player_data.map_id;
	my_x=players[player_id].player_data.x_pos;
	my_y=players[player_id].player_data.y_pos;

	if(map_list[map_id].bags_no)
	for(i=0;i<MAX_GROUND_BAGS;i++)
		{
			if(map_list[map_id].bags[i].in_use)
			if(map_list[map_id].bags[i].x==my_x)
			if(map_list[map_id].bags[i].y==my_y)
				{
					bag_at_the_feet=1;
					which_bag=i;
					if(players[player_id].opened_bag!=i)
						{
							players[player_id].opened_bag=i;
						}
					break;
				}
		}

	if(bag_at_the_feet)
		{
			//see if there is already an item with the same id
			for(i=0;i<50;i++)
				{
					if(map_list[map_id].bags[which_bag].items[i].quantity)
					if(map_list[map_id].bags[which_bag].items[i].item_category==item_category)
					goto space_found;
				}

			//see if there is enough room in this bag
			for(i=0;i<50;i++)
				{
					if(!map_list[map_id].bags[which_bag].items[i].quantity)
					break;
				}
			space_found:
					if(i<50)
						{
							//we found a free spot, now take the item from that guy and put it in the bag
							players[player_id].player_data.actor_items[which_item].quantity-=quantity;
							if(!players[player_id].player_data.actor_items[which_item].quantity)
								{
									str[0]=REMOVE_ITEM_FROM_INVENTORY;
									*((Uint16 *)(str+1))=2;
									str[3]=which_item;
									MY_SDLNet_TCP_Send(players[player_id].sock, str, 4);
								}
							else
							send_inventory_item(player_id,which_item);

							map_list[map_id].bags[which_bag].items[i].quantity+=quantity;
							map_list[map_id].bags[which_bag].items[i].item_category=players[player_id].player_data.actor_items[which_item].item_category;

							//update the last accessed time stamp
							str[9]=map_list[map_id].bags[which_bag].time_stamp=time_stamp;
							return;
						}
					else//not enough room, damn it! The player still losses that item
						{
							players[player_id].player_data.actor_items[which_item].quantity-=quantity;
							if(!players[player_id].player_data.actor_items[which_item].quantity)
								{
									str[0]=REMOVE_ITEM_FROM_INVENTORY;
									*((Uint16 *)(str+1))=2;
									str[3]=which_item;
									MY_SDLNet_TCP_Send(players[player_id].sock, str, 4);
								}
							else
							send_inventory_item(player_id,which_item);

							return;
						}
		}//ok, there is no bag at our feet, create one


		for(i=0;i<MAX_GROUND_BAGS;i++)
		{
			if(!map_list[map_id].bags[i].in_use)
				{
					all_bags_taken=0;
					which_bag=i;
					break;
				}
		}

		if(all_bags_taken)//no free bag spot, must delete an old one...
		for(i=0;i<MAX_GROUND_BAGS;i++)
			{
				if(map_list[map_id].bags[which_bag].in_use)
				if(map_list[map_id].bags[i].time_stamp<min_time_stamp)
					{
						min_time_stamp=map_list[map_id].bags[i].time_stamp;
						which_bag=i;
					}
			}


		if(all_bags_taken)destroy_bag(map_id,which_bag);

		//ok, now add a new bag
		map_list[map_id].bags[which_bag].time_stamp=time_stamp;
		map_list[map_id].bags[which_bag].in_use=1;
		map_list[map_id].bags[which_bag].x=my_x;
		map_list[map_id].bags[which_bag].y=my_y;

		//increase the number of bags on this map
		map_list[map_id].bags_no++;

		//now, notify everyone we have a winner (in fact, a new bag)
		str[0]=GET_NEW_BAG;
		*((Uint16 *)(str+1))=6;
		*((Uint16 *)(str+3))=my_x;
		*((Uint16 *)(str+5))=my_y;
		*((Uint8 *)(str+7))=which_bag;
		send_message_to_map(map_id,str,8);


		//clear the bag content
		for(i=0;i<50;i++)
			{
				map_list[map_id].bags[which_bag].items[i].quantity=0;
			}

		i=0;//we are using the first item in the bag, since it is a new bag

		players[player_id].opened_bag=which_bag;
		//send the "hey, open the ground inventory" message to the client

		players[player_id].player_data.actor_items[which_item].quantity-=quantity;
		if(!players[player_id].player_data.actor_items[which_item].quantity)
			{
				str[0]=REMOVE_ITEM_FROM_INVENTORY;
				*((Uint16 *)(str+1))=2;
				str[3]=which_item;
				MY_SDLNet_TCP_Send(players[player_id].sock, str, 4);
			}
		else
		send_inventory_item(player_id,which_item);

		map_list[map_id].bags[which_bag].items[i].quantity+=quantity;
		map_list[map_id].bags[which_bag].items[i].item_category=players[player_id].player_data.actor_items[which_item].item_category;
		return;


}



int add_item_to_bank(int player_id,int item_type, int quantity)
{
	int i,j;
	int total_bank_items=0;
	char free_list[100]={0};

	for(36+8;i<100+36+8;i++)
		{
			if(players[player_id].player_data.actor_items[i].quantity)
				{
					if(players[player_id].player_data.actor_items[i].item_category==item_type)
						{
							//see if the quantity doesn't go over 0xffffffff
							if(players[player_id].player_data.actor_items[i].quantity+quantity<0xffffffff)
								{
									players[player_id].player_data.actor_items[i].quantity+=quantity;
									return 1;
								}
						}
					//mark this item as occupied in the free_list
					free_list[i-44]=1;
					total_bank_items++;
				}
		}
	if(total_bank_items>99)return 0;
	//find a free space in the bank
	for(i=0;i<100;i++)
		{
			if(!free_list[i])
				{
					//ok, we found a free position in the inventory
					if(!players[player_id].player_data.actor_items[i+44].quantity)
						{
							players[player_id].player_data.actor_items[i+44].quantity=quantity;
							players[player_id].player_data.actor_items[i+44].item_category=item_type;
							return 1;
						}
				}
		}

	return 0;

}


void remove_item_from_bank(int player_id, int item_type,int quantity)
{
	int i;
	for(i=36+8;i<100+36+8;i++)
		{
			if(players[player_id].player_data.actor_items[i].quantity)
				{
					if(players[player_id].player_data.actor_items[i].item_category==item_type)
						{
							if(players[player_id].player_data.actor_items[i].quantity>=quantity)
								{
									players[player_id].player_data.actor_items[i].quantity-=quantity;
									quantity=0;
								}
							else
								{
									quantity-=players[player_id].player_data.actor_items[i].quantity;
									players[player_id].player_data.actor_items[i].quantity=0;
								}
							if(!quantity)return;//finally, we are done with it
						}

				}
		}
}









