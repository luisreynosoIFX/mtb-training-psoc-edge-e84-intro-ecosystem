/*******************************************************************************
* File Name        : main.c
*
* Description      : This source file contains the main routine for non-secure
*                    application running on CM33 CPU.
*
* Related Document : See README.md
*
********************************************************************************
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
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cy_gpio.h"
#include "cybsp.h"
#include "cycfg_peripherals.h"
#include "cy_retarget_io.h"



/*******************************************************************************
 * Macros
 ******************************************************************************/
#define TIMER_INT_PRIORITY                (3U)
#define BIT_MASK_CHECK                    (0U)

/* The timeout value in microsecond used to wait for the CM55 core to be booted.
 * Use value 0U for infinite wait till the core is booted successfully.
 */
#define CM55_BOOT_WAIT_TIME_USEC            (10U)

/* App boot address for CM55 project */
#define CM55_APP_BOOT_ADDR                  (CYMEM_CM33_0_m55_nvm_START + \
                                                CYBSP_MCUBOOT_HEADER_SIZE)



/*******************************************************************************
 * Global Variables
 ******************************************************************************/
#define TRAINING_LED_PORT 	GPIO_PRT16
#define TRAINING_LED_PIN	(7U)

static mtb_hal_uart_t DEBUG_UART_hal_obj;
static cy_stc_scb_uart_context_t DEBUG_UART_context;

cy_stc_sysint_t timer_irq_cfg =
{
    .intrSrc = TIMER_IRQ,
    .intrPriority = TIMER_INT_PRIORITY
};

static bool timer_interrupt_flag = false;
static bool led_blink_active_flag = true;
static uint8_t uart_read_value;



/*******************************************************************************
* Function Name: handle_app_error
********************************************************************************
* Summary:
* User defined error handling function
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void handle_app_error(void)
{
    /* Disable all interrupts. */
    __disable_irq();

    CY_ASSERT(0);

    /* Infinite loop */
    while(true);
}


/*******************************************************************************
* Function Name: timer_isr
********************************************************************************
* Summary:
* This is the interrupt handler function for the timer interrupt.
*
* Parameters:
*    callback_arg    Arguments passed to the interrupt callback
*    event            Timer/counter interrupt triggers
*
* Return:
*  void
*******************************************************************************/
static void timer_isr(void)
{
    uint32_t interrupts = Cy_TCPWM_GetInterruptStatusMasked(TIMER_HW, TIMER_NUM);

    Cy_TCPWM_ClearInterrupt(TIMER_HW, TIMER_NUM, interrupts);

    /* Check for Timer Count event */
    if (BIT_MASK_CHECK != (CY_TCPWM_INT_ON_CC0 & interrupts))
    {
		timer_interrupt_flag = true;
    } 
}

/*******************************************************************************
* Function Name: timer_init
********************************************************************************
* Summary:
* This function creates and configures a Timer object. The timer ticks
* continuously and produces a periodic interrupt on every terminal count
* event. The period is defined by the 'period' and 'compare_value' of the
* timer configuration structure 'led_blink_timer_cfg'. Without any changes,
* this application is designed to produce an interrupt every 1 second.
*
* Parameters:
*  none
*
* Return :
*  void
*
*******************************************************************************/
void timer_init(void)
{
	/* Initialze TCPWM block with required timer configuration */
    if (CY_TCPWM_SUCCESS != Cy_TCPWM_Counter_Init(TIMER_HW, 
                                                  TIMER_NUM, 
                                                  &TIMER_config))
    {
        CY_ASSERT(0);
    }

    /* Set the interrupt line for COUNTER_HW */
    if (CY_SYSINT_SUCCESS != Cy_SysInt_Init(&timer_irq_cfg, &timer_isr))
    {
        CY_ASSERT(0);
    }

	/* setup the timer interrupt */
    NVIC_EnableIRQ(TIMER_IRQ);

    /* Enable the initialized counter */
    Cy_TCPWM_Counter_Enable(TIMER_HW, 
                            TIMER_NUM);

    /* Start the counter */
    Cy_TCPWM_TriggerStart_Single(TIMER_HW, 
                                 TIMER_NUM);
}


/*******************************************************************************
 * Function Name: main
 *******************************************************************************
 * Summary:
 * This is the main function for CM33 non-secure application. 
 *    1. It initializes the device and board peripherals.
 *    2. It sets up the CLIB support library for CM33 CPU. 
 *    3. It sets up the LPTimer instance for CM33 CPU. 
 *    4. It enables the CM55 CPU using 'Cy_SysEnableCM55'.
 *    5. It creates the FreeRTOS application task 'cm33_blinky_task'.
 *    6. It starts the RTOS task scheduler.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  int
 *
 ******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board initialization failed. Stop program execution */
    if (CY_RSLT_SUCCESS != result)
    {
        handle_app_error();
    }

	/* Set GPIO for use with LED */
	Cy_GPIO_SetDrivemode(TRAINING_LED_PORT, TRAINING_LED_PIN, CY_GPIO_DM_STRONG);

	/* Initialize UART for use with retarget-io*/
	Cy_SCB_UART_Init(CYBSP_DEBUG_UART_HW, &CYBSP_DEBUG_UART_config, &DEBUG_UART_context);
	Cy_SCB_UART_Enable(CYBSP_DEBUG_UART_HW);
	result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, 
	                                &CYBSP_DEBUG_UART_hal_config, 
	                                &DEBUG_UART_context, NULL);
	    
    /* UART setup failed. Stop program execution. */
    if (CY_RSLT_SUCCESS != result)
    {
        handle_app_error();
    }

	/*init retarget_io with uart*/
	cy_retarget_io_init(&DEBUG_UART_hal_obj);
   
    /* Enable CM55. */
    /* CM55_APP_BOOT_ADDR must be updated if CM55 memory layout is changed.*/
    Cy_SysEnableCM55(MXCM55, CM55_APP_BOOT_ADDR, CM55_BOOT_WAIT_TIME_USEC);

	/* Enable global interrupts */
    __enable_irq();

	/* Print hello world */
	printf("\x1b[2J\x1b[;H");
	
	printf("****************** "
	"PSOC Edge MCU: Hello world using retarget-IO"
	"****************** \r\n\n");
	
	printf("Hello World!!!\r\n\n");
	
	printf("For more projects, "
	"visit our code examples repositories:\r\n\n");
	
	printf("https://github.com/Infineon/"
	"Code-Examples-for-ModusToolbox™™-Software\r\n\n");

	/* initialize and startup the timer/counter */
	timer_init();

	
	/* main forever loop*/
    for(;;)
	{
		/* Check to see if we have received anything on the uart*/
		if (Cy_SCB_UART_GetNumInRxFifo(CYBSP_DEBUG_UART_HW) > 0)
	    {
	        scanf("%c", &uart_read_value);
	
	        if (uart_read_value == '\r')
	        {
	            led_blink_active_flag = !led_blink_active_flag;
	            if (led_blink_active_flag)
				{
					Cy_TCPWM_TriggerStart_Single(TIMER_HW, TIMER_NUM);
	            	printf("Enter key detected: Blinking Start\r\n");
				}
	            else
				{
	            	Cy_TCPWM_TriggerStopOrKill_Single(TIMER_HW, TIMER_NUM);
					printf("Enter key detected: Blinking Stops\r\n");
				}
	        }
	    }
	
		/* Check if timer elapsed (interrupt fired) and toggle the LED */
	    if (timer_interrupt_flag)
	    {
	        /* Clear the timer irq flag */
	        timer_interrupt_flag = false;
	
	        /* Toggle LED state */
	        Cy_GPIO_Inv(TRAINING_LED_PORT, TRAINING_LED_PIN);
	    }

	}
}

/* [] END OF FILE */
