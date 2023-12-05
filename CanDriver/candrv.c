/*
**====================================================================================
** Imported Public definitions
**====================================================================================
*/
#include "candrv.h"


/*
**====================================================================================
** definitions
**====================================================================================
*/
SemaphoreHandle_t xSemaphore_CTRL;
SemaphoreHandle_t xSemaphore_RX;
SemaphoreHandle_t xSemaphore_TX;


/*
**====================================================================================
** interface to suspend a task.
**====================================================================================
*/
void suspend_task(TaskHandle_t taskHandle) {
    if (taskHandle != NULL) {
        vTaskSuspend(taskHandle);
    }
}

/*
**====================================================================================
** interface to resume a task.
**====================================================================================
*/
void resume_task(TaskHandle_t taskHandle) {
    if (taskHandle != NULL) {
        vTaskResume(taskHandle);
    }
}

/*
**====================================================================================
** interface to initialize semaphores, queue and the tasks.
**====================================================================================
*/
void init_candrv()
{
	xSemaphore_CTRL = xSemaphoreCreateMutex();
	xSemaphore_RX = xSemaphoreCreateMutex();
	xSemaphore_TX = xSemaphoreCreateMutex();
	rxQueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
    // Check for semaphore creation success...
	xTaskCreate(increase_send_counter_app_task, "RX_TX_App", TASK_STACK_SIZE, NULL, 4, NULL);
	xTaskCreate(rx_handler_task, "RX_Handler", TASK_STACK_SIZE, NULL, 3, NULL);
	xTaskCreate(tx_handler_task, "TX_Handler", TASK_STACK_SIZE, NULL, 3, NULL);
	xTaskCreate(poll_can_rx_task, "POLL_CAN_RX", TASK_STACK_SIZE, NULL, 2, NULL);
	xTaskCreate(poll_can_tx_task, "POLL_CAN_TX", TASK_STACK_SIZE, NULL, 2, NULL);

	suspend_task(increase_send_counter_app_task);
	suspend_task(rx_handler_task);
	suspend_task(tx_handler_task);
	suspend_task(poll_can_rx_task);
	suspend_task(poll_can_tx_task);
}

/*
**====================================================================================
** interface to read signals from CAN driver rx register
**====================================================================================
*/
void read_can_signals(candrv_rxSignals* can_msg_signals)
{
	/*
	 * Check if semaphore is created
	 */
	if (xSemaphore_RX != NULL) {
		if (xSemaphoreTake(xSemaphore_RX, (TickType_t) 10) == pdTRUE) {
			U8 *data_rx = (U8*) DATA_RX;
			if(data_rx < 0x0FFF)
			{
				U16 id = (data_rx[10] << 8) | data_rx[9];
				U8 dlc = data_rx[8];
				if(id == 0x200 && dlc == candrv_DLC_4_BYTES){
					can_msg_signals->status_ecu = data_rx[0] & 0b00011111;
					can_msg_signals->value_example = (data_rx[2] << 8) | (data_rx[1]);
					can_msg_signals->value_example2 = (data_rx[3] >> 3) & 0b00011111;
					can_msg_signals->flag1 = (data_rx[3] >> 0) & 1;
					can_msg_signals->flag2 = (data_rx[3] >> 1) & 1;
					can_msg_signals->flag3 = (data_rx[3] >> 2) & 1;
				}
			}
			xSemaphoreGive( xSemaphore_RX );
		}
		else
		{
			/*
			 * Failed to take the semaphore (timeout occurred)
			 */
		}
	}
}

/*
**====================================================================================
** interface to write signals to the CAN driver tx register
**====================================================================================
*/
void write_can_signals(U8* send_counter)
{
	/*
	 * Check if semaphore is created
	 */
	if (xSemaphore_TX != NULL) {
		if (xSemaphoreTake(xSemaphore_TX, (TickType_t) 10) == pdTRUE) {
			U8 *data_tx = (U8*) DATA_TX;
			if(data_tx < 0x0FFF)
			{
				data_tx[9] = 0x0;
				data_tx[10] = 0x03;
				data_tx[8] = 1;
				data_tx[0] =  *send_counter & 0b00001111;
			}
			/* We have finished accessing the shared resource.  Release the
			semaphore. */
			xSemaphoreGive( xSemaphore_TX );
		}
		else
		{
			/*
			 * Failed to take the semaphore (timeout occurred)
			 */
		}
	}
}

/*
**====================================================================================
** interface to configure the CAN driver
**====================================================================================
*/
void candrv_configure(const candrv_Config* can_config)
{
	/*
	 * Check if semaphore is created
	 */
	if (xSemaphore_CTRL != NULL) {
		if (xSemaphoreTake(xSemaphore_CTRL, (TickType_t) 10) == pdTRUE) {
			*((U16 *)CTRLREG) = ((can_config->act_tx_irq << 1) | (can_config->act_rx_irq << 0)) & 0x001F;
			/* We have finished accessing the shared resource.  Release the
			semaphore. */
			xSemaphoreGive( xSemaphore_CTRL );
		}
		else
		{
			/*
			 * Failed to take the semaphore (timeout occurred)
			 */
		}
	}

	/*
	 * suspend polling if IRQ is active
	 */
	if(can_config->act_tx_irq) suspend_task(poll_can_tx_task);
	if(can_config->act_rx_irq) suspend_task(poll_can_rx_task);

}

/*
**====================================================================================
** interface to start the CAN driver
**====================================================================================
*/
void candrv_start()
{
	/*
	 * Check if semaphore is created
	 */
	if (xSemaphore_CTRL != NULL) {
		if (xSemaphoreTake(xSemaphore_CTRL, (TickType_t) 10) == pdTRUE) {

			*((U16 *)CTRLREG) |= 0x0010;
			/* We have finished accessing the shared resource.  Release the
			semaphore. */
			xSemaphoreGive( xSemaphore_CTRL );
		}
		else
		{
			/*
			 * Failed to take the semaphore (timeout occurred)
			 */
		}
	}
}

/*
**====================================================================================
** interface to stop the CAN driver
**====================================================================================
*/
void candrv_stop(){
	/*
	 * Check if semaphore is created
	 */
	if (xSemaphore_CTRL != NULL) {
		if (xSemaphoreTake(xSemaphore_CTRL, (TickType_t) 10) == pdTRUE) {

			*((U16 *)CTRLREG) &= 0xFFEF;
			/* We have finished accessing the shared resource.  Release the
			semaphore. */
			xSemaphoreGive( xSemaphore_CTRL );
		}
		else
		{
			/*
			 * Failed to take the semaphore (timeout occurred)
			 */
		}
	}
}

/*
**====================================================================================
** interrupt handler for rx
**====================================================================================
*/
void IRQ_MESSAGE_AVAILABLE_RX(void)
{
	can_rx_available = TRUE;
}

/*
**====================================================================================
** interrupt handler for tx
**====================================================================================
*/
void IRQ_CAN_SEND_MESSAGE_TX(void)
{
	can_tx_available = TRUE;
}

/*
**====================================================================================
** handler to send a send counter value when the tx hardware is available.
**====================================================================================
*/

void tx_handler_task(void *pvParameters)
{
	while (1) {
		if(can_tx_available == FALSE){
			 vTaskDelay(pdMS_TO_TICKS(5));
			 continue;
		}
		/*
		 * Check if semaphore is created
		 */
		if (xSemaphore_CTRL != NULL) {
			if (xSemaphoreTake(xSemaphore_CTRL, (TickType_t) 10) == pdTRUE) {
				*((U16 *)CTRLREG) &= 0xFFFB;
				/* We have finished accessing the shared resource.  Release the
				semaphore. */
				xSemaphoreGive( xSemaphore_CTRL );
			}
			else
			{
				/*
				 * Failed to take the semaphore (timeout occurred)
				 */
			}
		}
		can_tx_available = FALSE;
		write_can_signals(&send_counter);
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

/*
**====================================================================================
** handler to read can signals and write them into the queue when the rx hardware is
** available.
**====================================================================================
*/
void rx_handler_task(void *pvParameters)
{
	candrv_rxSignals* can_msg_signals;
	while (1) {
		if(can_rx_available == FALSE){
			 vTaskDelay(pdMS_TO_TICKS(5));
			 continue;
		}
		/*
		 * Check if semaphore is created
		 */
		if (xSemaphore_CTRL != NULL) {
			if (xSemaphoreTake(xSemaphore_CTRL, (TickType_t) 10) == pdTRUE) {
				*((U16 *)CTRLREG) &= 0xFFF7;
				/* We have finished accessing the shared resource.  Release the
				semaphore. */
				xSemaphoreGive( xSemaphore_CTRL );
			}
			else
			{
				/*
				 * Failed to take the semaphore (timeout occurred)
				 */
			}
		}
		can_rx_available = FALSE;
		read_can_signals(can_msg_signals);
		/*
		 * Send the message to the queue
		 */
		if (xQueueSendFromISR(rxQueue, &can_msg_signals, NULL) != pdPASS) {
			/*
			 *  Handle failure to enqueue message
			 */
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

/*
**====================================================================================
** increase the SEND_COUNTER signal by one each time the FLAG2 signal changes.
**====================================================================================
*/
void increase_send_counter_app_task(void *pvParameters)
{

	candrv_rxSignals* can_msg_signals;
	static Bool last_flag2_state = FALSE;
	const TickType_t xDelay = pdMS_TO_TICKS(10);  // 10 ms delay
	while (1) {
		if (xQueueReceive(rxQueue, &can_msg_signals, portMAX_DELAY) == pdTRUE) {
			if (can_msg_signals->flag2 != last_flag2_state) {
				(*send_counter)++;
			}
			last_flag2_state = can_msg_signals->flag2;
		}
		// Delay for 10 ms
		vTaskDelay(xDelay);
	}
}

/*
**====================================================================================
** poll if a frame is present in the input buffer
**====================================================================================
*/
void poll_can_rx_task(void *pvParameters) {
	while (1) {
		if(can_rx_available == TRUE){
			 vTaskDelay(pdMS_TO_TICKS(5));
			 continue;
		}
		/*
		 * Check if semaphore is created
		 */
		if (xSemaphore_CTRL != NULL) {
			if (xSemaphoreTake(xSemaphore_CTRL, (TickType_t) 10) == pdTRUE) {

				if((*((U16 *)CTRLREG)>>2) & 1){
					can_rx_available = TRUE;
				}

				/* We have finished accessing the shared resource.  Release the
				semaphore. */
				xSemaphoreGive( xSemaphore_CTRL );
			}
			else
			{
				/*
				 * Failed to take the semaphore (timeout occurred)
				 */
			}
		}
		vTaskDelay(pdMS_TO_TICKS(1));
	}

}

/*
**====================================================================================
** poll if a frame is ready to be sent in the receive buffer.
**====================================================================================
*/
void poll_can_tx_task(void *pvParameters) {
	while (1) {
		if(can_tx_available == TRUE){
			 vTaskDelay(pdMS_TO_TICKS(5));
			 continue;
		}
		/*
		 * Check if semaphore is created
		 */
		if (xSemaphore_CTRL != NULL) {
			if (xSemaphoreTake(xSemaphore_CTRL, (TickType_t) 10) == pdTRUE) {

				if((*((U16 *)CTRLREG)>>3) & 1){
					can_tx_available = TRUE;
				}

				/* We have finished accessing the shared resource.  Release the
				semaphore. */
				xSemaphoreGive( xSemaphore_CTRL );
			}
			else
			{
				/*
				 * Failed to take the semaphore (timeout occurred)
				 */
			}
		}
		vTaskDelay(pdMS_TO_TICKS(1));
	}

}

/*
**====================================================================================
** example function to start the CAN driver in polling mode
**====================================================================================
*/
void start_can_polling()
{
	candrv_Config* can_config;
	can_config->act_rx_irq = 0;
	can_config->act_tx_irq = 0;

	init_candrv();
	candrv_configure(can_config);
	candrv_start();
	resume_task(poll_can_tx_task);
	resume_task(poll_can_rx_task);
	resume_task(rx_handler_task);
	resume_task(tx_handler_task);
	resume_task(increase_send_counter_app_task);
}

/*
**====================================================================================
** example function to stop the CAN driver
**====================================================================================
*/
void stop_can_polling_irq()
{
	suspend_task(increase_send_counter_app_task);
	suspend_task(rx_handler_task);
	suspend_task(tx_handler_task);
	suspend_task(poll_can_rx_task);
	suspend_task(poll_can_tx_task);
	candrv_stop();

}

/*
**====================================================================================
** example function to start the CAN driver in irq mode
**====================================================================================
*/
void start_can_irq()
{
	candrv_Config* can_config;
	can_config->act_rx_irq = 1;
	can_config->act_tx_irq = 1;

	init_candrv();
	candrv_configure(can_config);
	candrv_start();

	resume_task(rx_handler_task);
	resume_task(tx_handler_task);
	resume_task(increase_send_counter_app_task);
}
