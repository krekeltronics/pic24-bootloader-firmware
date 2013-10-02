/********************************************************************
*
* PIC24F Serial Bootloader
*
*********************************************************************
* FileName:		boot.c
* Dependencies: memory.c, config.h, GenericTypeDefs.h
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
* TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FrOR A
* PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
* IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
* CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
*
*
* File Description:
*
* Bootloader for PIC24F devices compatable with AN851 communication protocol
* Based on PIC24F UART bootloader and PIC16/18 AN851 bootloader
*
*
* Change History:
*
* Author      	Revision #      Date        Comment
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Brant Ivey	1.00   			1-17-2008	Initial release of AN1157
* Brant Ivey    1.02            11-17-2008  Updated 'K' device support
*                                           Added extra configuration options
* Derek Baker(RedSlate)         17-10-2011  Re-Formatted and Modulised
********************************************************************/

#include <GenericTypeDefs.h>
#include "BootLoader.h"
#include "Memory.h"

//Globals ********************************
WORD responseBytes;                                                                 //Number of bytes in command response
DWORD_VAL sourceAddr;                                                               //General purpose address variable
DWORD_VAL userReset;                                                                //User code reset vector
DWORD_VAL userTimeout;                                                              //Bootloader entry timeout value
WORD userResetRead;                                                                 //Bool - for relocating user reset vector

#ifdef USE_RUNAWAY_PROTECT                                                          //Variables for storing runaway code protection keys
volatile WORD writeKey1 = 0xFFFF;
volatile WORD writeKey2 = 0x5555;
volatile WORD keyTest1 = 0x0000;
volatile WORD keyTest2 = 0xAAAA;
#endif

BYTE buffer[MAX_PACKET_SIZE+1];                                                     //Transmit/Recieve Buffer

/********************************************************************
* Function: 	void BootLoader()
*
* Precondition: UART Setup
*
* Input: 		None.
*
* Output:		None.
*
* Side Effects:	None.
*
* Overview: 	Starts the Boot Loader
*
* Note:		 	None.
********************************************************************/
void BootLoader(void)
{
	DWORD_VAL delay;

	sourceAddr.Val = DELAY_TIME_ADDR;                                               //Setup bootloader entry delay, Bootloader timer address
	delay.Val = ReadLatch(sourceAddr.word.HW, sourceAddr.word.LW);                  //Read BL timeout

	sourceAddr.Val = USER_PROG_RESET;                                               //Setup user reset vector
	userReset.Val = ReadLatch(sourceAddr.word.HW, sourceAddr.word.LW);

	if(userReset.Val == 0xFFFFFF) {                                                 //Prevent bootloader lockout - if no user reset vector, reset to BL start
		userReset.Val = BOOT_ADDR_LOW;
	}
	userResetRead = 0;
    delay.Val = 2;                                                                  //Set to 2 Seconds as default

	if(delay.v[0] == 0) {                                                           //If timeout is zero, check reset state.
                                                                                    //If device is returning from reset, BL is disabled call user code
                                                                                    //Otherwise assume the BL was called from use code and enter BL
		if(RCON & 0xFED3) {                                                         //If bootloader disabled, go to user code
			ResetDevice(userReset.Val);
		} else {
			delay.Val = 0xFF;
		}
	}

	T2CONbits.TON = 0;
	T2CONbits.T32 = 1;                                                              //Setup Timer 2/3 as 32 bit timer incrementing every clock
	IFS0bits.T3IF = 0;                                                              //Clear the Timer3 Interrupt Flag
	IEC0bits.T3IE = 0;                                                              //Disable Timer3 Interrupt Service Routine

	if((delay.Val & 0x000000FF) != 0xFF) {                                          //Enable timer if not in always-BL mode
		delay.Val = ((DWORD)(FCY)) * ((DWORD)(delay.v[0]));                         //Convert seconds into timer count value
		PR3 = delay.word.HW;                                                        //Setup timer timeout value
		PR2 = delay.word.LW;
		TMR2 = 0;
		TMR3 = 0;
		T2CONbits.TON=1;                                                            //Enable timer
	}

	#ifdef DEV_HAS_PPS                                                              //If using a part with PPS, map the UART I/O
		ioMap();
	#endif

	#ifdef UTX_ANA                                                                  //Configure UART pins to be digital I/O.
		UTX_ANA = 1;
	#endif
	#ifdef URX_ANA
		URX_ANA = 1;
	#endif

    UxMODEbits.UARTEN = 1;                                                          //SETUP UART COMMS: No parity, one stop bit, autobaud, polled, Enable uart
    #ifdef USE_AUTOBAUD
	    UxMODEbits.ABAUD = 1;                                                       //Use autobaud
    #else
        UxBRG = BAUDRATEREG;
    #endif
	#ifdef USE_HI_SPEED_BRG
		UxMODEbits.BRGH = 1;                                                        //Use high speed mode
	#endif
	UxSTA = 0x0400;                                                                 //Enable TX

        /// test
        //PutResponse(1);
        /// end test

        
    while(1) {
		#ifdef USE_RUNAWAY_PROTECT
			writeKey1 = 0xFFFF;                                                     //Modify keys to ensure proper program flow
			writeKey2 = 0x5555;
		#endif
		GetCommand();                                                               //Get full AN851 command from UART
		#ifdef USE_RUNAWAY_PROTECT
			writeKey1 += 10;                                                        //Modify keys to ensure proper program flow
			writeKey2 += 42;
		#endif
		HandleCommand();                                                            //Handle the command
		PutResponse(responseBytes);                                                 //Respond to sent command
	}
}

/********************************************************************
* Function: 	void GetCommand()
*
* Precondition: UART Setup
*
* Input: 		None.
*
* Output:		None.
*
* Side Effects:	None.
*
* Overview: 	Polls the UART to recieve a complete AN851 command.
*			 	Fills buffer[1024] with recieved data.
*
* Note:		 	None.
********************************************************************/
void GetCommand()
{
	BYTE RXByte;
	BYTE checksum;
	WORD dataCount;

	while(1){

		#ifndef USE_AUTOBAUD
        GetChar(&RXByte);                                                           //Get first STX
        if(RXByte == STX){
        #else
        AutoBaud();                                                                 //Get first STX and calculate baud rate
		RXByte = UxRXREG;                                                           //Dummy read
        #endif

		T2CONbits.TON = 0;                                                          //Disable timer - data received

		GetChar(&RXByte);                                                           //Read second byte
		if(RXByte == STX){                                                          //2 STX, beginning of data

			checksum = 0;                                                           //Reset checksum
			dataCount = 0;                                                          //Reset datacount

			while(dataCount <= MAX_PACKET_SIZE+1){                                  //Maximum num bytes to receive
				GetChar(&RXByte);
				switch(RXByte){
					case STX:                                                       //Start over if STX
						checksum = 0;
						dataCount = 0;
						break;

					case ETX:                                                       //End of packet if ETX
						checksum = ~checksum +1;                                    //Test checksum
						Nop();
						if(checksum == 0) return;                                   //Return if OK
						dataCount = 0xFFFF;                                         //Otherwise restart
						break;

					case DLE:                                                       //If DLE, treat next as data
						GetChar(&RXByte);
					default:                                                        //Get data, put in buffer
						checksum += RXByte;
						buffer[dataCount++] = RXByte;
						break;

				}                                                                   //End switch(RXByte)
			}                                                                       //End while(byteCount <= 1024)
		}                                                                           //End if(RXByte == STX)

        #ifndef USE_AUTOBAUD
        }                                                                           //End if(RXByte == STX)
        #endif
	}                                                                               //End while(1)
}                                                                                   //End GetCommand()

/********************************************************************
* Function: 	void HandleCommand()
*
* Precondition: data in buffer
*
* Input: 		None.
*
* Output:		None.
*
* Side Effects:	None.
*
* Overview: 	Handles commands received from host
*
* Note:		 	None.
********************************************************************/
void HandleCommand()
{
	BYTE Command;
	BYTE length;

	#if (defined(DEV_HAS_EEPROM) || defined(DEV_HAS_CONFIG_BITS))                   //Variables used in EE and CONFIG read/writes
		WORD i=0;
		WORD_VAL temp;
		WORD bytesRead = 0;
	#endif

	Command = buffer[0];                                                            //Get command from buffer
	length = buffer[1];                                                             //Get data length from buffer

	if(length == 0x00) {                                                            //RESET Command
        UxMODEbits.UARTEN = 0;                                                      //Disable UART
		ResetDevice(userReset.Val);
	}

	sourceAddr.v[0] = buffer[2];                                                    //Get 24-bit address from buffer
	sourceAddr.v[1] = buffer[3];
	sourceAddr.v[2] = buffer[4];
	sourceAddr.v[3] = 0;

	#ifdef USE_RUNAWAY_PROTECT
	writeKey1 |= (WORD)sourceAddr.Val;                                              // Modify keys to ensure proper program flow
	writeKey2 =  writeKey2 << 1;
	#endif

	//Handle Commands
	switch(Command)
	{
		case RD_VER:                                                                //Read version
			buffer[2] = MINOR_VERSION;
			buffer[3] = MAJOR_VERSION;
			responseBytes = 4;                                                      //Set length of reply
			break;
		case RD_FLASH:                                                              //Read flash memory
			ReadPM(length, sourceAddr);
				responseBytes = length*PM_INSTR_SIZE + 5;                           //Set length of reply
			break;
		case WT_FLASH:                                                              //Write flash memory
			#ifdef USE_RUNAWAY_PROTECT
				writeKey1 -= length;                                                //Modify keys to ensure proper program flow
				writeKey2 += Command;
			#endif

			WritePM(length, sourceAddr);
			responseBytes = 1;                                                      //Set length of reply
 			break;
		case ER_FLASH:                                                              //Erase flash memory
			#ifdef USE_RUNAWAY_PROTECT
				writeKey1 += length;                                                //Modify keys to ensure proper program flow
				writeKey2 -= Command;
			#endif

			ErasePM(length, sourceAddr);
			responseBytes = 1;                                                      //Set length of reply
			break;

		#ifdef DEV_HAS_EEPROM
		case RD_EEDATA:                                                             //Read EEPROM, If device has onboard EEPROM, allow EE reads
			while(i < length*2) {                                                   //Read length words of EEPROM
				temp.Val = ReadLatch(sourceAddr.word.HW,sourceAddr.word.LW);
				buffer[5+i++] = temp.v[0];
				buffer[5+i++] = temp.v[1];
				sourceAddr.Val += 2;
			}
			responseBytes = length*2 + 5;                                           //Set length of reply
			break;
		case WT_EEDATA:                                                             //Write EEPROM

			#ifdef USE_RUNAWAY_PROTECT
				writeKey1 -= length;                                                //Modify keys to ensure proper program flow
				writeKey2 += Command;
			#endif

			while(i < length*2) {                                                   //Write length words of EEPROM
				temp.byte.LB = buffer[5+i++];                                       //Load data to write
				temp.byte.HB = buffer[5+i++];
				WriteLatch(sourceAddr.word.HW,sourceAddr.word.LW,0, temp.Val);      //Write data to latch

				#ifdef USE_RUNAWAY_PROTECT
					writeKey1++;
					writeKey2--;
					keyTest1 =	(((0x0009 | (WORD)(sourceAddr.Val-i)) - length) + i/2) - 5;//setup program flow protection test keys
					keyTest2 =  (((0x557F << 1) + WT_EEDATA) - i/2) + 6;
					WriteMem(EE_WORD_WRITE);                                        //Initiate write sequence
					writeKey1 += 5;                                                 //Modify keys to ensure proper program flow
					writeKey2 -= 6;
				#else
					WriteMem(EE_WORD_WRITE);                                        //Initiate write sequence bypasssing runaway protection
				#endif
				sourceAddr.Val +=2;
			}
			responseBytes = 1;                                                      //Set length of reply
			break;
		#endif

		#ifdef DEV_HAS_CONFIG_BITS
		case RD_CONFIG:                                                             //Read config memory
			while(bytesRead < length) {                                             //Read length bytes from config memory
				temp.Val = ReadLatch(sourceAddr.word.HW, sourceAddr.word.LW);       //Read flash
				buffer[bytesRead+5] = temp.v[0];                                    //Put read data onto buffer
				bytesRead++;
				sourceAddr.Val += 2;                                                //Increment addr by 2
			}
			responseBytes = length + 5;
			break;
		case WT_CONFIG:                                                             //Write Config mem
			while(i < length) {                                                     //Write length  bytes of config memory
				temp.byte.LB = buffer[5+i++];                                       //Load data to write
				temp.byte.HB = 0;

                #ifdef USE_RUNAWAY_PROTECT
   					writeKey1++;
   					writeKey2--;
                #endif

                if(sourceAddr.Val >= CONFIG_START && sourceAddr.Val <= CONFIG_END) {//Make sure that config write is inside implemented configuration space
                    TBLPAG = sourceAddr.byte.UB;
                    __builtin_tblwtl(sourceAddr.word.LW,temp.Val);

                    #ifdef USE_RUNAWAY_PROTECT
       					keyTest1 =	(((0x0009 | (WORD)(sourceAddr.Val-i*2)) - length) + i) - 5;//Setup program flow protection test keys
    					keyTest2 =  (((0x557F << 1) + WT_CONFIG) - i) + 6;
    					WriteMem(CONFIG_WORD_WRITE);                                //Initiate write sequence
    					writeKey1 += 5;                                             //Modify keys to ensure proper program flow
    					writeKey2 -= 6;
    				#else
    					WriteMem(CONFIG_WORD_WRITE);                                //Initiate write sequence bypasssing runaway protection
    				#endif
                }                                                                   //End if(sourceAddr.Val...)
				sourceAddr.Val +=2;
			}                                                                       //End while(i < length)
			responseBytes = 1;                                                      //Set length of reply
			break;
		#endif
		case VERIFY_OK:
			#ifdef USE_RUNAWAY_PROTECT
				writeKey1 -= 1;                                                     //Modify keys to ensure proper program flow
				writeKey2 += Command;
			#endif

			WriteTimeout();
			responseBytes = 1;                                                      //Set length of reply
			break;
		default:
			break;
	}                                                                               //End switch(Command)
}

/********************************************************************
* Function: 	void PutResponse()
*
* Precondition: UART Setup, data in buffer
*
* Input: 		None.
*
* Output:		None.
*
* Side Effects:	None.
*
* Overview: 	Transmits responseBytes bytes of data from buffer
				with UART as a response to received command.
*
* Note:		 	None.
********************************************************************/
void PutResponse(WORD responseLen)
{
	WORD i;
	BYTE data;
	BYTE checksum;

	UxSTAbits.UTXEN = 1;                                                            //Make sure TX is enabled

	PutChar(STX);                                                                   //Put 2 STX characters
	PutChar(STX);

	checksum = 0;
	for(i = 0; i < responseLen; i++){
		asm("clrwdt");                                                              //Looping code so clear WDT
		data = buffer[i];                                                           //Get data from response buffer
		checksum += data;                                                           //Accumulate checksum
		if(data == STX || data == ETX || data == DLE){                         		//If control character, stuff DLE
			PutChar(DLE);
		}
		PutChar(data);                                                              //Send data
	}

	checksum = ~checksum + 1;                                                       //Keep track of checksum
	if(checksum == STX || checksum == ETX || checksum == DLE){                      //If control character, stuff DLE
		PutChar(DLE);
	}

	PutChar(checksum);                                                              //Put checksum
	PutChar(ETX);                                                                   //Put End of text
	while(!UxSTAbits.TRMT);                                                         //Wait for transmit to finish
}

/********************************************************************
* Function: 	void PutChar(BYTE Char)
*
* Precondition: UART Setup
*
* Input: 		Char - Character to transmit
*
* Output: 		None
*
* Side Effects:	Puts character into destination pointed to by ptrChar.
*
* Overview: 	Transmits a character on UART2.
*	 			Waits for an empty spot in TXREG FIFO.
*
* Note:		 	None
********************************************************************/
void PutChar(BYTE txChar)
{
	while(UxSTAbits.UTXBF);                                                         //Wait for FIFO space
	UxTXREG = txChar;                                                               //Put character onto UART FIFO to transmit
}

/********************************************************************
* Function:        void GetChar(BYTE * ptrChar)
*
* PreCondition:    UART Setup
*
* Input:		ptrChar - pointer to character received
*
* Output:
*
* Side Effects:	Puts character into destination pointed to by ptrChar.
*				Clear WDT
*
* Overview:		Receives a character from UART2.
*
* Note:			None
********************************************************************/
void GetChar(BYTE * ptrChar)
{
	BYTE dummy;
	while(1)
	{
		asm("clrwdt");                                                              //Looping code, so clear WDT
		if((UxSTA & 0x000E) != 0x0000) {                                            //Check for receive errors
			dummy = UxRXREG;                                                        //Dummy read to clear FERR/PERR
			UxSTAbits.OERR = 0;                                                     //Clear OERR to keep receiving
		}
		if(UxSTAbits.URXDA == 1) {                                                  //Get the data
			* ptrChar = UxRXREG;                                                    //Get data from UART RX FIFO
			break;
		}

        #ifndef USE_AUTOBAUD
		if(IFS0bits.T3IF == 1) {                                                    //If timer expired, jump to user code
			ResetDevice(userReset.Val);
		}
        #endif
	}                                                                               //End while(1)
}

/********************************************************************
* Function:     void ReadPM(WORD length, DWORD_VAL sourceAddr)
*
* PreCondition: None
*
* Input:		length		- number of instructions to read
*				sourceAddr 	- address to read from
*
* Output:		None
*
* Side Effects:	Puts read instructions into buffer.
*
* Overview:		Reads from program memory, stores data into buffer.
*
* Note:			None
********************************************************************/
void ReadPM(WORD length, DWORD_VAL sourceAddr)
{
	WORD bytesRead = 0;
	DWORD_VAL temp;

	while(bytesRead < length*PM_INSTR_SIZE) {                                       //Read length instructions from flash
		temp.Val = ReadLatch(sourceAddr.word.HW, sourceAddr.word.LW);               //Read flash
		buffer[bytesRead+5] = temp.v[0];                                            //Put read data onto
		buffer[bytesRead+6] = temp.v[1];                                            //Response buffer
		buffer[bytesRead+7] = temp.v[2];
		buffer[bytesRead+8] = temp.v[3];
        bytesRead+=PM_INSTR_SIZE;                                                   //4 bytes per instruction: low word, high byte, phantom byte
		sourceAddr.Val = sourceAddr.Val + 2;                                        //Increment addr by 2
	}                                                                               //End while(bytesRead < length*PM_INSTR_SIZE)
}

/********************************************************************
* Function:     void WritePM(WORD length, DWORD_VAL sourceAddr)
*
* PreCondition: Page containing rows to write should be erased.
*
* Input:		length		- number of rows to write
*				sourceAddr 	- row aligned address to write to
*
* Output:		None.
*
* Side Effects:	None.
*
* Overview:		Writes number of rows indicated from buffer into
*				flash memory
*
* Note:			None
********************************************************************/
void WritePM(WORD length, DWORD_VAL sourceAddr)
{
	WORD bytesWritten;
	DWORD_VAL data;
	#ifdef USE_RUNAWAY_PROTECT
	WORD temp = (WORD)sourceAddr.Val;
	#endif

	bytesWritten = 0;                                                               //First 5 buffer locations are cmd,len,addr

	while((bytesWritten) < length*PM_ROW_SIZE) {                                    //Write length rows to flash
		asm("clrwdt");
		data.v[0] = buffer[bytesWritten+5];                                         //Get data to write from buffer
		data.v[1] = buffer[bytesWritten+6];
		data.v[2] = buffer[bytesWritten+7];
		data.v[3] = buffer[bytesWritten+8];
		bytesWritten+=PM_INSTR_SIZE;                                                //4 bytes per instruction: low word, high byte, phantom byte

		#ifndef DEV_HAS_CONFIG_BITS                                                 //Flash configuration word handling
			if(sourceAddr.Val == CONFIG_END) {                                      //Mask of bit 15 of CW1 to ensure it is programmed as 0 as noted in PIC24FJ datasheets
				data.Val &= 0x007FFF;
			}
		#endif

		#ifdef USE_BOOT_PROTECT                                                     //Protect the bootloader & reset vector
			if(sourceAddr.Val == 0x0) {                                             //Protect BL reset & get user reset
				userReset.Val = data.Val & 0xFFFF;                                  //Get user app reset vector lo word
				data.Val = 0x040000 + (0xFFFF & BOOT_ADDR_LOW);                     //program low word of BL reset
				userResetRead = 1;
			}
			if(sourceAddr.Val == 0x2) {
				userReset.Val += (DWORD)(data.Val & 0x00FF)<<16;                    //Get user app reset vector hi byte
				data.Val = ((DWORD)(BOOT_ADDR_LOW & 0xFF0000))>>16;                 //Program high byte of BL reset
				userResetRead = 1;
			}
		#else
			if(sourceAddr.Val == 0x0) {                                             //Get user app reset vector lo word
				userReset.Val = data.Val & 0xFFFF;
				userResetRead = 1;
			}
			if(sourceAddr.Val == 0x2) {                                             //Get user app reset vector	hi byte
				userReset.Val |= ((DWORD)(data.Val & 0x00FF))<<16;
				userResetRead = 1;
			}
		#endif

		if(sourceAddr.Val == USER_PROG_RESET) {                                     //Put information from reset vector in user reset vector location
			if(userResetRead){                                                      //Has reset vector been grabbed from location 0x0?
				data.Val = userReset.Val;                                           //If yes, use that reset vector
			}else{
				userReset.Val = data.Val;                                           //If no, use the user's indicated reset vector
			}
		}
		if(sourceAddr.Val == DELAY_TIME_ADDR) {                                     //If address is delay timer location, store data and write empty word
			userTimeout.Val = data.Val;
			data.Val = 0xFFFFFF;
		}

		#ifdef USE_BOOT_PROTECT                                                     //Do not erase bootloader & reset vector
			if(sourceAddr.Val < BOOT_ADDR_LOW || sourceAddr.Val > BOOT_ADDR_HI) {
		#endif

		#ifdef USE_CONFIGWORD_PROTECT                                               //Do not erase last page
			if(sourceAddr.Val < (CONFIG_START & 0xFFFC00)) {
		#endif

		#ifdef USE_VECTOR_PROTECT                                                   //Do not erase first page
			//if(sourceAddr.Val >= PM_PAGE_SIZE/2) {
			if(sourceAddr.Val >= VECTOR_SECTION) {
		#endif

   		WriteLatch(sourceAddr.word.HW, sourceAddr.word.LW,data.word.HW, data.word.LW);//write data into latches

		#ifdef USE_VECTOR_PROTECT
			}                                                                       //End vectors protect
		#endif

		#ifdef USE_CONFIGWORD_PROTECT
			}                                                                       //End config protect
		#endif

		#ifdef USE_BOOT_PROTECT
			}                                                                       //End bootloader protect
		#endif

		#ifdef USE_RUNAWAY_PROTECT
			writeKey1 += 4;                                                         //Modify keys to ensure proper program flow
			writeKey2 -= 4;
		#endif

		if((bytesWritten % PM_ROW_SIZE) == 0) {                                     //Write to flash memory if complete row is finished
			#ifdef USE_RUNAWAY_PROTECT
				keyTest1 =  (0x0009 | temp) - length + bytesWritten - 5;            //Setup program flow protection test keys
				keyTest2 =  (((0x557F << 1) + WT_FLASH) - bytesWritten) + 6;
			#endif

			#ifdef USE_BOOT_PROTECT                                                 //Protect the bootloader & reset vector
				if((sourceAddr.Val < BOOT_ADDR_LOW || sourceAddr.Val > BOOT_ADDR_HI)) {
			#endif

			#ifdef USE_CONFIGWORD_PROTECT                                           //Do not erase last page
				if(sourceAddr.Val < (CONFIG_START & 0xFFFC00)) {
			#endif

			#ifdef USE_VECTOR_PROTECT                                               //Do not erase first page
				if(sourceAddr.Val >= VECTOR_SECTION) {
			#endif

			WriteMem(PM_ROW_WRITE);                                                 //Execute write sequence

			#ifdef USE_RUNAWAY_PROTECT
				writeKey1 += 5;                                                     //Modify keys to ensure proper program flow
				writeKey2 -= 6;
			#endif

			#ifdef USE_VECTOR_PROTECT
				}                                                                   //End vectors protect
			#endif

			#ifdef USE_CONFIGWORD_PROTECT
				}                                                                   //End config protect
			#endif

			#ifdef USE_BOOT_PROTECT
				}                                                                   //End boot protect
			#endif
		}

		sourceAddr.Val = sourceAddr.Val + 2;                                        //Increment addr by 2
	}                                                                               //End while((bytesWritten-5) < length*PM_ROW_SIZE)
}

/********************************************************************
* Function:     void ErasePM(WORD length, DWORD_VAL sourceAddr)
*
* PreCondition:
*
* Input:		length		- number of pages to erase
*				sourceAddr 	- page aligned address to erase
*
* Output:		None.
*
* Side Effects:	None.
*
* Overview:		Erases number of pages from flash memory
*
* Note:			None
********************************************************************/
void ErasePM(WORD length, DWORD_VAL sourceAddr)
{
	WORD i=0;
	#ifdef USE_RUNAWAY_PROTECT
	WORD temp = (WORD)sourceAddr.Val;
	#endif

	while(i<length) {
		i++;
		#ifdef USE_RUNAWAY_PROTECT
			writeKey1++;                                                            //Modify keys to ensure proper program flow
			writeKey2--;
		#endif

		#ifdef USE_BOOT_PROTECT                                                     //If protection enabled, protect BL and reset vector
			if(sourceAddr.Val < BOOT_ADDR_LOW || sourceAddr.Val > BOOT_ADDR_HI) {   //Do not erase bootloader
		#endif

		#ifdef USE_CONFIGWORD_PROTECT                                               //Do not erase last page
			if(sourceAddr.Val < (CONFIG_START & 0xFFFC00)) {
		#endif

		#ifdef USE_VECTOR_PROTECT                                                   //Do not erase first page
			if(sourceAddr.Val >= VECTOR_SECTION) {
		#endif

		#ifdef USE_RUNAWAY_PROTECT                                                  //Setup program flow protection test keys
			keyTest1 = (0x0009 | temp) + length + i + 7;
			keyTest2 = (0x557F << 1) - ER_FLASH - i + 3;
		#endif

		Erase(sourceAddr.word.HW, sourceAddr.word.LW, PM_PAGE_ERASE);              	//Perform erase

		#ifdef USE_RUNAWAY_PROTECT
			writeKey1 -= 7;                                                         //Modify keys to ensure proper program flow
			writeKey2 -= 3;
		#endif

		#ifdef USE_VECTOR_PROTECT
			}                                                                       //End vectors protect
		#elif  defined(USE_BOOT_PROTECT) || defined(USE_RESET_SAVE)
			DWORD_VAL blResetAddr;                                                  //Replace the bootloader reset vector
			if(sourceAddr.Val < PM_PAGE_SIZE/2) {
				blResetAddr.Val = 0;                                                //Replace BL reset vector at 0x00 and 0x02 if erased

				#ifdef USE_RUNAWAY_PROTECT
					keyTest1 = (0x0009 | temp) + length + i;                        //Setup program flow protection test keys
					keyTest2 = (0x557F << 1) - ER_FLASH - i;
				#endif

				replaceBLReset(blResetAddr);
			}
		#endif

		#ifdef USE_CONFIGWORD_PROTECT
			}                                                                       //End config protect
		#endif

		#ifdef USE_BOOT_PROTECT
			}                                                                       //End bootloader protect
		#endif

		sourceAddr.Val += PM_PAGE_SIZE/2;                                           //Increment by a page

	}                                                                               //End while(i<length)
}

/********************************************************************
* Function:     void WriteTimeout()
*
* PreCondition: The programmed data should be verified prior to calling
* 				this funtion.
*
* Input:		None.
*
* Output:		None.
*
* Side Effects:	None.
*
* Overview:		This function writes the stored value of the bootloader
*				timeout delay to memory.  This function should only
*				be called after sucessful verification of the programmed
*				data to prevent possible bootloader lockout
*
* Note:			None
********************************************************************/
void WriteTimeout()
{
	#ifdef USE_RUNAWAY_PROTECT
	WORD temp = (WORD)sourceAddr.Val;
	#endif
	
	#ifdef DEV_HAS_WORD_WRITE                                                       //Write timeout value to memory
		WriteLatch((DELAY_TIME_ADDR & 0xFF0000)>>16,(DELAY_TIME_ADDR & 0x00FFFF),userTimeout.word.HW, userTimeout.word.LW);//Write data into latches
	#else
		DWORD_VAL address;
		WORD bytesWritten;

		bytesWritten = 0;
		address.Val = DELAY_TIME_ADDR & (0x1000000 - PM_ROW_SIZE/2);

		//Program booloader entry delay to finalize bootloading
		//Load 0xFFFFFF into all other words in row to prevent corruption
		while(bytesWritten < PM_ROW_SIZE){
			if(address.Val == DELAY_TIME_ADDR) {
				WriteLatch(address.word.HW, address.word.LW,userTimeout.word.HW,userTimeout.word.LW);
			}else{
				WriteLatch(address.word.HW, address.word.LW,0xFFFF,0xFFFF);
			}

			address.Val += 2;
			bytesWritten +=4;
		}
	#endif

	#ifdef USE_RUNAWAY_PROTECT
		keyTest1 =  (0x0009 | temp) - 1 - 5;                                        //Setup program flow protection test keys
		keyTest2 =  ((0x557F << 1) + VERIFY_OK) + 6;
	#endif

	#ifdef DEV_HAS_WORD_WRITE                                                       //Perform write to enable BL timeout
		WriteMem(PM_WORD_WRITE);                                                    //Execute write sequence
	#else
		WriteMem(PM_ROW_WRITE);                                                     //Execute write sequence
	#endif

	#ifdef USE_RUNAWAY_PROTECT
		writeKey1 += 5;                                                             //Modify keys to ensure proper program flow
		writeKey2 -= 6;
	#endif

}

/*********************************************************************
* Function:     void AutoBaud()
*
* PreCondition: UART Setup
*
* Input:		None.
*
* Output:		None.
*
* Side Effects:	Resets WDT.
*
* Overview:		Sets autobaud mode and waits for completion.
*
* Note:			Contains code to handle UART errata issues for
				PIC24FJ128 family parts, A2 and A3 revs.
********************************************************************/
void AutoBaud()
{
	BYTE dummy;
	UxMODEbits.ABAUD = 1;                                                           //Set autobaud mode

	while(UxMODEbits.ABAUD)	{                                                       //Wait for sync character 0x55
		asm("clrwdt");                                                              //looping code so clear WDT
		if(IFS0bits.T3IF == 1) {                                                    //if timer expired, jump to user code
			ResetDevice(userReset.Val);
		}
		if(UxSTAbits.OERR) UxSTAbits.OERR = 0;
		if(UxSTAbits.URXDA) dummy = UxRXREG;
	}

	#ifdef USE_WORKAROUNDS                                                          //Workarounds for autobaud errata in some silicon revisions
		if(UxBRG == 0xD) UxBRG--;                                                   //Workaround for autobaud innaccuracy
		if(UxBRG == 0x1A) UxBRG--;
		if(UxBRG == 0x09) UxBRG--;

		#ifdef USE_HI_SPEED_BRG                                                     //Workarounds for ABAUD incompatability w/ BRGH = 1
			UxBRG = (UxBRG+1)*4 -1;
			if(UxBRG == 0x13) UxBRG=0x11;
			if(UxBRG == 0x1B) UxBRG=0x19;
			if(UxBRG == 0x08) UxBRG=0x22;

			if (UxBRG & 0x0001)	UxBRG++;                                            //Workaround for Odd BRG recieve error when BRGH = 1
		#endif
	#endif

}

#ifdef DEV_HAS_PPS
/*********************************************************************
* Function:     void ioMap()
*
* PreCondition: None.
*
* Input:		None.
*
* Output:		None.
*
* Side Effects:	Locks IOLOCK bit.
*
* Overview:		Maps UART IO for communications on PPS devices.
*
* Note:			None.
********************************************************************/
void ioMap()
{
	__builtin_write_OSCCONL(OSCCON & 0xFFBF);                                       //Clear the IOLOCK bit
	PPS_URX_REG = PPS_URX_PIN;                                                      //UxRX = RP19
	PPS_UTX_PIN = UxTX_IO;                                                          //RP25 = UxTX
	__builtin_write_OSCCONL(OSCCON | 0x0040);                                       //Lock the IOLOCK bit so that the IO is not accedentally changed.
}
#endif

#if defined(USE_BOOT_PROTECT) || defined(USE_RESET_SAVE)
/*********************************************************************
* Function:     void replaceBLReset(DWORD_VAL sourceAddr)
*
* PreCondition: None.
*
* Input:		sourceAddr - the address to begin writing reset vector
*
* Output:		None.
*
* Side Effects:	None.
*
* Overview:		Writes bootloader reset vector to input memory location
*
* Note:			None.
********************************************************************/
void replaceBLReset(DWORD_VAL sourceAddr)
{
	DWORD_VAL data;
	#ifndef DEV_HAS_WORD_WRITE
		WORD i;
	#endif
	#ifdef USE_RUNAWAY_PROTECT
		WORD tempkey1;
		WORD tempkey2;

		tempkey1 = keyTest1;
		tempkey2 = keyTest2;
	#endif

	data.Val = 0x040000 + (0xFFFF & BOOT_ADDR_LOW);                                 //Get BL reset vector low word and write
	WriteLatch(sourceAddr.word.HW, sourceAddr.word.LW, data.word.HW, data.word.LW);

	#ifdef DEV_HAS_WORD_WRITE                                                       //Write low word back to memory on word write capable devices
		#ifdef USE_RUNAWAY_PROTECT
			writeKey1 += 5;                                                         //Modify keys to ensure proper program flow
			writeKey2 -= 6;
		#endif
		WriteMem(PM_WORD_WRITE);                                                    //Perform BL reset vector word write bypassing flow protect
	#endif

	data.Val = ((DWORD)(BOOT_ADDR_LOW & 0xFF0000))>>16;                             //Get BL reset vector high byte and write
	WriteLatch(sourceAddr.word.HW,sourceAddr.word.LW+2,data.word.HW,data.word.LW);

	#ifdef USE_RUNAWAY_PROTECT
		keyTest1 = tempkey1;
		keyTest2 = tempkey2;
	#endif

	#ifdef DEV_HAS_WORD_WRITE                                                       //Write high byte back to memory on word write capable devices
		#ifdef USE_RUNAWAY_PROTECT
			writeKey1 += 5;                                                         //Modify keys to ensure proper program flow
			writeKey2 -= 6;
		#endif

		WriteMem(PM_WORD_WRITE);                                                    //Perform BL reset vector word write

	#else                                                                           //Otherwise initialize row of memory to F's and write row containing reset
		for(i = 4; i < (PM_ROW_SIZE/PM_INSTR_SIZE*2); i+=2) {
			WriteLatch(sourceAddr.word.HW,sourceAddr.word.LW+i,0xFFFF,0xFFFF);
		}

		#ifdef USE_RUNAWAY_PROTECT
			writeKey1 += 5;                                                         //Modify keys to ensure proper program flow
			writeKey2 -= 6;
		#endif

		WriteMem(PM_ROW_WRITE);                                                     //Perform BL reset vector word write

	#endif

}
#endif
