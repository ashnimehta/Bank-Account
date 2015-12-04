#ifndef SERVER_H
#define SERVER_H

#include "utils.h"

typedef struct BAccount{
	char name[100];
	float balance;
	int isf; /*in session flag*/
	sem_t acc_lock; /*lock the account while in use by client thread*/
} account;

typedef struct Bank{
	int currAccounts;
	Account acc_arr[20];
	sem_t bank_lock; /*lock the bank while printing*/
} Bank;

