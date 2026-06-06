
#include "Uart_Thread.h"
#include <stdio.h>
#include <string.h>

extern SemaphoreHandle_t g_sensor_mutex;
extern float g_shared_temp;
extern float g_shared_humi;
extern float g_shared_eco2;
extern float g_shared_tvoc;


volatile bool uart_tx_complete = false;
volatile bool has_synced_rtc = false;

char esp_time[32] = "notime";
char uart_rx_buf[64];
unsigned int rx_index = 0;


void sync_time_to_rtc(char * time_str) {
    rtc_time_t set_time = {0};
    int year, month, day, hour, minute, second;

    if (sscanf(time_str, "%d/%d/%d-%d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
        set_time.tm_year = year - 1900;
        set_time.tm_mon  = month - 1;
        set_time.tm_mday = day;
        set_time.tm_hour = hour;
        set_time.tm_min  = minute;
        set_time.tm_sec  = second;
        R_RTC_CalendarTimeSet(&g_rtc0_ctrl, &set_time);
        has_synced_rtc = true;
    }
}


void user_uart_callback(uart_callback_args_t *p_args) {
    if (UART_EVENT_TX_COMPLETE == p_args->event) {
        uart_tx_complete = true;
    }
    else if (UART_EVENT_RX_CHAR == p_args->event) {
        char c = (char)p_args->data;
        if (c == '\n' || c == '\r') {
            if (rx_index > 0) {
                uart_rx_buf[rx_index] = '\0';
                sync_time_to_rtc(uart_rx_buf);
                rx_index = 0;
            }
        } else {
            if (rx_index < sizeof(uart_rx_buf) - 1) {
                uart_rx_buf[rx_index++] = c;
            }
        }
    }
}

void Uart_Thread_entry(void *pvParameters) {
    FSP_PARAMETER_NOT_USED(pvParameters);

    R_SCI_UART_Open(&g_uart3_ctrl, &g_uart3_cfg);
    R_RTC_Open(&g_rtc0_ctrl, &g_rtc0_cfg);

    char tx_msg[128];
    rtc_time_t get_time;


    float temp_copy = 0.0;
    float humi_copy = 0.0;
    float eco2_copy = 0.0; // Thêm dòng này
    float tvoc_copy = 0.0;

    while (1) {

        if (xSemaphoreTake(g_sensor_mutex, portMAX_DELAY) == pdTRUE) {
            temp_copy = g_shared_temp;
            humi_copy = g_shared_humi;
            eco2_copy = g_shared_eco2; // Thêm dòng này
            tvoc_copy = g_shared_tvoc;
            xSemaphoreGive(g_sensor_mutex);
        }


        if (has_synced_rtc) {
            R_RTC_CalendarTimeGet(&g_rtc0_ctrl, &get_time);
            sprintf(esp_time, "%04d/%02d/%02d-%02d:%02d:%02d",
                    get_time.tm_year + 1900, get_time.tm_mon + 1, get_time.tm_mday,
                    get_time.tm_hour, get_time.tm_min, get_time.tm_sec);
        }


        sprintf(tx_msg, "{\"time\":\"%s\", \"temp\":%.2f, \"hum\":%.2f, \"eco2\":%.2f, \"tvoc\":%.2f}\r\n",
                esp_time, temp_copy, humi_copy, eco2_copy, tvoc_copy);

        uart_tx_complete = false;
        R_SCI_UART_Write(&g_uart3_ctrl, (uint8_t *)tx_msg, strlen(tx_msg));
        while (!uart_tx_complete) { vTaskDelay(1); }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
