/**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: command.c
**
** ��   ��   ��: �Ž���
**
** �ļ���������: 2017 �� 1 �� 31 ��
**
** ��        ��: ��stm32ͨ�ŵ����

** ��־:
2017.01.31  �������ļ�
*********************************************************************************************************/
#include "command.h"
#include "user_hspi.h"
#include "mesh_global.h"

#include "osapi.h"
#include "mesh_device.h"

char sdk_version[10];
int sdk_version_len = 0;

uint8  send_buffer[1024*5] = {0}; /* ���ͻ����� */
char *err_str;

extern void mesh_init (void);

void command_execute(void)
{
	struct ip_info info;
	char          *recv = NULL;
	uint8_t        hspi_send_back = 1;
    
	os_printf("cmd: %d, lenth: %d, crc: %08X\r\n", wifi_pack_recv.cmd, wifi_pack_recv.lenth, wifi_pack_recv.crc);
	switch (wifi_pack_recv.cmd) {
			
	/* ֱ�ӷ��ؽ��յ������� */
	case CMD_RETURN_RECV:
		wifi_pack_send.lenth = wifi_pack_recv.lenth;
		wifi_pack_send.data = wifi_pack_recv.data;
	    break;
		
	/* ��ȡsdk�汾�� */
	case CMD_GET_SDK_VERSION:
		if (sdk_version_len == 0)
		{
			char *recv = (char *)system_get_sdk_version();
			sdk_version_len = os_strlen(recv) + 1;
			os_memcpy(sdk_version, recv, sdk_version_len);
		}
		wifi_pack_send.lenth = sdk_version_len;
		wifi_pack_send.data = sdk_version;
	    break;
		
	/* ��ѯFlash size�Լ�Flash map */
	case CMD_GET_FLASH_SIZE_MAP:
		send_buffer[0] = system_get_flash_size_map();
		wifi_pack_send.lenth = 1;
		wifi_pack_send.data = send_buffer;
	    break;
		
	/* ��ѯIP��ַ */
	case CMD_WIFI_GET_IP_INFO:
		if(wifi_get_ip_info(STATION_IF, &info) == false)
		{
			err_str = "err: wifi_get_ip_info failed\r\n";
			wifi_pack_send.lenth = os_strlen(err_str) + 1;
			wifi_pack_send.data = err_str;
			break;
		}
		os_memcpy(send_buffer, &info, sizeof(struct ip_info));
		if(wifi_get_ip_info(SOFTAP_IF, &info) == false)
		{
			err_str = "err: wifi_get_ip_info failed\r\n";
			wifi_pack_send.lenth = os_strlen(err_str) + 1;
			wifi_pack_send.data = err_str;
			break;
		}
		os_memcpy(send_buffer + sizeof(struct ip_info), &info, sizeof(struct ip_info));
		wifi_pack_send.lenth = sizeof(struct ip_info)*2;
		wifi_pack_send.data = send_buffer;
	    break;
		
	/* ��ѯmac��ַ */
	case CMD_WIFI_GET_MACADDR:
		if(wifi_get_macaddr(STATION_IF, send_buffer) == false)
		{
			err_str = "err: wifi_get_macaddr failed\r\n";
			wifi_pack_send.lenth = os_strlen(err_str) + 1;
			wifi_pack_send.data = err_str;
			break;
		}
		if(wifi_get_macaddr(SOFTAP_IF, send_buffer + 6) == false)
		{
			err_str = "err: wifi_get_macaddr failed\r\n";
			wifi_pack_send.lenth = os_strlen(err_str) + 1;
			wifi_pack_send.data = err_str;
			break;
		}
		wifi_pack_send.lenth = 12;
		wifi_pack_send.data = send_buffer;
	    break;
		
	/* ��ȡmesh�豸�б� */
	case CMD_GET_DEVICE_LIST:
		if ((g_mesh_device_init == false)||(g_node_list.scale < 1))
		{
			err_str = "err: mesh device not init\r\n";
			wifi_pack_send.lenth = os_strlen(err_str) + 1;
			wifi_pack_send.data = err_str;
			break;
		}
		if (g_node_list.scale < 2)
		{
			os_memcpy(send_buffer, g_node_list.root.mac, 6);
			wifi_pack_send.lenth = 6;
			wifi_pack_send.data = send_buffer;
			break;
		}
		else
		{
			os_memcpy(send_buffer, g_node_list.root.mac, 6);
			os_memcpy(send_buffer + 6, g_node_list.list, 6 * (g_node_list.scale - 1));
			wifi_pack_send.lenth = 6 * g_node_list.scale;
			wifi_pack_send.data = send_buffer;
			break;
		}
	    break;

	/* ��mesh�����з������� */
	case CMD_SEND_MESH_DATA:
		hspi_send_back = 0;
        //os_printf("wifi_pack_recv.lenth:%d\r\n",recv_lenth);
		esp_mesh_data_send(wifi_pack_recv.data, wifi_pack_recv.data + 6, wifi_pack_recv.lenth - 6);
		break;

	/* ���÷�����IP��˿� */
	case CMD_SERVER_ADDR_SET:
        os_memcpy(g_server_ip, wifi_pack_recv.data, 4);
        os_memcpy(&g_server_port, wifi_pack_recv.data + 4, 2);
        
        err_str = "CMD_SERVER_ADDR_SET success\r\n";
        wifi_pack_send.lenth = os_strlen(err_str) + 1;
        wifi_pack_send.data = err_str;
        break;

    /* ����MESH��ID */
	case CMD_MESH_GROUP_ID_SET:
        os_memcpy(g_mesh_group_id, wifi_pack_recv.data, 6);
        
        err_str = "CMD_MESH_GROUP_ID_SET success\r\n";
        wifi_pack_send.lenth = os_strlen(err_str) + 1;
        wifi_pack_send.data = err_str;
        break;

    /* ����·������Ϣ */
	case CMD_ROUTER_SET:
        os_memcpy(g_mesh_router_ssid,   wifi_pack_recv.data + 0,                    32 + 1);
        os_memcpy(g_mesh_router_passwd, wifi_pack_recv.data + 32 + 1,               256 + 1);
        os_memcpy(&g_mesh_auth,         wifi_pack_recv.data + 32 + 1 + 256 + 1,     1);
        os_memcpy(g_mesh_router_bssid,  wifi_pack_recv.data + 32 + 1 + 256 + 1 + 1, 6);
        
        err_str = "CMD_ROUTER_SET success\r\n";
        wifi_pack_send.lenth = os_strlen(err_str) + 1;
        wifi_pack_send.data = err_str;
        break;

    /* ����MESH������Ϣ */
	case CMD_MESH_WIFI_SET:
        os_memcpy(g_mesh_ssid_prefix, wifi_pack_recv.data + 0,      32 + 1);
        os_memcpy(g_mesh_passwd,      wifi_pack_recv.data + 32 + 1, 256 + 1);
        
        err_str = "CMD_MESH_WIFI_SET success\r\n";
        wifi_pack_send.lenth = os_strlen(err_str) + 1;
        wifi_pack_send.data = err_str;
        break;

    /* ��ʼ��MESH */
	case CMD_MESH_INIT:
        mesh_init();
        
        err_str = "CMD_MESH_INIT success\r\n";
        wifi_pack_send.lenth = os_strlen(err_str) + 1;
        wifi_pack_send.data = err_str;
        break;
        
	/* default */
	default:
		err_str = "err: unknown command\r\n";
		wifi_pack_send.cmd = wifi_pack_recv.cmd;
		wifi_pack_send.lenth = os_strlen(err_str) + 1;
		wifi_pack_send.data = err_str;
	    break;
		
	}
	
	/* ����ACK */
	if (1 == hspi_send_back) {
//        os_printf("%s", wifi_pack_send.data);
		wifi_pack_send.cmd = wifi_pack_recv.cmd;
		system_os_post(HSPI_SEND_TASK_PRIO,HSPI_SEND,0);
	}
}
