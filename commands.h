#ifndef COMMANDS_H
#define COMMANDS_H

#include "bankAccount.h"

int debit(float amount, Account account);
int credit(float amount, Account account);
float currentbalance(Account account);



#endif
