/*---------------------------------------------------------------------------------

default ARM7 core

Copyright (C) 2005 - 2010
	Michael Noland (joat)
	Jason Rogers (dovoto)
	Dave Murphy (WinterMute)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
	must not claim that you wrote the original software. If you use
	this software in a product, an acknowledgment in the product
	documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and
	must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source
	distribution.

---------------------------------------------------------------------------------*/
#include <nds.h>

#define SD_IRQ_STATUS (*(vu32*)0x400481C)

#include "fifocheck.h"

void VcountHandler() { inputGetAndSend(); }

int main(void) {

	irqInit();
	
	readUserSettings();

	initClockIRQ();

	fifoInit();

	SetYtrigger(80);

	installSystemFIFO();
	
	fifoSendValue32(FIFO_USER_01, SD_IRQ_STATUS);

	irqSet(IRQ_VCOUNT, VcountHandler);

	irqEnable( IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK );

	while (1) {
		/* if (*(u32*)0x02FFFD0C == 0x54534453) { // 'SDST'
			fifoSendValue32(FIFO_USER_01, SD_IRQ_STATUS);
			*(u32*)0x02FFFD0C = 0;
		} */
		fifocheck();
		swiWaitForVBlank();
	}
}

