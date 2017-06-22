/**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: user_hspi.c
**
** ��   ��   ��: �Ž���
**
** �ļ���������: 2016 �� 11 �� 12 ��
**
** ��        ��: HSPI����

** ��־:
2016.11.12  �������ļ�
*********************************************************************************************************/

#include "user_hspi.h"
#include "command.h"

#include "osapi.h"
#include "user_interface.h"
#include "driver/spi_interface.h"
#include "gpio.h"
#include "mem.h"

#define start_bit (1 << 7) /* �װ���־λ */
#define end_bit   (1 << 6) /* ĩ����־λ */

/* ��ȡ�ṹ���Աƫ�ƺ궨�� */
#define OFFSET(Type, member)      ((uint32)&(((struct Type*)0)->member))
#define MEMBER_SIZE(Type, member) sizeof(((struct Type*)0)->member)

os_event_t hspi_recv_Queue[HSPI_RECV_QUEUE_LEN];
os_event_t hspi_send_Queue[HSPI_SEND_QUEUE_LEN];
struct wifi_pack wifi_pack_send;
struct wifi_pack wifi_pack_recv;

/* SPI���ղ��ֱ��� */
static uint8  recv_pack[1024*5] = {0}; /* ���ջ����� */
static uint32 recv_lenth          = 0; /* ���յ������ݰ��ĳ��� */
static uint8  is_recv_pack = 0;

/* SPI������ɱ�־λ */
volatile static uint8  wr_rdy     = 1;

void ICACHE_FLASH_ATTR hspi_data_process(uint8 *pack, uint32 lenth);

/*******************************************************************************
* ������ 	: spi_send_data
* ����   	: ͨ��SPI����ָ���ֽ�����
* ����     	: - pack: ���������� - lenth: ����
* ���     	: None
* ����ֵ    : -1: ʧ�� 0: �ɹ�
*******************************************************************************/
s8 spi_send_data(uint8 *pack, uint32 lenth)
{
	static uint32 i              = 0;
	static uint32 j              = 0;
	volatile static uint32 t     = 0;
	static uint32 num            = 0; /* ���� */
	static uint32 last_byte      = 0; /* ���һ����Ч�ֽ��� */
	static uint8  send_buf[32] = {0}; /* ���ͻ����� */

	num = lenth / 31 + 1;               /* ������� */
	last_byte = lenth % 31;             /* �������һ����Ч�ֽ��� */
	if (last_byte == 0)
	{
		num--;
		last_byte = 31;
	}
//	os_printf("lenth: %d, num: %d, last_byte: %d\r\n", lenth, num, last_byte);
	/* �������Ϸ��� */
	if (pack == NULL) {
		os_printf("pack addr is NULL\r\n");
		return -1;
	}
	
	for (i = 0; i < num; i++) {
		send_buf[0] = 0;                         /* ���command byte */
		
		/* ����command byte */
		if (i != num - 1) { 
			send_buf[0] = 31;                    /* �����Ϊ��ĩ��,���õ�ǰ������ */
		} else {
			send_buf[0] = (end_bit | last_byte); /* ���õ�ǰ�����Ȳ����ĩ����־λ */
		}
		if (i == 0) {
			send_buf[0] |= start_bit;            /* ����װ���־λ */
			wr_rdy = 1;
		}
		
		/* �����ݸ��Ƶ�����buf�� */
		if (i == num - 1) {
			for (j = 0; j < last_byte; j++) {
				send_buf[j + 1] = pack[i * 31 + j]; 
			}
		} else {
			for (j = 0; j < 31; j++) {
				send_buf[j + 1] = pack[i * 31 + j];
			}
		}
		
		/* �ȴ�����׼���ñ�־λ */
		t = 3000000;
		while((wr_rdy != 1)&&(t != 0)) {
			t--;
		}
		wr_rdy = 0;
		if (t == 0) {
			os_printf("spi send data out time\r\n");
			GPIO_OUTPUT_SET(5, 0); /* GPIO5��0 */
			return -1;             /* �ȴ���ʱ�����ش��� */
		}
		
		/* ��buf�е�������䵽SPI�ļĴ����� */
		SPISlaveSendData(SpiNum_HSPI, (uint32_t *)send_buf, 8);
		GPIO_OUTPUT_SET(5, 1); /* GPIO5��1 */
	}
	
	return 0;
}

/*******************************************************************************
* ������ 	: spi_slave_isr_sta
* ����   	: SPI�жϷ������ص�����
* ����     	: None
* ���     	: None
* ����ֵ    : None
*******************************************************************************/
void spi_slave_isr_sta(void *para)
{
	static uint32 regvalue            = 0; /* ����Ĵ���ֵ */
	static uint32 i                   = 0; /* ѭ������ */
	
	/* SPI���ղ��ֱ��� */
	static uint32 recv_data 		  = 0;   /* ����SPI���ռĴ�����ֵ */
	static uint8 buf[32]              = {0}; /* ���ջ����� */
	static uint8 isReceive = 0;
	static uint32 pack_counter = 0;          /* �������ݰ������� */
	
	if (READ_PERI_REG(0x3ff00020) & BIT4)                    /* SPI�ж� */
	{
		
		/* �ر�SPI�ж� */
		CLEAR_PERI_REG_MASK(SPI_SLAVE(SpiNum_SPI), 0x3ff);
	} else if (READ_PERI_REG(0x3ff00020)&BIT7) {           /* HSPI�ж� */
	
		/* ��¼�ж����� */
		regvalue = READ_PERI_REG(SPI_SLAVE(SpiNum_HSPI));
		
		/* �ر��ж�ʹ�� */
		SPIIntClear(SpiNum_HSPI);
		
		/* ��HSPI�ָ�����ͨ��״̬, ׼����һ��ͨ�� */
		SET_PERI_REG_MASK(SPI_SLAVE(SpiNum_HSPI), SPI_SYNC_RESET);
		
		/* ����жϱ�־λ */
		SPIIntClear(SpiNum_HSPI);
		
		/* ���ж�ʹ�� */
		SPIIntEnable(SpiNum_HSPI, SpiIntSrc_WrStaDone |
                                  SpiIntSrc_RdStaDone |
                                  SpiIntSrc_WrBufDone |
                                  SpiIntSrc_RdBufDone);
		
		/* ����д�룬�ӻ����մ������ */
		if (0 != (regvalue & SPI_SLV_WR_BUF_DONE)) {
			
			/* GPIO4��0 */
			GPIO_OUTPUT_SET(4, 0); 
			
			/* ȡ8�Σ�ÿ��ȡ��һ��32λ������ȡ��32*8=256λ��Ҳ��32���ֽ� */
			for (i = 0; i < 8; i++) {
				recv_data=READ_PERI_REG(SPI_W0(SpiNum_HSPI)+(i<<2));
				buf[(i<<2)+0] = (recv_data>>0)&0xff;
				buf[(i<<2)+1] = (recv_data>>8)&0xff;
				buf[(i<<2)+2] = (recv_data>>16)&0xff;
				buf[(i<<2)+3] = (recv_data>>24)&0xff;
			}

			if (is_recv_pack == 0) {
				 
				/* �����װ� */
				if (( buf[0] & start_bit) == start_bit) {
					i = 0;
					is_recv_pack = 0;
					isReceive = 1;
					pack_counter = 0;
					recv_lenth = 0;
				}
				
				/* �������װ�֮��ʼ�������� */
				if (isReceive) {
					recv_lenth += buf[0] & 0x3f;					/* ��¼���յ����ֽ��� */
					if (( buf[0] & end_bit) == end_bit) { 			/* �ж��Ƿ��յ�ĩ�� */
						for (i = 0; i < (buf[0] & 0x3f); i++) {
							recv_pack[pack_counter * 31 + i] = buf[i + 1];
						}
						isReceive = 0;
						is_recv_pack = 1;							/* �յ�ĩ��֮��ֹͣ�������ݲ���λ������ɱ�־ */
					} else {
						for (i = 0; i < 31; i++) {
							recv_pack[pack_counter * 31 + i] = buf[i + 1];
						}
						pack_counter++; 							/* ��¼���յ��İ��� */
					}
				}
				
				/* ������� */
				if (is_recv_pack == 1) {
					//os_printf("%d\r\n",recv_lenth);
					system_os_post(HSPI_RECV_TASK_PRIO, HSPI_RECV, 0);
				}
			}
			
			GPIO_OUTPUT_SET(4, 1); /* GPIO4��1 */
			SET_PERI_REG_MASK(SPI_SLAVE(SpiNum_HSPI),SPI_SLV_WR_BUF_DONE_EN);
		}
		
		/* ������ȡ���ӻ����ʹ������ */
		if (regvalue & SPI_SLV_RD_BUF_DONE) 
		{
			GPIO_OUTPUT_SET(5, 0); /* GPIO5��0 */
			wr_rdy = 1;
		}
	}
}

/*******************************************************************************
* ������ 	: hspi_slave_init
* ����   	: ��ʼ��HSPI
* ����     	: None
* ���     	: None
* ����ֵ    : None
*******************************************************************************/
void ICACHE_FLASH_ATTR hspi_slave_init()
{
	// SPI initialization configuration, speed = 0 in slave mode
	SpiAttr hSpiAttr;
	hSpiAttr.bitOrder = SpiBitOrder_MSBFirst;
	hSpiAttr.speed = 0;
	hSpiAttr.mode = SpiMode_Slave;
	hSpiAttr.subMode = SpiSubMode_0;
	// Init HSPI GPIO
	// Configure MUX to allow HSPI
	WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2);//configure io to spi mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2);//configure io to spi mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2);//configure io to spi mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 2);//configure io to spi mode
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);//GPIO4�������ʹӻ����ջ���״̬
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);//GPIO5�������մӻ����ͻ���״̬

	GPIO_OUTPUT_SET(4, 0);	//GPIO4��0
	GPIO_OUTPUT_SET(5, 0);	//GPIO5��0
	os_printf("\r\n============= spi init slave =============\r\n");
	SPIInit(SpiNum_HSPI, &hSpiAttr);
	// Set spi interrupt information.
	SpiIntInfo spiInt;
	spiInt.src = (SpiIntSrc_TransDone
	|SpiIntSrc_WrStaDone
	|SpiIntSrc_RdStaDone
	|SpiIntSrc_WrBufDone
	|SpiIntSrc_RdBufDone);
	spiInt.isrFunc = spi_slave_isr_sta;
	SPIIntCfg(SpiNum_HSPI, &spiInt);
	// SHOWSPIREG(SpiNum_HSPI);
	SPISlaveRecvData(SpiNum_HSPI);
	// set the value of status register
	WRITE_PERI_REG(SPI_RD_STATUS(SpiNum_HSPI), 0x8A);
	WRITE_PERI_REG(SPI_WR_STATUS(SpiNum_HSPI), 0x83);
	GPIO_OUTPUT_SET(4, 1);	//GPIO4��1,��ʾ�ӻ�׼���ã��������Է������ݵ��ӻ�
}

/*******************************************************************************
* ������ 	: hspi_send_task
* ����   	: hspi��������
* ����     	: - e: �¼�
* ���     	: None
* ����ֵ    : None
*******************************************************************************/
void ICACHE_FLASH_ATTR hspi_send_task(os_event_t *e)
{
    switch(e->sig)
	{
	    case HSPI_SEND:
		{
			wifi_send(wifi_pack_send.cmd, wifi_pack_send.lenth, wifi_pack_send.data);
		} break;
        default: break;
    }
}

/*******************************************************************************
* ������ 	: hspi_recv_task
* ����   	: hspi��������
* ����     	: - e: �¼�
* ���     	: None
* ����ֵ    : None
*******************************************************************************/
void ICACHE_FLASH_ATTR hspi_recv_task(os_event_t *e)
{
    switch(e->sig)
	{
	    case HSPI_RECV:
		{
			hspi_data_process(recv_pack,recv_lenth);
			is_recv_pack = 0;
		} break;
        default: break;
    }
}

/*******************************************************************************
* ������ 	: hspi_data_process
* ����   	: hspi���ݴ���
* ����     	: - pack: ���յ������� - lenth: ���ݴ�С
* ���     	: None
* ����ֵ     : None
*******************************************************************************/
void ICACHE_FLASH_ATTR hspi_data_process(uint8 *pack, uint32 lenth)
{
	static uint32 par_lenth = sizeof(struct wifi_pack) - MEMBER_SIZE(wifi_pack, data); /* ���еĲ�����С */
	static uint32 crc_lenth = MEMBER_SIZE(wifi_pack, crc); /* CRC��С */
	static uint32 i = 0;

	os_memcpy(&wifi_pack_recv, recv_pack, par_lenth);
	
	/* У������� */
	if (wifi_pack_recv.lenth + par_lenth != recv_lenth) {
		is_recv_pack = 0; /* ���µȴ��������� */
		os_printf("lenth verify failed\r\n");
		return;
	}
	
	/* CRCУ�� */
	if (wifi_pack_recv.crc != 
        CRC32Software(recv_pack + crc_lenth, par_lenth - crc_lenth + wifi_pack_recv.lenth)) {
		is_recv_pack = 0; /* ���µȴ��������� */
		os_printf("crc verify failed %08X\r\n", wifi_pack_recv.crc);
		return;
	}
	
	wifi_pack_recv.data = recv_pack + par_lenth;
	command_execute();
#if 0
	wifi_send(0x09, wifi_pack_recv.lenth, wifi_pack_recv.data);
	os_printf("cmd: %d, lenth: %d, crc: %08X\r\n", wifi_pack_recv.cmd, wifi_pack_recv.lenth, wifi_pack_recv.crc);
	
	for (i = 0; i < wifi_pack_recv.lenth; i++)
	{
		os_printf("%02X ", *(wifi_pack_recv.data + i));
	}
		os_printf("\r\n");
#endif
	is_recv_pack = 0; /* ���µȴ��������� */
}

s8 wifi_send(u8 cmd, u16 data_lenth, u8 *data)
{
	struct wifi_pack wifi_pack_send;
	u8 *send_pack = NULL;
	u32 par_lenth = sizeof(struct wifi_pack) - MEMBER_SIZE(wifi_pack, data); /* ���еĲ�����С */
	u32 crc_lenth = MEMBER_SIZE(wifi_pack, crc); /* CRC��С */
	
	/* �����ڴ� */
	send_pack = (u8 *)os_malloc(data_lenth + par_lenth);
    if (!send_pack) 
    { 
        os_printf("send_pack memory failed\r\n");
        return -1;
    }

	wifi_pack_send.cmd = cmd;                /* ���� */
	wifi_pack_send.lenth = data_lenth;       /* ���ݳ��� */
	
	/* ����CRC֮����������������������� */
	os_memcpy(send_pack + crc_lenth, (u8 *)&wifi_pack_send + crc_lenth, par_lenth - crc_lenth);
	
	/* �����ݿ����������� */
	os_memcpy(send_pack + par_lenth, data, data_lenth);
	
	/* ����CRC */
	wifi_pack_send.crc = CRC32Software(send_pack + crc_lenth, par_lenth - crc_lenth + data_lenth);
	
	/* ��CRC������������ */
	os_memcpy(send_pack, &wifi_pack_send, crc_lenth);
	
	/* �������� */
	if (spi_send_data(send_pack, par_lenth + data_lenth) != 0)
	{
		os_free(send_pack);
		return -1;
	}
	
	os_free(send_pack);
	return 0;
}

