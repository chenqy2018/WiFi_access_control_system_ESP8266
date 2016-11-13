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

#define HSPI_SEND  0
#define HSPI_RECV  1
#define HSPI_QUEUE_LEN 8
#define HSPI_TASK_PRIO USER_TASK_PRIO_1

extern os_event_t hspiQueue[HSPI_QUEUE_LEN];

extern void ICACHE_FLASH_ATTR hspi_slave_init();
extern void ICACHE_FLASH_ATTR hspi_task(os_event_t *e);

#endif
