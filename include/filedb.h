#ifndef FILE_DB
#define	FILE_DB

#define 	SHA256_LEN		32

#define		FILE_USERS		"./db/users"
#define		USER_ISACTIVE		0x1
#define		USER_ISPLAYING		0x2

#define		DB_CREATE		0
#define		DB_ERR_CREATE_LENGTH	1
#define		DB_ERR_CREATE_EXISTS	2

typedef struct User_s
{
	int userId;		// keep int32_t for padding
	char username[32];
	char userpass[32];
	unsigned int flags;
} User_t;

char 		db_createUser( const char *username, const char *password );
int 		db_getUserFlag( const char *username, const int flag );
void 		db_setUserFlag( const char *username, const int value );
char 		db_verifyUser( const char *username, unsigned char userpass[] );
void		db_hashPass( unsigned char *pass, unsigned char *buffer );
char 		db_verifyUserHash( const char *username, unsigned char *userpass );

#endif
