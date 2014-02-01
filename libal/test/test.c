#include <stdio.h>
#include <pthread.h>

#include "../include/libal.h"

#define		THREADS		128


typedef struct Test_t 
{
	int x;
	int y;
} Test_t;

void *alloc( void *z )
{
	for( int i = 0; i < 1024; i++ ) {
		Test_t *t;
		t = almalloc( sizeof( Test_t ) );
		alfree( t );
	}

	return NULL;
}	

int main()
{
	pthread_t tid[ THREADS ];

	for( int i = 0; i < THREADS ; i++ ) {
		pthread_create( &(tid[ i ]), NULL, &alloc, NULL  );
	}

	for( int i = 0; i < THREADS ; i++ ) {	
        	pthread_join( tid[ i ], NULL );
	}

	printf( "final amount: %d\n", albytes() );
	return 0;
}
