#include "server.h"

static int index;
static char glob_sd;
static Bank* glob_shm_addr;
static int glob_shm_id;
static int busy=0;
static int currentfd;

void printlist()
{
   String noAcc = "No accounts are currently in the bank.";
    
    char printedoutput[20][150];
    String flagval;
    
    char ourout[1500];
    memset(ourout, 0, 1500);

    if(glob_shm_addr->currAccounts <= 0){
        write(currentfd, noAcc, strlen(NoAcc) + 1);
        return;
    }    
    
    /*Lock the bank*/
    sem_wait(&glob_shm_addr -> lock);

    int i;
    int j;
    
    /*Read in the bank values*/
    for(i = 0; i < glob_shm_addr->currAccounts; i++){
        if(glob_shm_addr->acc_arr[i]->isf == 0)
        {
            flagval = "false";
        }
        else flagval = true;
        sprintf(printedoutput[i], "Account at index %d: Name = %s, Balance = %.2f, In Session = %s \n", i, glob_shm_addr->acc_arr[i].name, glob_shm_addr->acc_arr[i]->balance, flagval);
    }

    for (j = 0; j<glob_shm_addr->currAccounts; j++){
        strcat(ourout, printedoutput[j]);
    }

    outlen = strlen(outlen);

    /*write to stdout*/
    write(currentfd, ourout, outlen + 1);

    /*unlock the mutex*/
    sem_post(&glob_shm_addr->lock);
    return;
}

int finish(){
    char message [300];
    memset (message, 0, 300);
    if(busy==0){
        write(currentfd, message, sprintf("You are not currently in a session."));        
        return -1;
    }
    busy = 0;
    glob_shm_addr>acc_arr[index].isf=0;
    sem_post(&glob_shm_addr->acc_arr[index]->lock);
    return 0;
}

int findaccount(String accname){
    int i=0;
    for(i=0;i<20;i++){
        if(strcmp(accname,glob_shm_addr->acc_arr[i]->name)==0){
            return i;
        }
    }
    return -1;
}


int start(String accname){
    int current;
    char message [300];
    memset (message, 0, 300);
    if(busy==1){
        write(currentfd, message, sprintf("You are already in a session."));
        return -1;
    }
    if((current=findaccount(accname))==-1){
        write(currentfd, message, sprintf("Account does not exist."));
        return -1;
    }
    index = current;
    sem_wait(&glob_shm_addr->acc_arr[i]->lock);
    glob_shm_addr->acc_arr[index].isf = 1;
    busy = 1;
    return 0;
}

int detrequest(){
    String arg1 = malloc(6);
    String arg2 = malloc(101);
    float amount;
    int function;
    char message [300];
    char command [300];
    memset (message, 0, 300);
    memset (command, 0, 300);
    
    while ( read(currentfd, command, sizeof(command) ) > 0 ){
    
    if(!command){
        write(currentfd, message, sprintf("Not a valid command."));
        return -1;
    }
    if(strcmp(command, "exit") == 0){
    	if(busy){
		sem_post(&glob_shmaddr->acc_arr[index].lock);
		glob_shmaddr->acc_arr[index].isf = 0;
	}
	exit(0);
    }
    else if(strcmp(command,"balance")==0){
        balance();
    }
    else if(strcmp(command,"finish")==0){
        finish();

    }
    if(sscanf(command,"%s %s"arg1,arg2)!=2){
        write(currentfd, message, sprintf("Not a valid command."));
        return -1;
    }

    if(strcmp(arg1,"open")==0){
        makeAccount(arg2);

    }
    else if(strcmp(arg1,"start")==0){
        start();

    }
    else if(strcmp(arg1,"credit")==0){
        if((amount=atof(arg2))==0.0){
            write(currentfd, message, sprintf("Not a valid amount."));
        }
        credit(amount);

    }
    else if(strcmp(arg1,"debit")==0){
        if((amount=atof(arg2))==0.0){
            write(currentfd, message, sprintf("Not a valid amount."));
        }
        debit(amount);
    }
    }
    free(arg1);
    free(arg2);
    return 0;
}

int balance(req){
    if(busy == 0)
        return -1;
    
    char output [256];
    memset (output, 0, 256);
    
    sprintf(output, "Current balance is %.2f", glob_shm_addr->acc_arr[index]->balance);
    write(currentfd, output, strlen(output) + 1);
    return 1; /*balance function code is 1*/
}

int credit(float amount){
    if(busy == 0)
        return -1;
    
    acc_curr_val = glob_shm_addr->acc_arr[index]->balance;
    if(amount<=0){
    	return -1;
    }
        glob_shm_addr->acc_arr[index]->balance = acc_curr_val + amount;
        
    char output [256];
    memset (output, 0, 256);
    
    sprintf(output, "CREDIT: New balance is %.2f", glob_shm_addr->acc_arr[index]->balance);
    write(currentfd, output, strlen(output)+ 1);
    return 5;
}

int debit(float amount){
    if(busy == 0)
        return -1;
    acc_curr_val = glob_shm_addr->acc_arr[index]->balance;
    if(amount <= 0)
        return -1;
    if(amount > acc_curr_val)
        return -1;
    glob_shm_addr->acc_arr[index]->balance = acc_curr_val - amount;
    
    char output [256];
    memset (output, 0, 256);
    
    sprintf(output, "DEBIT: New balance is %.2f", glob_shm_addr->acc_arr[index]->balance);
    write(currentfd, output, strlen(output)+ 1);
    
    return 6;
}

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
        write(1, message, strlen(message)+1);
        exit(1);
    }
    
    global_shm_id = shm_id;

    if (*(int*)(glob_shm_addr = shmat(shm_id, NULL, 0)) == -1)
    {
        message = "ERROR: shmat() failed.";
        write(1, message, strlen(message)+1);
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
        write(1, message, strlen(message)+1);
        exit(1);
    }

    if (*(int*)(glob_shm_addr = shmat(shm_id, NULL, 0)) == -1)
    {
        message = "ERROR: shmat() failed.";
        write(1, message, strlen(message)+1);
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

int makeAccount(String name){
    int i;
    int num = glob_shm_addr->currAccounts;

    char message [2048];
    memset (message, 0, 2048);
    
    if(busy==1){
    	write(currentfd, message, sprintf("You cannot create another account while already in a session."));
        return -1;
    }
    
    if(num == 20)
    {
        write(currentfd, message, sprintf("Sorry, the bank is full and cannot hold a new account."));
        return -1;
    }

    for(i = 0; i < num; i++){
        if(strcmp(name, glob_shm_addr->acc_arr[i].name) == 0)
        {
            write(currentfd, message, sprintf("Sorry, an account with that name already exists."));
            return -1;
        }
    }

    sem_wait(&glob_shm_addr->lock);

    /*set the semaphore to 1*/
    sem_init (&glob_shm_addr->acc_arr[num].lock, 1, 1);
    strcpy(glob_shm_addr->acc_arr[num].name, name);
    
    /*set the values of this new account*/
    int namelength = strlen(name);
    glob_shm_addr->acc_arr[num].name[namelength] = '\0';
    glob_shm_addr->acc_arr[num].balance = 0;
    glob_shm_addr->acc_arr[num].isf = 0;
    glob_shm_addr->currAccounts++;

    /*unlock the semaphore for next use*/
    sem_post(&glob_shm_addr->lock);

    /*success*/
    return 0;
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
    alarm(3);

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
        write( 1, message, sprintf( message,  "\x1b[1;31mCould not bind to port %s errno %s\x1b[0m\n", "36963", strerror( errno ) ) );
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
            currentfd = fd;
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

