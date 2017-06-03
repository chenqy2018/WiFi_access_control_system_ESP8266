/**
 * \file
 * \brief ������Ϣ
 * 
 * \internal
 * \par Modification history
 * - 1.00 17-06-03  zhangjinke, first implementation.
 * \endinternal
 */

#include "user_config.h"

/** \brief �������˿� */
uint16_t g_server_port  = 0;

/** \brief ������IP��ַ */
uint8_t  g_server_ip[4] = {0, 0, 0, 0};

/** \brief ���ڲ����� */
uint32_t g_uart_baud_ratio = 76800;

/** \brief g_mesh_group_id��MESH_SSID_PREFIX��ͬ��ΪͬһMESH���� */
uint8_t  g_mesh_group_id[6] = {0,0,0,0,0,0};

/** \brief ·����MAC��ַ */
uint8_t  g_mesh_router_bssid[6] = {0, 0, 0, 0, 0, 0};

/** \brief ·�������� */
uint8_t g_mesh_router_ssid[32 + 1] = "default";

/** \brief ·�������� */
uint8_t g_mesh_router_passwd[256 + 1] = "default";

/** \brief ·�������ܷ�ʽ */
uint8_t g_mesh_auth = AUTH_OPEN;

/** \brief MESH�������� */
uint8_t g_mesh_ssid_prefix[32 + 1] = "default";

/** \brief MESH�������� */
uint8_t g_mesh_passwd[256 + 1] = "default";

/** \brief MESH����������� */
uint8_t g_mesh_max_hop = 4;

/* end of file */
