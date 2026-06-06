#include "Sensor_Thread.h"

/* 1. KHAI BÁO BIẾN THỰC SỰ (Không có chữ extern) */
float g_shared_temp = 0.0f;
float g_shared_humi = 0.0f;
float g_shared_eco2 = 0.0f;
float g_shared_tvoc = 0.0f;

/* 2. KHAI BÁO CÁC HÀM CALLBACK TRỐNG ĐỂ LIÊN KẾT FSP KHÔNG BÁO LỖI */
void zmod4xxx_comms_i2c_callback(rm_zmod4xxx_callback_args_t * p_args) { FSP_PARAMETER_NOT_USED(p_args); }
void zmod4xxx_irq_callback(rm_zmod4xxx_callback_args_t * p_args)       { FSP_PARAMETER_NOT_USED(p_args); }
void hs300x_callback(rm_hs300x_callback_args_t * p_args)               { FSP_PARAMETER_NOT_USED(p_args); }

void Sensor_Thread_entry(void *pvParameters) {
    FSP_PARAMETER_NOT_USED(pvParameters);

    /* Mở các Driver cảm biến */
    g_hs300x_sensor0.p_api->open(g_hs300x_sensor0.p_ctrl, g_hs300x_sensor0.p_cfg);
    g_zmod4xxx_sensor0.p_api->open(g_zmod4xxx_sensor0.p_ctrl, g_zmod4xxx_sensor0.p_cfg);

    /* Cấp nguồn và Reset theo Schematic */
    R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_06_PIN_00, BSP_IO_LEVEL_HIGH); // Nguồn HS3001
    R_IOPORT_PinWrite(&g_ioport_ctrl, BSP_IO_PORT_03_PIN_07, BSP_IO_LEVEL_HIGH); // Reset ZMOD
    vTaskDelay(pdMS_TO_TICKS(500));

    while (1) {
        rm_hs300x_raw_data_t hs_raw;
        rm_hs300x_data_t hs_final;

        rm_zmod4xxx_raw_data_t zm_raw;
        rm_zmod4xxx_iaq_2nd_data_t zm_final;

        /* 1. Đọc và tính toán HS3001 */
        g_hs300x_sensor0.p_api->read(g_hs300x_sensor0.p_ctrl, &hs_raw);
        g_hs300x_sensor0.p_api->dataCalculate(g_hs300x_sensor0.p_ctrl, &hs_raw, &hs_final);

        /* 2. Đọc và tính toán ZMOD4410 */
        g_zmod4xxx_sensor0.p_api->read(g_zmod4xxx_sensor0.p_ctrl, &zm_raw);
        g_zmod4xxx_sensor0.p_api->iaq2ndGenDataCalculate(g_zmod4xxx_sensor0.p_ctrl, &zm_raw, &zm_final);

        /* 3. Cập nhật vào két sắt Mutex */
        if (xSemaphoreTake(g_sensor_mutex, portMAX_DELAY) == pdTRUE) {
            g_shared_temp = (float)hs_final.temperature.integer_part + (float)hs_final.temperature.decimal_part / 100.0f;
            g_shared_humi = (float)hs_final.humidity.integer_part + (float)hs_final.humidity.decimal_part / 100.0f;

            g_shared_eco2 = zm_final.eco2;
            g_shared_tvoc = zm_final.tvoc;
            xSemaphoreGive(g_sensor_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
