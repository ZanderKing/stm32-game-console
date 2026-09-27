/*
 * media.h
 *
 *  Created on: Oct 26, 2025
 *      Author: alvin
 */

#ifndef SRC_MEDIA_H_
#define SRC_MEDIA_H_

#include <stdint.h>
#include "stdio.h"
#include "../../Drivers/MY_ADDITION/SSD1306/ssd1306_all.h"
void ssd1306_Game1(void);
void ssd1306_Game2(int mode);
void ssd1306_GameOver(void);
//define for digits matrix
#define DS_PORT GPIOD
#define DS_PIN GPIO_PIN_6
#define STCP_PORT GPIOD
#define STCP_PIN GPIO_PIN_5
#define SHCP_PORT GPIOD
#define SHCP_PIN GPIO_PIN_8
#endif /* SRC_MEDIA_H_ */
