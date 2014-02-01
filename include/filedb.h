#ifndef __FILE_DB
#define	__FILE_DB

#define		FILE_USERS		"./db/users"
#define		USER_ISACTIVE		0x1
#define		USER_ISPLAYING		0x2

#define		DEFAULT_ELO		1300

#define		DB_CREATE		0x0
#define		DB_ERR_CREATE_LENGTH	0x1
#define		DB_ERR_CREATE_EXISTS	0x2

typedef struct User_s
{
	int32_t userId;		// keep int32_t for padding
	char username[32];
	char userpass[32];
	int32_t elorating;	
	char isActive;
	char isPlaying;
	char _flag2;
	char _flag3;
	 
} User_t;

char *readFile();
void updateValue( const int *pos, char *value, const int len );
void addUser( User_t *u );
char userExists( const char *username );
int32_t getNextID();
char createUser( const char *username, const char *password );
int32_t getPosByUser( const char *username, char *buffer );
void setEloByUser( const char *username, const int elo );
int32_t getEloByUser( const char *username );
int32_t getUserFlag( const char *username, const int flag );
void setUserFlag( const char *username, const int flag, const int value );
char verifyUser( const char *username, unsigned char userpass[] );

#endif
