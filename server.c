#include "server.h"

int main (int argc, char** argv){
	/*whaaaaaaat in the world*/
	/*ok let's focus*/

	int sd;
	pthread_attr_t kernel;
	struct sockaddr_in c_address;
	pid_t child_pid;

}

int claim_port( const char * port )
{
    struct addrinfo addrinfo;
    struct addrinfo *	result;
    int	sd;
    char message[256];
    int on = 1;

    addrinfo.ai_flags = AI_PASSIVE;		// for bind()
    addrinfo.ai_family = AF_INET;		// IPv4 only
    addrinfo.ai_socktype = SOCK_STREAM;	// Want TCP/IP
    addrinfo.ai_protocol = 0;		// Any protocol
    addrinfo.ai_addrlen = 0;
    addrinfo.ai_addr = NULL;
    addrinfo.ai_canonname = NULL;
    addrinfo.ai_next = NULL;

    if ( getaddrinfo( 0, port, &addrinfo, &result ) != 0 )
    {
        fprintf(stderr, "ERROR: getaddrinfo(%s) failed errno is %s.  File %s line %d.", port, strerror( errno ), __FILE__, __LINE__ );
        return -1;
    }
    else if ( errno = 0, (sd = socket( result->ai_family, result->ai_socktype, result->ai_protocol )) == -1 )
    {
        write( 1, message, sprintf( message, "ERROR: socket() failed.  File %s line %d.\n", __FILE__, __LINE__ ) );
        freeaddrinfo( result );
        return -1;
    }
    else if ( setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) == -1 )
    {
        write( 1, message, sprintf( message, "ERROR: setsockopt() failed.  File %s line %d.\n", __FILE__, __LINE__ ) );
        freeaddrinfo( result );
        close( sd );
        return -1;
    }
    else if ( bind( sd, result->ai_addr, result->ai_addrlen ) == -1 )
    {
        freeaddrinfo( result );
        close( sd );
        write( 1, message, sprintf( message, "Binding to port %s.\n", port ) );
        return -1;
    }
    else
    {
        write( 1, message, sprintf( message,  "SUCCESS : Bind to port %s.\n", port ) );
        freeaddrinfo( result );		
        return sd;			// bind() succeeded;
    }
}