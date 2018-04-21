
// postprocessing_ini.h

/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

//--- SDK include
#include <fbsdk/fbsdk.h>

struct ConfigHelper
{
	static bool getOrSetBool(FBConfigFile &lConfigFile, char *buffer, const char *section, const char *name, const char *comment, bool defValue);
	static int getOrSetInt(FBConfigFile &lConfigFile, char *buffer, const char *section, const char *name, const char *comment, int defValue);
	static double getOrSetDouble(FBConfigFile &lConfigFile, char *buffer, const char *section, const char *name, const char *comment, double defValue);
	static void getOrSetString(FBString &value, FBConfigFile &lConfigFile, const char *section, const char *name, const char *comment);
};

