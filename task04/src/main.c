#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include "ili9341.h"
#include "fonts.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define GPIO_PIN_RESET	LCD_PIN_RESET
#define GPIO_PIN_DC		LCD_PIN_DC
#define GPIO_PIN_CS		LCD_PIN_CS
#define GPIO_DIR		"/sys/class/gpio"

#define BUF_SIZE		64

static const char *device = "/dev/spidev0.0";
static uint8_t mode = 0;
static uint8_t bits = 8;
static uint32_t speed = 5000000;
static uint16_t delay = 0;

#define DATA_SIZE	1000
uint16_t frame_buffer[LCD_WIDTH * LCD_HEIGHT];
int fd;

static int gpio_init(unsigned int gpio) {
	int fd, len;
	char buf[BUF_SIZE];

	fd = open (GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror ("gpio/export");
		return fd;
	}

	len = snprintf (buf, sizeof(buf), "%d", gpio);
	write (fd, buf, len);

	snprintf (buf, sizeof(buf), GPIO_DIR "/gpio%d/direction", gpio);

	fd = open (buf, O_WRONLY);
	if (fd < 0) {
		perror ("gpio/direction");
		return fd;
 	}
	
	write (fd, "out", sizeof("out"));
	close (fd);

	return 0;
}

static int gpio_free(unsigned int gpio) {
	int fd, len;
 	char buf[BUF_SIZE];

	fd = open (GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror ("gpio/unexport");
		return fd;
	}

	len = snprintf (buf, sizeof(buf), "%d", gpio);
	write (fd, buf, len);
 	close (fd);
	
	return 0;
}

static int gpio_set_value(unsigned int gpio, unsigned int value) {
	int fd;
	char buf[BUF_SIZE];

	snprintf (buf, sizeof(buf), GPIO_DIR "/gpio%d/value", gpio);

	fd = open (buf, O_WRONLY);
	if (fd < 0) {
		perror ("gpio/set-value");
		return fd;
	}

	if (value) {
		write (fd, "1", 2);
	}
	else {
		write (fd, "0", 2);
	}

	close (fd);
 	return 0;
}

static void lcd_reset(void) {

	gpio_set_value(GPIO_PIN_RESET, 0);
	usleep(5000);
	gpio_set_value(GPIO_PIN_RESET, 1);
	gpio_set_value(GPIO_PIN_CS, 0);
}

static void spi_write(uint8_t *tx_buffer, uint16_t len) {
	int ret;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx_buffer,
		.rx_buf = (unsigned long)NULL,
		.len = len,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		printf("can't send spi message\n");
}

static void lcd_write_command(uint8_t cmd)
{
	gpio_set_value(GPIO_PIN_DC, 0);
	spi_write(&cmd, sizeof(cmd));
}

static void lcd_write_data(uint8_t *buff, unsigned long buff_size)
{
	unsigned long i = 0;
	
	gpio_set_value(LCD_PIN_DC, 1);
	while (buff_size > DATA_SIZE) {
		spi_write(buff + i, DATA_SIZE);
		i += DATA_SIZE;
		buff_size -= DATA_SIZE;
	}
	spi_write(buff + i, buff_size);
}

void lcd_init_ili9341(void)
{
	// SOFTWARE RESET
	lcd_write_command(0x01);
	sleep(1);

	// POWER CONTROL A
	lcd_write_command(0xCB);
	{
		uint8_t data[] = { 0x39, 0x2C, 0x00, 0x34, 0x02 };
		lcd_write_data(data, sizeof(data));
	}

	// POWER CONTROL B
	lcd_write_command(0xCF);
	{
		uint8_t data[] = { 0x00, 0xC1, 0x30 };
		lcd_write_data(data, sizeof(data));
	}

	// DRIVER TIMING CONTROL A
	lcd_write_command(0xE8);
	{
		uint8_t data[] = { 0x85, 0x00, 0x78 };
		lcd_write_data(data, sizeof(data));
	}

	// DRIVER TIMING CONTROL B
	lcd_write_command(0xEA);
	{
		uint8_t data[] = { 0x00, 0x00 };
		lcd_write_data(data, sizeof(data));
	}

	// POWER ON SEQUENCE CONTROL
	lcd_write_command(0xED);
	{
		uint8_t data[] = { 0x64, 0x03, 0x12, 0x81 };
		lcd_write_data(data, sizeof(data));
	}

	// PUMP RATIO CONTROL
	lcd_write_command(0xF7);
	{
		uint8_t data[] = { 0x20 };
		lcd_write_data(data, sizeof(data));
	}

	// POWER CONTROL,VRH[5:0]
	lcd_write_command(0xC0);
	{
		uint8_t data[] = { 0x23 };
		lcd_write_data(data, sizeof(data));
	}

	// POWER CONTROL,SAP[2:0];BT[3:0]
	lcd_write_command(0xC1);
	{
		uint8_t data[] = { 0x10 };
		lcd_write_data(data, sizeof(data));
	}

	// VCM CONTROL
	lcd_write_command(0xC5);
	{
		uint8_t data[] = { 0x3E, 0x28 };
		lcd_write_data(data, sizeof(data));
	}

	// VCM CONTROL 2
	lcd_write_command(0xC7);
	{
		uint8_t data[] = { 0x86 };
		lcd_write_data(data, sizeof(data));
	}

	// PIXEL FORMAT
	lcd_write_command(0x3A);
	{
		uint8_t data[] = { 0x55 };
		lcd_write_data(data, sizeof(data));
	}
	
	// FRAME RATIO CONTROL, STANDARD RGB COLOR
	lcd_write_command(0xB1);
	{
		uint8_t data[] = { 0x00, 0x18 };
		lcd_write_data(data, sizeof(data));
	}
	
	// DISPLAY FUNCTION CONTROL
	lcd_write_command(0xB6);
	{
		uint8_t data[] = { 0x08, 0x82, 0x27 };
		lcd_write_data(data, sizeof(data));
	}

	// 3GAMMA FUNCTION DISABLE
	lcd_write_command(0xF2);
	{
		uint8_t data[] = { 0x00 };
		lcd_write_data(data, sizeof(data));
	}

	// GAMMA CURVE SELECTED
	lcd_write_command(0x26);
	{
		uint8_t data[] = { 0x01 };
		lcd_write_data(data, sizeof(data));
	}
	
	// POSITIVE GAMMA CORRECTION
	lcd_write_command(0xE0);
	{
		uint8_t data[] = { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                           0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 };
		lcd_write_data(data, sizeof(data));
	}
	
	// NEGATIVE GAMMA CORRECTION
	lcd_write_command(0xE1);
	{
		uint8_t data[] = { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                           0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F };
		lcd_write_data(data, sizeof(data));
	}

	// EXIT SLEEP
	lcd_write_command(0x11);
	usleep(12000);
    
	// TURN ON DISPLAY
	lcd_write_command(0x29);

	// MEMORY ACCESS CONTROL
	lcd_write_command(0x36);
	{
		uint8_t data[] = { 0x28 };
		lcd_write_data(data, sizeof(data));
	}

	// INVERSION
	//lcd_write_command(0x21);
}

static void lcd_set_address_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{

	lcd_write_command(LCD_CASET);
	{
		uint8_t data[] = { (x0 >> 8) & 0xFF, x0 & 0xFF,
				 (x1 >> 8) & 0xFF, x1 & 0xFF };
		lcd_write_data(data, sizeof(data));
	}

	lcd_write_command(LCD_RASET);
	{
		uint8_t data[] = { (y0 >> 8) & 0xFF, y0 & 0xFF,
				(y1 >> 8) & 0xFF, y1 & 0xFF };
		lcd_write_data(data, sizeof(data));
	}

	lcd_write_command(LCD_RAMWR);
}

static void lcd_update_screen(void)
{
	lcd_write_data((uint8_t*)frame_buffer, sizeof(uint16_t) * LCD_WIDTH * LCD_HEIGHT);
}

void lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
	if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT)) {
		return;
	}
        
	frame_buffer[x + LCD_WIDTH * y] = (color >> 8) | (color << 8);
	lcd_update_screen();
}

void lcd_fill_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	uint16_t i;
	uint16_t j;

	if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT)) {
		return;
	}

	if ((x + w - 1) > LCD_WIDTH) {
		w = LCD_WIDTH - x;
	}

	if ((y + h - 1) > LCD_HEIGHT) {
		h = LCD_HEIGHT - y;
	}

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			frame_buffer[(x + LCD_WIDTH * y) + (i + LCD_WIDTH * j)] = (color >> 8) | (color << 8);
		}
	}
	lcd_update_screen();
}

void lcd_fill_screen(uint16_t color)
{
	lcd_fill_rectangle(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, color);
}

static void lcd_put_char(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor) 
{
	uint32_t i, b, j;

	for (i = 0; i < font.height; i++) {
		b = font.data[(ch - 32) * font.height + i];
		for (j = 0; j < font.width; j++) {
			if ((b << j) & 0x8000)  {
				frame_buffer[(x + LCD_WIDTH * y) + (j + LCD_WIDTH * i)] = 
					(color >> 8) | (color << 8);
			} 
			else {
				frame_buffer[(x + LCD_WIDTH * y) + (j + LCD_WIDTH * i)] = 
					(bgcolor >> 8) | (bgcolor << 8);
			}
		}
	}
}

static void lcd_put_str(uint16_t x, uint16_t y, const char *ch, FontDef font, uint16_t color, uint16_t bg_color)
{
    uint16_t tmp = x;
    for (int i = 0; i < strlen(ch); ++i) {
        lcd_put_char(x, y, ch[i], font, color, bg_color);
        x+=font.width;
        if( x >= 305){
            x = tmp;
            y += font.height;
        }

    }
}



int main(int argc, char *argv[]) {
	int ret = 0;
	

	gpio_init(GPIO_PIN_RESET);
	gpio_init(GPIO_PIN_DC);
	gpio_init(GPIO_PIN_CS);

	lcd_reset();
	
	fd = open(device, O_RDWR);
	if (fd < 0)
		printf("can't open device\n");

	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		printf("can't set spi mode\n");
	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		printf("can't get spi mode\n");

	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		printf("can't set bits per word\n");
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		printf("can't get bits per word\n");

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't set max speed hz\n");
	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't get max speed hz\n");
	
	lcd_init_ili9341();
	printf("LCD init ok\n");
	lcd_set_address_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

	lcd_fill_screen(COLOR_BLUE);


    lcd_put_str(20, 30, " Hi raspberry! ", Font_11x18, COLOR_RED, COLOR_BLACK);



    lcd_update_screen();

	sleep(2);

	close(fd);

	gpio_free(GPIO_PIN_RESET);
	gpio_free(GPIO_PIN_DC);
	gpio_free(GPIO_PIN_CS);

	return EXIT_SUCCESS;
}



