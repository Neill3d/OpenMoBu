
// References_exchange.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "References_Exchange.h"

int	gState = 0;

void REFERENCE_SPEC SetReferenceState(int value)
{
	gState = value;
}

int REFERENCE_SPEC GetReferenceState()
{
	return gState;
}
