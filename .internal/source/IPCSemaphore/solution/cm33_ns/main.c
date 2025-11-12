/*****************************************************************************
* File Name        : main.c
*
* Description      : This source file contains the main routine for 
*                    non-secure application in CM33 CPU for 
*                    the IPC semaphore application
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
#include "cy_ipc_sema.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include <stdlib.h>
#include "ipc_def.h"

/*****************************************************************************
* Macros
*****************************************************************************/
#define CM55_BOOT_WAIT_TIME_USEC   10U

/* App boot address for CM55 project */
#define CM55_APP_BOOT_ADDR          (CYMEM_CM33_0_m55_nvm_START + \
                                        CYBSP_MCUBOOT_HEADER_SIZE)


/*****************************************************************************
* Global Definitions
*****************************************************************************/

CY_SECTION_SHAREDMEM uint32_t sema_data[CY_IPC_SEMA_COUNT / CY_IPC_SEMA_PER_WORD];

/*****************************************************************************
* Function Name: main
******************************************************************************
* Summary:
* This is the main function of the non-secure CM33 CPU application.
*
* It initializes the system resources and starts the CM55 application.
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
        /* Disable all interrupts. */
        __disable_irq();

        CY_ASSERT(0);

        /* Infinite loop */
        while(true);

    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize IPC subsytem */
    ipc_status = Cy_IPC_Sema_Init(IPC_CHANNEL_NUM, CY_IPC_SEMA_COUNT, sema_data);

    if (CY_IPC_SEMA_SUCCESS != ipc_status)
    {
        /* Disable all interrupts. */
        __disable_irq();

        CY_ASSERT(0);

        /* Infinite loop */
        while(true);
    }

    /* lock IPC semaphore SEMA_NUM*/
    Cy_IPC_Sema_Set(SEMA_NUM, false);

    /* Enable CM55. */
    /* CM55_APP_BOOT_ADDR must be updated if CM55 memory layout is changed.*/
    Cy_SysEnableCM55(MXCM55, CM55_APP_BOOT_ADDR, CM55_BOOT_WAIT_TIME_USEC);


	/* Wait for cm55 to finish startup and release the semaphore*/
	while(Cy_IPC_Sema_Status(SEMA_NUM) == CY_IPC_SEMA_STATUS_LOCKED) {}

    for (;;)
    {
        /* If there is a user button press */
        if (Cy_GPIO_Read(CYBSP_USER_BTN_PORT, CYBSP_USER_BTN_PIN) == 0)
        {
            /* Send cm33 message to the UART */
            Cy_SCB_UART_PutString(CYBSP_DEBUG_UART_HW, "Message sent from CM33 \r\n");

        #if ENABLE_SEMA
            /* Lock the semaphore */
            Cy_IPC_Sema_Set(SEMA_NUM, false);

            /* Wait for CM55 to release the semaphore */
            while(Cy_IPC_Sema_Status(SEMA_NUM) == CY_IPC_SEMA_STATUS_LOCKED) {}
        #endif

        }
    }
}

/* [] END OF FILE */