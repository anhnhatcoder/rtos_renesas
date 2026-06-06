/* generated thread header file - do not edit */
#ifndef SENSOR_THREAD_H_
#define SENSOR_THREAD_H_
#include "bsp_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "hal_data.h"
#ifdef __cplusplus
                extern "C" void Sensor_Thread_entry(void * pvParameters);
                #else
extern void Sensor_Thread_entry(void *pvParameters);
#endif
#include "rm_comms_i2c.h"
#include "rm_comms_api.h"
#include "../ra/fsp/src/rm_zmod4xxx/zmod4xxx_types.h"
#include "../ra/fsp/src/rm_zmod4xxx/iaq_2nd_gen/iaq_2nd_gen.h"
#include "rm_zmod4xxx.h"
#include "rm_zmod4xxx_api.h"
#include "rm_hs300x.h"
#include "rm_hs300x_api.h"
FSP_HEADER
/* I2C Communication Device */
extern const rm_comms_instance_t g_comms_i2c_device1;
extern rm_comms_i2c_instance_ctrl_t g_comms_i2c_device1_ctrl;
extern const rm_comms_cfg_t g_comms_i2c_device1_cfg;
#ifndef rm_zmod4xxx_comms_i2c_callback
void rm_zmod4xxx_comms_i2c_callback(rm_comms_callback_args_t *p_args);
#endif
/* ZMOD4410 IAQ 2nd Gen. */
extern rm_zmod4xxx_lib_extended_cfg_t g_zmod4xxx_sensor0_extended_cfg;
/* ZMOD4XXX Sensor */
extern const rm_zmod4xxx_instance_t g_zmod4xxx_sensor0;
extern rm_zmod4xxx_instance_ctrl_t g_zmod4xxx_sensor0_ctrl;
extern const rm_zmod4xxx_cfg_t g_zmod4xxx_sensor0_cfg;
#ifndef zmod4xxx_comms_i2c_callback
void zmod4xxx_comms_i2c_callback(rm_zmod4xxx_callback_args_t *p_args);
#endif
#ifndef rm_zmod4xxx_irq_callback
void rm_zmod4xxx_irq_callback(external_irq_callback_args_t *p_args);
#endif
#ifndef zmod4xxx_irq_callback
void zmod4xxx_irq_callback(rm_zmod4xxx_callback_args_t *p_args);
#endif
/* I2C Communication Device */
extern const rm_comms_instance_t g_comms_i2c_device0;
extern rm_comms_i2c_instance_ctrl_t g_comms_i2c_device0_ctrl;
extern const rm_comms_cfg_t g_comms_i2c_device0_cfg;
#ifndef rm_hs300x_callback
void rm_hs300x_callback(rm_comms_callback_args_t *p_args);
#endif
/* HS300X Sensor */
extern const rm_hs300x_instance_t g_hs300x_sensor0;
extern rm_hs300x_instance_ctrl_t g_hs300x_sensor0_ctrl;
extern const rm_hs300x_cfg_t g_hs300x_sensor0_cfg;
#ifndef hs300x_callback
void hs300x_callback(rm_hs300x_callback_args_t *p_args);
#endif
FSP_FOOTER
#endif /* SENSOR_THREAD_H_ */
