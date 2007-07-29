#if defined _client_serv_included
  #endinput
#endif
#define _client_serv_included

enum actor_commands
{
 	nothing = 0,
	kill_me = 1,
	die1 = 3,
	die2 = 4,
	pain1 = 5,
	pain2 = 17,
	pick = 6,
	drop = 7,
	idle = 8,
	harvest = 9,
	cast = 10,
	ranged = 11,
	meele = 12,
	sit_down = 13,
	stand_up = 14,
	turn_left = 15,
	turn_right = 16,
	enter_combat = 18,
	leave_combat = 19,

	move_n = 20,
	move_ne = 21,
	move_e = 22,
	move_se = 23,
	move_s = 24,
	move_sw = 25,
	move_w = 26,
	move_nw = 27,

	run_n = 30,
	run_ne = 31,
	run_e = 32,
	run_se = 33,
	run_s = 34,
	run_sw = 35,
	run_w = 36,
	run_nw = 37,

	turn_n = 38,
	turn_ne = 39,
	turn_e = 40,
	turn_se = 41,
	turn_s = 42,
	turn_sw = 43,
	turn_w = 44,
	turn_nw = 45,

	attack_up_1 = 46,
	attack_up_2 = 47,
	attack_up_3 = 48,
	attack_up_4 = 49,
	attack_down_1 = 50,
	attack_down_2 = 51
}

