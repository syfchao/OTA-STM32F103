#include "BSP_24l01.h"
#include <string.h>

u8  OTA_RX[OTA_RX_LEN] __attribute__ ((at(0X20001000)));	//���ջ���,���OTA_RX_LEN���ֽ�,��ʼ��ַΪ0X20001000. 
u8  SPI_RX_BUF[SPI_REC_LEN];
u8  SPI_TX_BUF[SPI_REC_LEN];

u8 TX_ADDRESS[TX_ADR_WIDTH]; //���͵�ַ
u8 RX_ADDRESS[RX_ADR_WIDTH];

/**
  * @brief  NRF24L01��������
  * @retval 0 û������ or 1 ���յ�����
  */
u8 NRF24L01_Rx(u16 x)
{	  		    		  
	u8 i = 0;
	u16 Index;
	Index = x *32;
	
	if(NRF24L01_RxPacket(SPI_RX_BUF) == 0)	//һ�����յ���Ϣ,����ʾ����.
	{
		for (i = 0; i<32; i++)
			OTA_RX[Index + i] = SPI_RX_BUF[i];	// ���浽SRAM
		return 1; 
	}
	return 0;
}

/**
  * @brief  NRF24L01���͹̼�����
  * @retval 0,���ͳɹ� or 1,����ʧ��
  */
u8 NRF24L01_TxLenth(u8 * x)
{  
		u8 i = 0;

		for (i = 0; i<32; i++)
			SPI_TX_BUF[i] = x[i];
		
		if(NRF24L01_TxPacket(SPI_TX_BUF) == TX_OK)	
		{
			printf("���ͳ��ȳɹ���\r\n"); 
			return 0;
		}
		else	
		{
			return 1;
		}
}

/**
  * @brief  NRF24L01��������
  * @retval 0,���ͳɹ� or 1,����ʧ��
  */
u8 NRF24L01_Tx(u16 x)
{  
	u8 i = 0;
	
	SPI_TX_BUF[0] = x % 256;
	SPI_TX_BUF[1] = x / 256;	
	
	for (i = 0; i<Single_DataSize; i++)
	{
		SPI_TX_BUF[3 + i] = OTA_RX[x *Single_DataSize + i];
	}

	if(NRF24L01_TxPacket(SPI_TX_BUF) == TX_OK)
	{
		printf("������ %3d %3d\r\n",SPI_TX_BUF[4],SPI_TX_BUF[1]*256+SPI_TX_BUF[0]);  // ����ʹ��
		return 0;
	}else
	{										   	
		printf("%d ",SPI_TX_BUF[0]);
		return 1;
	}
}

/**
  * @brief  ����NRF24L01����һ������
  * @param  txbuf:�����������׵�ַ
  * @retval �������״��
  */
u8 NRF24L01_TxPacket(u8 *txbuf)
{
	u8 sta;

	NRF24L01_CE(RESET);
	NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//д���ݵ�TX BUF  TX_PLOAD_WIDTH:32���ֽ�
 	NRF24L01_CE(SET);//��������	  

	while(NRF24L01_IRQ!=0);//�ȴ��������

	sta=NRF24L01_Read_Reg(STATUS);  //��ȡ״̬�Ĵ�����ֵ	   
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־
	
	if(sta&MAX_TX)//�ﵽ����ط�����
	{
		NRF24L01_Write_Reg(FLUSH_TX,0xff);	//���TX FIFO�Ĵ��� 
		return MAX_TX; 
	}
	
	if(sta&TX_OK)//�������
	{
		return TX_OK;
	}
	
	return 0xff;//����ԭ����ʧ��
}

/**
  * @brief  ����NRF24L01����һ������
  * @param  txbuf:�����������׵�ַ
  * @retval 0��������ɣ��������������
  */
u8 NRF24L01_RxPacket(u8 *rxbuf)
{
	u8 sta;	
	
	//SPI2_SetSpeed(SPI_BaudRatePrescaler_8); //spi�ٶ�Ϊ9Mhz��24L01�����SPIʱ��Ϊ10Mhz��   
	sta=NRF24L01_Read_Reg(STATUS);  //��ȡ״̬�Ĵ�����ֵ    	 
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //���TX_DS��MAX_RT�жϱ�־

	if(sta&RX_OK)//���յ�����
	{
		NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//��ȡ����
		NRF24L01_Write_Reg(FLUSH_RX,0xff);//���RX FIFO�Ĵ��� 
		return 0; 
	}	   
	return 1;//û�յ��κ�����
}					 


/**
  * @brief  ��ʼ��24L01��IO��
  * @retval NONE
  */
void NRF24L01_Init(void)
{
	HAL_GPIO_WritePin(NRF_IRQ_GPIO_Port,NRF_IRQ_Pin,GPIO_PIN_SET);	// NRF_IRQ_Pin����

	while(NRF24L01_Check())
	{
		printf("NRF24L01 Error\r\n");
		HAL_Delay(200);
	}
	printf("NRF24L01 Init OK\r\n");
}

/**
  * @brief  ���24L01�Ƿ����
  * @param  txbuf:�����������׵�ַ
  * @retval 0���ɹ� or 1��ʧ��
  */
u8 NRF24L01_Check(void)
{
	u8 buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 i;
	
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,buf,5);//д��5���ֽڵĵ�ַ.	
	NRF24L01_Read_Buf(TX_ADDR,buf,5); //����д��ĵ�ַ
		
	for(i=0;i<5;i++) if(buf[i]!=0XA5) break;	 							   
	if(i!=5) return 1;	//���24L01����	
	return 0;		 //��⵽24L01

}

/**
  * @brief  ��ָ��λ��дָ�����ȵ�����
  * @param  reg:�Ĵ���(λ��)
	* @param	*pBuf:����ָ��
	* @param	len:���ݳ���
  * @retval �˴ζ�����״̬�Ĵ���ֵ
  */
u8 NRF24L01_Write_Buf(u8 reg, u8 *pBuf, u16 len)
{	 
		u8 status,u8_ctr;	
	
		NRF24L01_CSN(RESET);          //ʹ��SPI����
  	status = SPI2_ReadWriteByte(reg);		//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬
  	for(u8_ctr=0; u8_ctr<len; u8_ctr++) SPI2_ReadWriteByte(*pBuf++); //д������	 
		NRF24L01_CSN(SET);       //�ر�SPI����
		
		return status;          //���ض�����״ֵ̬
}			

/**
  * @brief  ��ָ��λ�ö���ָ�����ȵ�����
  * @param  reg:�Ĵ���(λ��)
	* @param	*pBuf:����ָ��
	* @param	len:���ݳ���
  * @retval �˴ζ�����״̬�Ĵ���ֵ 
  */
u8 NRF24L01_Read_Buf(u8 reg,u8 *pBuf,u8 len)
{	 
	u8 status,u8_ctr;	    
	
	NRF24L01_CSN (RESET);           //ʹ��SPI����
	status=SPI2_ReadWriteByte(reg);	//���ͼĴ���ֵ(λ��),����ȡ״ֵ̬   	   
 	for(u8_ctr=0;u8_ctr<len;u8_ctr++) pBuf[u8_ctr]=SPI2_ReadWriteByte(0XFF);	//��������
	NRF24L01_CSN (SET);       //�ر�SPI����
	
	return status;        //���ض�����״ֵ̬
}

/**
  * @brief  SPIд�Ĵ���
  * @param  reg:ָ���Ĵ�����ַ
	* @param	value:д���ֵ
  * @retval ����״ֵ̬
  */
u8 NRF24L01_Write_Reg(u8 reg,u8 value)
{
		u8 status;	
	
   	NRF24L01_CSN(RESET);                 //ʹ��SPI����
  	status = SPI2_ReadWriteByte(reg);	//���ͼĴ����� 
  	SPI2_ReadWriteByte(value);     	 //д��Ĵ�����ֵ
  	NRF24L01_CSN(SET);                 //��ֹSPI����	  
	
  	return(status);       			//����״ֵ̬
}

/**
  * @brief  ��ȡSPI�Ĵ���ֵ
  * @param  reg:Ҫ���ļĴ���
  * @retval ����״ֵ̬
  */
u8 NRF24L01_Read_Reg(u8 reg)
{
		u8 reg_val;	    
	
		NRF24L01_CSN(RESET);          //ʹ��SPI����	
  	SPI2_ReadWriteByte(reg);   //���ͼĴ�����
  	reg_val=SPI2_ReadWriteByte(0XFF);//��ȡ�Ĵ�������
  	NRF24L01_CSN(SET);          //��ֹSPI����		    
  
		return(reg_val);           //����״ֵ̬
}	

/**
  * @brief  ������ʼ��NRF24L01��RXģʽ
  * @param  ����RX��ַ,дRX���ݿ���,ѡ��RFƵ��,�����ʺ�LNA HCURR,
						��CE��ߺ�,������RXģʽ,�����Խ���������		
  * @retval NONE
  */
void NRF24L01_RX_Mode(void)
{
		NRF24L01_CE(RESET);	  
  	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH);//дRX�ڵ��ַ
	  
  	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);    //ʹ��ͨ��0���Զ�Ӧ��    
  	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01); //ʹ��ͨ��0�Ľ��յ�ַ  	 
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,40);	     //����RFͨ��Ƶ��		  
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH); //ѡ��ͨ��0����Ч���ݿ��� 	    
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f); //����TX�������,0db����,2Mbps,���������濪��   
  	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0f); //���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����ģʽ 
  	NRF24L01_CE(SET); //CEΪ��,�������ģʽ 
		HAL_Delay(1);
}		

/**
  * @brief  ������ʼ��NRF24L01��TXģʽ
  * @param  ����TX��ַ,дTX���ݿ���,����RX�Զ�Ӧ��ĵ�ַ,���TX��������,ѡ��RFƵ��,�����ʺ�LNA HCURR��PWR_UP,CRCʹ��
						��CE��ߺ�,������TXģʽ,�����Է���������
						CEΪ�ߴ���10us,����������.
  * @retval NONE
  */ 
void NRF24L01_TX_Mode(void)
{														 
	NRF24L01_CE(RESET);	    
  
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(u8*)TX_ADDRESS,TX_ADR_WIDTH);//дTX�ڵ��ַ 
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(u8*)RX_ADDRESS,RX_ADR_WIDTH); //����TX�ڵ��ַ,��ҪΪ��ʹ��ACK	  
	
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);     //ʹ��ͨ��0���Զ�Ӧ��    
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01); //ʹ��ͨ��0�Ľ��յ�ַ  
	NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_RETR,0x1a); //�����Զ��ط����ʱ��:500us + 86us;����Զ��ط�����:10��
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,40);       //����RFͨ��Ϊ40
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);  //����TX�������,0db����,2Mbps,���������濪��   
	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,0x0e);    //���û�������ģʽ�Ĳ���;PWR_UP,EN_CRC,16BIT_CRC,����1ģʽ,���������ж�
	
	NRF24L01_CE(SET);	//CEΪ��,10us����������
	HAL_Delay(1);
}







 





