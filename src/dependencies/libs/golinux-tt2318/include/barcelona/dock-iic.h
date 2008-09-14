/* include/barcelona/dock-iic.h
 *
 * Implementation of the GPIO IIC driver.
 *
 * Copyright (C) 2006 TomTom BV <http://www.tomtom.com/>
 * Author: Jeroen Taverne <jeroen.taverne@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

void IOIIC_Start(void);
void IOIIC_Stop(void);
int IOIIC_Transmit(unsigned char data);
unsigned char IOIIC_Receive(int generateAck);
