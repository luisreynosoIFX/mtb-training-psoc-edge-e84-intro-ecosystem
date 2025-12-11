/*****************************************************************************
* File Name        : main.c
*
* Description      : This source file contains the main routine for non-secure
*                    application in the CM33 CPU
*
* Related Document : See README.md
*
*******************************************************************************
* Copyright 2023-2024, Cypress Semiconductor Corporation (an Infineon company) or
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
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"



/*****************************************************************************
* Macros
*****************************************************************************/


/*****************************************************************************
* Global Variables
*****************************************************************************/
/*****************************************************************************
* Function Prototypes
*****************************************************************************/


/*****************************************************************************
* Function Name: main
******************************************************************************
* Summary:
* This is the main function for non-secure application of CM33 CPU.
*
* It initializes the system and HAL IPC queues.
* On receiving the START/STOP command in IPC command queue, it generates random 
* number and adds to IPC message queue every 1s and stops respectively. 
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
	/* Local variables */
	cy_rslt_t result;
	
    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (CY_RSLT_SUCCESS != result)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Allocating (shared) memory pools for queue1 and queue2 */

    /* Allocating (shared) memory for queue handles and initialize handles*/

	/* Init IPC queues */

	/* Enable CM55. CY_CORTEX_M55_APPL_ADDR must be updated if CM55 memory layout is changed. */
	/* Main loop */

		/* Check if MCWDT interrupt has occurred. */



    return 0;
}



/*******************************************************************************
* Function Name: mcwdt_handler
********************************************************************************
* Summary:
*   Watchdog handler to periodically wake up the CM33.
*
*******************************************************************************/


/* [] END OF FILE */
