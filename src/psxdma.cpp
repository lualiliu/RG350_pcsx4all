/***************************************************************************
 *   Copyright (C) 2007 Ryan Schultz, PCSX-df Team, PCSX team              *
 *   schultz.ryan@gmail.com, http://rschultz.ath.cx/code.php               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02111-1307 USA.            *
 ***************************************************************************/

/*
* Handles PSX DMA functions.
*/

#include "psxdma.h"

// Dma0/1 in Mdec.c
// Dma3   in CdRom.c

void spuInterrupt(void) {
#ifdef DEBUG_ANALYSIS
	dbg_anacnt_spuInterrupt++;
#endif
    HW_DMA4_CHCR &= SWAP32(~0x01000000);
    DMA_INTERRUPT(4);
}

void psxDma4(u32 madr, u32 bcr, u32 chcr) { // SPU
#ifdef DEBUG_ANALYSIS
	dbg_anacnt_psxDma4++;
#endif
	u16 *ptr;
	u32 size;

	switch (chcr) {
		case 0x01000201: //cpu to spu transfer
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA4 SPU - mem2spu *** %x addr = %x size = %x\n", chcr, madr, bcr);
#endif
			ptr = (u16 *)PSXM(madr);
			SPU_writeDMAMem(ptr, (bcr >> 16) * (bcr & 0xffff) * 2);
			SPUDMA_INT((bcr >> 16) * (bcr & 0xffff) / 2);
			return;

		case 0x01000200: //spu to cpu transfer
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA4 SPU - spu2mem *** %x addr = %x size = %x\n", chcr, madr, bcr);
#endif
			ptr = (u16 *)PSXM(madr);
			size = (bcr >> 16) * (bcr & 0xffff) * 2;
    		SPU_readDMAMem(ptr, size);
			#ifdef PSXREC
			psxCpu->Clear(madr, size);
			#endif
			break;

#ifdef PSXDMA_LOG
		default:
			PSXDMA_LOG("*** DMA4 SPU - unknown *** %x addr = %x size = %x\n", chcr, madr, bcr);
			break;
#endif
	}

	HW_DMA4_CHCR &= SWAP32(~0x01000000);
	DMA_INTERRUPT(4);
}

void psxDma2(u32 madr, u32 bcr, u32 chcr) { // GPU
#ifdef DEBUG_ANALYSIS
	dbg_anacnt_psxDma2++;
#endif
	u32 *ptr;
	u32 size;

	switch(chcr) {
		case 0x01000200: // vram2mem
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA2 GPU - vram2mem *** %x addr = %x size = %x\n", chcr, madr, bcr);
#endif
			ptr = (u32 *)PSXM(madr);
			size = (bcr >> 16) * (bcr & 0xffff);
			GPU_readDataMem(ptr, size);
			#ifdef PSXREC
			psxCpu->Clear(madr, size);
			#endif
			break;

		case 0x01000201: // mem2vram
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA 2 - GPU mem2vram *** %x addr = %x size = %x\n", chcr, madr, bcr);
#endif
			ptr = (u32 *)PSXM(madr);
			size = (bcr >> 16) * (bcr & 0xffff);
			GPU_writeDataMem(ptr, size);
			GPUDMA_INT(size / 4);
			return;

		case 0x01000401: // dma chain
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA 2 - GPU dma chain *** %x addr = %x size = %x\n", chcr, madr, bcr);
#endif
			GPU_dmaChain((u32 *)psxM, madr & 0x1fffff);
			break;

#ifdef PSXDMA_LOG
		default:
			PSXDMA_LOG("*** DMA 2 - GPU unknown *** %x addr = %x size = %x\n", chcr, madr, bcr);
			break;
#endif
	}

	HW_DMA2_CHCR &= SWAP32(~0x01000000);
	DMA_INTERRUPT(2);
}

void gpuInterrupt() {
#ifdef DEBUG_ANALYSIS
	dbg_anacnt_gpuInterrupt++;
#endif
	HW_DMA2_CHCR &= SWAP32(~0x01000000);
	DMA_INTERRUPT(2);
}

void psxDma6(u32 madr, u32 bcr, u32 chcr) {
#ifdef DEBUG_ANALYSIS
	dbg_anacnt_psxDma6++;
#endif
	u32 *mem = (u32 *)PSXM(madr);

#ifdef PSXDMA_LOG
	PSXDMA_LOG("*** DMA6 OT *** %x addr = %x size = %x\n", chcr, madr, bcr);
#endif

	if (chcr == 0x11000002) {
		while (bcr--) {
			*mem-- = SWAP32((madr - 4) & 0xffffff);
			madr -= 4;
		}
		mem++; *mem = 0xffffff;
	}
#ifdef PSXDMA_LOG
	else {
		// Unknown option
		PSXDMA_LOG("*** DMA6 OT - unknown *** %x addr = %x size = %x\n", chcr, madr, bcr);
	}
#endif

	HW_DMA6_CHCR &= SWAP32(~0x01000000);
	DMA_INTERRUPT(6);
}

