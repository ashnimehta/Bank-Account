int makeAccount(string name, int fd){

    int i;
    int num = glob_shm_addr->currAccounts;

    char message [2048];
    memset (message, 0, 2048);
    
    if(num == 20)
    {
        write(fd, message, sprintf("Sorry, the bank is full and cannot hold a new account."));
        return -1;
    }

    for(i = 0; i < num; i++){
        if(strcmp(name, glob_shm_addr->acc_arr[i].name) == 0)
        {
            write(fd, message, sprintf("Sorry, an account with that name already exists."));
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