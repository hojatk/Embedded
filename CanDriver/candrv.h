/*
**====================================================================================
** Imported Public definitions
**====================================================================================
*/
#include "typedefs.h"
#include "FreeRTOS"
#include "semphr.h"
#include "queue.h"
#include "task.h"

/*
 **===================================================================================
 ** Macro definitions
 **===================================================================================
 */

#define candrv_MAX_DATA_LENGTH 8U
#define QUEUE_LENGTH 10  // Number of items the queue can hold
#define QUEUE_ITEM_SIZE sizeof(candrv_rxSignals)
#define TASK_STACK_SIZE 100
/*
**====================================================================================
** Register definitions for CanDriver
**====================================================================================
*/

#ifndef CAN_REGISTERS_H
#define CAN_REGISTERS_H

extern volatile void* CTRLREG;
extern volatile void* DATA_RX;
extern volatile void* DATA_TX;

#endif // CAN_REGISTERS_H

/*
**====================================================================================
** Public type definitions for CanDriver
**====================================================================================
*/

typedef enum
{
   candrv_DLC_0_BYTES  = 0U,  /**< Zero bytes. */
   candrv_DLC_1_BYTES  = 1U,  /**< One byte. */
   candrv_DLC_2_BYTES  = 2U,  /**< Two bytes. */
   candrv_DLC_3_BYTES  = 3U,  /**< Three bytes. */
   candrv_DLC_4_BYTES  = 4U,  /**< Four bytes. */
   candrv_DLC_5_BYTES  = 5U,  /**< Five bytes. */
   candrv_DLC_6_BYTES  = 6U,  /**< Six bytes. */
   candrv_DLC_7_BYTES  = 7U,  /**< Seven bytes. */
   candrv_DLC_8_BYTES  = 8U,  /**< Eight bytes. */
} candrv_DataLength;

typedef struct
{
	Bool act_tx_irq;
	Bool act_rx_irq;

} candrv_Config;


typedef struct
{
   U16 id;
   U8 dlc;
   U8 can_msg[candrv_MAX_DATA_LENGTH];
} candrv_message;

typedef struct
{
   U8 status_ecu;
   U16 value_example;
   U8 value_example2;
   Bool flag1;
   Bool flag2;
   Bool flag3;
} candrv_rxSignals;

QueueHandle_t rxQueue;

static Bool can_rx_available = FALSE;
static Bool can_tx_available = FALSE;
static U8 send_counter = 0;

/*
**====================================================================================
** Function prototype declarations for CanDriver external access
**====================================================================================
*/
extern void IRQ_MESSAGE_AVAILABLE_RX(void);
extern void IRQ_CAN_SEND_MESSAGE_TX(void);
extern void read_can_signals(candrv_rxSignals* can_msg_signals);
extern void write_can_signals(U8* send_counter);
extern void candrv_configure(const candrv_Config* can_config);
extern void candrv_start();
extern void candrv_stop();
extern void rx_handler_task(void *pvParameters);
extern void tx_handler_task(void *pvParameters);
extern void increase_send_counter_app_task(void *pvParameters);
extern void poll_can_rx_task(void *pvParameters);
extern void poll_can_tx_task(void *pvParameters);
extern void start_can_polling();
extern void stop_can_polling();
extern void start_can_irq();
extern void stop_can_irq();


