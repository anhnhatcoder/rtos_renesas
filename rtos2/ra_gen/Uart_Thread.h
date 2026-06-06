/* generated thread header file - do not edit */
#ifndef UART_THREAD_H_
#define UART_THREAD_H_
#include "bsp_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hal_data.h"
#ifdef __cplusplus
                extern "C" void Uart_Thread_entry(void * pvParameters);
                #else
extern void Uart_Thread_entry(void *pvParameters);
#endif
#include "r_rtc.h"
#include "r_rtc_api.h"
#include "r_sci_uart.h"
#include "r_uart_api.h"
FSP_HEADER
/* RTC Instance. */
extern const rtc_instance_t g_rtc0;

/** Access the RTC instance using these structures when calling API functions directly (::p_api is not used). */
extern rtc_instance_ctrl_t g_rtc0_ctrl;
extern const rtc_cfg_t g_rtc0_cfg;

#ifndef NULL
void NULL(rtc_callback_args_t *p_args);
#endif
/** UART on SCI Instance. */
extern const uart_instance_t g_uart3;

/** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
extern sci_uart_instance_ctrl_t g_uart3_ctrl;
extern const uart_cfg_t g_uart3_cfg;
extern const sci_uart_extended_cfg_t g_uart3_cfg_extend;

#ifndef user_uart_callback
void user_uart_callback(uart_callback_args_t *p_args);
#endif
FSP_FOOTER
#endif /* UART_THREAD_H_ */
