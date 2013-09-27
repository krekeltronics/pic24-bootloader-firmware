/*
 * Copyright (c) 2011 Redslate Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of the copyright holders nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @author Derek Baker <derek@red-slate.co.uk>
 */

#include "BootLoader.h"

// Pic24fj64GB004 register configs
#ifndef DEV_HAS_CONFIG_BITS
    #ifdef DEV_HAS_USB
    _CONFIG1(WDTPS_PS1024 & FWPSA_PR32 & WINDIS_OFF & FWDTEN_OFF & ICS_PGx1 & GWRP_OFF & GCP_OFF & JTAGEN_OFF)
	_CONFIG2(POSCMOD_NONE & OSCIOFNC_ON & FNOSC_FRCPLL & PLL96MHZ_ON & PLLDIV_DIV2)
    #else
    _CONFIG1(JTAGEN_OFF & GWRP_OFF & ICS_PGx2 & FWDTEN_OFF)
    _CONFIG2(POSCMOD_HS & FNOSC_PRIPLL)
    #endif
#else
    _FOSCSEL(FNOSC_PRIPLL)
    _FOSC(POSCFREQ_MS & POSCMOD_XT)
    _FWDT(FWDTEN_OFF)
    _FICD(ICS_PGx3)
#endif

//
// Main
//

int main()
{
    CLKDIVbits.CPDIV=0;                                                             //Set the system clock to 32mhz.
	CLKDIVbits.RCDIV=0;                                                             //Set internal RC to product 8mhz clock.
    while(OSCCONbits.LOCK==0);                                                      //Wait for the PLL to lock up.

    BootLoader();                                                                   //Call the Boot Loader
    return 0;
}
