#include <stdint.h>
#include <stdbool.h>
#include "state.h"

static volatile bool stateArr[NUM_OF_STATES] = {false};

void SetState(uint8_t state,bool val)
{
	stateArr[state] = val;
}

bool GetState(uint8_t state)
{
	return stateArr[state];
}

