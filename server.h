#ifndef SERVER_H
#define SERVER_H

#include "utils.h"
/*use a semamphore because oracle lists it as one of the synchronization
mechanisms*/

typedef struct Account{
	char name[100];
	float balance;
	int isf; /*in session flag*/
	sem_t lock; /*lock the account while in use by client thread*/
} account;

typedef struct Bank{
	int currAccounts;
	Account acc_arr[20];
	sem_t lock; /*lock the bank while printing*/
} Bank;

void alarmhandler(int sig);
void childhandler(int sig);
void inthandler(int sig);
void alarmSetup(sigaction* sig);
void intSetup(sigaction* sig);
void chldSetup(sigaction* sig);
int claim_port( const char * port );


