/**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: command.h
**
** ��   ��   ��: �Ž���
**
** �ļ���������: 2017 �� 1 �� 31 ��
**
** ��        ��: ��stm32ͨ�ŵ����

** ��־:
2017.01.31  �������ļ�
*********************************************************************************************************/

#ifndef _COMMAND_H_
#define _COMMAND_H_

#include "user_hspi.h"

#define CMD_RETURN_RECV                   (0)	 /* ֱ�ӷ��ؽ��յ������� */
#define CMD_GET_SDK_VERSION               (1)    /* ��ȡsdk�汾�� */
#define CMD_GET_FLASH_SIZE_MAP            (2)    /* ��ѯFlash size�Լ�Flash map */
#define CMD_WIFI_GET_IP_INFO              (3)    /* ��ѯIP��ַ */
#define CMD_WIFI_GET_MACADDR              (4)    /* ��ѯmac��ַ */
#define CMD_GET_DEVICE_LIST               (5)    /* ��ȡmesh�豸�б� */
#define CMD_SEND_MESH_DATA                (6)    /* ��mesh�����з������� */
#define CMD_SERVER_ADDR_SET               (7)    /* ���÷�����IP��˿� */
#define CMD_MESH_GROUP_ID_SET             (8)    /* ����MESH��ID */
#define CMD_ROUTER_SET                    (9)    /* ����·������Ϣ */
#define CMD_MESH_WIFI_SET                 (10)   /* ����MESH������Ϣ */
#define CMD_MESH_INIT                     (11)   /* ��ʼ��MESH */

#define CMD_SEND_DATA_TO_MCU              (254)  /* �������ݵ�mcu */

extern uint8 send_buffer[1024*5]; /* ���ͻ����� */

extern void command_execute(void);

#endif
