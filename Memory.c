/********************************************************************
*
* PIC24F Serial Bootloader
*
*********************************************************************
* FileName:		memory.c
* Dependencies: boot.c  
* Processor:	PIC24F Family
* Compiler:		C30 v3.00 or later
* Company:		Microchip Technology, Inc.
*
* Software License Agreement:
*
* The software supplied herewith by Microchip Technology Incorporated
* (the “Company”) for its PICmicro® Microcontroller is intended and
* supplied to you, the Company’s customer, for use solely and
* exclusively on Microchip PICmicro Microcontroller products. The
* software is owned by the Company and/or its supplier, and is
* protected under applicable copyright laws. All rights are reserved.
* Any use in violation of the foregoing restrictions may subject the
* user to criminal sanctions under applicable laws, as well as to
* civil liability for the breach of the terms and conditions of this
* license.
*
* THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
* TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
* IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
* CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
*
*
* File Description:
*
* Flash program memory read and write functions for use with 
* PIC24F Serial Bootloader.
*
* Change History:
*
* Author      	Revision #      Date        Comment
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Brant Ivey	1.00   			1-17-2008	Initial release with AN1157
*********************************************************************/

#include <p24fxxxx.h>
#include <GenericTypeDefs.h>
#include "Memory.h"

//Variables for storing runaway code protection keys
#ifdef USE_RUNAWAY_PROTECT
extern volatile WORD writeKey1;
extern volatile WORD writeKey2;
extern volatile WORD keyTest1;
extern volatile WORD keyTest2;
#endif

/********************************************************************
; Function: 	void WriteMem(WORD cmd)
;
; PreCondition: Appropriate data written to latches with WriteLatch
;
; Input:    	cmd - type of memory operation to perform
;                               
; Output:   	None.
;
; Side Effects: 
;
; Overview: 	Write stored registers to flash memory
;*********************************************************************/
void WriteMem(WORD cmd)
{
	NVMCON = cmd;

	#ifdef USE_RUNAWAY_PROTECT
		writeKey1-=5;
		writeKey2+=6;

		if(writeKey1 == keyTest1 && writeKey2 == keyTest2){
	#endif

	__builtin_write_NVM();


	while(NVMCONbits.WR == 1);

	#ifdef USE_RUNAWAY_PROTECT

		}//end if(writeKey1...

		keyTest1 = 0x0000;
		keyTest2 = 0xAAAA;
	#endif
}

/********************************************************************
; Function: 	void WriteLatch(WORD page, WORD addrLo, 
;				        		WORD dataHi, WORD dataLo)
;
; PreCondition: None.
;
; Input:    	page 	- upper byte of address
;				addrLo 	- lower word of address
;				dataHi 	- upper byte of data
;				addrLo	- lower word of data
;                               
; Output:   	None.
;
; Side Effects: TBLPAG changed
;
; Overview: 	Stores data to write in hardware latches
;*********************************************************************/	
void WriteLatch(WORD page, WORD addrLo, WORD dataHi, WORD dataLo)
{
	TBLPAG = page;

	__builtin_tblwtl(addrLo,dataLo);
	__builtin_tblwth(addrLo,dataHi);
	
}	

/********************************************************************
; Function: 	DWORD ReadLatch(WORD page, WORD addrLo)
;
; PreCondition: None.
;
; Input:    	page 	- upper byte of address
;				addrLo 	- lower word of address
;                               
; Output:   	data 	- 32-bit data in W1:W0
;
; Side Effects: TBLPAG changed
;
; Overview: 	Read from location in flash memory
;*********************************************************************/
DWORD ReadLatch(WORD page, WORD addrLo)
{
	DWORD_VAL temp;

	TBLPAG = page;

	temp.word.LW = __builtin_tblrdl(addrLo);
	temp.word.HW = __builtin_tblrdh(addrLo);

	return temp.Val;
}

/*********************************************************************
; Function: 	void ResetDevice(WORD addr);
;
; PreCondition: None.
;
; Input:    	addr 	- 16-bit address to vector to
;                               
; Output:   	None.
;
; Side Effects: None.
;
; Overview: 	used to vector to user code
;**********************************************************************/
void ResetDevice(WORD addr)
{
	asm("goto %0" : : "r"(addr));
}

/********************************************************************
; Function: 	void Erase(WORD page, WORD addrLo, WORD cmd);
;
; PreCondition: None.
;
; Input:    	page 	- upper byte of address
;				addrLo 	- lower word of address
;				cmd		- type of memory operation to perform
;                               
; Output:   	None.
;
; Side Effects: TBLPAG changed
;
; Overview: 	Erases page of flash memory at input address
*********************************************************************/	
void Erase(WORD page, WORD addrLo, WORD cmd)
{
	WORD temp;	

	temp = TBLPAG;
	TBLPAG = page;

	NVMCON = cmd;

	__builtin_tblwtl(addrLo,addrLo);

	#ifdef USE_RUNAWAY_PROTECT
		writeKey1+=7;
		writeKey2+=3;

		if(writeKey1 == keyTest1 && writeKey2 == keyTest2){
	#endif

	__builtin_write_NVM();


	while(NVMCONbits.WR == 1);

	#ifdef USE_RUNAWAY_PROTECT

		}//end if(writekey1...

		keyTest1 = 0x0000;
		keyTest2 = 0xAAAA;
	#endif

	TBLPAG = temp;
}

