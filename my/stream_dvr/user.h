
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

#define ERR(...) fprintf( stderr, "\7" __VA_ARGS__ ), exit( EXIT_FAILURE )

struct parm {
   int blk, vis, mlt;
};

struct parm parms( int argc, char *argv[], int par ) {
   int c;
   struct parm p = { 0, 0, 0 };
   while( ( c = getopt( argc, argv, "bvm" ) ) != EOF )
      switch( c ) {
         case 'b': p.blk = 1; break;
         case 'm': p.mlt = 1; break;
         case 'v': p.vis++; break;
         default: goto err;
   }
   // par > 0 - pecho; par < 0 - pcat
   if( ( par != 0 && ( argc - optind ) != abs( par ) )) goto err;
   if( par < 0 && atoi( argv[ optind ] ) <= 0 ) goto err;
   return p;
err:
   ERR( "usage: %s [-b][-m][-v] %s\n", argv[ 0 ], par < 0 ? 
        "<read block size>" : "<write string>" );
}


const char *interval( struct timeval b, struct timeval a ) {
   static char res[ 40 ];
   long msec = ( a.tv_sec - b.tv_sec ) * 1000 + ( a.tv_usec - b.tv_usec ) / 1000;
   if( ( a.tv_usec - b.tv_usec ) % 1000 >= 500 ) msec++;
   sprintf( res, "%02d:%03d", msec / 1000, msec % 1000 );
   return res;
};
