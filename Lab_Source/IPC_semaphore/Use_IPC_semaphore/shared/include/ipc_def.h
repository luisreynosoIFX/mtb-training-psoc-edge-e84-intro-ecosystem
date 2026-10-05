#ifndef IPC_DEF_H
#define IPC_DEF_H

/* IPC Channel to use */
#define IPC_CHANNEL_NUM 8U

/* Semaphore number to be used in this example. Semaphores 0-15 are reserved
 * for system use. */
#define MAX_SEMA_NUM 32U
#define MY_SEMA_NUM 16U

/* Delay to allow other core to acquire the semaphore */
#define SEMA_DELAY 10U

#endif /* IPC_DEF_H */
