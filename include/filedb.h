#ifndef FILEDB_H_
#define	FILEDB_H_

#define 	SHA256_LEN		64
#define		USER_MAXLEN		64

#define		DB_CREATE		0
#define		DB_ERR_CREATE_LENGTH	1
#define		DB_ERR_CREATE_EXISTS	2

typedef struct User_s
{
	int userId;		
	char username[ USER_MAXLEN ];
	char userpass[ SHA256_LEN ];
	unsigned int flags;
} User_t;

char 		db_createUser( const char *username, const char *password );
int 		db_getUserFlag( const char *username, const int flag );
void 		db_setUserFlag( const char *username, const int value );
char 		db_verifyUser( const char *username, unsigned char userpass[] );
void		db_hashPass( const char *pass, unsigned char *buffer );
char 		db_verifyUserHash( const char *username, const char *userpass );

#endif
