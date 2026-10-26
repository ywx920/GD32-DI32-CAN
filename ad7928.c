#include "ad7928.h"
#include "gd32f4xx.h"
#include "gd32f450z_eval.h"
#include "gd32f4xx_spi.h"

void AD7928_INIT()
{
	AD7928_INIT_SPI3();
	AD7928_INIT_SPI2();
}

void AD7928_INIT_SPI3()
{
	//spi_parameter_struct spi_init_struct;

  rcu_periph_clock_enable(RCU_GPIOG);
  //rcu_periph_clock_enable(RCU_SPI3);
  //SCLK1	  -- SCLK1_S -- S_IO_43 -- SPI3_CLK	  -- PG11
	//DIN1 		-- MOSI1   -- S_IO_41 -- SPI3_MOSI  -- PG13
	//CS1   	-- CS1_S   -- S_IO_42 -- SPI3_CS	  -- PG14
	//DOUT1		-- MISO1   -- S_IO_44 -- SPI3_MISO  -- PG12
  //gpio_af_set(GPIOG, GPIO_AF_5, SPI3_CLK|SPI3_MOSI| SPI3_MISO);
  //gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI3_CLK|SPI3_MOSI| SPI3_MISO);
  gpio_mode_set(GPIOG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI3_CLK|SPI3_MOSI);
  gpio_mode_set(GPIOG, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SPI3_MISO);
  gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI3_CLK|SPI3_MOSI| SPI3_MISO);

  gpio_mode_set(GPIOG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI3_CS);
  gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI3_CS);

  SPI3_FLASH_CS_HIGH();
  //================================================================================================
	//MFD -- 全双工主机模式
	//配置SPI_CTL0的PSC[2:0]来生成预期波特率的SCK信号
	//配置SPI_CTL0的FF16位
	//配置时钟时序SPI_CTL0的 CKCL和CKCH
	//配置帧格式SPI_CTL0的 LF位
	//配置NSS模式为软件/硬件输出模式（SWNSSEN = 0; NSSRV = 1）。使能SPI之后，NSS保持高电平，当接收或者发送开始时，变为低电平
	//配置MSTMOD，RO，BDEN，BDOEN位
	//使能SPI(SPIEN置1)
  //================================================================================================
  //* SPI3 parameter config */
  //spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
  //spi_init_struct.device_mode          = SPI_MASTER;
  //spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
	//spi_init_struct.frame_size           = SPI_FRAMESIZE_16BIT;
  //spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
  //spi_init_struct.nss                  = SPI_NSS_SOFT;
  //spi_init_struct.prescale             = SPI_PSC_32;
  //spi_init_struct.endian               = SPI_ENDIAN_MSB;
  //spi_init(SPI3, &spi_init_struct);
	//spi_crc_off(SPI3);  /* set crc polynomial */
  //spi_enable(SPI3);  /* enable SPI3 */
}

void AD7928_INIT_SPI2()
{
	//spi_parameter_struct spi_init_struct;

  rcu_periph_clock_enable(RCU_GPIOB);
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_SPI2);
	//SCLK2	  -- SCLK2_S -- SPI_CLK(SPI2)         -- PB3  
	//DIN2 		-- MOSI2   -- SPI_MOSI(SPI2)        -- PB5
	//CS2   	-- CS2_S   -- SPI_CS(SPI2)          -- PA15
	//DOUT2		-- MISO2   -- SPI_MISO(SPI2)        -- PB4
  //gpio_af_set(GPIOB, GPIO_AF_5, SPI2_CLK|SPI2_MOSI| SPI2_MISO);
  //gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, SPI2_CLK|SPI2_MOSI| SPI2_MISO);
  gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,SPI2_CLK|SPI2_MOSI);
  gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE,SPI2_MISO);
  gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI2_CLK|SPI2_MOSI| SPI2_MISO);

  /* SPI5_CS(PG9) GPIO pin configuration */
  gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SPI2_CS);
  gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI2_CS);
  /* chip select invalid */
  SPI2_FLASH_CS_HIGH();
}

unsigned int AD7928_SPI3_READ_BIT16(unsigned int RegValue)
{
	unsigned short ReadData;
	char i;
	//Aducm360_output_CNV_low();	
	SPI3_FLASH_CS_LOW();
	ADS7928_delayus();
	for(i=0; i<16; i++)//16Bit
	{
		//Aducm360_output_SCLK_high();
		SPI3_FLASH_CLK_SET();
		if(0x80000 == ((RegValue&0xffff) & 0x8000))
		{
			//Aducm360_output_SDIO_high();	
			SPI3_FLASH_MOSI_SET();
		}
		else
		{
			SPI3_FLASH_MOSI_CLR();	  
		}	
		ADS7928_delayus();
		RegValue <<= 1;	
		ReadData <<= 1;
		if(SPI3_FLASH_MISO_GET() == SET)  		
    {
				ReadData |= 0x0001;  		 
		}
    else  
    {
				ReadData &= ~0x0001;
		}
		ADS7928_delayus();
		//Aducm360_output_SCLK_low();
		SPI3_FLASH_CLK_CLR();
		ADS7928_delayus();
	}
	SPI3_FLASH_CS_HIGH();	//bring CS high again
	ADS7928_delayus();
	ADS7928_delayus();
	ADS7928_delayus();
	return ReadData;
}

unsigned int AD7928_SPI2_READ_BIT16(unsigned int RegValue)
{
	unsigned short ReadData;
	char i;
	//Aducm360_output_CNV_low();	
	SPI2_FLASH_CS_LOW();
	ADS7928_delayus();
	for(i=0; i<16; i++)//16Bit
	{
		//Aducm360_output_SCLK_high();
		SPI2_FLASH_CLK_SET();
		if(0x80000 == ((RegValue&0xffff) & 0x8000))
		{
			//Aducm360_output_SDIO_high();	
			SPI2_FLASH_MOSI_SET();
		}
		else
		{
			SPI2_FLASH_MOSI_CLR();	  
		}	
		ADS7928_delayus();
		RegValue <<= 1;	
		ReadData <<= 1;
		if(SPI2_FLASH_MISO_GET() == SET)  		
    {
				ReadData |= 0x0001;  		 
		}
    else  
    {
				ReadData &= ~0x0001;
		}
		ADS7928_delayus();
		//Aducm360_output_SCLK_low();
		SPI2_FLASH_CLK_CLR();
		ADS7928_delayus();
	}
	SPI2_FLASH_CS_HIGH();	//bring CS high again
	ADS7928_delayus();
	ADS7928_delayus();
	ADS7928_delayus();
	return ReadData;
}

void AD7928_SPI3_WRITE_BIT16(unsigned int RegValue)
{
	char i;
	//Aducm360_output_CNV_low(); 
	SPI3_FLASH_CS_LOW();
	ADS7928_delayus();
	for(i=0; i<16; i++)
	{
		//Aducm360_output_SCLK_high();
		SPI3_FLASH_CLK_SET();
		if(0x8000 == ((RegValue&0xffff) & 0x8000))
		{
			//Aducm360_output_SDIO_high();
			SPI3_FLASH_MOSI_SET();					
		}
		else
		{
			//Aducm360_output_SDIO_low();
			SPI3_FLASH_MOSI_CLR();			
		}
		ADS7928_delayus();
		SPI3_FLASH_CLK_CLR();
		RegValue<<= 1;//data left.	
		ADS7928_delayus();
	}
	SPI3_FLASH_CS_HIGH();	
	ADS7928_delayus();
	return;
}

void AD7928_SPI2_WRITE_BIT16(unsigned int RegValue)
{
	char i;
	//Aducm360_output_CNV_low(); 
	SPI2_FLASH_CS_LOW();
	ADS7928_delayus();
	for(i=0; i<16; i++)
	{
		//Aducm360_output_SCLK_high();
		SPI2_FLASH_CLK_SET();
		if(0x8000 == ((RegValue&0xffff) & 0x8000))
		{
			//Aducm360_output_SDIO_high();
			SPI2_FLASH_MOSI_SET();					
		}
		else
		{
			//Aducm360_output_SDIO_low();
			SPI2_FLASH_MOSI_CLR();			
		}
		ADS7928_delayus();
		SPI2_FLASH_CLK_CLR();
		RegValue<<= 1;//data left.	
		ADS7928_delayus();
	}
	SPI2_FLASH_CS_HIGH();	
	ADS7928_delayus();
	return;	
}

unsigned int GetRegDataForWrite(unsigned int ich,unsigned int __Range)//ich -- 0~7;__Range -- 0/1
{
	unsigned int ival;
	unsigned int ICHANNEL;
	unsigned int PM1, PM0;
	unsigned int Shadow;
	//unsigned int Range;
	unsigned int SEQ;
	unsigned int Coding;
	unsigned int Write;
	
	
	ICHANNEL = ich & 0x7;
	Coding = 1;//0:补码；1：2进制
	//Range = __Range;
	//Shadow = 1;
	Shadow = 0;
	PM1 = 1;
	PM0 = 1;
	//SEQ = 1;
	SEQ = 0;
	Write = 1;
	ival = (Write << 11) | (SEQ << 10) | (ICHANNEL << 6) | (PM1 << 5) | 
	       (PM0 << 4) | (Shadow << 3) | (__Range << 1) | (Coding); 
	ival = ival << 4;//抬高4位
	return ival;
	//Range = 0时
	//Range = 1时
}

void ADS7928_delayus()
{
	int i= 5;
	while(i--);	
}

