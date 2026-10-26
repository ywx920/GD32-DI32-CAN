#include "FNRAM.h"

/*状态标志*/
static uint8 ack;	         /*应答标志位*/

static void delayus(void)
{
    uint8 i = 5;
    while(i--);     
}

void FNRAM_init(void)
{	
	rcu_periph_clock_enable(RCU_GPIOB);
	
	//sda
	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,GPIO_PIN_11); 
	gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_11);
	GPIO_BC(GPIOB) = GPIO_PIN_11;
	//scl
	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,GPIO_PIN_10); 
	gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_10);
	GPIO_BC(GPIOB) = GPIO_PIN_10;
}

/*******************************************************************
                     起动总线函数               
函数原型: void  Start_I2c();  
功能:启动I2C总线,即发送I2C起始条件. 
********************************************************************/
static void Start_I2c(void)
{
  	SET_SDA_FM;      /*发送起始条件的数据信号*/
  	delayus();
  	
  	SET_SCL_FM;
  	delayus();    /*起始条件建立时间大于4.7us,延时*/
  	delayus();
 	  delayus();
  	delayus();
  	delayus();   
  	 
  	CLR_SDA_FM;      /*发送起始信号*/
  	delayus();    /* 起始条件锁定时间大于4μs*/
  	delayus();
  	delayus();
  	delayus();
  	delayus();      
  	 
  	CLR_SCL_FM;      /*钳住I2C总线，准备发送或接收数据 */
  	delayus();
  	delayus();
}

/*******************************************************************
                      结束总线函数               
函数原型: void  Stop_I2c();  
功能:结束I2C总线,即发送I2C结束条件.  
********************************************************************/
static void Stop_I2c(void)
{
    CLR_SDA_FM;     /*发送结束条件的数据信号*/
    delayus();   /*发送结束条件的时钟信号*/
    
    SET_SCL_FM;     /*结束条件建立时间大于4μs*/
    delayus();
    delayus();
    delayus();
    delayus();
    delayus();
    
    SET_SDA_FM;     /*发送I2C总线结束信号*/
    delayus();
    delayus();
    delayus();
    delayus();
}

/*******************************************************************
                 字节数据传送函数               
函数原型: void  SendByte(uint8 c);
功能: 将数据c发送出去,可以是地址,也可以是数据,发完后等待应答,并对
      此状态位进行操作.(不应答或非应答都使ack=0 假)     
      发送数据正常，ack=1; ack=0表示被控器无应答或损坏。
********************************************************************/
static void  SendByte(uint8 c)
{
    uint8 BitCnt;
 
    for(BitCnt = 0; BitCnt < 8; BitCnt ++)  /*要传送的数据长度为8位*/
    {
        if((c<<BitCnt)&0x80) SET_SDA_FM;   /*判断发送位*/
        else  CLR_SDA_FM;                
        delayus();
        SET_SCL_FM;                 /*置时钟线为高，通知被控器开始接收数据位*/
        delayus(); 
        delayus();               /*保证时钟高电平周期大于4μs*/
        delayus();
        delayus();
        delayus();         
        CLR_SCL_FM; 
    }
    
    delayus();
    delayus();
    SET_SDA_FM;               /*8位发送完后释放数据线，准备接收应答位*/
    delayus();
    delayus();   
    SET_SCL_FM;
    delayus();
    delayus();
    delayus();
    
    DATA_IN_FM;  
    delayus(); 
    if(READ_IO_FM) ack=0;     
    else ack=1;        /*判断是否接收到应答信号*/   
    delayus();
    DATA_OUT_FM;
       
    CLR_SCL_FM;
    delayus();
    delayus();
}

/*******************************************************************
                 字节数据传送函数               
函数原型: uint8  RcvByte();
功能:  用来接收从器件传来的数据,并判断总线错误(不发应答信号)，
     发完后请用应答函数。  
********************************************************************/	
static uint8  RcvByte()
{
    uint8 retc;
    uint8 BitCnt;
  
    retc = 0; 
    SET_SDA_FM;             /*置数据线为输入方式*/
    
    for(BitCnt = 0; BitCnt < 8; BitCnt++)
    {
        delayus();           
        CLR_SCL_FM;       /*置时钟线为低，准备接收数据位*/
        delayus();
        delayus();         /*时钟低电平周期大于4.7μs*/
        delayus();
        delayus();
        delayus();
        SET_SCL_FM;       /*置时钟线为高使数据线上数据有效*/
        delayus();
        delayus();
        retc=retc<<1;
        
        DATA_IN_FM;
        delayus();
        if(READ_IO_FM) retc=retc+1; /*读数据位,接收的数据位放入retc中 */
        delayus();
        DATA_OUT_FM;
        
        delayus();
        delayus(); 
    } 
    
    CLR_SCL_FM;    
    delayus();
    delayus();
    return(retc);
}

/********************************************************************
                     应答子函数
原型:  void Ack_I2c(bit a);
 
功能:主控器进行应答信号,(可以是应答或非应答信号)
********************************************************************/
void Ack_I2c(uint8 a)
{
    if(a==0)   CLR_SDA_FM;     /*在此发出应答或非应答信号 */
	else   SET_SDA_FM;
	delayus();
	delayus();
	delayus(); 
	     
	SET_SCL_FM;
    delayus();
    delayus();              /*时钟低电平周期大于4μs*/
    delayus();
    delayus();
    delayus();  
    
 	CLR_SCL_FM;                /*清时钟线，钳住I2C总线以便继续接收*/
    delayus();
    delayus();    
}

static uint8 ISendStr(uint8 sla,uint8 subah, uint8 subal, uint8 *s,uint8 no)
{
   	uint8 i;

   	Start_I2c();              		/*启动总线*/
  
   	SendByte(sla);            		/*发送器件地址*/
   	if(ack == 0) { Stop_I2c(); return(0); }
     
   	SendByte(subah);  				/*发送器件子地址*/
   	if(ack == 0) { Stop_I2c(); return(0); }			//高位
   	
    SendByte(subal);            	
    if(ack == 0) { Stop_I2c(); return(0); }			//低位


   	for(i = 0; i < no; i++)
    {   
     	SendByte(*s);          		/*发送数据*/
       	if(ack == 0)
       	{
       		Stop_I2c(); return(0);
     	}
     	s++;
    } 
 	Stop_I2c();                 	/*结束总线*/ 
 	
  	return(1);
}

static uint8 IRcvStr(uint8 sla,uint8 subah, uint8 subal, uint8 *s,uint8 no)
{
   	uint8 i;

   	Start_I2c();               	/*启动总线*/
   	SendByte(sla);            	/*发送器件地址*/
 	if(ack==0) { Stop_I2c(); return(0); }
     
     
	SendByte(subah);            	/*发送器件地址*/
    if(ack == 0) { Stop_I2c(); return(0); }		//高位
    
    SendByte(subal);            
    if(ack == 0) { Stop_I2c(); return(0); }		//低位
     
     
	Start_I2c();
	SendByte(sla + 1);
    if(ack == 0) { Stop_I2c(); return(0); }

   	for(i = 0; i < (no - 1); i++)
   	{   
	 	*s = RcvByte();          /*发送数据*/
	   	Ack_I2c(0);            /*发送就答位*/  
	   	s++;
   	} 
    
   	*s=RcvByte();
    Ack_I2c(1);                /*发送非应位*/
 	Stop_I2c();                /*结束总线*/ 
 	
  	return(1);
}

//对外接口函数
/*******************************************************************
                    ROM存储器读写操作             
功能: 		ROM存储器读写操作
参数定义：	suba：将要读或写数据的器件子地址首地址
			s：	  发送时，代表将要发的数据区的首地址
				  接收时，代表接收到的数据保存至的首地址
			num： 接收或发送的数据个数
********************************************************************/
uint8 FM_SendStr(uint16 suba, uint8 *s, uint8 num)
{
	uint8 err;
	uint8 subah;
	uint8 subal;

	subah = (uint8)(suba >> 8);
	subal = (uint8)suba;
	err = ISendStr(FM_ADDRESS, subah, subal, &*s, num);
	return err;
}

uint8 FM_ReadStr(uint16 suba, uint8 *s, uint8 num)
{
	uint8 err;
	uint8 subah;
	uint8 subal;

	subah = (uint8)(suba >> 8);
	subal = (uint8)suba;
	err = IRcvStr(FM_ADDRESS, subah, subal, &*s, num);
	return err;
}





