#include "BankAccount.h"
#define MAX_ACCOUNTS 20

BankAccount* bankArray;
int num_acc;
int i;

void freeBankAcc(BankAccount ba){
	if(!ba){
		//error, void BankAccount pointer
	}
	free(ba->name);
	free(ba);
	return;
}

void freeBankArr(){
	for(i = 0; i<num_acc; i++){
		freeBankAcc(bankArray[i]);
	}
	free(bankArray);
	return;
}

void initArray(){
	bankArray = malloc(MAX_ACCOUNTS * sizeof (BankAccount));
	num_acc = 0;
	return;
}

void printList(){
	for(i=0;i<num_acc;i++){
		printf("%s,%d"bankArray[i]->name,bankArray[i]->balance);
		if(bankArray[i]->isf == 1){
			printf(",IN SERVICE\n");
		}
		else{
			printf("\n");
		}
	}
	return;
}

BankAccount initBankAccount(string accname){
	if(num_acc + 1 == MAX_ACCOUNTS){
		//error - array is full
	}
	if(accname == NULL){
		//error - accName can't be null
	}
	for (i = 0; i<num_acc; i++){
		if(strcmp(bankArray[i]->name, accname) == 0)
		{
			//error - name already exists within array
		}
	}
	//now we know it's good

	BankAccount ba = malloc(sizeof(_BankAccount));
	ba->name = malloc(101*sizeof(char));
	strcpy(ba->name, accname);
	ba->balance = 0.00;
	isf = 1; /*currently in session*/
	return ba;	
}



