
#ifndef LOG_H_
#define LOG_H_


/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

//------------------------------------------------------------------//
//- class LOG ------------------------------------------------------//
//------------------------------------------------------------------//
//- Creates a log text file, to keep track of what went successful,-//
//- what went wrong, and other information.  Defined in Log.cpp.   -//
//------------------------------------------------------------------//
class LOG
	{
	private:
		FILE* logfile;

		bool Init(void) {
			//Clear the log contents
			if((logfile=fopen("Log.txt", "wb"))==NULL)
				return false;

			//Close the file, and return a success!
			fclose(logfile);
			return true;
		}

		bool Shutdown(void) {
			if(logfile)
				fclose(logfile);

			return true;
		}


	public:


		bool Output(char* text, ...) {
			va_list arg_list;

			//Initialize variable argument list
			va_start(arg_list, text);

			//Open the log file for append
			if((logfile = fopen("Log.txt", "a+"))==NULL)
				return false;

			//Write the text and a newline
			vfprintf(logfile, text, arg_list);
			putc('\n', logfile);

			//Close the file
			fclose(logfile);
			va_end(arg_list);

			return true;
		}

	LOG()
		{	Init();	}

	~LOG()
		{	Shutdown();	}
	};


#endif