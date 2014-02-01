#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <gcrypt.h>

#include "../../libal/include/libal.h"
#include "../include/filedb.h"

int main() 
{
	if( createUser( "mark1", "modano2" ) != 0 )
		printf( "createUser() failed.\n" );

	/*	
	setUserFlag( "mrkva", USER_ISACTIVE, 0 );
	setUserFlag( "mrkva", USER_ISACTIVE, !USER_ISACTIVE );
	if( getUserFlag( "mrkva", USER_ISPLAYING ) == USER_ISPLAYING )
		printf( "playing\n" );
	setEloByUser( "mrkva", 1337 );
	getEloByUser( "mrkva" );
	*/	
	if( verifyUser( "mark1", "modano2" ) == 0 )
		printf( "ok u/pass\n" );
	
	printf( "final amount: %d\n", albytes() );
		
	return 0;
}
