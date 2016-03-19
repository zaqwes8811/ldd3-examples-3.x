
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

#define LEN_MSG 256

int main( int argc, char *argv[] )
{
	char name[] = "/dev/stream_dvr0";
	int i;
	int dfd = -1;
	if( ( dfd = open( name, O_RDWR ) ) < 0 ){
		ERR( "open device error: %m\n" );
	}

	// операции в режиме O_NONBLOCK
	int cur_flg = fcntl( dfd, F_GETFL );
	if( -1 == fcntl( dfd, F_SETFL, cur_flg | O_NONBLOCK ) ){
		ERR( "fcntl device error: %m\n" );
	}

	struct pollfd fds[ 1 ] = {
			{
			  .fd = dfd,
			  .events = POLLOUT | POLLWRNORM,// | POLLIN | POLLRDNORM,
			}
	};

	const char *data = "haloooooooo";
	for( i = 0; i < 2; ++i ){
		int stat = poll( fds, 1, 500 );
		fprintf( stdout, "stat:%d\n", stat );
		if( stat > 0 ){
			// приозошло событие на одном или более дескрипторов
		}

//		int res = write( dfd, data, strlen( data ) );
		continue;

		int res = 0;
		fprintf( stdout, "write %d bytes: ", res );

		if( res < 0 ){
		   ERR( "write error: %m\n" );
		}
		else if( 0 == res ) {
		  if( errno == EAGAIN ){
			 fprintf( stdout, "device NOT READY!\n" );
		  }
		}
		else{
		   fprintf( stdout, "%s\n", data );
		}
	}

//	if( 0 ){
//			int blk = 1;//LEN_MSG;
//			while( 1 ){
//				char buf[ LEN_MSG + 2 ];
//				struct timeval t1, t2;
//
//				gettimeofday( &t1, NULL );
//				int res = poll( client, 1, -1 );
//
//				res = read( dfd, buf, blk );
//				gettimeofday( &t2, NULL );
//
//				fprintf( stdout, "interval %s read %d bytes: ", interval( t1, t2 ), res );
//				fflush( stdout );
//
//				if( res < 0 ) {
//					if( errno == EAGAIN ) {
//						fprintf( stdout, "device NOT READY\n" );
//					}
//					else{
//						ERR( "read error: %m\n" );
//					}
//				}
//				else if( 0 == res ) {
//					fprintf( stdout, "read EOF\n" );
//					break;
//				}
//				else {
//					buf[ res ] = '\0';
//					fprintf( stdout, "%s\n", buf );
//				}
//			}
//		}
//	}

	// out
	close( dfd );
	return EXIT_SUCCESS;
}
