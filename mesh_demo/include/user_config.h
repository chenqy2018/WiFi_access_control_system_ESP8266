/**
 * \file
 * \brief ������Ϣ
 * 
 * \internal
 * \par Modification history
 * - 1.00 17-06-03  zhangjinke, first implementation.
 * \endinternal
 */
#ifndef __USER_CONFIG_H
#define __USER_CONFIG_H

#include "c_types.h"
#include "user_interface.h"

#define MESH_DEMO_PRINT  ets_printf
#define MESH_DEMO_STRLEN ets_strlen
#define MESH_DEMO_MEMCPY ets_memcpy
#define MESH_DEMO_MEMSET ets_memset
#define MESH_DEMO_FREE   os_free

/** \brief �������˿� */
extern uint16_t g_server_port;

/** \brief ������IP��ַ */
extern uint8_t  g_server_ip[4];

/** \brief ���ڲ����� */
extern uint32_t g_uart_baud_ratio;

/** \brief g_mesh_group_id��MESH_SSID_PREFIX��ͬ��ΪͬһMESH���� */
extern uint8_t  g_mesh_group_id[6];

/** \brief ·����MAC��ַ */
extern uint8_t  g_mesh_router_bssid[6];

/** \brief ·�������� */
extern uint8_t g_mesh_router_ssid[32 + 1];

/** \brief ·�������� */
extern uint8_t g_mesh_router_passwd[256 + 1];

/** \brief MESH�������� */
extern uint8_t g_mesh_ssid_prefix[32 + 1];

/** \brief ·�������ܷ�ʽ */
extern uint8_t g_mesh_auth;

/** \brief MESH�������� */
extern uint8_t g_mesh_passwd[256 + 1];

/** \brief MESH����������� */
extern uint8_t g_mesh_max_hop;

#endif

