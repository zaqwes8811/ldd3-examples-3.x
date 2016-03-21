
#include "user_kern.h"

#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

using namespace std;

#define ERR(...) fprintf( stderr, "\7" __VA_ARGS__ ), exit( EXIT_FAILURE )

#define LEN_MSG 256

int main( int argc, char *argv[] )
{
	char name[] = "/dev/stream_dvr0";
	int i;
	int dfd = -1;
	if( ( dfd = open( name, O_RDWR ) ) < 0 ){
		ERR( "open device error: %m\n" );
	}

	int cur_flg = fcntl( dfd, F_GETFL );
	if( -1 == fcntl( dfd, F_SETFL, cur_flg | O_NONBLOCK ) ){
		ERR( "fcntl device error: %m\n" );
	}

	struct pollfd fds[ 1 ];
	fds[0].fd = dfd;
	fds[0].events = POLLOUT | POLLWRNORM// | POLLIN | POLLRDNORM,
		;

	vector<img_hndl_t> hndls;
	for ( int i = 0; i < 10; i++){
		auto h = img_hndl_t();
		h.id = i;
		hndls.emplace_back( h );
	}

	int ptr = 0;
	while (true) {
		if( ptr == hndls.size() ){
			break;
		}

		int status = poll( fds, 1, 500 );
		// fixme: check status

		auto count = sizeof( struct img_hndl_t );
		auto res = write( dfd, (void*)&hndls[ ptr ],  count);
		if( res < 0 ){
		   continue;
		}
		else if( 0 == res ) {
		  if( errno == EAGAIN ){
			 continue;
		  }
		}
		else if( count == res ){
			++ptr;
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
