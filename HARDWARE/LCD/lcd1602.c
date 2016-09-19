#include "lcd1602.h"
#include "delay.h"	
#include "usart.h"	
#include "font.h" 


_lcd_dev lcddev;

//写寄存器函数
//regval:寄存器值
void LCD_WR_REG(vu16 regval)
{   
	regval=regval;
	LCD->LCD_REG=regval;
}

//写LCD数据
//data:要写入的值
void LCD_WR_DATA(vu16 data)
{	  
	data=data;			//使用-O2优化的时候,必须插入的延时
	LCD->LCD_RAM=data;		 
}


u16 LCD_RD_DATA(void)
{
	vu16 ram;			//防止被优化
	ram=LCD->LCD_RAM;	
	return ram;	 
}				

void LCD_WriteReg(u16 LCD_Reg,u16 LCD_RegValue)
{	
	LCD->LCD_REG = LCD_Reg;		//写入要写的寄存器序号	 
	LCD->LCD_RAM = LCD_RegValue;//写入数据	    		 
}	  

void LCD_Init(void)
{ 	 
	RCC->AHB1ENR|=0XF<<3;    	//使能PD,PE,PF,PG时钟  
	RCC->AHB1ENR|=1<<1;     	//使能PB时钟  
	RCC->AHB3ENR|=1<<0;     	//使能FSMC时钟  
	GPIO_Set(GPIOB,PIN15,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);							//PB15 推挽输出,控制背光
	GPIO_Set(GPIOD,(3<<0)|(3<<4)|(7<<8)|(3<<14),GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);	//PD0,1,4,5,8,9,10,14,15 AF OUT
	GPIO_Set(GPIOE,(0X1FF<<7),GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);						//PE7~15,AF OUT
	GPIO_Set(GPIOF,PIN12,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);							//PF12,FSMC_A6
	GPIO_Set(GPIOG,PIN12,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU);							//PG12,FSMC_NE4
	GPIO_AF_Set(GPIOD,0,12);	//PD0,AF12 FSMC  //E
// 	GPIO_AF_Set(GPIOD,1,12);	//PD1,AF12
// 	GPIO_AF_Set(GPIOD,4,12);	//PD4,AF12
// 	GPIO_AF_Set(GPIOD,5,12);	//PD5,AF12 
// 	GPIO_AF_Set(GPIOD,8,12);	//PD8,AF12
// 	GPIO_AF_Set(GPIOD,9,12);	//PD9,AF12
// 	GPIO_AF_Set(GPIOD,10,12);	//PD10,AF12 
 	GPIO_AF_Set(GPIOD,14,12);	//PD14,AF12  //CS
 	GPIO_AF_Set(GPIOD,15,12);	//PD15,AF12  //RD
	
// 	GPIO_AF_Set(GPIOE,7,12);	//PE7,AF12
 	GPIO_AF_Set(GPIOA,8,12);	//PE8,AF12  //data
 	GPIO_AF_Set(GPIOA,9,12);	//PE9,AF12  //data
 	GPIO_AF_Set(GPIOA,10,12);	//PE10,AF12 //data
 	GPIO_AF_Set(GPIOE,11,12);	//PE11,AF12  //data
 	GPIO_AF_Set(GPIOE,12,12);	//PE12,AF12  //data
 	GPIO_AF_Set(GPIOE,13,12);	//PE13,AF12  //data
 	GPIO_AF_Set(GPIOE,14,12);	//PE14,AF12  //data
 	GPIO_AF_Set(GPIOE,15,12);	//PE15,AF12  //data
	
 	GPIO_AF_Set(GPIOF,12,12);	//PF12,AF12
 	GPIO_AF_Set(GPIOG,12,12);	//PG12,AF12
	//寄存器清零
	//bank1有NE1~4,每一个有一个BCR+TCR，所以总共八个寄存器。
	//这里我们使用NE1 ，也就对应BTCR[0],[1]。				    
	FSMC_Bank1->BTCR[6]=0X00000000;
	FSMC_Bank1->BTCR[7]=0X00000000;
	FSMC_Bank1E->BWTR[6]=0X00000000;
	//操作BCR寄存器	使用异步模式
	FSMC_Bank1->BTCR[6]|=1<<12;		//存储器写使能
	FSMC_Bank1->BTCR[6]|=1<<14;		//读写使用不同的时序
	FSMC_Bank1->BTCR[6]|=00<<4; 		//存储器数据宽度为16bit 	    
	//操作BTR寄存器	
	//读时序控制寄存器 							    
	FSMC_Bank1->BTCR[7]|=0<<28;		//模式A 	 						  	 
	FSMC_Bank1->BTCR[7]|=0XF<<0; 	//地址建立时间(ADDSET)为15个HCLK 1/168M=6ns*15=90ns	
	//因为液晶驱动IC的读数据的时候，速度不能太快,尤其是个别奇葩芯片。
	FSMC_Bank1->BTCR[7]|=60<<8;  	//数据保存时间(DATAST)为60个HCLK	=6*60=360ns
	//写时序控制寄存器  
	FSMC_Bank1E->BWTR[6]|=0<<28; 	//模式A 	 							    
	FSMC_Bank1E->BWTR[6]|=9<<0;		//地址建立时间(ADDSET)为9个HCLK=54ns
 	//9个HCLK（HCLK=168M）,某些液晶驱动IC的写信号脉宽，最少也得50ns。  	 
	FSMC_Bank1E->BWTR[6]|=8<<8; 	//数据保存时间(DATAST)为6ns*9个HCLK=54ns
	//使能BANK1,区域4
	FSMC_Bank1->BTCR[6]|=1<<0;		//使能BANK1，区域1	    
			 
 	delay_ms(50); // delay 50 ms 
 	LCD_WriteReg(0x0000,0x0001);
	delay_ms(50); // delay 50 ms 
	
	FSMC_Bank1E->BWTR[6]&=~(0XF<<0);//地址建立时间(ADDSET)清零 	 
	FSMC_Bank1E->BWTR[6]&=~(0XF<<8);//数据保存时间清零
	FSMC_Bank1E->BWTR[6]|=3<<0;		//地址建立时间(ADDSET)为3个HCLK =18ns  	 
	FSMC_Bank1E->BWTR[6]|=2<<8; 	//数据保存时间(DATAST)为6ns*3个HCLK=18ns
	
	LCD_WR_REG(0x32);
	LCD_WR_REG(0x28);
	LCD_WR_REG(0x0c);
	LCD_WR_REG(0x06);
	LCD_WR_REG(0x06);
	LCD_WR_REG(0x06);	
}

//在指定位置显示一个字符
//x,y:起始坐标
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16/24
//mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChars(void)
{  							  
  LCD_WR_REG(0x0010);
	LCD_WR_DATA("041");
		
		
}   

