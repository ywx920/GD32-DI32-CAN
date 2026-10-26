#ifndef __AD7928_H__
#define __AD7928_H__

//SCLK1	  -- SCLK1_S -- S_IO_43 -- SPI3_CLK	  -- PG11
//DIN1 		-- MOSI1   -- S_IO_41 -- SPI3_MOSI  -- PG13
//CS1   	-- CS1_S   -- S_IO_42 -- SPI3_CS	  -- PG14  -- LATCH
//DOUT1		-- MISO1   -- S_IO_44 -- SPI3_MISO  -- PG12
//SCLK2	  -- SCLK2_S -- SPI_CLK(SPI2)         -- PB3  
//DIN2 		-- MOSI2   -- SPI_MOSI(SPI2)        -- PB5
//CS2   	-- CS2_S   -- SPI_CS(SPI2)          -- PA15  -- LATCH
//DOUT2		-- MISO2   -- SPI_MISO(SPI2)        -- PB4

#define SPI3_CLK GPIO_PIN_11
#define SPI3_MOSI GPIO_PIN_13
#define SPI3_CS GPIO_PIN_14
#define SPI3_MISO GPIO_PIN_12

#define SPI2_CLK GPIO_PIN_3
#define SPI2_MOSI GPIO_PIN_5
#define SPI2_CS GPIO_PIN_15
#define SPI2_MISO GPIO_PIN_4

#define SPI3_FLASH_CLK_CLR()  	gpio_bit_reset(GPIOG,SPI3_CLK)
#define SPI3_FLASH_CLK_SET()  	gpio_bit_set(GPIOG,SPI3_CLK)
#define SPI3_FLASH_CS_LOW()			gpio_bit_reset(GPIOG,SPI3_CS)
#define SPI3_FLASH_CS_HIGH()		gpio_bit_set(GPIOG,SPI3_CS)
#define SPI3_FLASH_MOSI_SET()   gpio_bit_set(GPIOG,SPI3_MOSI)
#define SPI3_FLASH_MOSI_CLR()   gpio_bit_reset(GPIOG,SPI3_MOSI)
#define SPI3_FLASH_MISO_GET()   gpio_input_bit_get(GPIOG,SPI3_MISO)


#define SPI2_FLASH_CLK_CLR()  	gpio_bit_reset(GPIOB,SPI2_CLK)
#define SPI2_FLASH_CLK_SET()  	gpio_bit_set(GPIOB,SPI2_CLK)
#define SPI2_FLASH_CS_LOW()			gpio_bit_reset(GPIOA,SPI2_CS)
#define SPI2_FLASH_CS_HIGH()		gpio_bit_set(GPIOA,SPI2_CS)
#define SPI2_FLASH_MOSI_SET()   gpio_bit_set(GPIOB,SPI2_MOSI)
#define SPI2_FLASH_MOSI_CLR()   gpio_bit_reset(GPIOB,SPI2_MOSI)
#define SPI2_FLASH_MISO_GET()   gpio_input_bit_get(GPIOB,SPI2_MISO)

//#define SPI3_FLASH_CS_LOW()		gpio_bit_reset(GPIOA,SPI3_CS)
//#define SPI3_FLASH_CS_HIGH()		gpio_bit_set(GPIOA,SPI3_CS)
//#define SPI2_FLASH_CS_LOW()		gpio_bit_reset(GPIOA,SPI2_CS)
//#define SPI2_FLASH_CS_HIGH()		gpio_bit_set(GPIOA,SPI2_CS)

extern void AD7928_INIT(void);
extern void AD7928_INIT_SPI3(void);
extern void AD7928_INIT_SPI2(void);
extern unsigned int AD7928_SPI3_READ_BIT16(unsigned int RegValue);
extern unsigned int AD7928_SPI2_READ_BIT16(unsigned int RegValue);
extern void AD7928_SPI3_WRITE_BIT16(unsigned int RegValue);
extern void AD7928_SPI2_WRITE_BIT16(unsigned int RegValue);
extern unsigned int GetRegDataForWrite(unsigned int ich,unsigned int __Range);
extern void ADS7928_delayus(void);

#endif
