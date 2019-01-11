#define SHOWSTRING(HEAD) 							\
	do {									\
	    void ShowString( char *tstring, int length, void*, int ) ;		\
	    ShowString( tstring, length, HEAD, wid ) ;				\
	} while( 0 ) ;
