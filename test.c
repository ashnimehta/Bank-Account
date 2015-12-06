/*Set up shared memory for the Bank*/
void create_shm()
{
    char message[2048];
    int shm_id;
    key_t key;
    
    key = ftok("/server", 's');

    if ((shm_id = shmget(key, sizeof(Bank), IPC_CREAT | 0666)) < 0)
    {
        message = "Error: shmget() failed.";
        write(glob_sd, message, strlen(message)+1);
        exit(1);
    }
    
    global_shm_id = shm_id;

    if (*(int*)(glob_shm_addr = shmat(shm_id, NULL, 0)) == -1)
    {
        message = "ERROR: shmat() failed.";
        write(glob_sd, message, strlen(message)+1);
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
        write(glob_sd, message, strlen(message)+1);
        exit(1);
    }

    if (*(int*)(glob_shm_addr = shmat(shm_id, NULL, 0)) == -1)
    {
        message = "ERROR: shmat() failed.";
        write(glob_sd, message, strlen(message)+1);
        exit(1);
    }
}