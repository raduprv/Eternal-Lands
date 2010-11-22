#include <iostream>
#include <sstream>

#include "actors.h"
#include "asc.h"
#include "awards.h"
#include "text.h"

extern "C" void requested_awards_for_player(actor *player)
{
	std::ostringstream buf;
	buf << "REQUESTED AWARDS FOR [" << ((player != NULL) ?player->actor_name :"") << "]";
	LOG_TO_CONSOLE(c_green1, reinterpret_cast<const unsigned char *>(buf.str().c_str()));
}

extern "C" void here_is_awards_data(Uint32 *data)
{
	// bit position w1(lsb) ....w5(msb) map to award id 0...159
	std::vector<Uint16> awards;
	for (size_t i=0; i<AWARD_32BIT_WORDS; i++)
	{
		Uint32 word = data[i];
		for (size_t j=0; j<sizeof(Uint32)*8; ++j)
		{
			if (word & 1)
				awards.push_back(i*sizeof(Uint32)*8+j);
			word >>= 1;
		}
	}

	// write to the console 
	std::ostringstream buf;
	buf << "RECEIVED SEND_AWARDS ids: ";
	for (size_t i=0; i<awards.size(); ++i)
		buf << ((i==0)?"":", ") << awards[i];
	LOG_TO_CONSOLE(c_green1, reinterpret_cast<const unsigned char *>(buf.str().c_str()));
}
