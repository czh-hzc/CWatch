#ifndef __BLE_RX_H__
#define __BLE_RX_H__

#include <rtthread.h>

/* * 协议处理入口函数 
 * 接收来自蓝牙底层的数据并进行分发处理
 */
void ble_rx_handler(rt_uint8_t *data, rt_uint16_t len);

#endif /* __BLE_RX_H__ */