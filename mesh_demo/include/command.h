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

#define cmd_return_recv                   (0)	 /* ֱ�ӷ��ؽ��յ������� */
#define cmd_get_sdk_version               (1)    /* ��ȡsdk�汾�� */
#define cmd_get_flash_size_map            (2)    /* ��ѯFlash size�Լ�Flash map */
#define cmd_wifi_get_ip_info              (3)    /* ��ѯIP��ַ */
#define cmd_wifi_get_macaddr              (4)    /* ��ѯmac��ַ */
#define cmd_get_device_list               (5)    /* ��ȡmesh�豸�б� */

#define cmd_send_data_to_mcu              (254)  /* �������ݵ�mcu */

extern uint8 send_buffer[1024*5]; /* ���ͻ����� */

extern void command_execute(void);

#endif
