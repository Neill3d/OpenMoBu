
#pragma once

// References_Exchange.h
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#ifdef MOBU_DLL
#define REFERENCE_SPEC __declspec( dllexport )
#else 
#define REFERENCE_SPEC __declspec(dllimport)
#endif


void REFERENCE_SPEC SetReferenceState(int value);
int REFERENCE_SPEC GetReferenceState();