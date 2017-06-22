/**
 * \file
 * \brief ȫ�ֱ���
 *
 * \internal
 * \par Modification History
 * - 1.00 17-04-22  zhangjinke, first implementation.
 * \endinternal
 */

#ifndef __MESH_GLOBAL_H
#define __MESH_GLOBAL_H

/*
 * \brief ��mesh�����з�������
 *
 * \param[in] dst   Ŀ���ַ
 * \param[in] data  �����׵�ַ ȫ0Ϊ���͵�������
 * \param[in] lenth ���ݳ���
 *
 * \return ��
 */
void ICACHE_FLASH_ATTR esp_mesh_data_send (uint8_t *p_dst, 
                                           uint8_t *p_data, 
                                           uint32_t lenth);

#endif /* __MESH_GLOBAL_H */

/* end of file */
