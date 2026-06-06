#include "new_thread0.h"
#include <stdio.h>
#include <string.h>

/* =========================================================================
 * 1. KHAI BÁO BIẾN TOÀN CỤC
 * ========================================================================= */
volatile bool uart_tx_complete = false;
volatile bool i2c_transfer_complete = false;
volatile bool has_synced_rtc = false; // Cờ báo hiệu đã nhận giờ từ ESP32

char esp_time[32] = "notime"; // Chứa thời gian hiển thị
char uart_rx_buf[64];         // Đệm nhận dữ liệu từ ESP32
int rx_index = 0;

float temperature = 0.0;
float humidity = 0.0;


/* =========================================================================
 * 2. HÀM ĐỒNG BỘ THỜI GIAN VÀO RTC
 * Cập nhật mỗi 30s khi nhận được chuỗi định dạng: YYYY/MM/DD-HH:MM:SS\n
 * ========================================================================= */
void sync_time_to_rtc(char * time_str) {
    rtc_time_t set_time = {0};
    int year, month, day, hour, minute, second;

    // Tách chuỗi 6 thành phần bao gồm cả Giây
    if (sscanf(time_str, "%d/%d/%d-%d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
        set_time.tm_year = year - 1900; // FSP yêu cầu năm tính từ 1900
        set_time.tm_mon  = month - 1;   // FSP yêu cầu tháng từ 0 đến 11
        set_time.tm_mday = day;
        set_time.tm_hour = hour;
        set_time.tm_min  = minute;
        set_time.tm_sec  = second;

        // Ghi đè vào chip Renesas (Triệt tiêu sai số của LOCO)
        R_RTC_CalendarTimeSet(&g_rtc0_ctrl, &set_time);
        has_synced_rtc = true;
    }
}


/* =========================================================================
 * 3. CÁC HÀM CALLBACK CỦA NGOẠI VI
 * ========================================================================= */
void user_uart_callback(uart_callback_args_t *p_args) {
    if (UART_EVENT_TX_COMPLETE == p_args->event) {
        uart_tx_complete = true;
    }
    else if (UART_EVENT_RX_CHAR == p_args->event) {
        char c = (char)p_args->data;

        // Khi nhận được ký tự Enter từ ESP32
        if (c == '\n' || c == '\r') {
            if (rx_index > 0) {
                uart_rx_buf[rx_index] = '\0'; // Đóng chuỗi
                sync_time_to_rtc(uart_rx_buf); // Nạp ngay vào RTC
                rx_index = 0; // Reset để nhận chuỗi tiếp theo sau 30s
            }
        } else {
            // Nạp từng ký tự vào buffer
            if (rx_index < sizeof(uart_rx_buf) - 1) {
                uart_rx_buf[rx_index++] = c;
            }
        }
    }
}

void i2c_master_callback(i2c_master_callback_args_t *p_args) {
    if (I2C_MASTER_EVENT_TX_COMPLETE == p_args->event ||
        I2C_MASTER_EVENT_RX_COMPLETE == p_args->event ||
        I2C_MASTER_EVENT_ABORTED == p_args->event) {
        i2c_transfer_complete = true;
    }
}


/* =========================================================================
 * 4. LUỒNG THỰC THI CHÍNH (RTOS THREAD)
 * ========================================================================= */
/* =========================================================================
 * 4. LUỒNG THỰC THI CHÍNH (RTOS THREAD) - CẬP NHẬT SENSOR MỚI
 * ========================================================================= */
void new_thread0_entry(void *pvParameters) {
    FSP_PARAMETER_NOT_USED(pvParameters);

    /* Khởi tạo phần cứng */
    R_SCI_UART_Open(&g_uart3_ctrl, &g_uart3_cfg);
    R_IIC_MASTER_Open(&g_i2c_master0_ctrl, &g_i2c_master0_cfg);
    R_RTC_Open(&g_rtc0_ctrl, &g_rtc0_cfg);

    // Bật nguồn cảm biến (Nếu board mạch của bạn yêu cầu kích nguồn qua chân P600 giống code cũ)
    R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_06_PIN_00, BSP_IO_LEVEL_HIGH);
    vTaskDelay(pdMS_TO_TICKS(500)); // Chờ cảm biến khởi động

    uint8_t sensor_data[4]; // Chỉ đọc 4 byte theo đúng code cũ
    char tx_msg[128];
    rtc_time_t get_time;

    while (1) {
        /* --- BƯỚC 1: ĐỌC CẢM BIẾN (Theo logic chuẩn của code cũ) --- */
        i2c_transfer_complete = false;

        // Kích hoạt đo bằng cách ghi 0 byte (NULL)
        R_IIC_MASTER_Write(&g_i2c_master0_ctrl, NULL, 0, false);
        while (!i2c_transfer_complete) { vTaskDelay(1); }

        // Chờ 100ms để cảm biến đo xong (Giống code cũ của bạn)
        vTaskDelay(pdMS_TO_TICKS(100));

        i2c_transfer_complete = false;
        // Đọc 4 byte dữ liệu về
        R_IIC_MASTER_Read(&g_i2c_master0_ctrl, sensor_data, 4, false);
        while (!i2c_transfer_complete) { vTaskDelay(1); }

        // Xử lý bitwise chuẩn xác theo code cũ
        uint16_t h_raw = (uint16_t)(((sensor_data[0] & 0x3F) << 8) | sensor_data[1]);
        uint16_t t_raw = (uint16_t)((sensor_data[2] << 6) | (sensor_data[3] >> 2));

        humidity = (float)h_raw * 100.0f / 16383.0f;
        temperature = (float)t_raw * 165.0f / 16383.0f - 40.0f;


        /* --- BƯỚC 2: LẤY THỜI GIAN THỰC TỪ RTC --- */
        if (has_synced_rtc) {
            R_RTC_CalendarTimeGet(&g_rtc0_ctrl, &get_time);
            sprintf(esp_time, "%04d/%02d/%02d-%02d:%02d:%02d",
                    get_time.tm_year + 1900,
                    get_time.tm_mon + 1,
                    get_time.tm_mday,
                    get_time.tm_hour,
                    get_time.tm_min,
                    get_time.tm_sec);
        }


        /* --- BƯỚC 3: ĐÓNG GÓI JSON & GỬI UART --- */
        sprintf(tx_msg, "{\"time\":\"%s\", \"temp\":%.2f, \"hum\":%.2f}\r\n", esp_time, temperature, humidity);

        uart_tx_complete = false;
        R_SCI_UART_Write(&g_uart3_ctrl, (uint8_t *)tx_msg, strlen(tx_msg));
        while (!uart_tx_complete) { vTaskDelay(1); }

        /* --- BƯỚC 4: NGHỈ NGƠI --- */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
