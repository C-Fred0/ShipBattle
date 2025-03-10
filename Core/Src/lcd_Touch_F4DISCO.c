// 2014, Petr Machala, en otra: Copyright (C) Tilen Majerle, 2014

#include "stm32f4xx_hal.h"
#include "lcd_fonts.h"


/*------------- LCD -----------
ILI9341      STM32F4xx    DESCRIPTION
SDO (MISO    PF8          Output from LCD for SPI5.	Not used, can be left
LED          3.3V         Backlight
SCK          PF7          SPI5 clock
SDI (MOSI)   PF9          SPI5 master output
WRX or D/C   PD13         Data/Command register
RESET        PD12         Reset LCD
CS           PC2          Chip select for SPI5
GND          GND          Ground
VCC          3.3V         Positive power supply

---------------- TOUCH ----------



Dependencias: entrar con el SPI5 inicializado a 2Mbits/seg, inicializado sale con 45Mbits/seg
							entrar con el I2C3 inicializado a 100Kbits/seg. "Necesita stm32fxx_hal_i2c"
*/

//********************************************** DEFINICIONES ************************************************
#define ILI9341_RST_SET				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET)
#define ILI9341_RST_RESET			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET)
#define ILI9341_CS_SET				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET)
#define ILI9341_CS_RESET			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET)
#define ILI9341_WRX_SET				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET)
#define ILI9341_WRX_RESET			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET)

#define ILI9341_WIDTH 					240
#define ILI9341_HEIGHT					320
#define ILI9341_PIXEL						76800 //= 320x240 (16 bit )



#define ILI9341_SPI_IMAGE_BLOCK	51200 //? (8 bit)

//Colors
#define ILI9341_COLOR_WHITE			0xFFFF
#define ILI9341_COLOR_BLACK			0x0000
#define ILI9341_COLOR_RED			0xF800
#define ILI9341_COLOR_GREEN			0x07E0
#define ILI9341_COLOR_GREEN2		0xB723
#define ILI9341_COLOR_BLUE			0x001F
#define ILI9341_COLOR_BLUE2			0x051D
#define ILI9341_COLOR_YELLOW		0xFFE0
#define ILI9341_COLOR_ORANGE		0xFBE4
#define ILI9341_COLOR_CYAN			0x07FF
#define ILI9341_COLOR_MAGENTA		0xA254
#define ILI9341_COLOR_GRAY			0x7BEF //1111 0111 1101 1110
#define ILI9341_COLOR_BROWN			0xBBCA

//Commands
#define ILI9341_RESET			0x01
#define ILI9341_SLEEP_OUT		0x11
#define ILI9341_GAMMA			0x26
#define ILI9341_DISPLAY_OFF		0x28
#define ILI9341_DISPLAY_ON		0x29
#define ILI9341_COLUMN_ADDR		0x2A
#define ILI9341_PAGE_ADDR		0x2B
#define ILI9341_GRAM			0x2C
#define ILI9341_MAC				0x36
#define ILI9341_PIXEL_FORMAT	0x3A
#define ILI9341_WDB				0x51
#define ILI9341_WCD				0x53
#define ILI9341_RGB_INTERFACE	0xB0
#define ILI9341_FRC				0xB1
#define ILI9341_BPC				0xB5
#define ILI9341_DFC				0xB6
#define ILI9341_POWER1			0xC0
#define ILI9341_POWER2			0xC1
#define ILI9341_VCOM1			0xC5
#define ILI9341_VCOM2			0xC7
#define ILI9341_POWERA			0xCB
#define ILI9341_POWERB			0xCF
#define ILI9341_PGAMMA			0xE0
#define ILI9341_NGAMMA			0xE1
#define ILI9341_DTCA			0xE8
#define ILI9341_DTCB			0xEA
#define ILI9341_POWER_SEQ		0xED
#define ILI9341_3GAMMA_EN		0xF2
#define ILI9341_INTERFACE		0xF6
#define ILI9341_PRC				0xF7

//----------------------------------- TOUCH ---------------
#define TP_ADDR                   0x82
#define STMPE811_ID                0x0811

#define TP_I2C_DR                 ((uint32_t)0x40005C10)

//REGISTROS DEL STMPE811
/* Identification registers */
#define TP_REG_CHP_ID             0x00
#define TP_REG_ID_VER             0x02

/* General Control Registers */
#define TP_REG_SYS_CTRL1          0x03
#define TP_REG_SYS_CTRL2          0x04
#define TP_REG_SPI_CFG            0x08

/* Interrupt Control register */
#define TP_REG_INT_CTRL           0x09
#define TP_REG_INT_EN             0x0A
#define TP_REG_INT_STA            0x0B
#define TP_REG_GPIO_INT_EN        0x0C
#define TP_REG_GPIO_INT_STA       0x0D

/* GPIO Registers */
#define TP_REG_GPIO_SET_PIN       0x10
#define TP_REG_GPIO_CLR_PIN       0x11
#define TP_REG_GPIO_MP_STA        0x12
#define TP_REG_GPIO_DIR           0x13
#define TP_REG_GPIO_ED            0x14
#define TP_REG_GPIO_RE            0x15
#define TP_REG_GPIO_FE            0x16
#define TP_REG_GPIO_AF            0x17

/* ADC Registers */
#define TP_REG_ADC_INT_EN         0x0E
#define TP_REG_ADC_INT_STA        0x0F
#define TP_REG_ADC_CTRL1          0x20
#define TP_REG_ADC_CTRL2          0x21
#define TP_REG_ADC_CAPT           0x22
#define TP_REG_ADC_DATA_CH0       0x30
#define TP_REG_ADC_DATA_CH1       0x32
#define TP_REG_ADC_DATA_CH2       0x34
#define TP_REG_ADC_DATA_CH3       0x36
#define TP_REG_ADC_DATA_CH4       0x38
#define TP_REG_ADC_DATA_CH5       0x3A
#define TP_REG_ADC_DATA_CH6       0x3B
#define TP_REG_ADC_DATA_CH7       0x3C

/* TouchPanel Registers  */
#define TP_REG_TP_CTRL            0x40
#define TP_REG_TP_CFG             0x41
#define TP_REG_WDM_TR_X           0x42
#define TP_REG_WDM_TR_Y           0x44
#define TP_REG_WDM_BL_X           0x46
#define TP_REG_WDM_BL_Y           0x48
#define TP_REG_FIFO_TH            0x4A
#define TP_REG_FIFO_STA           0x4B
#define TP_REG_FIFO_SIZE          0x4C
#define TP_REG_TP_DATA_X          0x4D
#define TP_REG_TP_DATA_Y          0x4F
#define TP_REG_TP_DATA_Z          0x51
#define TP_REG_TP_DATA_XYZ        0x52
#define TP_REG_TP_FRACT_XYZ       0x56
#define TP_REG_TP_DATA            0x57
#define TP_REG_TP_I_DRIVE         0x58
#define TP_REG_TP_SHIELD          0x59

/*IO Expander Functionalities definitions  */
#define TP_ADC_FCT                0x01
#define TP_TP_FCT                 0x02
#define TP_IO_FCT                 0x04

/* IO Pins  */
#define IO_Pin_0                   0x01
#define IO_Pin_1                   0x02
#define IO_Pin_2                   0x04
#define IO_Pin_3                   0x08
#define IO_Pin_4                   0x10
#define IO_Pin_5                   0x20
#define IO_Pin_6                   0x40
#define IO_Pin_7                   0x80
#define IO_Pin_ALL                 0xFF

/* Touch Panel Pins definition  */
#define TOUCH_YD                   IO_Pin_1
#define TOUCH_XD                   IO_Pin_2
#define TOUCH_YU                   IO_Pin_3
#define TOUCH_XU                   IO_Pin_4
#define TOUCH_IO_ALL               (uint32_t)(IO_Pin_1 | IO_Pin_2 | IO_Pin_3 | IO_Pin_4)


//************************************************ VARIABLES *************************************************
uint32_t Timeout=5000; //para enviar imagen por el spi
uint32_t pix=0;
uint8_t  aux8=0;
uint32_t tickstart = 0;

SPI_HandleTypeDef lcd_spi;

//Select orientation for LCD
typedef enum {
	LCD_ILI9341_Orientation_Portrait_1,
	LCD_ILI9341_Orientation_Portrait_2,
	LCD_ILI9341_Orientation_Landscape_1,
	LCD_ILI9341_Orientation_Landscape_2
} LCD_ILI9341_Orientation_t;

//Orientation, Used private
typedef enum {
	LCD_ILI9341_Landscape,
	LCD_ILI9341_Portrait
} LCD_ILI9341_Orientation;

//LCD options, Used private
typedef struct {
	uint16_t width;
	uint16_t height;
	LCD_ILI9341_Orientation orientation; // 1 = portrait; 0 = landscape
} LCD_ILI931_Options_t;

uint16_t ILI9341_x;
uint16_t ILI9341_y;
LCD_ILI931_Options_t ILI9341_Opts;
uint8_t ILI9341_INT_CalledFromPuts = 0;

//solo aqui: Select font
LCD_FontDef_t LCD_Font_7x10; //extern
LCD_FontDef_t LCD_Font_11x18;
LCD_FontDef_t LCD_Font_16x26;


//-------------------------------------- TOUCH ----------------------
I2C_HandleTypeDef hi2c3;

typedef struct
{
  uint16_t TouchDetected;
  uint16_t X;
  uint16_t Y;
  uint16_t Z;
}TP_STATE;


TP_STATE TP_State;              /* The global structure holding the TS state */
//uint32_t TP_TimeOut = TIMEOUT_MAX; /* Value of Timeout when I2C communication fails */


//************************************** DECLARACION DE FUNCIONES LCD *****************************************
//Initialize ILI9341 LCD
void LCD_ILI9341_Init(SPI_HandleTypeDef *lcd_spi); 																				//OK
//Simple delay, - volatile unsigned int delay: clock cycles
void LCD_ILI9341_Delay(volatile unsigned int delay);																			//OK
//Draw single pixel to LCD, x: X position for pixel, y: Y position for pixel, color: color of pixel
void LCD_ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
//Send data to LCD via SPI, - uint8_t data: data to be sent
void LCD_ILI9341_SendData(uint8_t data);																									//OK
//Send command to LCD via SPI,	- uint8_t data: data to be sent
void LCD_ILI9341_SendCommand(uint8_t data);																								//OK
//Set cursor position
void LCD_ILI9341_SetCursorPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);		//OK
//Fill entire LCD with color,  color: Color to be used in fill
void LCD_ILI9341_Fill(uint16_t color);																										//OK con varias diferencias
//Rotate LCD,
//*		- LCD_ILI9341_Orientation_Portrait_1: no rotation
//*		- LCD_ILI9341_Orientation_Portrait_2: rotate 180deg
//*		- LCD_ILI9341_Orientation_Landscape_1: rotate 90deg
//*		- LCD_ILI9341_Orientation_Landscape_2: rotate -90deg
void LCD_ILI9341_Rotate(LCD_ILI9341_Orientation_t orientation);														//OK
//Put string to LCD,
// - x: X position of top left corner of first character in string
// - y: Y position of top left corner of first character in string
// - *str: pointer to first character
// - *font: pointer to used font
// - foreground: color for string
// - background: color for string background
void LCD_ILI9341_Puts(uint16_t x, uint16_t y, char *str, LCD_FontDef_t *font, uint16_t foreground, uint16_t background);
//Get width and height of box with text,
// - *str: pointer to first character
// - *font: pointer to used font
// - *width: Pointer to variable to store width
// - *height: ointer to variable to store height
void LCD_ILI9341_GetStringSize(char *str, LCD_FontDef_t *font, uint16_t *width, uint16_t *height);//OK
//Put single character to LCD, x: X position of top left corner, y: Y position of top left corner
// 	- c: character to be displayed, *font: pointer to used font
// 	- foreground: color for char, - background: color for char background
void LCD_ILI9341_Putc(uint16_t x, uint16_t y, char c, LCD_FontDef_t *font, uint16_t foreground, uint16_t background);
//Draw line to LCD
// - x0: X coordinate of starting point
// - y0: Y coordinate of starting point
// - x1: X coordinate of ending point
// - y1: Y coordinate of ending point
// - color: line color
void LCD_ILI9341_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);	//OK
//Draw rectangle on LCD
// - x0: X coordinate of top left point
// - y0: Y coordinate of top left point
// - x1: X coordinate of bottom right point
// - y1: Y coordinate of bottom right point
// - color: rectangle color
void LCD_ILI9341_DrawRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color); //OK
//Draw filled rectangle on LCD
// - x0: X coordinate of top left point
// - y0: Y coordinate of top left point
// - x1: X coordinate of bottom right point
// - y1: Y coordinate of bottom right point
// - color: rectangle color
void LCD_ILI9341_DrawFilledRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color); //OK
//Draw circle on LCD
// - x0: X coordinate of center circle point
// - y0: Y coordinate of center circle point
// - r: circle radius
// - color: circle color
void LCD_ILI9341_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
//Draw filled circle on LCD
// - x0: X coordinate of center circle point
// - y0: Y coordinate of center circle point
// - r: circle radius
// - color: circle color
void LCD_ILI9341_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);

//envio por SPI mas respuesta
void LCD_SPI_Send(uint8_t* data, uint16_t size);
//Set higher SPI baud rate for faster LCD refresh rate
void LCD_SPI_BaudRateUp(void);
//Show selected QVGA image on entire LCD, image: Array of selected image in RB565 format
HAL_StatusTypeDef LCD_ILI9341_DisplayImage(uint32_t image[ILI9341_PIXEL]);

/* FUNCTIONS TO WRITE DIRECTLY IN THE IMAGE FRAME BUFFER */

//Draw single pixel to image buffer
// - x: X position for pixel
// - y: Y position for pixel
// - color: color of pixel
// - *image: pointer to image buffer
void LCD_ILI9341_DrawPixel_ImageBuffer(uint16_t x, uint16_t y, uint16_t color, uint32_t image[ILI9341_PIXEL]);
//Put single character to image buffer
// - x: X position of top left corner
// - y: Y position of top left corner
// - c: character to be displayed
// - *font: pointer to used font
// - foreground: color for char
// - *image: pointer to image buffer
void LCD_ILI9341_Putc_ImageBuffer(uint16_t x, uint16_t y, char c, LCD_FontDef_t *font, uint16_t foreground, uint32_t image[ILI9341_PIXEL]);
//Put string to image buffer
// - x: X position of top left corner of first character in string
// - y: Y position of top left corner of first character in string
// - *str: pointer to first character
// - *font: pointer to used font
// - foreground: color for string
// - *image: pointer to image buffer
void LCD_ILI9341_Puts_ImageBuffer(uint16_t x, uint16_t y, char *str, LCD_FontDef_t *font, uint16_t foreground, uint32_t image[ILI9341_PIXEL]);
//Draw line to image buffer
// - x0: X coordinate of starting point
// - y0: Y coordinate of starting point
// - x1: X coordinate of ending point
// - y1: Y coordinate of ending point
// - *image: pointer to image buffer
void LCD_ILI9341_DrawLine_ImageBuffer(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color, uint32_t image[ILI9341_PIXEL]);
//Draw rectangle in image buffer
// - x0: X coordinate of top left point
// - y0: Y coordinate of top left point
// - x1: X coordinate of bottom right point
// - y1: Y coordinate of bottom right point
// - color: rectangle color
// - *image: pointer to image buffer
void LCD_ILI9341_DrawRectangle_ImageBuffer(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color, uint32_t image[ILI9341_PIXEL]);
//Draw filled rectangle in image buffer
// - x0: X coordinate of top left point
// - y0: Y coordinate of top left point
// - x1: X coordinate of bottom right point
// - y1: Y coordinate of bottom right point
// - color: rectangle color
// - *image: pointer to image buffer
void LCD_ILI9341_DrawFilledRectangle_ImageBuffer(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color, uint32_t image[ILI9341_PIXEL]);


//--------------------------- DECLARACION DE FUNCIONES TOUCH -------------------------------------------
uint8_t  TP_Config(void);
TP_STATE* TP_GetState(void);
static uint16_t TP_Read_X(void);
static uint16_t TP_Read_Y(void);
static uint16_t TP_Read_Z(void);
uint8_t  TP_Reset(void);
uint8_t  TP_FnctCmd(uint8_t Fct, FunctionalState NewState);
uint8_t  TP_IOAFConfig(uint8_t IO_Pin, FunctionalState NewState);
uint8_t  TP_ReadDeviceRegister(uint8_t RegisterAddr);
uint8_t  TP_WriteDeviceRegister(uint8_t RegisterAddr, uint8_t RegisterValue);
uint16_t TP_ReadDataBuffer(uint32_t RegisterAddr);




//************************************************ FUNCIONES *************************************************
void LCD_ILI9341_Init(SPI_HandleTypeDef* lcd_hspi)
{
	ILI9341_CS_SET;
	lcd_spi = *lcd_hspi;

	ILI9341_RST_SET;
	LCD_ILI9341_SendCommand(ILI9341_RESET);
	LCD_ILI9341_Delay(200000);
	LCD_ILI9341_SendCommand(ILI9341_POWERA);
	LCD_ILI9341_SendData(0x39);
	LCD_ILI9341_SendData(0x2C);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendData(0x34);
	LCD_ILI9341_SendData(0x02);
	LCD_ILI9341_SendCommand(ILI9341_POWERB);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendData(0xC1);
	LCD_ILI9341_SendData(0x30);
	LCD_ILI9341_SendCommand(ILI9341_DTCA);
	LCD_ILI9341_SendData(0x85);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendData(0x78);
	LCD_ILI9341_SendCommand(ILI9341_DTCB);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendCommand(ILI9341_POWER_SEQ);
	LCD_ILI9341_SendData(0x64);
	LCD_ILI9341_SendData(0x03);
	LCD_ILI9341_SendData(0x12);
	LCD_ILI9341_SendData(0x81);
	LCD_ILI9341_SendCommand(ILI9341_PRC);
	LCD_ILI9341_SendData(0x20);
	LCD_ILI9341_SendCommand(ILI9341_POWER1);
	LCD_ILI9341_SendData(0x23);
	LCD_ILI9341_SendCommand(ILI9341_POWER2);
	LCD_ILI9341_SendData(0x10);
	LCD_ILI9341_SendCommand(ILI9341_VCOM1);
	LCD_ILI9341_SendData(0x3E);
	LCD_ILI9341_SendData(0x28);
	LCD_ILI9341_SendCommand(ILI9341_VCOM2);
	LCD_ILI9341_SendData(0x86);
	LCD_ILI9341_SendCommand(ILI9341_MAC);
	LCD_ILI9341_SendData(0x48); //LCD_ILI9341_SendData(0x48);
	LCD_ILI9341_SendCommand(ILI9341_PIXEL_FORMAT);
	LCD_ILI9341_SendData(0x55);
	LCD_ILI9341_SendCommand(ILI9341_FRC);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendData(0x18);
	LCD_ILI9341_SendCommand(ILI9341_DFC);
	LCD_ILI9341_SendData(0x08);
	LCD_ILI9341_SendData(0x82);
	LCD_ILI9341_SendData(0x27);
	LCD_ILI9341_SendCommand(ILI9341_3GAMMA_EN);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendCommand(ILI9341_COLUMN_ADDR);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendData(0xEF);
	LCD_ILI9341_SendCommand(ILI9341_PAGE_ADDR);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendData(0x01);
	LCD_ILI9341_SendData(0x3F);
	LCD_ILI9341_SendCommand(ILI9341_GAMMA);
	LCD_ILI9341_SendData(0x01);
	LCD_ILI9341_SendCommand(ILI9341_PGAMMA);
	LCD_ILI9341_SendData(0x0F);
	LCD_ILI9341_SendData(0x31);
	LCD_ILI9341_SendData(0x2B);
	LCD_ILI9341_SendData(0x0C);
	LCD_ILI9341_SendData(0x0E);
	LCD_ILI9341_SendData(0x08);
	LCD_ILI9341_SendData(0x4E);
	LCD_ILI9341_SendData(0xF1);
	LCD_ILI9341_SendData(0x37);
	LCD_ILI9341_SendData(0x07);
	LCD_ILI9341_SendData(0x10);
	LCD_ILI9341_SendData(0x03);
	LCD_ILI9341_SendData(0x0E);
	LCD_ILI9341_SendData(0x09);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendCommand(ILI9341_NGAMMA);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendData(0x0E);
	LCD_ILI9341_SendData(0x14);
	LCD_ILI9341_SendData(0x03);
	LCD_ILI9341_SendData(0x11);
	LCD_ILI9341_SendData(0x07);
	LCD_ILI9341_SendData(0x31);
	LCD_ILI9341_SendData(0xC1);
	LCD_ILI9341_SendData(0x48);
	LCD_ILI9341_SendData(0x08);
	LCD_ILI9341_SendData(0x0F);
	LCD_ILI9341_SendData(0x0C);
	LCD_ILI9341_SendData(0x31);
	LCD_ILI9341_SendData(0x36);
	LCD_ILI9341_SendData(0x0F);

	LCD_ILI9341_SendCommand(ILI9341_INTERFACE);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendData(0x00);
	LCD_ILI9341_SendData(0x00);

	LCD_ILI9341_SendCommand(ILI9341_SLEEP_OUT);
	LCD_ILI9341_Delay(1000000);
	LCD_ILI9341_SendCommand(ILI9341_DISPLAY_ON);
	LCD_ILI9341_SendCommand(ILI9341_GRAM);

	ILI9341_x = ILI9341_y = 0;
	ILI9341_Opts.width = ILI9341_WIDTH;
	ILI9341_Opts.height = ILI9341_HEIGHT;
	ILI9341_Opts.orientation = LCD_ILI9341_Portrait;

	LCD_SPI_BaudRateUp(); //45Mbits/seg
}

//Igual en ambas
void LCD_ILI9341_Delay(volatile unsigned int delay) {
	for (; delay != 0; delay--);
}

//En otra, pero con la diferencia abajo
void LCD_ILI9341_SendCommand(uint8_t data) {
	ILI9341_WRX_RESET;
	ILI9341_CS_RESET;
	LCD_SPI_Send(&data, 1);	//En otra: TM_SPI_Send(ILI9341_SPI, data);
	ILI9341_CS_SET;
}

//En otra, pero con la diferencia abajo
void LCD_ILI9341_SendData(uint8_t data) {
	ILI9341_WRX_SET;
	ILI9341_CS_RESET;
	LCD_SPI_Send(&data, 1);  //En otra: TM_SPI_Send(ILI9341_SPI, data);
	ILI9341_CS_SET;
}

//En otra, pero con uint32_t color
void LCD_ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
	LCD_ILI9341_SetCursorPosition(x, y, x, y);
	LCD_ILI9341_SendCommand(ILI9341_GRAM);
	LCD_ILI9341_SendData(color >> 8);
	LCD_ILI9341_SendData(color & 0xFF);
}

//En ambas igual
void LCD_ILI9341_SetCursorPosition(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	LCD_ILI9341_SendCommand(ILI9341_COLUMN_ADDR);
	LCD_ILI9341_SendData(x1 >> 8);
	LCD_ILI9341_SendData(x1 & 0xFF);
	LCD_ILI9341_SendData(x2 >> 8);
	LCD_ILI9341_SendData(x2 & 0xFF);

	LCD_ILI9341_SendCommand(ILI9341_PAGE_ADDR);
	LCD_ILI9341_SendData(y1 >> 8);
	LCD_ILI9341_SendData(y1 & 0xFF);
	LCD_ILI9341_SendData(y2 >> 8);
	LCD_ILI9341_SendData(y2 & 0xFF);
}

//En la otra uint32_t color y se compone de TM_ILI9341_Fill(uint32_t color)
// + void TM_ILI9341_INT_Fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
void LCD_ILI9341_Fill(uint16_t color)
{
	//solo en otra: uint32_t pixels_count;
	unsigned int n, i, j;
	i = color >> 8;
	j = color & 0xFF;

	//igual
	LCD_ILI9341_SetCursorPosition(0, 0, ILI9341_Opts.width - 1, ILI9341_Opts.height - 1);
	//igual
	LCD_ILI9341_SendCommand(ILI9341_GRAM);

	for (n = 0; n < ILI9341_PIXEL; n++) {
		LCD_ILI9341_SendData(i);
		LCD_ILI9341_SendData(j);
	}

	//solo en la otra
	//pixels_count = (x1 - x0 + 1) * (y1 - y0 + 1); 				// Calculate pixels count
	//ILI9341_CS_RESET;																			// Send everything
	//ILI9341_WRX_SET;
	//TM_SPI_SetDataSize(ILI9341_SPI, TM_SPI_DataSize_16b); // Go to 16-bit SPI mode
	//  Send first 65535 bytes, SPI MUST BE IN 16-bit MODE
	//TM_SPI_DMA_SendHalfWord(ILI9341_SPI, color, (pixels_count > 0xFFFF) ? 0xFFFF : pixels_count);
	//while (TM_SPI_DMA_Working(ILI9341_SPI));							// Wait till done
	//if (pixels_count > 0xFFFF) 														// Check again
	//{
	//	TM_SPI_DMA_SendHalfWord(ILI9341_SPI, color, pixels_count - 0xFFFF); // Send remaining data
	//	while (TM_SPI_DMA_Working(ILI9341_SPI));						// Wait till done
	//}
	//ILI9341_CS_SET;
	//TM_SPI_SetDataSize(ILI9341_SPI, TM_SPI_DataSize_8b);	// Go back to 8-bit SPI mode
}

//Igual en ambas
void LCD_ILI9341_Rotate(LCD_ILI9341_Orientation_t orientation) {
	LCD_ILI9341_SendCommand(ILI9341_MAC);
	if (orientation == LCD_ILI9341_Orientation_Portrait_1) {
		LCD_ILI9341_SendData(0x58); //LCD_ILI9341_SendData(0x58); 0101
	} else if (orientation == LCD_ILI9341_Orientation_Portrait_2) {
		LCD_ILI9341_SendData(0x88);//LCD_ILI9341_SendData(0x88);  1000
	} else if (orientation == LCD_ILI9341_Orientation_Landscape_1) {
		LCD_ILI9341_SendData(0x28);//LCD_ILI9341_SendData(0x28);  0010
	} else if (orientation == LCD_ILI9341_Orientation_Landscape_2) {
		LCD_ILI9341_SendData(0xE8);//LCD_ILI9341_SendData(0xE8);  1110
	}

	if (orientation == LCD_ILI9341_Orientation_Portrait_1 || orientation == LCD_ILI9341_Orientation_Portrait_2) {
		ILI9341_Opts.width = ILI9341_WIDTH;
		ILI9341_Opts.height = ILI9341_HEIGHT;
		ILI9341_Opts.orientation = LCD_ILI9341_Portrait;
	} else {
		ILI9341_Opts.width = ILI9341_HEIGHT;
		ILI9341_Opts.height = ILI9341_WIDTH;
		ILI9341_Opts.orientation = LCD_ILI9341_Landscape;
	}
}

//En la otra: uint32_t foreground, uint32_t background, lo demas igual
void LCD_ILI9341_Puts(uint16_t x, uint16_t y, char *str, LCD_FontDef_t *font, uint16_t foreground, uint16_t background)
{
	uint16_t startX = x;

	ILI9341_x = x;						// Set X and Y coordinates
	ILI9341_y = y;

	while (*str) {
		//New line
		if (*str == '\n') {
			ILI9341_y += font->FontHeight + 1;
			//if after \n is also \r, than go to the left of the screen
			if (*(str + 1) == '\r') {
				ILI9341_x = 0;
				str++;
			} else {
				ILI9341_x = startX;
			}
			str++;
			continue;
		} else if (*str == '\r') {
			str++;
			continue;
		}

		LCD_ILI9341_Putc(ILI9341_x, ILI9341_y, *str++, font, foreground, background);
	}
}

//Igual en ambas
void LCD_ILI9341_GetStringSize(char *str, LCD_FontDef_t *font, uint16_t *width, uint16_t *height) {
	uint16_t w = 0;
	*height = font->FontHeight;
	while (*str++) {
		w += font->FontWidth;
	}
	*width = w;
}


//Igual ambas, pero la otra: uint32_t foreground, uint32_t background
void LCD_ILI9341_Putc(uint16_t x, uint16_t y, char c, LCD_FontDef_t *font, uint16_t foreground, uint16_t background) {
	uint32_t i, b, j;
	/* Set coordinates */
	ILI9341_x = x;
	ILI9341_y = y;
	if ((ILI9341_x + font->FontWidth) > ILI9341_Opts.width) {
		//If at the end of a line of display, go to new line and set x to 0 position
		ILI9341_y += font->FontHeight;
		ILI9341_x = 0;
	}

	//solo en la otra:
	//TM_ILI9341_INT_Fill(ILI9341_x, ILI9341_y, ILI9341_x + font->FontWidth, ILI9341_y + font->FontHeight, background);

	for (i = 0; i < font->FontHeight; i++) {
		b = font->data[(c - 32) * font->FontHeight + i];
		for (j = 0; j < font->FontWidth; j++) {
			if ((b << j) & 0x8000)
			{
				LCD_ILI9341_DrawPixel(ILI9341_x + j, (ILI9341_y + i), foreground);
			}
			else //Esto no esta en la otra
			{
				LCD_ILI9341_DrawPixel(ILI9341_x + j, (ILI9341_y + i), background);
			}
		}
	}
	ILI9341_x += font->FontWidth;
}

//En la otra: uint32_t color
void LCD_ILI9341_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
	int16_t dx, dy, sx, sy, err, e2;
	//en la otra: uint16_t tmp;

	/* Check for overflow */
	if (x0 >= ILI9341_Opts.width) {
		x0 = ILI9341_Opts.width - 1;
	}
	if (x1 >= ILI9341_Opts.width) {
		x1 = ILI9341_Opts.width - 1;
	}
	if (y0 >= ILI9341_Opts.height) {
		y0 = ILI9341_Opts.height - 1;
	}
	if (y1 >= ILI9341_Opts.height) {
		y1 = ILI9341_Opts.height - 1;
	}

	//solo en otra
	//if (x0 > x1) {			// Check correction
	//	tmp = x0;
	//	x0 = x1;
	//	x1 = tmp;
	//}
	//if (y0 > y1) {
	//	tmp = y0;
	//	y0 = y1;
	//	y1 = tmp;
	//}
	//dx = x1 - x0;
	//dy = y1 - y0;

	//solo aqui
	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1);
	dy = (y0 < y1) ? (y1 - y0) : (y0 - y1);

	//solo en otra:
	//if (dx == 0 || dy == 0) {		// Vertical or horizontal line
	//	TM_ILI9341_INT_Fill(x0, y0, x1, y1, color);
	//	return;
	//}

	//Ambas
	sx = (x0 < x1) ? 1 : -1;
	sy = (y0 < y1) ? 1 : -1;
	err = ((dx > dy) ? dx : -dy) / 2;

	while (1) {
		LCD_ILI9341_DrawPixel(x0, y0, color);
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

//En la otra: uint32_t color
void LCD_ILI9341_DrawRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
	LCD_ILI9341_DrawLine(x0, y0, x1, y0, color); //Top
	LCD_ILI9341_DrawLine(x0, y0, x0, y1, color);	//Left
	LCD_ILI9341_DrawLine(x1, y0, x1, y1, color);	//Right
	LCD_ILI9341_DrawLine(x0, y1, x1, y1, color);	//Bottom
}

//En otra: uint32_t color
void LCD_ILI9341_DrawFilledRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
	for (; x0 < x1; x0++) {
		LCD_ILI9341_DrawLine(x0, y0, x0, y1, color);
	}

	//En otra:
	//uint16_t tmp;
	//if (x0 > x1) {			// Check correction
	//	tmp = x0;
	//	x0 = x1;
	//	x1 = tmp;
	//}
	//if (y0 > y1) {
	//	tmp = y0;
	//	y0 = y1;
	//	y1 = tmp;
	//}
	//TM_ILI9341_INT_Fill(x0, y0, x1, y1, color); // Fill rectangle, funcion parte de TM_ILI9341_Fill
	//ILI9341_CS_SET;															// CS HIGH back
}

//Iguales, pero en otra: uint32_t color
void LCD_ILI9341_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	//Igual ambas
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    LCD_ILI9341_DrawPixel(x0, y0 + r, color);
    LCD_ILI9341_DrawPixel(x0, y0 - r, color);
    LCD_ILI9341_DrawPixel(x0 + r, y0, color);
    LCD_ILI9341_DrawPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        LCD_ILI9341_DrawPixel(x0 + x, y0 + y, color);
        LCD_ILI9341_DrawPixel(x0 - x, y0 + y, color);
        LCD_ILI9341_DrawPixel(x0 + x, y0 - y, color);
        LCD_ILI9341_DrawPixel(x0 - x, y0 - y, color);

        LCD_ILI9341_DrawPixel(x0 + y, y0 + x, color);
        LCD_ILI9341_DrawPixel(x0 - y, y0 + x, color);
        LCD_ILI9341_DrawPixel(x0 + y, y0 - x, color);
        LCD_ILI9341_DrawPixel(x0 - y, y0 - x, color);
    }
}

//Iguales, pero en otra: uint32_t color
void LCD_ILI9341_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    LCD_ILI9341_DrawPixel(x0, y0 + r, color);
    LCD_ILI9341_DrawPixel(x0, y0 - r, color);
    LCD_ILI9341_DrawPixel(x0 + r, y0, color);
    LCD_ILI9341_DrawPixel(x0 - r, y0, color);
    LCD_ILI9341_DrawLine(x0 - r, y0, x0 + r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        LCD_ILI9341_DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, color);
        LCD_ILI9341_DrawLine(x0 + x, y0 - y, x0 - x, y0 - y, color);

        LCD_ILI9341_DrawLine(x0 + y, y0 + x, x0 - y, y0 + x, color);
        LCD_ILI9341_DrawLine(x0 + y, y0 - x, x0 - y, y0 - x, color);
    }
}







//------------------------------------- SOLO AQUI -----------------------------------------------
void LCD_SPI_Send(uint8_t* data, uint16_t size) {
	switch(HAL_SPI_Transmit(&lcd_spi, data, size, 5000))
  {
  case HAL_OK:
    //GREEN LED on
    break;

  case HAL_TIMEOUT:
    /* Call Timeout Handler */
    //Timeout_Error_Handler();
    break;

  case HAL_ERROR:
    /* Call Error Handler */
    //Error_Handler();
    break;

  default:
    break;
  }
}


//CORREGIDA ---------------------------------------------------------------------
HAL_StatusTypeDef LCD_ILI9341_DisplayImage(uint32_t image[ILI9341_PIXEL])
{
	LCD_ILI9341_SetCursorPosition(0, 0, ILI9341_Opts.width - 1, ILI9341_Opts.height - 1);		//OK
	LCD_ILI9341_SendCommand(ILI9341_GRAM);
	ILI9341_WRX_SET;
	ILI9341_CS_RESET;

	if((&lcd_spi)->State == HAL_SPI_STATE_READY)
  {
    assert_param(IS_SPI_DIRECTION_2LINES_OR_1LINE((&lcd_spi)->Init.Direction)); // Check the parameters
    __HAL_LOCK((&lcd_spi));																			// Process Locked
    (&lcd_spi)->State = HAL_SPI_STATE_BUSY_TX;									// Configure communication
    (&lcd_spi)->ErrorCode   = HAL_SPI_ERROR_NONE;
    (&lcd_spi)->TxISR = 0;																			// Init field not used in handle to zero
    (&lcd_spi)->RxISR = 0;
    (&lcd_spi)->RxXferSize   = 0;
    (&lcd_spi)->RxXferCount  = 0;
    if((&lcd_spi)->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE) // Reset CRC Calculation
    {
      SPI_RESET_CRC((&lcd_spi));
    }
    if((&lcd_spi)->Init.Direction == SPI_DIRECTION_1LINE)
    {
      SPI_1LINE_TX((&lcd_spi));																	// Configure communication direction : 1Line
    }
    if(((&lcd_spi)->Instance->CR1 &SPI_CR1_SPE) != SPI_CR1_SPE) // Check if the SPI is already enabled
    {
      __HAL_SPI_ENABLE((&lcd_spi));															// Enable SPI peripheral
    }

		for(uint32_t k=0; k<IMG_TOTAL_BUF32; k++)
		{
			pix=image[k];

			//------------------------Primer pixel en la parte baja ---------------------------------------------
			aux8=(uint8_t)(pix >> 8);
      (&lcd_spi)->Instance->DR = aux8;  											// Transmit data in 8 Bit mode
      // Wait until TXE flag is set to send data
			tickstart = HAL_GetTick();														// Get tick
			while(__HAL_SPI_GET_FLAG((&lcd_spi),SPI_FLAG_TXE) == RESET)
			{
				if(Timeout != HAL_MAX_DELAY)
				{
					if((Timeout == 0)||((HAL_GetTick() - tickstart ) > Timeout))
					{
						__HAL_SPI_DISABLE_IT((&lcd_spi), (SPI_IT_TXE | SPI_IT_RXNE | SPI_IT_ERR)); // Disable TXE, RXNE and ERR interrupts for the interrupt process
						__HAL_SPI_DISABLE((&lcd_spi));									// Disable SPI peripheral
						if((&lcd_spi)->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE)					// Reset CRC Calculation
						{
							SPI_RESET_CRC((&lcd_spi));
						}
						(&lcd_spi)->State= HAL_SPI_STATE_READY;
						__HAL_UNLOCK((&lcd_spi));												// Process Unlocked
						ILI9341_CS_SET;
						ILI9341_WRX_RESET;
						return HAL_TIMEOUT;
					}
				}
			}

			aux8=(uint8_t)(pix & 0x000000FF);
      (&lcd_spi)->Instance->DR = aux8;  											// Transmit data in 8 Bit mode
      // Wait until TXE flag is set to send data
			tickstart = HAL_GetTick();														// Get tick
			while(__HAL_SPI_GET_FLAG((&lcd_spi),SPI_FLAG_TXE) == RESET)
			{
				if(Timeout != HAL_MAX_DELAY)
				{
					if((Timeout == 0)||((HAL_GetTick() - tickstart ) > Timeout))
					{
						__HAL_SPI_DISABLE_IT((&lcd_spi), (SPI_IT_TXE | SPI_IT_RXNE | SPI_IT_ERR)); // Disable TXE, RXNE and ERR interrupts for the interrupt process
						__HAL_SPI_DISABLE((&lcd_spi));									// Disable SPI peripheral
						if((&lcd_spi)->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE)					// Reset CRC Calculation
						{
							SPI_RESET_CRC((&lcd_spi));
						}
						(&lcd_spi)->State= HAL_SPI_STATE_READY;
						__HAL_UNLOCK((&lcd_spi));												// Process Unlocked
						ILI9341_CS_SET;
						ILI9341_WRX_RESET;
						return HAL_TIMEOUT;
					}
				}
			}
			//------------------------Segundo pixel en la parte alta --------------------------------------------
			aux8=(uint8_t)(pix >> 24);
      (&lcd_spi)->Instance->DR = aux8;  											// Transmit data in 8 Bit mode
      // Wait until TXE flag is set to send data
			tickstart = HAL_GetTick();														// Get tick
			while(__HAL_SPI_GET_FLAG((&lcd_spi),SPI_FLAG_TXE) == RESET)
			{
				if(Timeout != HAL_MAX_DELAY)
				{
					if((Timeout == 0)||((HAL_GetTick() - tickstart ) > Timeout))
					{
						__HAL_SPI_DISABLE_IT((&lcd_spi), (SPI_IT_TXE | SPI_IT_RXNE | SPI_IT_ERR)); // Disable TXE, RXNE and ERR interrupts for the interrupt process
						__HAL_SPI_DISABLE((&lcd_spi));									// Disable SPI peripheral
						if((&lcd_spi)->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE)					// Reset CRC Calculation
						{
							SPI_RESET_CRC((&lcd_spi));
						}
						(&lcd_spi)->State= HAL_SPI_STATE_READY;
						__HAL_UNLOCK((&lcd_spi));												// Process Unlocked
						ILI9341_CS_SET;
						ILI9341_WRX_RESET;
						return HAL_TIMEOUT;
					}
				}
			}

			aux8=(uint8_t)(pix >> 16);
      (&lcd_spi)->Instance->DR = aux8;  											// Transmit data in 8 Bit mode
      // Wait until TXE flag is set to send data
			tickstart = HAL_GetTick();														// Get tick
			while(__HAL_SPI_GET_FLAG((&lcd_spi),SPI_FLAG_TXE) == RESET)
			{
				if(Timeout != HAL_MAX_DELAY)
				{
					if((Timeout == 0)||((HAL_GetTick() - tickstart ) > Timeout))
					{
						__HAL_SPI_DISABLE_IT((&lcd_spi), (SPI_IT_TXE | SPI_IT_RXNE | SPI_IT_ERR)); // Disable TXE, RXNE and ERR interrupts for the interrupt process
						__HAL_SPI_DISABLE((&lcd_spi));									// Disable SPI peripheral
						if((&lcd_spi)->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE)					// Reset CRC Calculation
						{
							SPI_RESET_CRC((&lcd_spi));
						}
						(&lcd_spi)->State= HAL_SPI_STATE_READY;
						__HAL_UNLOCK((&lcd_spi));												// Process Unlocked
						ILI9341_CS_SET;
						ILI9341_WRX_RESET;
						return HAL_TIMEOUT;
					}
				}
			}
			//---------------------------------------------------------------------------------------
    }
		ILI9341_WRX_RESET;
		ILI9341_CS_SET;

    if((&lcd_spi)->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE) // Enable CRC Transmission
    {
      (&lcd_spi)->Instance->CR1 |= SPI_CR1_CRCNEXT;
    }
    // Wait until TXE flag is set to send data
		tickstart = HAL_GetTick();														// Get tick
		while(__HAL_SPI_GET_FLAG((&lcd_spi),SPI_FLAG_TXE) == RESET)
		{
			if(Timeout != HAL_MAX_DELAY)
			{
				if((Timeout == 0)||((HAL_GetTick() - tickstart ) > Timeout))
				{
					__HAL_SPI_DISABLE_IT((&lcd_spi), (SPI_IT_TXE | SPI_IT_RXNE | SPI_IT_ERR)); // Disable TXE, RXNE and ERR interrupts for the interrupt process
					__HAL_SPI_DISABLE((&lcd_spi));									// Disable SPI peripheral
					if((&lcd_spi)->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE)					// Reset CRC Calculation
					{
						SPI_RESET_CRC((&lcd_spi));
					}
					(&lcd_spi)->State= HAL_SPI_STATE_READY;
					(&lcd_spi)->ErrorCode |= HAL_SPI_ERROR_FLAG;		//puesto aqui
					__HAL_UNLOCK((&lcd_spi));												// Process Unlocked
					ILI9341_WRX_RESET;
					ILI9341_CS_SET;
					return HAL_TIMEOUT;
				}
			}
		}
    // Wait until Busy flag is reset before disabling SPI
		tickstart = HAL_GetTick();				// Get tick
		while(__HAL_SPI_GET_FLAG((&lcd_spi), SPI_FLAG_BSY) != RESET)
    {
      if(Timeout != HAL_MAX_DELAY)
      {
        if((Timeout == 0)||((HAL_GetTick() - tickstart ) > Timeout))
        {
          __HAL_SPI_DISABLE_IT((&lcd_spi), (SPI_IT_TXE | SPI_IT_RXNE | SPI_IT_ERR)); // Disable TXE, RXNE and ERR interrupts for the interrupt process
          __HAL_SPI_DISABLE((&lcd_spi));		// Disable SPI peripheral
          if((&lcd_spi)->Init.CRCCalculation == SPI_CRCCALCULATION_ENABLE)						// Reset CRC Calculation
          {
            SPI_RESET_CRC((&lcd_spi));
          }
          (&lcd_spi)->State= HAL_SPI_STATE_READY;
					(&lcd_spi)->ErrorCode |= HAL_SPI_ERROR_FLAG;		//Se puso aqui
          __HAL_UNLOCK((&lcd_spi));												// Process Unlocked
					ILI9341_WRX_RESET;
					ILI9341_CS_SET;
          return HAL_TIMEOUT;
        }
      }
    }
    // Clear OVERRUN flag in 2 Lines communication mode because received is not read
    if((&lcd_spi)->Init.Direction == SPI_DIRECTION_2LINES)
    {
      __HAL_SPI_CLEAR_OVRFLAG((&lcd_spi));
    }
    (&lcd_spi)->State = HAL_SPI_STATE_READY;
    __HAL_UNLOCK((&lcd_spi));															// Process Unlocked
		ILI9341_WRX_RESET;
		ILI9341_CS_SET;
    return HAL_OK;
  }
  else
  {
		ILI9341_WRX_RESET;
		ILI9341_CS_SET;
    return HAL_BUSY;
  }
}


void LCD_SPI_BaudRateUp()
{
	HAL_SPI_DeInit(&lcd_spi);
	lcd_spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; // 45 MBits/s
  HAL_SPI_Init(&lcd_spi);
}




/* FUNCTIONS TO WRITE DIRECTLY IN THE IMAGE FRAME BUFFER */
//CORREGIDO
void LCD_ILI9341_DrawPixel_ImageBuffer(uint16_t x, uint16_t y, uint16_t color, uint32_t image[ILI9341_PIXEL])
{

	uint32_t pix,dir16,dir32,aux,aux2;
	dir16=y;
	aux=x;
	dir16=ILI9341_HEIGHT*dir16 + aux;	//direccion 16 bits
	dir32=dir16/2;										//direccion 32 bits
	aux2=dir32*2;											//para obtener paridad
	aux2=dir16-aux2;
	pix=image[dir32];									//Leer original
	aux=color;
	if(aux2!=0){ pix=pix&0x0000FFFF; pix=pix|(aux << 16); }//si dir16 impar (segundo pixel en la parte baja)
	else{ pix=pix&0xFFFF0000; pix=pix|aux; }//si dir16 par (primer pixel en la parte alta)
	image[dir32]=pix;


	//usado para uint16_t image
	//*(image + ILI9341_HEIGHT*y + x) = color;
	//usados para uint8_t image
	//*(image + ILI9341_HEIGHT*y*2 + x*2) = color >> 8;
	//*(image + ILI9341_HEIGHT*y*2 + x*2+1) = color & 0xFF;
}

//void LCD_ILI9341_Putc_ImageBuffer(uint16_t x, uint16_t y, char c, LCD_FontDef_t *font, uint16_t foreground, uint16_t* image)
void LCD_ILI9341_Putc_ImageBuffer(uint16_t x, uint16_t y, char c, LCD_FontDef_t *font, uint16_t foreground, uint32_t image[ILI9341_PIXEL])
{
	uint32_t i, b, j;
	/* Set coordinates */
	ILI9341_x = x;
	ILI9341_y = y;
	if ((ILI9341_x + font->FontWidth) > ILI9341_Opts.width) {
		//If at the end of a line of display, go to new line and set x to 0 position
		ILI9341_y += font->FontHeight;
		ILI9341_x = 0;
	}
	for (i = 0; i < font->FontHeight; i++) {
		b = font->data[(c - 32) * font->FontHeight + i];
		for (j = 0; j < font->FontWidth; j++) {
			if ((b << j) & 0x8000) {
				LCD_ILI9341_DrawPixel_ImageBuffer(ILI9341_x + j, (ILI9341_y + i), foreground, image);
			}
		}
	}
	ILI9341_x += font->FontWidth;
}

//void LCD_ILI9341_Puts_ImageBuffer(uint16_t x, uint16_t y, char *str, LCD_FontDef_t *font, uint16_t foreground, uint16_t* image)
void LCD_ILI9341_Puts_ImageBuffer(uint16_t x, uint16_t y, char *str, LCD_FontDef_t *font, uint16_t foreground, uint32_t image[ILI9341_PIXEL])
{
	uint16_t startX = x;

	/* Set X and Y coordinates */
	ILI9341_x = x;
	ILI9341_y = y;

	while (*str) {
		//New line
		if (*str == '\n') {
			ILI9341_y += font->FontHeight + 1;
			//if after \n is also \r, than go to the left of the screen
			if (*(str + 1) == '\r') {
				ILI9341_x = 0;
				str++;
			} else {
				ILI9341_x = startX;
			}
			str++;
			continue;
		} else if (*str == '\r') {
			str++;
			continue;
		}

		LCD_ILI9341_Putc_ImageBuffer(ILI9341_x, ILI9341_y, *str++, font, foreground, image);
	}
}


//void LCD_ILI9341_DrawLine_ImageBuffer(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color, uint16_t* image)
void LCD_ILI9341_DrawLine_ImageBuffer(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color, uint32_t image[ILI9341_PIXEL])
{
	int16_t dx, dy, sx, sy, err, e2;

	/* Check for overflow */
	if (x0 >= ILI9341_Opts.width) {
		x0 = ILI9341_Opts.width - 1;
	}
	if (x1 >= ILI9341_Opts.width) {
		x1 = ILI9341_Opts.width - 1;
	}
	if (y0 >= ILI9341_Opts.height) {
		y0 = ILI9341_Opts.height - 1;
	}
	if (y1 >= ILI9341_Opts.height) {
		y1 = ILI9341_Opts.height - 1;
	}

	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1);
	dy = (y0 < y1) ? (y1 - y0) : (y0 - y1);
	sx = (x0 < x1) ? 1 : -1;
	sy = (y0 < y1) ? 1 : -1;
	err = ((dx > dy) ? dx : -dy) / 2;

	while (1) {
		LCD_ILI9341_DrawPixel_ImageBuffer(x0, y0, color, image);
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

//void LCD_ILI9341_DrawRectangle_ImageBuffer(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color, uint16_t* image)
void LCD_ILI9341_DrawRectangle_ImageBuffer(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color, uint32_t image[ILI9341_PIXEL])
{
	LCD_ILI9341_DrawLine_ImageBuffer(x0, y0, x1, y0, color, image); //Top
	LCD_ILI9341_DrawLine_ImageBuffer(x0, y0, x0, y1, color, image);	//Left
	LCD_ILI9341_DrawLine_ImageBuffer(x1, y0, x1, y1, color, image);	//Right
	LCD_ILI9341_DrawLine_ImageBuffer(x0, y1, x1, y1, color, image);	//Bottom
}


//void LCD_ILI9341_DrawFilledRectangle_ImageBuffer(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color, uint16_t* image)
void LCD_ILI9341_DrawFilledRectangle_ImageBuffer(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color, uint32_t image[ILI9341_PIXEL])
{
	for (; x0 < x1; x0++) {
		LCD_ILI9341_DrawLine_ImageBuffer(x0, y0, x0, y1, color, image);
	}
}






//************************************** FUNCIONES DEL TOUCH *************************************************************************
// Configures the touch Panel Controller (Single point detection)
uint8_t TP_Config(void)
{
	uint16_t tmp = 0;

  tmp = TP_ReadDeviceRegister(0);															// Read IO Expander device ID
  tmp = (uint32_t)(tmp << 8);
  tmp |= (uint32_t)TP_ReadDeviceRegister(1);
  if( tmp != (uint16_t)STMPE811_ID ){ return 1; }     				// Return Error '1' if the ID is not correct

  TP_Reset(); 																								// Generate IO Expander Software reset
  TP_FnctCmd(TP_ADC_FCT, ENABLE);															// Touch Panel controller and ADC configuration
	TP_FnctCmd(TP_TP_FCT, ENABLE);															// Enable touch Panel functionality
  TP_WriteDeviceRegister(TP_REG_ADC_CTRL1, 0x49);						  // Select Sample Time, bit number and ADC Reference
  LCD_ILI9341_Delay(200);																			// Wait for ~20 ms
  TP_WriteDeviceRegister(TP_REG_ADC_CTRL2, 0x01);							// Select the ADC clock speed: 3.25 MHz
  TP_IOAFConfig((uint8_t)TOUCH_IO_ALL, DISABLE);							// Select TSC pins in non default mode, NO se necesita por STM32F4i-DISCO
  TP_WriteDeviceRegister(TP_REG_TP_CFG, 0x9A); 								// Select 2 nF filter capacitor
  TP_WriteDeviceRegister(TP_REG_FIFO_TH, 0x01);								// Select single point reading
  TP_WriteDeviceRegister(TP_REG_FIFO_STA, 0x01);							// Write 0x01 to clear the FIFO memory content.
  TP_WriteDeviceRegister(TP_REG_FIFO_STA, 0x00);							// Write 0x00 to put the FIFO back into operation mode
  TP_WriteDeviceRegister(TP_REG_TP_FRACT_XYZ, 0x01);					// set the data format for Z value: 7 fractional part and 1 whole part
  TP_WriteDeviceRegister(TP_REG_TP_I_DRIVE, 0x01);						// set the driving capability of the device for TSC pins: 50mA
  TP_WriteDeviceRegister(TP_REG_TP_CTRL, 0x03);								// Use no tracking index, touch-panel controller operation mode (XYZ) and enable the TSC
  TP_WriteDeviceRegister(TP_REG_INT_STA, 0xFF); 							//  Clear all the status pending bits
  TP_State.TouchDetected = TP_State.X = TP_State.Y = TP_State.Z = 0;// Initialize the TS structure to their default values

  return 0; 																									// Configuration is OK
}


// Returns Status and positions of the Touch Panel.
// retval Pointer to TP_STATE structure holding Touch Panel information.
TP_STATE* TP_GetState(void)
{
  uint32_t xDiff, yDiff , x , y;
  static uint32_t _x = 0, _y = 0;

  TP_State.TouchDetected = (TP_ReadDeviceRegister(TP_REG_TP_CTRL) & 0x80); // Check if the Touch detect event happened

  if(TP_State.TouchDetected)
  {
		x=1; y=1;
		x = TP_Read_X();
    y = TP_Read_Y();
    xDiff = x > _x? (x - _x): (_x - x);
    yDiff = y > _y? (y - _y): (_y - y);
    if (xDiff + yDiff > 5)
    {
      _x = x;
      _y = y;
    }
  }
  TP_State.X = _x;																						// Update the X position
  TP_State.Y = _y;																						// Update the Y position
  TP_State.Z = TP_Read_Z();  																	// Update the Z Pression index

  TP_WriteDeviceRegister(TP_REG_FIFO_STA, 0x01);							// Clear the interrupt pending bit and enable the FIFO again
  TP_WriteDeviceRegister(TP_REG_FIFO_STA, 0x00);

  return &TP_State; 																					// Return pointer to the updated structure
}



// Return Touch Panel X position value
// retval: X position.
static uint16_t TP_Read_X(void)
{
  int32_t x, xr;

  x = TP_ReadDataBuffer(TP_REG_TP_DATA_X);										// Read x value from DATA_X register

  if(x <= 3000)																								// x value first correction
  {
		x = 3870 - x;
  }
  else
  {
		x = 3800 - x;
  }
  xr = x / 15;																								// x value second correction
  if(xr <= 0)																									// return x position value
  {
    xr = 0;
  }
  else if (xr > 240)
  {
    xr = 239;
  }
  else
  {}
  return (uint16_t)(xr);
}

// Return Touch Panel Y position value
// retval: Y position.
static uint16_t TP_Read_Y(void)
{
  int32_t y, yr;

  y = TP_ReadDataBuffer(TP_REG_TP_DATA_Y);							  		// Read y value from DATA_Y register
	y -= 360;  																									// y value first correction
  yr = y / 11;																								// y value second correction
  if(yr <= 0)																									// return y position value
  {
    yr = 0;
  }
  else if (yr > 320)
  {
    yr = 319;
  }
  else
  {}
  return (uint16_t)(yr);
}

// Return Touch Panel Z position value
// retval: Z position.
static uint16_t TP_Read_Z(void)
{
  uint32_t z;

  z = TP_ReadDataBuffer(TP_REG_TP_DATA_Z);										// Read z value from DATA_Z register
	if(z <= 0)																									// return z position value
    z = 0;
  return (uint16_t)(z);
}


// Resets the IO Expander by Software (SYS_CTRL1, RESET bit).
// retval 0: if all initializations are OK. Other value if error.
uint8_t TP_Reset(void)
{
  TP_WriteDeviceRegister(TP_REG_SYS_CTRL1, 0x02);							// Power Down the IO_Expander
  LCD_ILI9341_Delay(2); 																			// wait for a delay to insure registers erasing
  TP_WriteDeviceRegister(TP_REG_SYS_CTRL1, 0x00);							// Power On the Codec after the power off => all registers are reinitialized
  return 0;    																								// If all OK return IOE_OK
}

// Configures the selected IO Expander functionalities.
// param:   Fct: the functions to be configured. could be any combination of the following values:
//          arg:  IOE_IO_FCT : IO function, IOE_TP_FCT : Touch Panel function, IOE_ADC_FCT : ADC function
// param:  NewState: can be ENABLE pr DISABLE
// retval: IOE_OK: if all initializations are OK. Other value if error.
uint8_t TP_FnctCmd(uint8_t Fct, FunctionalState NewState)
{
  uint8_t tmp = 0;

  tmp = TP_ReadDeviceRegister(TP_REG_SYS_CTRL2);							// Get the register value
  if (NewState != DISABLE)
  {
    tmp &= ~(uint8_t)Fct;																			// Set the Functionalities to be Enabled
  }
  else
  {
    tmp |= (uint8_t)Fct;  																		// Set the Functionalities to be Disabled
  }
  TP_WriteDeviceRegister(TP_REG_SYS_CTRL2, tmp);							// Set the register value
  return 0;    																								// If all OK return IOE_OK
}

// Configures the selected pin to be in Alternate function or not. NO necesitada funcion STM32F4i disco
// param:  IO_Pin: IO_Pin_x, Where x can be from 0 to 7, NewState: State of the AF for the selected pin, could be ENABLE or DISABLE.
// retval: 0: if all initializations are OK. Other value if error.
uint8_t TP_IOAFConfig(uint8_t IO_Pin, FunctionalState NewState)
{
  uint8_t tmp = 0;
  tmp = TP_ReadDeviceRegister(TP_REG_GPIO_AF);								// Get the current state of the GPIO_AF register
  if (NewState != DISABLE)
  {
    tmp |= (uint8_t)IO_Pin;																		// Enable the selected pins alternate function
  }
  else
  {
    tmp &= ~(uint8_t)IO_Pin;   																// Disable the selected pins alternate function
  }
  TP_WriteDeviceRegister(TP_REG_GPIO_AF, tmp);  							// Write back the new value in GPIO_AF register
  return 0;																										// If all OK return IOE_OK
}








// Reads a register of the device through I2C without DMA.
// param:  RegisterAddr: The target register address (between 00x and 0x24)
// retval: The value of the read register (0xAA if Timeout occurred)
uint8_t TP_ReadDeviceRegister(uint8_t RegisterAddr)
{
  uint8_t tmp = 0;
	uint8_t Address;
	uint8_t buf[2] = {RegisterAddr,RegisterAddr};

  __disable_irq();
	Address = TP_ADDR;
  Address &= (uint8_t)~((uint8_t)I2C_OAR1_ADD0);     					// Reset the address bit0 for write
	if (HAL_I2C_Master_Transmit(&hi2c3, Address, buf, (uint16_t)1, 1000) != HAL_OK) { return 0xAA; }
	Address = TP_ADDR;
  Address |= I2C_OAR1_ADD0;																		// Set the address bit0 for read
	if (HAL_I2C_Master_Receive(&hi2c3, Address, &buf[0], 1, 1000) != HAL_OK) {   return 0xAA; }
	tmp=buf[0];
	__enable_irq();

  return tmp;
}


// Writes a value in a register of the device through I2C.
// param;  RegisterAddr: The target register address,  RegisterValue: The target register value to be written
// retval: 0 if all operations are OK. Other value if error.
uint8_t TP_WriteDeviceRegister(uint8_t RegisterAddr, uint8_t RegisterValue)
{
	uint8_t Address;
  uint8_t buf[2] = {RegisterAddr, RegisterValue};

	__disable_irq();
	Address = TP_ADDR;
  Address &= (uint8_t)~((uint8_t)I2C_OAR1_ADD0);     					// Reset the address bit0 for write
  if (HAL_I2C_Master_Transmit(&hi2c3, Address, buf, (uint16_t)2, 1000) != HAL_OK){ return 1; }
	__enable_irq();
  return 0;
}

/**
  * @brief  Reads a buffer of 2 bytes from the device registers.
  * @param  RegisterAddr: The target register adress (between 00x and 0x24)
  * @retval The data in the buffer containing the two returned bytes (in halfword).
  */
uint16_t TP_ReadDataBuffer(uint32_t RegisterAddr)
{
	//uint8_t tmp = 0;
	uint8_t Address;
  uint8_t TP_BufferRX[2] = {RegisterAddr,RegisterAddr};
	uint8_t buf[2] = {RegisterAddr,RegisterAddr};

  __disable_irq();
	Address = TP_ADDR;
  Address &= (uint8_t)~((uint8_t)I2C_OAR1_ADD0);     					// Reset the address bit0 for write
	if (HAL_I2C_Master_Transmit(&hi2c3, Address, buf, (uint16_t)1, 1000) != HAL_OK) { return 0xAA; }
	Address = TP_ADDR;
  Address |= I2C_OAR1_ADD0;																		// Set the address bit0 for read
	if (HAL_I2C_Master_Receive(&hi2c3, Address, &TP_BufferRX[1], 1, 1000) != HAL_OK) {   return 0xAA; }
	if (HAL_I2C_Master_Receive(&hi2c3, Address, &TP_BufferRX[0], 1, 1000) != HAL_OK) {   return 0xAA; }
  __enable_irq();








	//TP_BufferRX[1] = TP_ReadDeviceRegister(RegisterAddr);
  //TP_BufferRX[0] = TP_ReadDeviceRegister(RegisterAddr);

/*
  // Enable the I2C peripheral
  I2C_GenerateSTART(IOE_I2C, ENABLE);

  // Test on EV5 and clear it
  IOE_TimeOut = TIMEOUT_MAX;
  while (!I2C_GetFlagStatus(IOE_I2C, I2C_FLAG_SB))
  {
    if (IOE_TimeOut-- == 0) return(IOE_TimeoutUserCallback());
  }

  // Send device address for write -------------------------------------------------------------
  I2C_Send7bitAddress(IOE_I2C, IOE_ADDR, I2C_Direction_Transmitter);

  // Test on EV6 and clear it
  IOE_TimeOut = TIMEOUT_MAX;
  while (!I2C_GetFlagStatus(IOE_I2C, I2C_FLAG_ADDR))
  {
    if (IOE_TimeOut-- == 0) return(IOE_TimeoutUserCallback());
  }

  // Read status register 2 to clear ADDR flag
  IOE_I2C->SR2;

  // Test on EV8 and clear it
  IOE_TimeOut = TIMEOUT_MAX;
  while (!I2C_GetFlagStatus(IOE_I2C, I2C_FLAG_TXE))
  {
    if (IOE_TimeOut-- == 0) return(IOE_TimeoutUserCallback());
  }

  // Send the device's internal address to write to
  I2C_SendData(IOE_I2C, RegisterAddr);  --------------------------------------------------------

  // Send START condition a second time
  I2C_GenerateSTART(IOE_I2C, ENABLE);

  // Test on EV5 and clear it
  IOE_TimeOut = TIMEOUT_MAX;
  while (!I2C_GetFlagStatus(IOE_I2C, I2C_FLAG_SB))
  {
    if (IOE_TimeOut-- == 0) return(IOE_TimeoutUserCallback());
  }

  // Send IO Expander address for read
  I2C_Send7bitAddress(IOE_I2C, IOE_ADDR, I2C_Direction_Receiver);--------------------------------

  // Test on EV6 and clear it
  IOE_TimeOut = TIMEOUT_MAX;
  while (!I2C_GetFlagStatus(IOE_I2C, I2C_FLAG_ADDR))
  {
    if (IOE_TimeOut-- == 0) return(IOE_TimeoutUserCallback());
  }

  // Disable Acknowledgement and set Pos bit
  I2C_AcknowledgeConfig(IOE_I2C, DISABLE);
  I2C_NACKPositionConfig(IOE_I2C, I2C_NACKPosition_Next);

  // Read status register 2 to clear ADDR flag
  IOE_I2C->SR2;

  // Test on EV7 and clear it
  IOE_TimeOut = TIMEOUT_MAX;
  while (!I2C_GetFlagStatus(IOE_I2C, I2C_FLAG_BTF))
  {
    if (IOE_TimeOut-- == 0) return(IOE_TimeoutUserCallback());
  }

  // Send STOP Condition
  I2C_GenerateSTOP(IOE_I2C, ENABLE);-----------------------------------------------------------

  // Read the first byte from the IO Expander
  IOE_BufferRX[1] = I2C_ReceiveData(IOE_I2C);

  // Read the second byte from the IO Expander
  IOE_BufferRX[0] = I2C_ReceiveData(IOE_I2C);

  // Enable Acknowledgement and reset POS bit to be ready for another reception
  I2C_AcknowledgeConfig(IOE_I2C, ENABLE);
  I2C_NACKPositionConfig(IOE_I2C, I2C_NACKPosition_Current);
*/

  // return the data
  return ((uint16_t) TP_BufferRX[0] | ((uint16_t)TP_BufferRX[1]<< 8));
}
