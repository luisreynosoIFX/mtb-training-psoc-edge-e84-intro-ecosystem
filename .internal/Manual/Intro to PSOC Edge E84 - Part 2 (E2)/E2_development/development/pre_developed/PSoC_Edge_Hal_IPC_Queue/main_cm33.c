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
#include "ipc_communication.h"

/*****************************************************************************
* Macros
*****************************************************************************/
#define MCWDT_HW                MCWDT_STRUCT0
#define CM55_BOOT_WAIT_TIME_US  10u
#define MCWDT_IRQ_PRIORITY      (2U)
#define MCWDT_WAIT_TIME         (0U)
#define WDT_FLAG_SET            (1)
#define WDT_FLAG_RESET          (0)

/* TRNG constants */
#define GARO31_INITSTATE        (0x04c11db7)
#define FIRO31_INITSTATE        (0x04c11db7)
#define MAX_TRNG_BIT_SIZE       (32UL)

/*****************************************************************************
* Global Variables
*****************************************************************************/
static const cy_stc_sysint_t mcwdt_flag_cfg =
{
    .intrSrc          = srss_interrupt_mcwdt_0_IRQn,
    .intrPriority     = MCWDT_IRQ_PRIORITY
};

/* Watchdog timer configuration */
static const cy_stc_mcwdt_config_t mcwdt_cfg =
{
    .c0Match          = 32768,
    .c1Match          = 32768,
    .c0Mode           = CY_MCWDT_MODE_INT,
    .c1Mode           = CY_MCWDT_MODE_NONE ,
    .c2ToggleBit      = 16,
    .c2Mode           = CY_MCWDT_MODE_NONE ,
    .c0ClearOnMatch   = true,
    .c1ClearOnMatch   = false,
    .c0c1Cascade      = false,
    .c1c2Cascade      = false,
    .c0LowerLimitMode = CY_MCWDT_LOWER_LIMIT_MODE_NOTHING,
    .c0LowerLimit     = 0U,
    .c1LowerLimitMode = CY_MCWDT_LOWER_LIMIT_MODE_NOTHING,
    .c1LowerLimit     = 0U
};

/* Global variable to detect MCWDT ISR handler execution */
volatile int wdt_flag = WDT_FLAG_RESET;


/*****************************************************************************
* Function Prototypes
*****************************************************************************/
static void mcwdt_handler(void);


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
    cy_en_mcwdt_status_t mcwdt_status;
    cy_en_sysint_status_t sysint_status;
    cy_en_crypto_status_t crypt_status;
    cyhal_ipc_t task0_queue1;
    cyhal_ipc_t task0_queue2;
     void *queue1_pool, *queue2_pool;
    cyhal_ipc_queue_t *queue1_handle, *queue2_handle;

    uint32_t random_number;
    uint32_t task0_msg = 0;            // Value to be sent to the other core.
    uint32_t task0_cmd = 0;

	
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
    CYHAL_IPC_QUEUE_POOL_ALLOC(queue1_pool, QUEUE1_ITEMS, sizeof(task0_cmd));
    CYHAL_IPC_QUEUE_POOL_ALLOC(queue2_pool, QUEUE2_ITEMS, sizeof(task0_msg));

    /* Allocating (shared) memory for queue handles and initialize handles*/
	CYHAL_IPC_QUEUE_HANDLE_ALLOC(queue1_handle);
	CYHAL_IPC_QUEUE_HANDLE_ALLOC(queue2_handle);

	queue1_handle->channel_num = CMD_CHANNEL_NUM;
	queue1_handle->queue_num   = QUEUE1_NUM;
	queue1_handle->queue_pool  = queue1_pool;
	queue1_handle->num_items   = QUEUE1_ITEMS;
	queue1_handle->item_size   = sizeof(task0_cmd);

	queue2_handle->channel_num = MSG_CHANNEL_NUM;
	queue2_handle->queue_num   = QUEUE2_NUM;
	queue2_handle->queue_pool  = queue2_pool;
	queue2_handle->num_items   = QUEUE2_ITEMS;
	queue2_handle->item_size   = sizeof(task0_msg);

	/* Init IPC queues */
	result = cyhal_ipc_queue_init(&task0_queue1, queue1_handle);
	if (CY_RSLT_SUCCESS != result)
	{
		CY_ASSERT(0);
	}

	result = cyhal_ipc_queue_init(&task0_queue2, queue2_handle);
	if (CY_RSLT_SUCCESS != result)
	{
		CY_ASSERT(0);
	}

	cyhal_ipc_queue_enable_event(&task0_queue1, CYHAL_IPC_QUEUE_WRITE, (uint8_t)IPC_QUEUE_IRQ_PRIORITY, true);

	/* Enable CM55. CY_CORTEX_M55_APPL_ADDR must be updated if CM55 memory layout is changed. */
	Cy_SysEnableCM55(CY_CM55_APP_BOOT_ADDR, CM55_BOOT_WAIT_TIME_US);

	/* Main loop */
	for(;;)
	{
		cyhal_syspm_deepsleep();

		if (cyhal_ipc_queue_count(&task0_queue1))
		{
			result = cyhal_ipc_queue_get(&task0_queue1, &task0_cmd, CYHAL_IPC_NEVER_TIMEOUT);
			if (CY_RSLT_SUCCESS != result)
			{
				CY_ASSERT(0);
			}

			switch (task0_cmd)
			{

				case IPC_CMD_INIT:

					SystemCoreClockUpdate();

					sysint_status = Cy_SysInt_Init(&mcwdt_flag_cfg, mcwdt_handler);
					if (CY_SYSINT_SUCCESS != sysint_status)
					{
						CY_ASSERT(0);
					}
					NVIC_ClearPendingIRQ((IRQn_Type) mcwdt_flag_cfg.intrSrc);
					NVIC_EnableIRQ((IRQn_Type) mcwdt_flag_cfg.intrSrc);


					mcwdt_status = Cy_MCWDT_Init(MCWDT_HW, &mcwdt_cfg);
					if (CY_MCWDT_SUCCESS != mcwdt_status)
					{
						CY_ASSERT(0);
					}
					Cy_MCWDT_SetInterruptMask(MCWDT_HW, CY_MCWDT_CTR0);
					task0_cmd = IPC_CMD_DEFAULT;

					break;

				case IPC_CMD_START:
					Cy_MCWDT_Enable(MCWDT_HW, CY_MCWDT_CTR0, MCWDT_WAIT_TIME);
					crypt_status = Cy_Crypto_Core_Enable(CRYPTO);
					if (CY_CRYPTO_SUCCESS != crypt_status)
					{
						CY_ASSERT(0);
					}

					task0_cmd = IPC_CMD_DEFAULT;

					break;

				case IPC_CMD_STOP:
					Cy_MCWDT_Disable(MCWDT_HW, CY_MCWDT_CTR0, MCWDT_WAIT_TIME);
					crypt_status = Cy_Crypto_Core_Disable(CRYPTO);
					if (CY_CRYPTO_SUCCESS != crypt_status)
					{
						CY_ASSERT(0);
					}

					wdt_flag = WDT_FLAG_RESET;
					task0_cmd = IPC_CMD_DEFAULT;

					break;

				default:
					break;
			}

		} /* if (cyhal_ipc_queue_count(&task0_queue1)) */

		/* Check if MCWDT interrupt has occurred. */
		if (wdt_flag)
		{
			/* Generate a random number */
			crypt_status = Cy_Crypto_Core_Trng(CRYPTO, GARO31_INITSTATE,
														FIRO31_INITSTATE,
														MAX_TRNG_BIT_SIZE, &random_number);
			if (CY_CRYPTO_SUCCESS != crypt_status)
			{
				CY_ASSERT(0);
			}

			/* Send the random number to CM55 to be printed */
			task0_msg = random_number;
			result = cyhal_ipc_queue_put(&task0_queue2, &task0_msg, CYHAL_IPC_NEVER_TIMEOUT);
			if (CY_RSLT_SUCCESS != result)
			{
				CY_ASSERT(0);
			}

			wdt_flag = WDT_FLAG_RESET;
		}
	}  /* End for loop */



    return 0;
}



/*******************************************************************************
* Function Name: mcwdt_handler
********************************************************************************
* Summary:
*   Watchdog handler to periodically wake up the CM33.
*
*******************************************************************************/
static void mcwdt_handler(void)
{
    uint32 mcwdtIsrMask;

    /* Get the Watchdog Interrupt Status */
    mcwdtIsrMask = Cy_MCWDT_GetInterruptStatus(MCWDT_HW);

    if(CY_MCWDT_CTR0 & mcwdtIsrMask)
    {
        /* Clear Watchdog Interrupt */
        Cy_MCWDT_ClearInterrupt(MCWDT_HW, CY_MCWDT_CTR0);

        wdt_flag = WDT_FLAG_SET;
    }
}


/* [] END OF FILE */
