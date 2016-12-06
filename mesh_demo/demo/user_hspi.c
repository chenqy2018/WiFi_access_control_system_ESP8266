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

#include "osapi.h"
#include "user_interface.h"
#include "driver/spi_interface.h"
#include "gpio.h"
#include "mem.h"

#define start_bit (1 << 7)
#define end_bit (1 << 6)

/* SPI���ղ��ֱ��� */
static uint8  recv_pack[1024*5] = {0}; /* ���ջ����� */
static uint32 recv_pack_lenth	  = 0; /* ���յ������ݰ��ĳ��� */

/* SPI������ɱ�־λ */
volatile static uint8  wr_rdy     = 1;

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

	num = lenth/31 + 1;               /* ������� */
	last_byte = lenth%31;             /* �������һ����Ч�ֽ��� */
	if (last_byte == 0)
	{
		num--;
		last_byte = 31;
	}
	/* �������Ϸ��� */
	if (pack == NULL)
	{
		return -1;
	}
	
	for (i = 0; i < num; i++)
	{
		send_buf[0] = 0;                              	/* ���command byte */
		/* ����command byte */
		if (i != num - 1)        					/* �����Ϊ��ĩ��,���õ�ǰ������ */
		{
			send_buf[0] = 31; 
		}
		else                              			/* ���õ�ǰ�����Ȳ����ĩ����־λ */
		{
			send_buf[0] = (end_bit | last_byte);
		}
		if (i == 0)
		{
			send_buf[0] |= start_bit;              /* ����װ���־λ */
			wr_rdy = 1;
		}
		
		/* �����ݸ��Ƶ�����buf�� */
		if(i == num - 1) 
		{
			for (j = 0; j < last_byte; j++) 
			{
				send_buf[j + 1] = pack[i*31 + j]; 
			}
		}
		else
		{
			for (j = 0; j < 31; j++) 
			{
				send_buf[j + 1] = pack[i*31 + j];
			}
		}
		
		/* �ȴ�����׼���ñ�־λ */
		t = 3000000;
		while((wr_rdy != 1)&&(t != 0))
		{
			t--;
		}
		wr_rdy = 0;
		if (t == 0)
		{
			os_printf("out time\r\n");
			GPIO_OUTPUT_SET(5, 0); /* GPIO5��0 */
			return -1; /* �ȴ���ʱ�����ش��� */
		}
		/* ��buf�е�������䵽SPI�ļĴ����� */
		SPISlaveSendData(SpiNum_HSPI, (uint32_t *)send_buf, 8);
		GPIO_OUTPUT_SET(5, 1); /* GPIO5��1 */
	}

	GPIO_OUTPUT_SET(5, 0); /* GPIO5��0 */
	return 0;
}

u8 is_recv_pack = 0;
u32 recv_lenth = 0;

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
	static uint32 recv_data 		  = 0; /* ����SPI���ռĴ�����ֵ */
	static uint8 buf[32]            = {0}; /* ���ջ����� */
	static uint8 isReceive = 0;
	static uint32 pack_counter = 0;        /* �������ݰ������� */
	
	/* SPI�ж� */
	if (READ_PERI_REG(0x3ff00020)&BIT4)
	{
		/* �ر�SPI�ж� */
		CLEAR_PERI_REG_MASK(SPI_SLAVE(SpiNum_SPI), 0x3ff);
	}
	/* HSPI�ж� */
	else if (READ_PERI_REG(0x3ff00020)&BIT7)
	{
		/* ��¼�ж����� */
		regvalue = READ_PERI_REG(SPI_SLAVE(SpiNum_HSPI));
		/* �ر��ж�ʹ�� */
		SPIIntClear(SpiNum_HSPI);
		/* ��HSPI�ָ�����ͨ��״̬, ׼����һ��ͨ�� */
		SET_PERI_REG_MASK(SPI_SLAVE(SpiNum_HSPI), SPI_SYNC_RESET);
		/* ����жϱ�־λ */
		SPIIntClear(SpiNum_HSPI);
		/* ���ж�ʹ�� */
		SPIIntEnable(SpiNum_HSPI,     SpiIntSrc_WrStaDone
									| SpiIntSrc_RdStaDone
									| SpiIntSrc_WrBufDone
									| SpiIntSrc_RdBufDone);
		/* ����д�룬�ӻ����մ������ */
		if (regvalue & SPI_SLV_WR_BUF_DONE) 
		{
			GPIO_OUTPUT_SET(4, 0); /* GPIO4��0 */
			
			/* ȡ8�Σ�ÿ��ȡ��һ��32λ������ȡ��32*8=256λ��Ҳ��32���ֽ� */
			for (i = 0; i < 8; i++)
			{
				recv_data=READ_PERI_REG(SPI_W0(SpiNum_HSPI)+(i<<2));
				buf[(i<<2)+0] = (recv_data>>0)&0xff;
				buf[(i<<2)+1] = (recv_data>>8)&0xff;
				buf[(i<<2)+2] = (recv_data>>16)&0xff;
				buf[(i<<2)+3] = (recv_data>>24)&0xff;
			}

			if (is_recv_pack == 0)
			{
				/* �����װ� */
				if (( buf[0] & start_bit) == start_bit)
				{
					i = 0;
					is_recv_pack = 0;
					isReceive = 1;
					pack_counter = 0;
					recv_lenth = 0;
				}
				/* �������װ�֮��ʼ�������� */
				if (isReceive)
				{
					recv_lenth += buf[0] & 0x3f;					/* ��¼���յ����ֽ��� */
					if (( buf[0] & end_bit) == end_bit) 			/* �ж��Ƿ��յ�ĩ�� */
					{
						for (i = 0; i < (buf[0] & 0x3f); i++)
						{
							recv_pack[pack_counter*31 + i] = buf[i + 1];
						}
						isReceive = 0;
						is_recv_pack = 1;							/* �յ�ĩ��֮��ֹͣ�������ݲ���λ������ɱ�־ */
					}
					else
					{
						for (i = 0; i < 31; i++)
						{
							recv_pack[pack_counter*31 + i] = buf[i + 1];
						}
						pack_counter++; 							/* ��¼���յ��İ��� */
					}
				}	
				/* ������� */
				if (is_recv_pack == 1)
				{
					os_printf("%d\r\n",recv_lenth);
					system_os_post(HSPI_SEND_TASK_PRIO,0,0);
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

os_event_t hspiQueue[HSPI_QUEUE_LEN];

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
			spi_send_data(recv_pack,recv_pack_lenth);
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
			
		} break;
        default: break;
    }
}


