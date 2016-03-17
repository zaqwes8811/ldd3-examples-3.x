
#include "user.h"

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

	struct pollfd client[ 1 ] = {
			{
			  .fd = dfd,
			  .events = POLLOUT | POLLWRNORM | POLLIN | POLLRDNORM,
			}
	};
	struct timeval t1, t2;
	gettimeofday( &t1, NULL );

	for( i = 0; i < 2; ++i ){
		// write
		{
			int res = poll( client, 1, -1 );
			const char *sout = "haloooooooo";
			res = write( dfd, sout, strlen( sout ) ); // запись
			gettimeofday( &t2, NULL );
			fprintf( stdout, "interval %s write %d bytes: ",
				   interval( t1, t2 ), res );
			if( res < 0 ){
			   ERR( "write error: %m\n" );
			}
			else if( 0 == res ) {
			  if( errno == EAGAIN ){
				 fprintf( stdout, "device NOT READY!\n" );
			  }
			}
			else{
			   fprintf( stdout, "%s\n", sout );
			}
		}

		// read
		{
			int blk = 1;//LEN_MSG;
			while( 1 ){
				char buf[ LEN_MSG + 2 ];
				struct timeval t1, t2;

				gettimeofday( &t1, NULL );
				int res = poll( client, 1, -1 );

				res = read( dfd, buf, blk );
				gettimeofday( &t2, NULL );

				fprintf( stdout, "interval %s read %d bytes: ", interval( t1, t2 ), res );
				fflush( stdout );

				if( res < 0 ) {
					if( errno == EAGAIN ) {
						fprintf( stdout, "device NOT READY\n" );
					}
					else{
						ERR( "read error: %m\n" );
					}
				}
				else if( 0 == res ) {
					fprintf( stdout, "read EOF\n" );
					break;
				}
				else {
					buf[ res ] = '\0';
					fprintf( stdout, "%s\n", buf );
				}
			}
		}
	}

	// out
	close( dfd );
	return EXIT_SUCCESS;
};
