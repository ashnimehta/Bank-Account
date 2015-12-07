#include "utils.h"

#define PORT "36963" /*It's a palindrome and all odd numbers so I thought it was cute*/

// A couple of functions for converting IP addresses to and from dotted decimal notation.
/*taken from BKR*/
/*Don't know if these are helpful yet but I included them below*/

/*
char *
get_istring( unsigned long x, char * s, unsigned int len )
{
	int		a,b,c,d;

	d = x & 0x000000ff;
	c = (x >> 8) & 0x000000ff;
	b = (x >> 16) & 0x000000ff;
	a = (x >> 24) & 0x000000ff;
	snprintf( s, len, "%d.%d.%d.%d", a,b,c,d );
	s[len-1] = '\0';
	return s;
}

long
get_iaddr_string( char * string )
{
	int		a,b,c,d;

	if ( string == 0 )
	{
		return 0;
	}
	else if ( sscanf( string, "%d.%d.%d.%d", &a, &b, &c, &d ) < 4 )
	{
		return 0;
	}
	else
	{
		return (a<<24) | (b<<16) | (c<<8) | d;
	}
}

*/

void* c_input (void* in){
	int sd = *(int*) in;
  String arg1 = malloc(6);
  String arg2 = malloc(108);
/*take client input and figure out what to do with it*/
	/*step 1) prompt for user input*/
	/*step 2) figure out what to do with it - is it a server process
	or a client process*/
	/*step 3) uh... */

	char c_cmd[200];
	int inputsize;
	char* ourOutput = "Please enter a command.";
  inputsize=0;
  memset (c_cmd, 0, 200);
	while( printf("%s\n",ourOutput) && (inputsize = read(0, c_cmd, 200) > 0))
	{
    sscanf(c_cmd,"%s %s",arg1,arg2);
		/*make sure client doesn't want to exit*/
		
			printf("Sending your request to the server...\n");
			write(sd, c_cmd, strlen(c_cmd)+1);
			/*how do I send the request to the server*/
      		

		/*does the client want to exit*/
		
		if(strcmp(arg1,"exit")==0){
			printf("Goodbye. Your session is terminating.\n");
      free(arg1);
      free(arg2);
			return 0;
		}
    sleep(2); /*throttle by 2*/
    memset (c_cmd, 0, 200);
	}
  free(arg1);
  free(arg2);
	return 0;
}


/*This needs to be modified somehow*/
void* s_output(void* out){

	int sd = *(int*)out;
	char s_out [1000];
  memset (s_out, 0, 1000);
	pthread_detach(pthread_self());

	while(read(sd, s_out, sizeof(s_out)) > 0){
    if(strcmp(s_out,"disconnect")==0){
      printf("Server has been disconnected.\n");
       exit(0);
    }
		printf("Receiving server output...%s\n",s_out);
    memset (s_out, 0, 1000);
	}
	return 0;
}

int serverconnect(char* server, char* port){
	
	struct addrinfo ai;
	struct addrinfo* res;

	/*initialize the addrinfo struct*/

	ai.ai_flags = 0;
	ai.ai_family = AF_INET;
	ai.ai_socktype = SOCK_STREAM;
	ai.ai_protocol = 0;
    ai.ai_addrlen = 0;
    ai.ai_addr = NULL;
    ai.ai_canonname = NULL;
    ai.ai_next = NULL;

    int sd; /*socket descriptor*/

    int ai_flag = getaddrinfo(server, port, &ai, &res);

    sd=0;
    /*Taken from netdb.h
    But found first on a stackoverflow question */

    if(ai_flag != 0){
    	if(ai_flag == -1)
    	{
    		printf("Invalid value for ai_flags field.");
    	}
    	else if (ai_flag == -2)
    	{
    		printf("NAME or SERVICE is unknown.");
    	}
    	else if (ai_flag == -3)
    	{
    		printf("Temporary failure in name resolution.");
    	}
    	else if (ai_flag == -4)
    	{
    		printf("Non-recoverable failure in name-resolution.");
    	}
    	else if (ai_flag == -6)
    	{
    		printf("ai_family not supported.");
    	}
    	else if (ai_flag == -7)
    	{
    		printf("ai_socktype not supported.");
    	}
    	else if (ai_flag == -8)
    	{
    		printf("SERVICE not supported for ai_socktype.");
    	}
    	else if (ai_flag == -10)
    	{
    		printf("Memory allocation failure.");
    	}
    	else if (ai_flag == -11)
    	{
    		printf("System error returned in errno.");
    	}
    	else if (ai_flag == -12)
    	{
    		printf("Argument buffer overflow");
    	}
    	else{
    		printf("Unresolvable error with getaddrinfo.");
    	}
    	return -1;
    }

    else if ((sd = socket( res->ai_family, res->ai_socktype, res->ai_protocol )) == -1 )
    {
        freeaddrinfo( res );
        return -1;
    }

    /*now we know that getaddrinfo worked*/
    do{
    	if(connect(sd, res->ai_addr, res -> ai_addrlen) == -1)
    	{
    		sleep(3);
    		printf("Attempting to connect to server\n");
    	}
    	else{
    		freeaddrinfo(res);
    		return sd; /*success*/
    	}

    } while (errno == ECONNREFUSED);
    
    freeaddrinfo(res);
    return -1; /*failure*/
}

void spawn_threads(int sd){
   /*use two threads and join on input, so we can keep accepting input*/
   pthread_t cinput;
   pthread_t coutput;
   pthread_attr_t kernel;
   int* sdptr;

   /*taken from BKR client code on site*/
   if(pthread_attr_init(&kernel) != 0)
   {
      printf("ERROR: pthread_attr_init failed.");
      exit(1);
   }
   
   /*check to see if the thread gets resources from the same place as
   all other threads in the scheduling allocaiton domain*/

   else if(pthread_attr_setscope(&kernel, PTHREAD_SCOPE_SYSTEM)!= 0)
   {
      printf ("ERROR: pthread_attr_setscope failed.");
      exit(1);
   }
   else
   {
      sdptr = (int*) malloc(sizeof(sd));
      *sdptr = sd;

      if(pthread_create(&cinput, &kernel, c_input, sdptr) != 0)
      {
         printf("pthread_create() failed.");
         exit(1);
      }
      if(pthread_create(&coutput, &kernel, s_output, sdptr) != 0)
      {
         printf("pthread_create() failed.");
         exit(1);
      }

      /*join the two threads*/

      if(pthread_join(cinput, NULL) != 0){
         printf("pthread_join() failed.");
         exit(1);
      }

      free(sdptr);
      return;
   }
   return;
}


int main(int argc, char** argv){
	int sd;
   sd=0;
   if(argc != 2)
	{
		printf("Please enter the name of the machine running the server process as a command line argument.\n");
		exit(1);
	}
	/*try to connect to the server every three seconds*/
	while((sd = serverconnect(argv[1], "36963")) == -1)
	{
		sleep(3);
	}

	/*Section 8, requirement 1 - announce completion of connection to server*/
	printf("Success! Client connected to server.\n");

	/*spawn threads*/
	/*lmao now what*/
	spawn_threads(sd);
	
	close(sd);

	/*Disconnect from server*/
	printf("Client has disconnected from the server.\n");
	
	return 0; /*success*/
}

