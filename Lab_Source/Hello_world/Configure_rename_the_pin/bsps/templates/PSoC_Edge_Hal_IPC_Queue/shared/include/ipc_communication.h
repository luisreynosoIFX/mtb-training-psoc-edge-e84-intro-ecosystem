/******************************************************************************
* File Name:   ipc_communication.h
*
* Description: This file contains definitions of constants and structures for
*              setting up user pipe and function prototypes for configuring
*              system and user IPC pipe.
*
* Related Document: See README.md
*
*
*******************************************************************************
* $ Copyright 2020-YEAR Cypress Semiconductor $
*******************************************************************************/

#ifndef SOURCE_IPC_COMMUNICATION_H
#define SOURCE_IPC_COMMUNICATION_H

/* QUEUE1- command queue */
/* QUEUE2- msg queue */

#define CMD_CHANNEL_NUM                 CYHAL_IPC_CHAN_8
#define MSG_CHANNEL_NUM                 CYHAL_IPC_CHAN_9
#define QUEUE1_NUM                      (1UL)
#define QUEUE1_ITEMS                    (4UL)
#define QUEUE2_NUM                      (2UL)
#define QUEUE2_ITEMS                    (8UL)
#define IPC_QUEUE_IRQ_PRIORITY          (2U)

#define IPC_CMD_DEFAULT                 (0x80)
#define IPC_CMD_INIT                    (0x81)
#define IPC_CMD_START                   (0x82)
#define IPC_CMD_STOP                    (0x83)
#define IPC_CMD_STATUS                  (0x41)


#endif /* SOURCE_IPC_COMMUNICATION_H */

/* [] END OF FILE */
