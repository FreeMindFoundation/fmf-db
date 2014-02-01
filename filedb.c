#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

#include "../libal/include/libal.h"
#include "./include/filedb.h"
#include "./include/sha256.h"

long lSize;
pthread_mutex_t filemutex = PTHREAD_MUTEX_INITIALIZER;

inline char *readFile()
{
        FILE *file;
        char *buffer;
	size_t read;

	pthread_mutex_lock( &filemutex );
	file = fopen( FILE_USERS, "r" );
	if( file == NULL )
		printf("File error");
	fseek( file, 0, SEEK_END );
	lSize = ftell( file );
	rewind( file );
	buffer = (char*) almalloc( sizeof(char) * lSize );
	if( buffer == NULL ) {
		printf("Memory error");
		pthread_mutex_unlock( &filemutex );
		return NULL;
	}
	read = fread( buffer, 1, lSize, file );
	if( lSize != read )
		printf("read error");
	fclose( file );
	pthread_mutex_unlock( &filemutex );
	return buffer;
}

inline void updateValue( const int *pos, char *value, const int len )
{
 	FILE *pFile;
	char buffer[ len ];
	memcpy( buffer, value, len );
	pthread_mutex_lock( &filemutex );
	pFile = fopen( FILE_USERS, "rb+" );
	fseek( pFile, *pos, SEEK_SET );
	fwrite( buffer, 1, sizeof( buffer),  pFile );
  	fclose( pFile );
	pthread_mutex_unlock( &filemutex );
}

inline void addUser( User_t *u ) 
{
  	FILE *pFile;
	if( u == NULL )
		return;
	pthread_mutex_lock( &filemutex );
  	pFile = fopen ( FILE_USERS , "a" );
  	fwrite ( (char *)u , sizeof( User_t ), 1, pFile );
  	fclose (pFile);
	pthread_mutex_unlock( &filemutex );
}

inline char userExists( const char *username ) 
{
	char *buffer = NULL;
	char exists = 0;
	buffer = readFile();

	if( buffer == NULL )
		return exists;	

	User_t *u;
	u = (User_t *)buffer;
	for( int i = 0; i < lSize; i += sizeof( User_t ) ) {
		if( strcmp( u->username + i, username ) == 0 ) {
			exists = 1;
		}
	}

	alfree( buffer );
	return exists;	
}

inline int32_t getNextID()
{
	char *buffer;
	buffer = readFile();
	int userId = 0;	
	User_t *u;

	if( buffer == NULL )
		return userId;
	
	u = (User_t *)( buffer + lSize - sizeof( User_t ) );
	userId = u->userId + 1;
	alfree( buffer );
	return userId;
}

inline char createUser( const char *username, const char *password )
{
	if( strlen( username ) > 32 || strlen( password ) > 32 )
		return DB_ERR_CREATE_LENGTH;
	
	if( userExists( username ) )
		return DB_ERR_CREATE_EXISTS;

   	unsigned char hashpass[ 32 ];
	SHA256_CTX ctx;
	int32_t userId = 0;	
	User_t *user;

	sha256_init( &ctx );
   	sha256_update( &ctx, (unsigned char *)password, strlen( password ) );
   	sha256_final( &ctx, hashpass );
	userId = getNextID();
	user = almalloc( sizeof( User_t ) );
	memset( user, 0, sizeof( User_t ) );
	user->userId = userId;
	memcpy( user->username, username, strlen( username ) );
	memcpy( user->userpass, hashpass, sizeof( hashpass ) );
	user->elorating = DEFAULT_ELO;
	user->isActive = USER_ISACTIVE;
	addUser( user );
	alfree( user );
		
	return DB_CREATE;
}

inline int32_t getPosByUser( const char *username, char *buffer )
{
	User_t *u;
	u = (User_t *)buffer;

	for( int i = 0; i < lSize; i += sizeof( User_t ) ) {
		if( strcmp( u->username + i, username ) == 0 ) {
			return i;
		}
	}
	
	return -1;
}

inline void setEloByUser( const char *username, const int elo )
{
	if( strlen( username ) < 1 )
		return;

	int pos;
	User_t u;
	char *buffer;
	buffer = readFile();

	pos = getPosByUser( username, buffer );	
	pos += (int )&u.elorating - (int )&u;	
	updateValue( &pos, (char *)&elo, sizeof( elo ) );
	alfree( buffer );
}

inline int32_t getEloByUser( const char *username )
{
	if( strlen( username ) < 1 )
		return 0;

	int pos;
	User_t *u;
	char *buffer;
	int32_t elo = 0;

	buffer = readFile();
	pos = getPosByUser( username, buffer );	
	if( pos == -1 ) {
		printf("user not found\n");
		return -1;
	}
	u = (User_t *)( buffer + pos );
	elo = u->elorating;
	alfree( buffer );
	return elo;
}

inline int32_t getUserFlag( const char *username, const int flag )
{
	if( strlen( username ) < 1 )
		return 0;

	int pos;
	User_t *u;
	char *buffer;
	int32_t rFlag = -1;

	buffer = readFile();
	pos = getPosByUser( username, buffer );	
	if( pos == -1 ) {
		printf("user not found\n");
		return rFlag;
	}
	u = (User_t *)( buffer + pos );
	if( flag == USER_ISACTIVE )
		rFlag = u->isActive;
	else if( flag == USER_ISPLAYING )
		rFlag = u->isPlaying;
	alfree( buffer );
	return rFlag;
}

inline void setUserFlag( const char *username, const int flag, const int value )
{
	if( strlen( username ) < 1 )
		return;

	int pos;
	User_t u;
	char *buffer;

	buffer = readFile();
	pos = getPosByUser( username, buffer );	
	if( flag == USER_ISACTIVE )
		pos += (int )&u.isActive - (int )&u;	
	else if( flag == USER_ISPLAYING )
		pos += (int )&u.isPlaying - (int )&u;	
	updateValue( &pos, (char *)&value, sizeof( int32_t ) );

	alfree( buffer );
}

void print_hash( const unsigned char* c )
{
    printf( "The hash is: " );

    int index;
    for(index = 0; index < 32; index++)
        printf( "%X", *c++ );

    printf( "\n" );
}

inline char verifyUser( const char *username, unsigned char userpass[] )
{
	if( strlen( username ) < 1 )
		return -1;

	print_hash( userpass );

	int pos;
	User_t *u;
	char *buffer;
	char r = -1;	
	
	buffer = readFile();
	if( buffer == NULL )
		return r;
	
	pos = getPosByUser( username, buffer );	
	u = (User_t *)( buffer + pos );
	
	if( memcmp( u->userpass, userpass, sizeof( userpass ) ) == 0 )
		r = 0;
	
	alfree( buffer );
	
	return r;
}
