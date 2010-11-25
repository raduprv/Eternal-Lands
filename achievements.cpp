#include <iostream>
#include <sstream>

#include "achievements.h"
#include "actors.h"
#include "asc.h"
#include "text.h"

extern "C" void requested_achievements_for_player(actor *player)
{
	std::ostringstream buf;
	buf << "REQUESTED ACHIEVEMENTS FOR [" << ((player != NULL) ?player->actor_name :"") << "]";
	LOG_TO_CONSOLE(c_green1, reinterpret_cast<const unsigned char *>(buf.str().c_str()));
}

extern "C" void here_is_achievements_data(Uint32 *data)
{
	// bit position w1(lsb) ....w5(msb) map to achievement id 0...159
	std::vector<Uint16> achievements;
	for (size_t i=0; i<ACHIEVEMENT_32BIT_WORDS; i++)
	{
		Uint32 word = data[i];
		for (size_t j=0; j<sizeof(Uint32)*8; ++j)
		{
			if (word & 1)
				achievements.push_back(i*sizeof(Uint32)*8+j);
			word >>= 1;
		}
	}

	// write to the console 
	std::ostringstream buf;
	buf << "RECEIVED SEND_ACHIEVEMENTS ids: ";
	for (size_t i=0; i<achievements.size(); ++i)
		buf << ((i==0)?"":", ") << achievements[i];
	LOG_TO_CONSOLE(c_green1, reinterpret_cast<const unsigned char *>(buf.str().c_str()));
}
