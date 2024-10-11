#ifndef WISDOM_GATEWAY_H
#define WISDOM_GATEWAY_H

#include "gateway_interface.h"

// I don't like that this enum is duplicated
// I just don't have a better way until I refactor
// this code that I JUST NEED WORKING NOW
// TODO: cry
enum _modem_state_e {
	MODEM_UNINITIALIZED,
	MODEM_POWERED_DOWN,
	MODEM_STOPPED,
	MODEM_STARTED,
	MODEM_CN_ACTIVE,
	MODEM_SERVER_CONNECTED
};

extern int MODEM_CORE_STATE;

#endif // WISDOM_GATEWAY_H
