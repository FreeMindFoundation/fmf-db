#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <pthread.h>

#define CAPACITY	(1<<16)

#ifdef __i386__
typedef unsigned int ptr;
#else
typedef unsigned long long ptr;
#endif

struct llhashitem_s {
	int nkey;
	char *key;
	int nvalue;
	void *value;
	pthread_mutex_t lock;
	struct llhashitem_s *next;
};

struct llused_s {
	volatile struct llused_s *next;
	int i;
};

static struct llhashitem_s **ht;
volatile static struct llused_s *htused;
pthread_mutex_t *htlock;

#define cmpxchg( ptr, _old, _new, fail_label ) { \
 	volatile unsigned int *__ptr = (volatile unsigned int *)(ptr);   \
   	asm goto( "lock; cmpxchg %1,%0 \t\n"           \
   	"jnz %l[" #fail_label "] \t\n"               \
   	: : "m" (*__ptr), "r" (_new), "a" (_old)       \
   	: "memory", "cc"                             \
   	: fail_label );                              \
}

static inline void LL_ADD( void **list, void *node ) {
        volatile void *oldHead;
again:
   	oldHead = *(void **)list;
   	*((ptr *)node) = (ptr)oldHead;
   	cmpxchg( list, oldHead, node, again );
}

int ht_index( char *key, int nkey ) {
	int index = 0;	
	char *tmp = key;

	while( nkey-- != 0 ) {
		index += *tmp++;
	}

	return index;
}	

int ht_set( char *key, const int nkey, const void *value, const int nvalue ) {
	int i;
	struct llhashitem_s **hi;
	struct llused_s *used;

	if( nkey < 1 || nvalue < 1 ) {
		return -1;
	}

	i = ht_index( key, nkey );
	i = i % ( CAPACITY - 1 );

	if( ht[ i ] != NULL ) {
		for( hi = &(ht[ i ]->next); *hi != NULL; hi = &(*hi)->next );
	} else {
		hi = &ht[ i ];
		used = malloc( sizeof( *used ) );
		used->i = i;
		LL_ADD( (void *)&htused, used );
	}

	*hi = malloc( sizeof( **hi ) );
	(*hi)->key = malloc( nkey );
	memcpy( (*hi)->key, key, nkey );
	(*hi)->nkey = nkey;
	(*hi)->value = malloc( nvalue );
	memcpy( (*hi)->value, value, nvalue );
	(*hi)->nvalue = nvalue;
	(*hi)->next = NULL;

	return 0;
}

int ht_free() {
	struct llhashitem_s **hi, **del;
	struct llused_s *hu, *du;

	for( hu = htused; hu != NULL; ) {
		for( hi = &ht[ hu->i ]; *hi != NULL; ) {
			free( (*hi)->key );
			free( (*hi)->value );
			del = hi;
			hi = &(*hi)->next;
			free( *del );
		}
		du = hu;
		hu = hu->next;
		free( du );
	}

	free( ht );
	
	return 0;
}

struct llhashitem_s *ht_get( char *key, const int nkey ) {
	int i;
	struct llhashitem_s *hi;

	i = ht_index( key, nkey );
	i = i % ( CAPACITY - 1 );
	
	hi = ht[i];
	if( hi->next != NULL ) {
		for( ; hi != NULL; hi = hi->next ) {
			if( memcmp( key, hi->key, hi->nkey ) == 0 ) {
				return hi;
			}
		}
		printf("unknown error\n");
		return NULL;
	}

	return ht[ i ];
}

void ht_load() {
	FILE *fp;
	int size;
	char buffer[ 8192 ];
	char *tmp;

	int hsize;
	int nkey;
	char *key;
	char *value;
	int nvalue;

	fp = fopen( "./asdf", "r" );
	if( fp == NULL ) {
		printf("fp null\n");
	}

	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	rewind( fp );
	//buffer = malloc( size );
	fread( buffer, 1, sizeof(buffer), fp );
	fclose( fp );

	tmp = buffer;

	while( size > 0 ) {
		hsize = *(int *)tmp;
		nkey = *(int *)(tmp + sizeof(int));
		key = tmp + 2*sizeof(int);
		nvalue = hsize - nkey - 2*sizeof(int);
		value = tmp + 2*sizeof(int) + nkey;

		printf("[%.*s %.*s]\n", nkey, key, nvalue, value );
		
		tmp += hsize;
		size -= hsize;
	}
}

void ht_save() {
	struct llhashitem_s **hi;
	struct llused_s *hu;
	int size;
	//char buffer[ 4 ] = { 1, 2, 3, 3 };
	char tmp[ 8192 ];

	FILE *fp;
	fp = fopen( "./asdf", "w" );
	if( fp == NULL ) {
		printf("fp null\n");
	}
	fseek( fp, 0, SEEK_SET );
	//fwrite( buffer, 1, sizeof( buffer ), fp );
	//fwrite( buffer, 1, sizeof( buffer ), fp );
	for( hu = htused; hu != NULL; hu = hu->next ) {
		for( hi = &ht[ hu->i ]; *hi != NULL; hi = &(*hi)->next ) {
			size = sizeof(size) + sizeof((*hi)->nkey) + (*hi)->nkey + (*hi)->nvalue;
			if( size <= sizeof( tmp ) ) {
				memcpy( tmp, &size, sizeof( size ) );
				memcpy( tmp + sizeof( size ), &(*hi)->nkey, sizeof( (*hi)->nkey ) );
				memcpy( tmp + sizeof( size ) * 2, (*hi)->key, (*hi)->nkey );
				memcpy( tmp + sizeof( size ) * 2 + (*hi)->nkey, (*hi)->value, (*hi)->nvalue );
				fwrite( tmp, 1, size, fp );
			}
		}
	}

	fclose( fp );
}

void ht_list() {
	struct llhashitem_s **hi;
	struct llused_s *hu;

	for( hu = htused; hu != NULL; hu = hu->next ) {
		for( hi = &ht[ hu->i ]; *hi != NULL; hi = &(*hi)->next ) {
			printf("key: [%.*s] value: [%.*s]\n", (*hi)->nkey, (*hi)->key, (*hi)->nvalue, (*hi)->value );
		}
	}
}

int main() {
	struct llhashitem_s *hi;
	struct llused_s *t, *d;

	// startup
	//memset( ht, 0, sizeof( ht ) );
	ht = malloc( sizeof(struct llhashitem_s) * CAPACITY );
	memset( ht, 0, sizeof(void *) * CAPACITY );
	htused = malloc( sizeof( struct llused_s ) );
	memset( &htused, 0, sizeof( struct llused_s ) );

	ht_set( "asdf", 4, "werwer0", 7 );
	ht_set( "adsf", 4, "werwer1", 7 );
	ht_set( "afds", 4, "werwer2", 7 );
	ht_set( "fdsa", 4, "werwer3", 7 );
	
	ht_set( "123", 3, "werwer4", 7 );
	ht_set( "234", 3, "werwer5", 7 );
	ht_set( "534", 3, "werwer6", 7 );
	//hi = ht_get( "fdsa", 4 );

	//ht_list();
	ht_save();

	ht_load();
/*
	for( ; htused != NULL; htused = htused->next ) {
		printf( "%d\n", htused->i );
	}
*/
	//printf( "index [%.*s]\n", hi->nvalue, (char *)hi->value );

	ht_free();

	return 0;
}
