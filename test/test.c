#include <stdio.h>
#include <pthread.h>
#include <memory.h>

#include "../../libal/include/libal.h"
#include "../include/filedb.h"

int main() 
{
	if( db_createUser( "john", "doe" ) != 0 )
		printf( "createUser() failed.\n" );

	if( db_verifyUserHash( "john", "doe" ) == 0 ) {
		printf( "ok u/pass\n" );
	} else 	
		printf( "fail u/pass\n" );
		
	return 0;
}
