#include "server.h"

static int INDEX;
static Bank* glob_shm_addr;
static int busy=0;
static int currentfd;
static int glob_shmid;
void printlist()
{
   String noAcc = "No accounts are currently in the bank.\n";
    
    char printedoutput[20][150];
    String flagval;
    int outlen;
    char ourout[1500];
    memset(ourout, 0, 1500);
    if(glob_shm_addr->currAccounts <= 0){
        write(currentfd, noAcc, strlen(noAcc) + 1);
        return;
    }    
    
    /*Lock the bank*/
    sem_wait(&glob_shm_addr -> lock);

    int i;
    int j;
    
    /*Read in the bank values*/
     for(i = 0; i < glob_shm_addr->currAccounts; i++){
         if(glob_shm_addr->acc_arr[i].isf == 0)
         {
             flagval = "";
        }
       else flagval = "IN SERVICE";
         sprintf(printedoutput[i], "Account at Index %d: Name = %s, Balance = %.2f, %s \n", i, glob_shm_addr->acc_arr[i].name, glob_shm_addr->acc_arr[i].balance, flagval);
     }
     for (j = 0; j<glob_shm_addr->currAccounts; j++){
         strcat(ourout, printedoutput[j]);
     }

     outlen = strlen(ourout);

     /*write to stdout*/
     write(1, ourout, outlen + 1);

    // for(i = 0; i < glob_shm_addr->currAccounts; i++){
    //     if(glob_shm_addr->acc_arr[i].isf == 0)
    //      {
    //          flagval = "";
    //      }
    //      else flagval = "IN SERVICE";
    //      printf("Account at Index %d, ",i);
    //      puts(glob_shm_addr->acc_arr[i].name);
    //      printf("Balance = %.2f, %s \n",glob_shm_addr->acc_arr[i].balance, flagval);
    // }

    printf("\n");
    /*unlock the mutex*/
    sem_post(&glob_shm_addr->lock);
    return;
}

int finish(){
    char message [300];
    char completion[300];
    memset (message, 0, 300);
    memset (completion, 0, 300);
    if(busy==0){
        strcpy(message, "You are not currently in a session.");
        write(currentfd, message, strlen(message)+1);        
        return -1;
    }
    busy = 0;
    glob_shm_addr->acc_arr[INDEX].isf=0;
    sem_post(&glob_shm_addr->acc_arr[INDEX].lock);
    strcpy(completion, "Succesfully finished the customer session.");
    write(currentfd, completion, strlen(completion)+1);
    return 0;
}

int findaccount(String accname){
    int i=0;
    for(i=0;i<20;i++){
        if(strcmp(accname,glob_shm_addr->acc_arr[i].name)==0){
            return i;
        }
    }
    return -1;
}


int start(String accname){
    int current;
    char message [100];
    char continuation [100];
    memset (continuation, 0, 100);
    memset (message, 0, 100);
    if(busy==1){
        strcpy(message, "You are already in a session.");
        write(currentfd, message, strlen(message));
        return -1;
    }
    if((current=findaccount(accname))==-1){
        strcpy(message, "Account does not exist.");
        write(currentfd, message, strlen(message)+1);
        return -1;
    }
    INDEX = current;
        strcpy(message, "Account currently in use, please wait.");
        strcpy(continuation, "Account acquired. Please enter a command.");
    while(sem_trywait(&glob_shm_addr->acc_arr[INDEX].lock)!=0){   
        write(currentfd, message, strlen(message));
        sleep(5);
    }
    write(currentfd, continuation, strlen(continuation));
    glob_shm_addr->acc_arr[INDEX].isf = 1;
    busy = 1;
    return 0;
}

int detrequest(){
    printf("Server is receiving input...");
    String arg1 = malloc(6);
    String arg2 = malloc(101);
    float amount;
    char message [300];
    char command [300];
    memset (message, 0, 300);
    memset (command, 0, 300);
    
    while ( read(currentfd, command, sizeof(command) ) > 0 ){

            if(strlen(command)<4){
                strcpy(message, "Not a valid command.");
                write(currentfd, message, strlen(message)+1);
                memset (message, 0, 300);
                memset (command, 0, 300);
                continue;

            }

    sscanf(command," %s %[^\n]\n",arg1,arg2);
    if(strcmp(arg1, "exit") == 0){
    	if(busy==1){
		sem_post(&glob_shm_addr->acc_arr[INDEX].lock);
		glob_shm_addr->acc_arr[INDEX].isf = 0;
        busy = 0;
	}
    free(arg1);
    free(arg2);
	exit(0);
    }
    else if(strcmp(arg1,"balance")==0){
        balance();
        memset (message, 0, 300);
        memset (command, 0, 300);
        continue;
    }
    else if(strcmp(arg1,"finish")==0){
        finish();
        memset (message, 0, 300);
        memset (command, 0, 300);
        continue;

    }

    if(sscanf(command," %s %[^\n]\n",arg1,arg2)!=2){
        strcpy(message, "Not a valid command.");
        write(currentfd, message, strlen(message)+1);
        memset (message, 0, 300);
        memset (command, 0, 300);
        continue;
    }

    if(strcmp(arg1,"open")==0){
        makeAccount(arg2);
        memset (message, 0, 300);
        memset (command, 0, 300);
        continue;
    }
    else if(strcmp(arg1,"start")==0){
        start(arg2);
        memset (message, 0, 300);
        memset (command, 0, 300);
        continue;
    }
    else if(strcmp(arg1,"credit")==0){
        if((amount=atof(arg2))<=0.0){
            strcpy(message, "Not a valid amount. ");
            write(currentfd, message, strlen(message)+1);
            memset (message, 0, 300);
             memset (command, 0, 300);
            continue;
        }
        credit(amount);
        memset (message, 0, 300);
        memset (command, 0, 300);
        continue;
    }
    else if(strcmp(arg1,"debit")==0){
        if((amount=atof(arg2))<=0.0){
            strcpy(message, "Not a valid amount.");
            write(currentfd, message, strlen(message)+1);
            memset (message, 0, 300);
            memset (command, 0, 300);
            continue;
        }
        debit(amount);
        memset (message, 0, 300);
        memset (command, 0, 300);
        continue;
    }
    else{
        strcpy(message, "Not a valid command.");
        write(currentfd, message, strlen(message)+1);
        memset (message, 0, 300);
        memset (command, 0, 300);
        continue;
        }
    memset (message, 0, 300);
    memset (command, 0, 300);
    }
    free(arg1);
    free(arg2);
    return 0;
}

int balance(){
    char message [2048];
    if(busy == 0){
        strcpy(message, "You are not currently in a session.");
        write(currentfd, message, strlen(message)+1);
        return -1;
    }
    
    char output [256];
    memset (output, 0, 256);
    sprintf(output, "Current balance is %.2f", glob_shm_addr->acc_arr[INDEX].balance);
    write(currentfd, output, strlen(output) + 1);
    return 1; /*balance function code is 1*/
}

int credit(float amount){
    int acc_curr_val;
    char message [2048];
    if(busy == 0){
        strcpy(message, "You are not currently in a session.");
        write(currentfd, message, strlen(message)+1);
        return -1;
    }
    acc_curr_val = glob_shm_addr->acc_arr[INDEX].balance;
    if(amount<=0.0){
        strcpy(message, "Not a valid amount. ");
            write(currentfd, message, strlen(message)+1);
    	return -1;
    }
        glob_shm_addr->acc_arr[INDEX].balance = acc_curr_val + amount;
        
    char output [256];
    memset (output, 0, 256);
    
    sprintf(output, "CREDIT: New balance is %.2f", glob_shm_addr->acc_arr[INDEX].balance);
    write(currentfd, output, strlen(output)+ 1);
    return 5;
}

int debit(float amount){
    char message [2048];
    if(busy == 0){
        strcpy(message, "You are not currently in a session.");
        write(currentfd, message, strlen(message)+1);
        return -1;
    }
    float acc_curr_val;
    acc_curr_val = glob_shm_addr->acc_arr[INDEX].balance;
    if(amount <= 0){
        strcpy(message, "Not a valid amount. \n");
            write(currentfd, message, strlen(message)+1);
        return -1;
    }
    if(amount > acc_curr_val){
        strcpy(message, "Cannot debit more than current account balance.");
            write(currentfd, message, strlen(message)+1);
        return -1;
    }
    glob_shm_addr->acc_arr[INDEX].balance = acc_curr_val - amount;
    
    char output [256];
    memset (output, 0, 256);
    
    sprintf(output, "DEBIT: New balance is %.2f", glob_shm_addr->acc_arr[INDEX].balance);
    write(currentfd, output, strlen(output)+ 1);
    
    return 6;
}

void alarmSetup(struct sigaction* sig){
    sig->sa_handler = alarmhandler;
    sig->sa_flags = SA_RESTART; 
    sigaction(SIGALRM, sig, NULL);
}

void intSetup(struct sigaction* sig){
    sig->sa_handler = inthandler;
    sigemptyset(&sig->sa_mask);
    sig->sa_flags = 0;
}

void chldSetup(struct sigaction* sig){
    sig->sa_handler = childhandler; // reap all dead processes
    sigemptyset(&sig->sa_mask);
    sig->sa_flags = SA_RESTART;
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
    
    while((waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("Killing child process\n");
    }
}

void inthandler(int sig){
     if (shmdt(glob_shm_addr) != 0) { //clear up all shared memory
   perror("shmdt");
         exit(1);
     }

     shmctl(glob_shmid, IPC_RMID, NULL);
     write(currentfd, "disconnect", 11); //kill client process

    exit(0);
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
        strcpy(message, "Error: shmget() failed.");
        write(1, message, strlen(message)+1);
        exit(1);
    }
    glob_shmid = shm_id;
    if (*(int*)(glob_shm_addr = shmat(shm_id, NULL, 0)) == -1)
    {
        strcpy(message,"ERROR: shmat() failed.");
        write(1, message, strlen(message)+1);
        exit(1);
    }

    /*Initialize the semaphore for the bank*/
    sem_init(&glob_shm_addr->lock, 1, 1);

    /*The bank currently has 0 accounts*/
    glob_shm_addr->currAccounts = 0;

    key = ftok("/server", 's');

    /*Set up the shared memory*/
    if ((shm_id = shmget(key, sizeof(Bank), 0666)) < 0)
    {
        strcpy(message,"Error: shmget() failed.");
        write(1, message, strlen(message)+1);
        exit(1);
    }

    if (*(int*)(glob_shm_addr = shmat(shm_id, NULL, 0)) == -1)
    {
        strcpy(message,"ERROR: shmat() failed.");
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
        strcpy(message, "You cannot create another account while already in a session.");
    	write(currentfd, message, strlen(message)+1);
        return -1;
    }
    
    if(num == 20)
    {
        strcpy(message, "Sorry, the bank is full and cannot hold a new account.");
        write(currentfd, message, strlen(message)+1);
        return -1;
    }

    for(i = 0; i < num; i++){
        if(strcmp(name, glob_shm_addr->acc_arr[i].name) == 0)
        {
            strcpy(message, "Sorry, an account with that name already exists.");
            write(currentfd, message, strlen(message)+1);
            return -1;
        }
    }
    memset (message, 0, 2048);
    strcpy(message, "Bank currently in use.");
    while(sem_trywait(&glob_shm_addr->lock)!=0){   
        write(currentfd, message, strlen(message));
        sleep(3);
    }
    

    /*set the semaphore to 1*/
    sem_init (&glob_shm_addr->acc_arr[num].lock, 1, 1);
    strcpy(glob_shm_addr->acc_arr[num].name, name);
    
    /*set the values of this new account*/
    int namelength = strlen(name);
    glob_shm_addr->acc_arr[num].name[namelength] = '\0';
    glob_shm_addr->acc_arr[num].balance = 0.0;
    glob_shm_addr->acc_arr[num].isf = 0;
    glob_shm_addr->currAccounts++;

    /*unlock the semaphore for next use*/
    sem_post(&glob_shm_addr->lock);
    memset (message, 0, 2048);
    strcpy(message, "Succesfully created account.");
    write(currentfd, message, strlen(message)+1);
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

    alarmSetup(&sigalrm);
    alarm(20);

    chldSetup(&sigchld);
    if (sigaction(SIGCHLD, &sigchld, NULL) == -1) {
        perror("sigchld error");
        exit(1);
    }
    
    intSetup(&sigint);
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
                continue;
            }
           if(child==0){
            close(sd);
            currentfd = fd;
            printf("Server accepting a client connection.");
            detrequest();
            close(fd);
           }
           else{
                close(fd);
           } 
        }
        close(sd);
        return 0;
}

