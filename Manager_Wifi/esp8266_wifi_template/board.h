#ifndef	_BOARD_H
#define _BOARD_H

#define LED_STATUS_GPIO         2 /* 3(web485), 23(ken_sos) */
#define led_status_init()       pinMode(LED_STATUS_GPIO, OUTPUT)
#define led_status_toggle()     {digitalWrite(LED_STATUS_GPIO, !digitalRead(LED_STATUS_GPIO));}
#define led_status_on()         digitalWrite(LED_STATUS_GPIO, LOW)
#define led_status_off()        digitalWrite(LED_STATUS_GPIO, HIGH)

/*
SPI BUS
*/
#ifdef ESP32
#define SPI_SCK_PIN     		5
#define SPI_MISO_PIN     		18
#define SPI_MOSI_PIN     		17
#define SPI_NSS_PIN				16 
#elif defined(ESP8266)
#define SPI_SCK_PIN     		14
#define SPI_MISO_PIN     		12
#define SPI_MOSI_PIN     		13
#define SPI_NSS_PIN				15
#endif

// GPIO control power of SD card
#define SD_POWER_PIN     		5

/* GPIO to reset factory */
#define FACTORY_INPUT_PIN       0 /* Button1 36(web485), -1(ken_sos) */

#endif