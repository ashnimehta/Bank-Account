#ifndef COMMANDS_H
#define COMMANDS_H

#include "bankAccount.h"

int debit(float amount, bankAccount account);
int credit(float amount, bankAccount account);
float currentbalance(bankAccount account);



#endif