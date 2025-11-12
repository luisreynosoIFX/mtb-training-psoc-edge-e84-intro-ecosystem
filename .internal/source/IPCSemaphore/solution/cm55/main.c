/*****************************************************************************
* File Name        : main.c
*
* Description      : This source file contains the main routine for non-secure 
*                    application in CM55 CPU for the IPC semaphore application
*
* Related Document : See README.md
*
******************************************************************************
* Copyright 2023-2025, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*****************************************************************************/
#include "cy_gpio.h"
#include "cy_pdl.h"
#include "retarget_io_init.h"
#include <stdlib.h>
#include "ipc_def.h"

/*****************************************************************************
* Macros
*****************************************************************************/
/* Define the state of the USER_LED based on whether a semaphore is used*/
#if ENABLE_SEMA
#define LED_STATE   CYBSP_LED_STATE_OFF
#else
#define LED_STATE   CYBSP_LED_STATE_ON
#endif


/*****************************************************************************
* Global Variables
*****************************************************************************/


/*****************************************************************************
* Function Name: main
******************************************************************************
* Summary:
* This is the main function of the CM55 CPU application.
* It tries to acquire the semaphore and print message to UART.
* 
*
* Parameters:
*  void
*
* Return:
*  int
*
*****************************************************************************/
int main(void)
{
    cy_rslt_t result;
    /* variable to hold ipc_status*/
    cy_en_ipcsema_status_t ipc_status;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (CY_RSLT_SUCCESS != result)
    {
        handle_app_error();
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize IPC subsytem */
    ipc_status = Cy_IPC_Sema_Init(IPC_CHANNEL_NUM, 0UL, NULL);

    if(ipc_status != CY_IPC_SEMA_SUCCESS)
    {
        handle_app_error();
    }

    /* Initialize retarget_io middleware */
    init_retarget_io();

    /* Send Header message out to UART*/
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("******************  PSOC Edge MCU: IPC Semaphore Example  ****************** \r\n\n");

    printf("<Press the kit's user button to print messages>\r\n\n");

    /* Wait for UART traffic to stop */
    while(!(Cy_SCB_UART_IsTxComplete(CYBSP_DEBUG_UART_HW))) {}

	/* Turn on LED when Semaphores are not being used*/
	Cy_GPIO_Write(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_PIN, LED_STATE);

    /* unlock IPC semaphore SEMA_NUM*/
    Cy_IPC_Sema_Clear(SEMA_NUM, false);


    for (;;)
    {
        /* If there is a user button press */
        if (Cy_GPIO_Read(CYBSP_USER_BTN_PORT, CYBSP_USER_BTN_PIN) == 0)
        {
        #if ENABLE_SEMA
            /* Wait for CM33 to lock the semaphore */
            while(Cy_IPC_Sema_Status(SEMA_NUM) == CY_IPC_SEMA_STATUS_UNLOCKED) {}
        #endif
            /* Send cm55 message to the UART */
            printf("Message sent from CM55 \r\n");

        #if ENABLE_SEMA
            /* Release semaphore */
            Cy_IPC_Sema_Clear(SEMA_NUM, false);
        #endif
        }
    }
}

/* [] END OF FILE */