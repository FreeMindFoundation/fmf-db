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

static struct llhashitem_s ht[ CAPACITY ];
volatile static struct llused_s *htused;

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

	if( *(int *)&ht[ i ] != 0 ) {
		for( hi = &(ht[ i ].next); *hi != NULL; hi = &(*hi)->next );
		*hi = malloc( sizeof( **hi ) );
		(*hi)->key = malloc( nkey );
		memcpy( (*hi)->key, key, nkey );
		(*hi)->nkey = nkey;
		(*hi)->value = malloc( nvalue );
		memcpy( (*hi)->value, value, nvalue );
		(*hi)->nvalue = nvalue;
		(*hi)->next = NULL;
	
		goto done;
	}

	// ll for a faster access 
	used = malloc( sizeof( *used ) );
	used->i = i;
	//LL_ADD( (void *)&htused, used );
    used->next = htused;
    htused = used;

	ht[ i ].key = malloc( nkey );
	memcpy( ht[ i ].key, key, nkey );
	ht[ i ].nkey = nkey;
	ht[ i ].value = malloc( nvalue );
	memcpy( ht[ i ].value, value, nvalue );
	ht[ i ].nvalue = nvalue;
	ht[ i ].next = NULL;

done:
	return 0;
}

int ht_free() {
	struct llhashitem_s *hi, *del;
	struct llused_s *hu;

	for( hu = htused; hu != NULL; hu = hu->next ) {
		printf( "key %.*s\n", ( ht[ hu->i ].nkey ), ht[ hu->i ].key );
		for( hi = &ht[ hu->i ]; hi != NULL; ) {
			printf( "LINKED: key:[%.*s] value:[%.*s]\n", hi->nkey, hi->key, hi->nvalue, hi->value );
			free( ht[ hu->i ].key );
			free( ht[ hu->i ].value );
			del = hi;
			hi = hi->next;
			if( del != &ht[ hu->i ] ) {
				free( del );
			}
		}
	}
	
	return 0;
}

struct llhashitem_s *ht_get( char *key, const int nkey ) {
	int i;
	struct llhashitem_s *hi;

	i = ht_index( key, nkey );
	i = i % ( CAPACITY - 1 );
	
	hi = &ht[i];
	if( hi->next != NULL ) {
		for( ; hi != NULL; hi = hi->next ) {
			if( memcmp( key, hi->key, hi->nkey ) == 0 ) {
				return hi;
			}
		}
		printf("unknown error\n");
		return NULL;
	}

	return &ht[ i ];
}

int main() {
	struct llhashitem_s *hi;
	// test
	struct llused_s *t;

	// startup
	memset( ht, 0, sizeof( ht ) );
	htused = malloc( sizeof( struct llused_s ) );
	memset( &htused, 0, sizeof( struct llused_s ) );

	ht_set( "asdf", 4, "werwer0", 7 );
	ht_set( "adsf", 4, "werwer1", 7 );
	ht_set( "afds", 4, "werwer2", 7 );
	ht_set( "fdsa", 4, "werwer3", 7 );

	ht_set( "123", 3, "werwer4", 7 );
	ht_set( "234", 3, "werwer5", 7 );
	ht_set( "534", 3, "werwer6", 7 );
	hi = ht_get( "fdsa", 4 );

	printf( "index [%.*s]\n", hi->nvalue, (char *)hi->value );

	ht_free();
/*
	for( ; htused != NULL; htused = htused->next ) {
		printf( "%d\n", htused->i );
	}

	printf( "index [%.*s]\n", hi->nvalue, (char *)hi->value );
*/
	return 0;
}
