#include <stdio.h>
#include <pthread.h>

#include "../../libal/include/libal.h"
#include "../include/filedb.h"

int main() 
{
	if( db_createUser( "mark1", "modano2" ) != 0 )
		printf( "createUser() failed.\n" );

	if( db_verifyUserHash( "mark1", "modano2" ) == 0 ) {
		printf( "ok u/pass\n" );
	} else 	
		printf( "fail u/pass\n" );

	printf( "final amount: %d\n", albytes() );
		
	return 0;
}
