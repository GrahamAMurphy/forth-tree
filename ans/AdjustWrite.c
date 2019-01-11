/* Routines to list current input file */
static int forthdebug = -1 ;
static FILE *outpnt = NULL ;
static int numopen = 0 ;
static int maxopen = 1024 ;
static char *openpnts[1024] ;
static char *lf = "" ;

static int forthxref = 0 ;
static char *forthxfile = NULL ;
static char *forthxbegin = NULL ;

#ifdef fopen
#undef fopen
#endif
#ifdef fclose
#undef fclose
#endif

FILE *xfopen(char *name, char *mode)
{
    FILE *tmp ;
    int length ;

    if( forthdebug < 0 ) {
	char *fthdbg ;
	int i ;

	fthdbg = getenv( "FORTHDEBUG" ) ;
	if( fthdbg == NULL ) {
	    forthdebug = 0 ;
	} else {
	    forthdebug = strtol( fthdbg, NULL, 0 ) ;
	    if( forthdebug & 0x20 ) outpnt = stderr ;
	    else outpnt = stdout ;
	}

	for( i = 0 ; i < maxopen ; i++ ) openpnts[i] = NULL ;
	if( forthdebug & 0x10 ) lf = "\n" ;
	if( forthdebug == 0 ) {
	    fprintf( stderr, "FORTHDEBUG env:\n" ) ;
	    fprintf( stderr, "0x01 - show successful file opens.\n" ) ;
	    fprintf( stderr, "0x02 - show file closures.\n" ) ;
	    fprintf( stderr, "0x04 - show current open file.\n" ) ;
	    fprintf( stderr, "0x08 - show failed file opens.\n" ) ;
	    fprintf( stderr, "0x10 - append linefeed to all diagnostics.\n" ) ;
	    fprintf( stderr, "0x20 - send diagnostics to stderr.\n" ) ;
	}
	if( forthdebug < 0 ) forthdebug = 0 ;

	forthxfile = getenv( "FORTHXFILE" ) ;
	forthxbegin = getenv( "FORTHXBEGIN" ) ;
	if( forthxfile != NULL ) {
	    forthxref = 1 ;
	}
    }

    tmp = fopen(name, mode);
    length = strlen(name) ;

    if( forthdebug > 0 ) {
	char *zname ;

	fflush(outpnt) ; 

	zname = (char*)malloc( length+1 ) ;
	strcpy( zname, name ) ;

	if( tmp != NULL ) {
	    if( openpnts[numopen] != NULL ) {
		free( openpnts[numopen] ) ;
		openpnts[numopen] = NULL ;
	    }
	    openpnts[numopen] = zname ;
	    numopen++ ;
	}


	if( tmp == NULL && (forthdebug & 0x8) ) {
	    fputc( '\n', outpnt ) ;
	    for( int ix = 1 ; ix < numopen ; ix++ )
		fputc( ' ', outpnt ) ;

	    fprintf( outpnt, "Open failed: %s%s", zname, lf ) ;
	} else if( tmp != NULL ) {
	    fputc( '\n', outpnt ) ;
	    for( int ix = 1 ; ix < numopen ; ix++ )
		fputc( ' ', outpnt ) ;
	    fprintf( outpnt, "File Opened: %s%s", zname, lf ) ;
	}
	fflush(outpnt) ; 
    }

    return(tmp) ;
}

int xfclose( FILE *inp )
{
    int status ;

    status = fclose( inp ) ;

    if( forthdebug & 0x2 && numopen > 0 ) {

	fputc( '\n', outpnt ) ;
	for( int ix = 1 ; ix < numopen ; ix++ )
	    fputc( ' ', outpnt ) ;

	numopen-- ;
	fprintf( outpnt, "File Closed: %s%s", openpnts[numopen], lf ) ;
	fflush(outpnt) ; 
    }

    if( forthdebug & 0x4 && numopen > 0 ) {
	fprintf( outpnt, "\nNow in file: %s%s", openpnts[numopen-1], lf ) ;
	fflush(outpnt) ; 
    }
    return( status ) ;
}

void ShowString( char *string, int length, void *point, int wid )
{
    static FILE *ext = NULL ;
    static void *opoint = 0 ;
    static char temp[1024] = "" ;
    int i ;
    static void *pstate = 0L ;
    unsigned short *state ;

    // if( strncmp( "\\", string, length ) == 0 ) return ;

    if( forthxref == 0 ) return ;

    if( opoint == point && strncmp( temp, string, length ) == 0 ) 
	return ;

    memcpy( temp, string, length ) ;
    temp[length] = '\0' ;

    if( ext == NULL ) {
	if( ( forthxbegin == NULL ) || 
	    ( forthxbegin != NULL && strcmp( forthxbegin, temp ) == 0 ) ) {
	    ext = fopen( forthxfile, "w" ) ;
	    if( ext == NULL ) {
		fprintf( stderr, "Could not open FORTHXFILE (%s).\n", 
		    forthxfile ) ;
		forthxref = 0 ;
	    }
	}
    } else {
	char *slashpt ;
	slashpt = strrchr( openpnts[numopen-1], '/' ) ;
	if( slashpt ) slashpt++ ;
	else slashpt = openpnts[numopen-1] ;
	fprintf( ext, "<%s>%08x %04x %s\n", temp, (unsigned long)point,
	    wid,
	    slashpt // openpnts[numopen-1]
	    ) ;
	fflush( ext ) ;
    }
    opoint = point ;
}

#include "AdjustWrite.h"
