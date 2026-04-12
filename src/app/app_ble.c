#include <rtthread.h>
#include <string.h>
#include <stdlib.h>
#include "bf0_ble_gap.h"
#include "bf0_sibles.h"
#include "bf0_sibles_advertising.h"
#include "ble_connection_manager.h"
#include "app_ble.h"
#include "RTC_T.h"
#include "ble_rx.h"

static rt_mailbox_t g_app_mb;                   
static rt_thread_t  g_ble_thread;               
static rt_uint16_t  g_remote_cts_hdl = 0;       
static rt_uint16_t  g_target_char_handle = 0;   

SIBLES_ADVERTISING_CONTEXT_DECLAR(g_my_adv_context);

#define SVC_UUID {0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} // 128-bit UUID for advertising
#define CHAR_UUID {0xAB, 0xCD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} // 128-bit UUID for advertising
#define SERIAL_UUID_16(x)   {((rt_uint8_t)(x&0xff)),((rt_uint8_t)(x>>8))}

static rt_uint8_t g_svc_uuid[] = SVC_UUID;
static rt_uint8_t g_char_uuid[] = CHAR_UUID;

static rt_uint8_t char_data[128] = {0x11};
static rt_uint16_t char_len = 1;

// ====================================================================
// [模块一] GATT 服务端 
// ====================================================================
enum att_db_list
{
    SVC_IDX,            // 第 0 行：服务声明
    CHAR_DECL_IDX,      // 第 1 行：特征值声明
    CHAR_VALUE_IDX,     // 第 2 行：特征值
    ATT_MAX_NB          // ATT 数据库总行数
};

struct attm_desc_128 att_db[ATT_MAX_NB] =
{
    [SVC_IDX] = {SERIAL_UUID_16(ATT_DECL_PRIMARY_SERVICE), PERM(RD, ENABLE), 0, 0},
    [CHAR_DECL_IDX] = {SERIAL_UUID_16(ATT_DECL_CHARACTERISTIC), PERM(RD, ENABLE), 0, 0},
    [CHAR_VALUE_IDX] = {CHAR_UUID, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE) | PERM(WRITE_COMMAND, ENABLE), PERM(UUID_LEN, UUID_128) | PERM(RI, ENABLE), 128},
};

static rt_uint8_t *gatts_get_cbk(rt_uint8_t conn_idx, rt_uint8_t idx, rt_uint16_t *len)
{
    if(idx == CHAR_VALUE_IDX) 
    {
        *len = char_len;
        return char_data;  
    }
    *len = 0;
    return NULL;
}

static rt_uint8_t gatts_set_cbk(rt_uint8_t conn_idx, sibles_set_cbk_t *para)
{
    if (para->idx == CHAR_VALUE_IDX) 
    {
        rt_kprintf("Received data\r\n");

        ble_rx_handler(para->value, para->len);

        rt_uint16_t copy_len = para->len > 128 ? 128 : para->len;
        rt_memcpy(char_data, para->value, copy_len);
        char_len = copy_len;
    }
    return 0;
}

static void service_init(void)
{
    sibles_register_svc_128_t svc = {0};
    svc.att_db = (struct attm_desc_128 *)&att_db;
    svc.num_entry = ATT_MAX_NB;
    svc.sec_lvl = PERM(SVC_UUID_LEN, UUID_128) | PERM(SVC_MI, ENABLE);
    svc.uuid = g_svc_uuid;

    sibles_hdl srv_handle = sibles_register_svc_128(&svc);
    if (srv_handle) 
    {
        sibles_register_cbk(srv_handle, gatts_get_cbk, gatts_set_cbk);
        rt_kprintf("Custom GATT Service registered.\r\n");
    }
}

// ===================================================================
// [模块二] 广播模块 (呼叫手机)
// ===================================================================

static rt_uint8_t adv_event_handler(rt_uint8_t event, void *context, void *data) 
{ 
    return 0; 
}

static void start_adv(void)
{
    sibles_advertising_para_t para = {0};

    static const char device_name[] = "CWatch";
    rt_uint8_t name_len = strlen(device_name);
    rt_uint8_t svc_uuid_len = sizeof(g_svc_uuid);

    ble_gap_dev_name_t *dev_name = rt_malloc(sizeof(ble_gap_dev_name_t) + name_len);
    if (dev_name)
    {
        dev_name->len = name_len;
        rt_memcpy(dev_name->name, device_name, name_len);
        ble_gap_set_dev_name(dev_name);
        rt_free(dev_name);
    }

    para.own_addr_type = GAPM_STATIC_ADDR;               // 使用静态 MAC 地址（不乱变）
    para.config.adv_mode = SIBLES_ADV_CONNECT_MODE;      // 模式：这是一个【可以被连接】的广播
    para.config.mode_config.conn_config.duration = 0;    // 持续时间：0 代表永远发广播，不休息
    para.config.mode_config.conn_config.interval = 0x30; // 0x30 * 0.625ms = 30ms 广播间隔
    para.config.max_tx_pwr = 0x7F;                       // 使用系统最大发射功率，信号拉满
    para.config.is_auto_restart = 1;                     // 如果手机连上后又断开了，自动重新开始广播

    para.adv_data.disc_mode = GAPM_ADV_MODE_GEN_DISC;
    para.adv_data.completed_name = rt_malloc(name_len + sizeof(sibles_adv_type_name_t));
    if (para.adv_data.completed_name)
    {
        para.adv_data.completed_name->name_len = name_len;
        rt_memcpy(para.adv_data.completed_name->name, device_name, name_len);
    }

    para.adv_data.completed_uuid = rt_malloc(sizeof(sibles_adv_type_srv_uuid_t) + sizeof(sibles_adv_uuid_t));
    if (para.adv_data.completed_uuid)
    {
        para.adv_data.completed_uuid->count = 1;
        para.adv_data.completed_uuid->uuid_list[0].uuid_len = svc_uuid_len;
        rt_memcpy(para.adv_data.completed_uuid->uuid_list[0].uuid.uuid_128, g_svc_uuid, svc_uuid_len);
    }

    para.evt_handler = adv_event_handler;

    if (sibles_advertising_init(g_my_adv_context, &para) == SIBLES_ADV_NO_ERR) 
    {
        sibles_advertising_start(g_my_adv_context);
        rt_kprintf("Broadcast started with device name: %s\r\n", device_name);
    } 
    else 
    {
        rt_kprintf("Failed to start broadcast\r\n");
    }

    if (para.adv_data.completed_name) rt_free(para.adv_data.completed_name);
    if (para.adv_data.completed_uuid) rt_free(para.adv_data.completed_uuid);
}

// ====================================================================
// [模块三] GATT 客户端
// ====================================================================

static rt_uint16_t find_char_handle_by_uuid(sibles_svc_remote_svc_t *svc, rt_uint16_t target_uuid)
{
    rt_uint32_t i; rt_uint16_t offset = 0;
    sibles_svc_search_char_t *chara = (sibles_svc_search_char_t *)svc->att_db;

    for (i = 0; i < svc->char_count; i++) 
    {
        if (chara->uuid_len == 2) 
        {
            if ((chara->uuid[0] | (chara->uuid[1] << 8)) == target_uuid) 
            {
                return chara->pointer_hdl; 
            }
        }
        offset = sizeof(sibles_svc_search_char_t) + chara->desc_count * sizeof(struct sibles_disc_char_desc_ind);
        chara = (sibles_svc_search_char_t *)((uint8_t *)chara + offset);
    }
    return 0; 
}

static int gatt_client_event_handler(rt_uint16_t event_id, rt_uint8_t *data, rt_uint16_t len)
{
    if (event_id == SIBLES_READ_REMOTE_VALUE_RSP) 
    {
        sibles_read_remote_value_rsp_t *rsp = (sibles_read_remote_value_rsp_t *)data;
        
        if (rsp->handle == g_target_char_handle && rsp->length == 10) 
        {
            rt_uint16_t year = rsp->value[0] | (rsp->value[1] << 8); 
            // 第 2 字节：月份
            rt_uint8_t month = rsp->value[2];
            // 第 3 字节：日期
            rt_uint8_t day   = rsp->value[3];
            // 第 4 字节：小时
            rt_uint8_t hour  = rsp->value[4];
            // 第 5 字节：分钟
            rt_uint8_t min   = rsp->value[5];
            // 第 6 字节：秒数
            rt_uint8_t sec   = rsp->value[6];
            // 第 7 字节：星期几
            rt_uint8_t weekday = rsp->value[7]; 
            // 完美！把拿到的手机时间打印出来！
            rt_kprintf("current time: %04d-%02d-%02d %02d:%02d:%02d\r\n", year, month, day, hour, min, sec);
            rt_kprintf("weekday: %d\r\n", weekday);

            RTC_SetTime(year, month, day, hour, min, sec);
        }

    }
    return  0;
}

// ====================================================================
// [模块四] 全局事件拦截中心 (全盘监控！)
// ====================================================================

int ble_app_event_handler(rt_uint16_t event_id, rt_uint8_t *data, rt_uint16_t len, uint32_t context)
{
    rt_kprintf("Caught ID: %d\r\n", event_id);

    switch(event_id)
    {
        case BLE_POWER_ON_IND:
        {
            rt_kprintf("BLE Powered On. Starting service and advertising...\r\n");
            if(g_app_mb)
            {
                rt_mb_send(g_app_mb, BLE_POWER_ON_IND);
            }
            break;
        }

        case BLE_GAP_CONNECTED_IND:
        {
            connection_manager_connect_ind_t *ind = (connection_manager_connect_ind_t *)data;
            rt_kprintf("Connected! Searching for CTS on phone...\r\n");

            rt_uint8_t cts_uuid[2] = {0x05, 0x18}; // CTS 的 16-bit UUID 是 0x1805
            sibles_search_service(ind->conn_idx, 2, cts_uuid);
            break;
        }   
        
        case SIBLES_SEARCH_SVC_RSP:
        {
            sibles_svc_search_rsp_t *rsp = (sibles_svc_search_rsp_t *)data;

            if (rsp->result == 0 && rsp->svc != NULL) 
            {
                rt_kprintf("CTS found! Registering client...\r\n");

                g_remote_cts_hdl = sibles_register_remote_svc(rsp->conn_idx, rsp->svc->hdl_start, rsp->svc->hdl_end, gatt_client_event_handler);
                g_target_char_handle = find_char_handle_by_uuid(rsp->svc, 0x2A2B); // CTS 的 Current Time Characteristic UUID 是 0x2A2B
                
                if (g_target_char_handle) 
                {
                    rt_kprintf("Reading time (0x2A2B) Handle: %d...\r\n", g_target_char_handle);
                    sibles_read_remote_value_req_t read_req = {SIBLES_READ, g_target_char_handle, 0, 0};
                    sibles_read_remote_value(g_remote_cts_hdl, rsp->conn_idx, &read_req);
                }
            } 
            else 
            {
                rt_kprintf("CTS NOT found on phone.\r\n");
            }
            break;
        }

        case BLE_GAP_DISCONNECTED_IND:
        {
            rt_kprintf("Disconnected.\r\n");
            g_remote_cts_hdl = 0; 
            g_target_char_handle = 0;
            break;
        }
    }
    return 0;

}

BLE_EVENT_REGISTER(ble_app_event_handler, NULL);

// ====================================================================
// [模块五] 蓝牙专属线程
// ====================================================================

static void ble_app_thread_entry(void *parameter)
{
    rt_uint32_t msg_value;
    
    sifli_ble_enable();

    while(1)
    {
        if(rt_mb_recv(g_app_mb, &msg_value, RT_WAITING_FOREVER) == RT_EOK)
        {
            if(msg_value == BLE_POWER_ON_IND)
            {
                service_init();
                start_adv();
            }
        }
    }
}

void ble_app_init(void)
{
    g_app_mb = rt_mb_create("ble_mb", 8, RT_IPC_FLAG_FIFO);
    if (g_app_mb == NULL) 
    {
        rt_kprintf("Failed to create mailbox\r\n");
        return;
    }

    g_ble_thread = rt_thread_create("ble_thread", ble_app_thread_entry, NULL, 2048, 10, 10);
    if (g_ble_thread != RT_NULL) 
    {
        rt_thread_startup(g_ble_thread);
    } 
    else 
    {
        rt_kprintf("Failed to create thread\r\n");
        return;
    }

}
