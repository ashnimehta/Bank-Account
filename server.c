#include "server.h"

static int index;
static char glob_sd;
static Bank* glob_shm_addr;
static int glob_shm_id;

void alarmhandler(int sig){
    sigset_t alarmset;
    sigemptyset (&alarmset);
    sigaddset (&alarmset, SIGALRM);
    sigprocmask (SIG_BLOCK, &alarmset, NULL); //block more alarm signals from coming in
    printlist(); 
    sigprocmask (SIG_UNBLOCK, &alarmset, NULL);//unblock alarm signals 
    alarm(20);
}

void childhandler(int sig){
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {
        printf("WE BE KILLIN 'EM!\n");
    }
}

void inthandler(int sig){
    // if (shmdt(glob_shmaddr) != 0) { //clear up all shared memory
    //     perror("shmdt");
    //     exit(1);
    // }

    // shmctl(glob_shmid, IPC_RMID, NULL);
    // write(glob_sd, "disconnect", 11); //kill client process

    //DO WE WANT THIS?????????????????????????????????????????????????????????????
    exit(0);
}

void alarmSetup(sigaction* sig){
    sig->sa_handler = alarmhandler;
    sig->sa_flags = SA_RESTART; 
    sigaction(SIGALRM, sig, NULL);
}

void intSetup(sigaction* sig){
    sig->sa_handler = sigint_handler;
    sigemptyset(sig->sa_mask);
    sig->sa_flags = 0;
}

void chldSetup(sigaction* sig){
    sig->sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(sig->sa_mask);
    sig->sa_flags = SA_RESTART;
}

/*Set up shared memory for the Bank*/
void shm_setup()
{
    char message[2048];
    int shm_id;
    key_t key;
    
    key = ftok("/server", 's');

    if ((shm_id = shmget(key, sizeof(Bank), IPC_CREAT | 0666)) < 0)
    {
        message = "Error: shmget() failed.";
        write(glob_sd, message, strlen(message)+1);
        exit(1);
    }
    
    global_shm_id = shm_id;

    if (*(int*)(glob_shm_addr = shmat(shm_id, NULL, 0)) == -1)
    {
        message = "ERROR: shmat() failed.";
        write(glob_sd, message, strlen(message)+1);
        exit(1);
    }

    /*Initialize the semaphore for the bank*/
    sem_init(&glob_shm_addr->lock, 1, 1);

    /*The bank currently has 0 accounts*/
    glob_shm_addr->num_accounts = 0;

    key = ftok("/server", 's');

    /*Set up the shared memory*/
    if ((shm_id = shmget(key, sizeof(Bank), 0666)) < 0)
    {
        message = "Error: shmget() failed.";
        write(glob_sd, message, strlen(message)+1);
        exit(1);
    }

    if (*(int*)(glob_shm_addr = shmat(shm_id, NULL, 0)) == -1)
    {
        message = "ERROR: shmat() failed.";
        write(glob_sd, message, strlen(message)+1);
        exit(1);
    }
}


int claim_port( const char * port )
{
    struct addrinfo addrinfo;
    struct addrinfo *   result;
    int sd;
    char message[256];
    int on = 1;

    addrinfo.ai_flags = AI_PASSIVE;     // for bind()
    addrinfo.ai_family = AF_INET;       // IPv4 only
    addrinfo.ai_socktype = SOCK_STREAM; // Want TCP/IP
    addrinfo.ai_protocol = 0;       // Any protocol
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
        return sd;          // bind() succeeded;
    }
}


int main (int argc, char** argv){
	/*whaaaaaaat in the world*/
	/*ok let's focus*/

	int sd;
    char message[256];
    pthread_attr_t kernel_attr;
    socklen_t address_len;
    int fd;
    struct sockaddr_in address;
    pid_t child;
    struct sigaction sigint;
    struct sigaction sigchld;
    struct sigaction sigalrm;

    alarmSetup(sigalrm);
    //alarm(3); not sure why this is necessary


    
    chldSetup(sigchld);
    if (sigaction(SIGCHLD, &sigchld, NULL) == -1) {
        perror("sigchld error");
        exit(1);
    }
    intSetup(sigint);
    if (sigaction(SIGINT, &sigint, NULL) == -1) {
        perror("sigint error");
        exit(1);
    }




        if ( pthread_attr_init( &kernel_attr ) != 0 )
    {
        printf( "pthread_attr_init() failed in file %s line %d\n", __FILE__, __LINE__ );
        return 0;
    }
    else if ( pthread_attr_setscope( &kernel_attr, PTHREAD_SCOPE_SYSTEM ) != 0 )
    {
        printf( "pthread_attr_setscope() failed in file %s line %d\n", __FILE__, __LINE__ );
        return 0;
    }
    else if ( (sd = claim_port( "36963" )) == -1 )
    {
        write( 1, message, sprintf( message,  "\x1b[1;31mCould not bind to port %s errno %s\x1b[0m\n", "51268", strerror( errno ) ) );
        return 1;
    }
    else if ( listen( sd, 100 ) == -1 )
    {
        printf( "listen() failed in file %s line %d\n", __FILE__, __LINE__ );
        close( sd );
        return 0;
    }
	shm_setup();
        while ( (fd = accept( sd, (sockptr)&address, &address_len )) != -1 ){
            if ((child = fork()) < 0){
                //process not created
                continue;
            }
           if(child==0){
            close(sd);
            //send child off to do whatever the client wants
            close(fd);
           }
           else{
                close(fd);
           } 
        }
        close(sd);
        return 0;
}
