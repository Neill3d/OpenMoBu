
// postprocessing_ini.cpp
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

#include "postprocessing_ini.h"


bool ConfigHelper::getOrSetBool(FBConfigFile &lConfigFile, char *buffer, const char *section, const char *name, const char *comment, bool defValue)
{
	sprintf_s(buffer, sizeof(char)* 256, (defValue) ? "1" : "0");
	const char *lbuffer = buffer;
	lConfigFile.GetOrSet(section, name, lbuffer, comment);

	return (strcmp(buffer, "1") == 0);
};

int ConfigHelper::getOrSetInt(FBConfigFile &lConfigFile, char *buffer, const char *section, const char *name, const char *comment, int defValue)
{
	sprintf_s(buffer, sizeof(char)* 256, "%d", defValue);
	const char *lbuffer = buffer;
	lConfigFile.GetOrSet(section, name, lbuffer, comment);

	return atoi(buffer);
};

double ConfigHelper::getOrSetDouble(FBConfigFile &lConfigFile, char *buffer, const char *section, const char *name, const char *comment, double defValue)
{
	sprintf_s(buffer, sizeof(char)* 256, "%.2lf", defValue);
	const char *lbuffer = buffer;
	bool lStatus = lConfigFile.GetOrSet(section, name, lbuffer, comment);

	return atof(lbuffer);
};

void ConfigHelper::getOrSetString(FBString &value, FBConfigFile &lConfigFile, const char *section, const char *name, const char *comment)
{
	//sprintf_s( buffer, sizeof(char)*256, "%s", defValue );
	const char *lbuffer = value;
	lConfigFile.GetOrSet(section, name, lbuffer, comment);
	value = lbuffer;
}