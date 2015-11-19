#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>

typedef char*	string;

struct _BankAccount{
	string name;
	float balance;
	int isf; /*boolean flag indicating whether the current account is in session or not*/
};
typedef struct _BankAccount* BankAccount;

#endif