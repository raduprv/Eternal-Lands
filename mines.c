#ifdef MINES

#include <stdlib.h>
#include <string.h>
#include "mines.h"
#include "elwindows.h"
#include "eye_candy_wrapper.h"

mine mine_list[NUM_MINES];

char * get_mine_e3d(mine_type)
{
	switch (mine_type)
		{
		case MINE_TYPE_SMALL_MINE:
			{
				return MINE_SMALL_MINE_E3D;
			}
			break;
		case MINE_TYPE_MEDIUM_MINE:
			{
				return MINE_MEDIUM_MINE_E3D;
			}
			break;
		case MINE_TYPE_HIGH_EXPLOSIVE_MINE:
			{
				return MINE_HIGH_EXPLOSIVE_MINE_E3D;
			}
			break;
		case MINE_TYPE_TRAP:
			{
				return MINE_TRAP_E3D;
			}
			break;
		case MINE_TYPE_CALTROP:
			{
				return MINE_CALTROP_E3D;
			}
			break;
		case MINE_TYPE_POISONED_CALTROP:
			{
				return MINE_POISONED_CALTROP_E3D;
			}
			break;
		case MINE_TYPE_BARRICADE:
			{
				return MINE_BARRICADE_E3D;
			}
			break;
		case MINE_TYPE_MANA_DRAINER:
			{
				return MINE_MANA_DRAINER_E3D;
			}
			break;
		case MINE_TYPE_MANA_BURNER:
			{
				return MINE_MANA_BURNER_E3D;
			}
			break;
		case MINE_TYPE_UNINVIZIBILIZER:
			{
				return MINE_UNINVIZIBILIZER_E3D;
			}
			break;
		case MINE_TYPE_MAGIC_IMMUNITY_REMOVAL:
			{
				return MINE_MAGIC_IMMUNITY_REMOVAL_E3D;
			}
			break;
		}
	log_error("An invalid mine type was requested!\n");
	return "";
}

void put_mine_on_ground(int mine_x,int mine_y,int mine_type, int mine_id)
{
	float x,y,z;
	int obj_3d_id;

	//now, get the Z position
	if(mine_y*tile_map_size_x*6+mine_x>tile_map_size_x*tile_map_size_y*6*6) {
		//Warn about this error:
		log_error("A mine was placed OUTSIDE the map!\n");
		return;
	}
	
	z=-2.2f+height_map[mine_y*tile_map_size_x*6+mine_x]*0.2f;
	//convert from height values to meters
	x=(float)mine_x/2;
	y=(float)mine_y/2;
	//center the object
	x=x+0.25f;
	y=y+0.25f;

        //Launch the animation
#ifdef	EYE_CANDY
	if (use_eye_candy) ec_create_mine_drop(x, y, z, (poor_man ? 6 : 10));
#endif	//EYE_CANDY

	obj_3d_id=add_e3d(get_mine_e3d(mine_type),x,y,z,0,0,0,1,0,1.0f,1.0f,1.0f, 1);
	
	//now, find a place into the mines list, so we can destroy the mine properly
	mine_list[mine_id].x=mine_x;
	mine_list[mine_id].y=mine_y;
	mine_list[mine_id].type=mine_type;
	mine_list[mine_id].obj_3d_id=obj_3d_id;
}

void add_mines_from_list (const Uint8 *data)
{
	Uint16 mines_no;
	int i;
	int mine_x,mine_y,mine_type,my_offset;
	float x,y,z;
	int obj_3d_id, mine_id;
	
	// TESTING!!!!
	mine_type = 0;

	mines_no=data[0];

	if(mines_no > NUM_MINES) {
		return;//something nasty happened
	}
	
	for(i=0;i<mines_no;i++) {
		my_offset=i*5+1;
		mine_x=SDL_SwapLE16(*((Uint16 *)(data+my_offset)));
		mine_y=SDL_SwapLE16(*((Uint16 *)(data+my_offset+2)));
		mine_id=*((Uint8 *)(data+my_offset+4));
//		mine_type=*((Uint8 *)(data+my_offset+5));
		mine_type = 0;
		if(mine_id >= NUM_MINES) {
			continue;
		}
		//now, get the Z position
		if(mine_y*tile_map_size_x*6+mine_x>tile_map_size_x*tile_map_size_y*6*6)  {
			//Warn about this error!
			log_error("A mine was located OUTSIDE the map!\n");
			continue;
		}
		
		z=-2.2f+height_map[mine_y*tile_map_size_x*6+mine_x]*0.2f;
		//convert from height values to meters
		x=(float)mine_x/2;
		y=(float)mine_y/2;
		//center the object
		x=x+0.25f;
		y=y+0.25f;
	
		obj_3d_id=add_e3d(get_mine_e3d(mine_type),x,y,z,0,0,0,1,0,1.0f,1.0f,1.0f, 1);

		//now, find a place into the mines list, so we can destroy the mine properly
		if (mine_list[mine_id].obj_3d_id != -1) {
			char	buf[256];

			// oops, slot already taken!
			safe_snprintf(buf, sizeof(buf), "Oops, trying to add an existing mine! id=%d\n", mine_id);
			LOG_ERROR(buf);
			return;
		}

		mine_list[mine_id].x=mine_x;
		mine_list[mine_id].y=mine_y;
		mine_list[mine_id].type=mine_type;
		mine_list[mine_id].obj_3d_id=obj_3d_id;
	}
}

int find_mine_id (int object_id)
{
	int i;
	for(i=0;i<NUM_MINES;i++){
		if(mine_list[i].obj_3d_id==object_id){
			return i;
		}
	}
	// Mine not found
	return -1;
}

void click_mine(int object_id)
{
	int which_mine;
	
	// We need to find what mine we have got
	which_mine = find_mine_id(object_id);

	remove_mine(which_mine);
}

void remove_mine(int which_mine)
{
#ifdef EYE_CANDY
	float x, y, z;
#endif

	if (which_mine == -1 || which_mine >= NUM_MINES) return;

	if (mine_list[which_mine].obj_3d_id == -1) {
		// oops, no mine in that slot!
		LOG_ERROR("Oops, double-removal of mine!\n");
		return;
	}

 #ifdef EYE_CANDY
	x = mine_list[which_mine].x;
	y = mine_list[which_mine].y;
	z = -2.2f+height_map[mine_list[which_mine].y*tile_map_size_x*6+mine_list[which_mine].x]*0.2f;
	//convert from height values to meters
	x /= 2;
	y /= 2;
	//center the object
	x = x + 0.25f;
	y = y + 0.25f;
	if (use_eye_candy) ec_create_mine_remove(x, y, z, (poor_man ? 6 : 10));
 #else // EYE_CANDY
  #ifdef SFX
	add_particle_sys_at_tile ("./particles/bag_out.part", mine_list[which_mine].x, mine_list[which_mine].y, 1);
  #endif
 #endif // EYE_CANDY

	destroy_3d_object(mine_list[which_mine].obj_3d_id);
	mine_list[which_mine].obj_3d_id=-1;
}

void remove_all_mines(){
	int i;

	for(i=0; i<NUM_MINES; i++){    // clear mines list!!!!
		mine_list[i].obj_3d_id= -1;
	}
}

void prime_mine(int object_id)
{
#ifdef EYE_CANDY
	float x, y, z;
#endif
	int which_mine;
	
	// We need to find what mine we have got
	which_mine = find_mine_id(object_id);

	if (which_mine == -1 || which_mine >= NUM_MINES) return;

	if (mine_list[which_mine].obj_3d_id == -1) {
		// oops, no mine in that slot!
		LOG_ERROR("Oops, priming a non-existant mine!\n");
		return;
	}

 #ifdef EYE_CANDY
	x = mine_list[which_mine].x;
	y = mine_list[which_mine].y;
	z = -2.2f+height_map[mine_list[which_mine].y*tile_map_size_x*6+mine_list[which_mine].x]*0.2f;
	//convert from height values to meters
	x /= 2;
	y /= 2;
	//center the object
	x = x + 0.25f;
	y = y + 0.25f;
	if (use_eye_candy) ec_create_mine_prime(x, y, z, (poor_man ? 6 : 10));
 #else // EYE_CANDY
  #ifdef SFX
	switch (mine_list[which_mine].type)
		{
		case 0:
			{
				add_particle_sys_at_tile ("./particles/bag_out.part", mine_list[which_mine].x, mine_list[which_mine].y, 1);
			}
			break;
		case 1:
			{
				add_particle_sys_at_tile ("./particles/bag_out.part", mine_list[which_mine].x, mine_list[which_mine].y, 1);
			}
			break;
		case 2:
			{
				add_particle_sys_at_tile ("./particles/bag_out.part", mine_list[which_mine].x, mine_list[which_mine].y, 1);
			}
			break;
		}
  #endif // SFX
 #endif // EYE_CANDY
}

void detonate_mine(int which_mine)
{
#ifdef EYE_CANDY
	float x, y, z;
#endif

	if (which_mine == -1 || which_mine >= NUM_MINES) return;

	if (mine_list[which_mine].obj_3d_id == -1) {
		// oops, no mine in that slot!
		LOG_ERROR("Oops, detonating a non-existant mine!\n");
		return;
	}

 #ifdef EYE_CANDY
	x = mine_list[which_mine].x;
	y = mine_list[which_mine].y;
	z = -2.2f+height_map[mine_list[which_mine].y*tile_map_size_x*6+mine_list[which_mine].x]*0.2f;
	//convert from height values to meters
	x /= 2;
	y /= 2;
	//center the object
	x = x + 0.25f;
	y = y + 0.25f;
	if (use_eye_candy) ec_create_mine_detonate(x, y, z, mine_list[which_mine].type, (poor_man ? 6 : 10));
 #else // EYE_CANDY
  #ifdef SFX
	switch (mine_list[which_mine].type)
		{
		case 0:
			{
				add_particle_sys_at_tile ("./particles/bag_out.part", mine_list[which_mine].x, mine_list[which_mine].y, 1);
			}
			break;
		case 1:
			{
				add_particle_sys_at_tile ("./particles/bag_out.part", mine_list[which_mine].x, mine_list[which_mine].y, 1);
			}
			break;
		case 2:
			{
				add_particle_sys_at_tile ("./particles/bag_out.part", mine_list[which_mine].x, mine_list[which_mine].y, 1);
			}
			break;
		}
  #endif // SFX
 #endif // EYE_CANDY

	destroy_3d_object(mine_list[which_mine].obj_3d_id);
	mine_list[which_mine].obj_3d_id=-1;
}
#endif // MINES
