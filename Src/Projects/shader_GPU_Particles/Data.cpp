//
// Basic class to handle reading of static data
//
// Author: Alex V. Boreskoff
//

#include	<stdio.h>

#ifdef	_WIN32
    #include	<fcntl.h>
    #include	<io.h>
    #include	<sys/stat.h>
#else
    #include	<unistd.h>
    #include	<sys/types.h>
    #include	<sys/stat.h>
    #include	<fcntl.h>
    #define	O_BINARY	0
#endif

#ifdef	MACOSX
	#include	<stdlib.h>
#else
#include	<malloc.h>
#endif

#include	<memory.h>
#include	<string.h>
#include	"Data.h"

Data :: Data ( void * ptr, int len )
{
	bits   = (byte *) ptr;
	length = len;
	pos    = 0;
}

Data :: Data ( const char * fileName )
{
												// make a fix for windows to replace '/' in file path
												// to windoze style '\\' if under windoze
	char * name = strdup ( fileName );

#ifdef	_WIN32
	char * ptr;

	while ( ( ptr = strchr ( name, '/' ) ) != NULL )
		*ptr = '\\';
#endif

	bits   = NULL;
	length = 0;
	pos    = 0;
	file   = name;

	int	fd = open ( name, O_RDONLY | O_BINARY );

	free ( name );

	if ( fd == -1 )
		return;

#ifndef _WIN32
	struct	stat statBuf;
	
	fstat ( fd, &statBuf );
	
	long	len = statBuf.st_size; 
#else	
	long	len = filelength ( fd );
#endif

	if ( len == -1 )
	{
		close ( fd );

		return;
	}

	bits = (byte *) malloc ( len );

	if ( bits == NULL )
	{
		close ( fd );

		return;
	}

	length = read ( fd, bits, len );

	close ( fd );
}

bool	Data :: isOk () const
{
	return bits != NULL;
}

void * Data :: getPtr ( int offs ) const
{
	if ( offs < 0 || offs >= length )
		return NULL;

	return bits + offs;
}

int	Data :: getBytes ( void * ptr, int len )
{
	if ( pos >= length )
		return -1;

	if ( pos + len > length )
		len = length - pos;

	memcpy ( ptr, bits + pos, len );

	pos += len;

	return len;
}

bool	Data ::  getString ( string& str, char term )
{
	if ( pos >= length )
		return false;

	str = "";

	while ( pos < length && bits [pos] != static_cast<byte>(term) )
		str += static_cast<char>(bits [pos++]);

	if ( pos < length && bits [pos] == static_cast<byte>(term) )
		pos ++;
													// skin OA part of line terminator (0D,0A)
	if ( term == '\r' && pos + 1 < length && static_cast<char>(bits [pos+1]) == '\n' )
		pos++;

	return true;
}

bool	Data :: saveToFile ( const char * name ) const
{
	int	fd = open ( name, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IWRITE );

	if ( fd == -1 )
		return false;

	const int bytesWritten = write ( fd, bits, length );
	close ( fd );

	return bytesWritten == length;

}
