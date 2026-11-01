#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>

// Define the message structure
struct msg_buffer
{
    long msg_type; // Must be > 0
    int data;      // The actual payload (item produced)
};

int main()
{
    key_t key;
    int msqid;
    pid_t pid;

    // 1. Generate a unique key using ftok
    key = ftok("progfile", 65);
    if (key == -1)
    {
        perror("ftok failed");
        exit(1);
    }

    // 2. Create the message queue (IPC_CREAT creates it, 0666 sets read/write permissions)
    msqid = msgget(key, 0666 | IPC_CREAT);
    if (msqid == -1)
    {
        perror("msgget failed");
        exit(1);
    }

    // 3. Fork the process into Producer and Consumer
    pid = fork();

    if (pid < 0)
    {
        perror("Fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        // ====CONSTRUCTING THE CONSUMER (Child Process) ======
        struct msg_buffer message;

        for (int i = 1; i <= 5; i++)
        {
            // msgrcv blocks automatically if the queue is empty
            if (msgrcv(msqid, &message, sizeof(message.data), 1, 0) == -1)
            {
                perror("msgrcv failed");
                exit(1);
            }
            printf("[Consumer] Consumed item: %d\n", message.data);
            sleep(2); // Simulate time taken to consume the item
        }
        exit(0);
    }
    else
    {
        // ====== CONSTRUCTING THE PRODUCER (Parent Process) =======
        struct msg_buffer message;
        message.msg_type = 1; // Message type must be positive

        for (int i = 1; i <= 5; i++)
        {
            message.data = i * 10; // Produce an item (e.g., 10, 20, 30...)
            printf("[Producer] Produced item: %d\n", message.data);

            // msgsnd sends the message to the queue
            if (msgsnd(msqid, &message, sizeof(message.data), 0) == -1)
            {
                perror("msgsnd failed");
                exit(1);
            }
            sleep(1); // Simulate time taken to produce the next item
        }

        // Wait for the consumer child process to finish
        wait(NULL);

        // 4. Clean up and delete the message queue from the kernel
        msgctl(msqid, IPC_RMID, NULL);
        printf("Message queue destroyed. Program finished.\n");
    }

    return 0;
}
