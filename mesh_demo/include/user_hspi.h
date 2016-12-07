/**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: user_hspi.h
**
** ��   ��   ��: �Ž���
**
** �ļ���������: 2016 �� 11 �� 12 ��
**
** ��        ��: HSPI����

** ��־:
2016.11.12  �������ļ�
*********************************************************************************************************/

#ifndef __USER_HSPI_H__
#define __USER_HSPI_H__

#include "user_interface.h"

#pragma pack(1)
struct wifi_pack
{
	uint32  crc;				/* crcУ�� */
	uint8   cmd;				/* ���� */
	uint16  lenth;			    /* ���ݳ��� */
	uint8  *data;				/* ���� */
};
#pragma pack()

#define HSPI_SEND  0
#define HSPI_RECV  1

#define HSPI_SEND_QUEUE_LEN 8
#define HSPI_SEND_TASK_PRIO USER_TASK_PRIO_2
extern os_event_t hspi_send_Queue[HSPI_SEND_QUEUE_LEN];

#define HSPI_RECV_QUEUE_LEN 8
#define HSPI_RECV_TASK_PRIO USER_TASK_PRIO_1
extern os_event_t hspi_recv_Queue[HSPI_RECV_QUEUE_LEN];

extern void ICACHE_FLASH_ATTR hspi_slave_init();
extern void ICACHE_FLASH_ATTR hspi_task(os_event_t *e);
extern void ICACHE_FLASH_ATTR hspi_send_task(os_event_t *e);
extern void ICACHE_FLASH_ATTR hspi_recv_task(os_event_t *e);

extern s8 wifi_send(u8 cmd, u16 data_lenth, u8 *data);

#endif
