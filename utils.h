#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <semaphore.h>

typedef char* String;
typedef struct sockaddr* sockptr;