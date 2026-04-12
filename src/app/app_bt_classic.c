#include <rtthread.h>
#include <string.h>
#include "bts2_app_inc.h"
#include "avrcp_api.h"
#include "sc_api.h"

static const char g_bt_local_name[] = "CWatch Audio";

static int bt_classic_event_handler(uint16_t type, uint16_t event_id, uint8_t *data, uint16_t data_len)
{
    if (type == BT_NOTIFY_COMMON)
    {
        switch (event_id)
        {
        case BT_NOTIFY_COMMON_BT_STACK_READY:
            bt_interface_set_local_name(strlen(g_bt_local_name), (void *)g_bt_local_name);
            bt_interface_set_scan_mode(TRUE, TRUE);
            rt_kprintf("[BT Audio] stack ready, device discoverable: %s\r\n", g_bt_local_name);
            break;

        case BT_NOTIFY_COMMON_USER_CONFIRM_IND:
        {
            bt_notify_pair_confirm_t *confirm_info = (bt_notify_pair_confirm_t *)data;
            bt_interface_user_confirm_res(confirm_info->mac.addr, 1);
            rt_kprintf("[BT Audio] pair confirm accepted\r\n");
            break;
        }

        case BT_NOTIFY_COMMON_ACL_CONNECTED:
        {
            bt_notify_device_acl_conn_info_t *acl_info = (bt_notify_device_acl_conn_info_t *)data;
            rt_kprintf("[BT Audio] ACL connected, res=%d, cod=0x%06x\r\n", acl_info->res, acl_info->dev_cls);
            break;
        }

        case BT_NOTIFY_COMMON_ACL_DISCONNECTED:
        {
            bt_notify_device_base_info_t *device_info = (bt_notify_device_base_info_t *)data;
            rt_kprintf("[BT Audio] ACL disconnected, res=%d\r\n", device_info->res);
            break;
        }

        default:
            break;
        }
    }
    else if (type == BT_NOTIFY_A2DP)
    {
        switch (event_id)
        {
        case BT_NOTIFY_A2DP_PROFILE_CONNECTED:
        {
            bt_notify_profile_state_info_t *profile_info = (bt_notify_profile_state_info_t *)data;
            rt_kprintf("[BT Audio] A2DP connected, res=%d\r\n", profile_info->res);
            break;
        }

        case BT_NOTIFY_A2DP_PROFILE_DISCONNECTED:
        {
            bt_notify_profile_state_info_t *profile_info = (bt_notify_profile_state_info_t *)data;
            rt_kprintf("[BT Audio] A2DP disconnected, res=%d\r\n", profile_info->res);
            break;
        }

        default:
            break;
        }
    }
    else if (type == BT_NOTIFY_AVRCP)
    {
        switch (event_id)
        {
        case BT_NOTIFY_AVRCP_PROFILE_CONNECTED:
        {
            bt_notify_profile_state_info_t *profile_info = (bt_notify_profile_state_info_t *)data;
            bt_interface_set_avrcp_role_ext(&profile_info->mac, AVRCP_CT);
            rt_kprintf("[BT Audio] AVRCP connected, res=%d\r\n", profile_info->res);
            break;
        }

        case BT_NOTIFY_AVRCP_PROFILE_DISCONNECTED:
        {
            bt_notify_profile_state_info_t *profile_info = (bt_notify_profile_state_info_t *)data;
            rt_kprintf("[BT Audio] AVRCP disconnected, res=%d\r\n", profile_info->res);
            break;
        }

        case BT_NOTIFY_AVRCP_VOLUME_CHANGED_REGISTER:
            rt_kprintf("[BT Audio] AVRCP absolute volume registered\r\n");
            break;

        case BT_NOTIFY_AVRCP_ABSOLUTE_VOLUME:
        {
            uint8_t *volume = (uint8_t *)data;
            rt_kprintf("[BT Audio] AVRCP absolute volume=%d\r\n", *volume);
            break;
        }

        default:
            break;
        }
    }

    return 0;
}

void bt_sc_io_capability_rsp(BTS2S_BD_ADDR *bd)
{
    bt_io_capability_rsp(bd, IO_CAPABILITY_NO_INPUT_NO_OUTPUT, FALSE, TRUE);
}

void bt_classic_app_init(void)
{
    bt_interface_register_bt_event_notify_callback(bt_classic_event_handler);
}
