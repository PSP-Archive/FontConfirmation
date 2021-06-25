/*
 * fonttest.c
 * This file is used to display the PSP's internal font (pgf and bwfon firmware files)
 * intraFont Version 0.31 by BenHur - http://www.psp-programming.com/benhur
 *
 * Uses parts of pgeFont by InsertWittyName - http://insomniac.0x89.org
 *
 * This work is licensed under the Creative Commons Attribution-Share Alike 3.0 License.
 * See LICENSE for more details.
 *
 */

#include <pspkernel.h>
#include <stdio.h>
#include <string.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <time.h>     //only needed for the blinking effect
#include <pspdebug.h> //only needed for indicating loading progress
#include <pspctrl.h>

#include "intraFont.h"

PSP_MODULE_INFO("intraFontTest", PSP_MODULE_USER, 1, 1);

static int running = 1;
static unsigned int __attribute__((aligned(16))) list[262144];

int exit_callback(int arg1, int arg2, void *common) {
	running = 0;
	return 0;
}

int CallbackThread(SceSize args, void *argp) {
	int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	sceKernelSleepThreadCB();
	return 0;
}

int SetupCallbacks(void) {
	int thid = sceKernelCreateThread("CallbackThread", CallbackThread, 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
	if (thid >= 0) sceKernelStartThread(thid, 0, 0);
	return thid;
}

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

int main() {
	SetupCallbacks();

	// Colors
	enum colors {
		RED =	0xFF0000FF,
		GREEN =	0xFF00FF00,
		BLUE =	0xFFFF0000,
		WHITE =	0xFFFFFFFF,
		LITEGRAY = 0xFFBFBFBF,
		GRAY =  0xFF7F7F7F,
		DARKGRAY = 0xFF3F3F3F,		
		BLACK = 0xFF000000,
	};
    
    // Init intraFont library
    intraFontInit();
    
     // Load fonts
	intraFont* ltn0 = intraFontLoad("flash0:/font/ltn0.pgf",0);    
	intraFontSetStyle(ltn0, 1.0f, WHITE, BLACK, 0);

    intraFont* jpn0 = intraFontLoad("flash0:/font/jpn0.pgf",INTRAFONT_STRING_SJIS); //japanese font with SJIS text string encoding
	intraFontSetStyle(jpn0, 0.8f, WHITE, BLACK, 0);                              //scale to 80%
        
	intraFont* kr0 = intraFontLoad("flash0:/font/kr0.pgf", INTRAFONT_STRING_UTF8);  //Korean font (not available on all systems) with UTF-8 encoding
	intraFontSetStyle(kr0, 0.8f, WHITE, BLACK, 0);                               //scale to 80%
           
	intraFont* arib = intraFontLoad("flash0:/font/arib.pgf",0);                     //Symbols (not available on all systems)
	intraFontSetStyle(arib, 0.8f, WHITE, BLACK, 0);                              //scale to 80%

	intraFont* chn = intraFontLoad("flash0:/font/gb3s1518.bwfon", 0);               //chinese font
	intraFontSetStyle(chn, 0.8f, WHITE, BLACK, 0);                               //scale to 80%

	// Set alternative fonts that are used in case a certain char does not exist in the main font
	intraFontSetAltFont(ltn0, jpn0);                     //japanese font is used for chars that don't exist in latin
	intraFontSetAltFont(jpn0, chn);                        //chinese font (bwfon) is used for chars that don't exist in japanese (and latin)
	intraFontSetAltFont(chn, kr0);                         //korean font is used for chars that don't exist in chinese (and jap and ltn)
	intraFontSetAltFont(kr0, arib);                        //symbol font is used for chars that don't exist in korean (and chn, jap & ltn)
	// NB: This is an extreme case - usually you need only one alternative font (e.g. japanese & chinese)
	// Also: if one of the fonts failed to load (e.g. chn) the chain breaks and all subequent fonts are not used (e.g. kr0 and arib)
	
	// Init GU    
    sceGuInit();
	sceGuStart(GU_DIRECT, list);

	sceGuDrawBuffer(GU_PSM_8888, (void*)0, BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, (void*)0x88000, BUF_WIDTH);
	sceGuDepthBuffer((void*)0x110000, BUF_WIDTH);
 
	sceGuOffset(2048 - (SCR_WIDTH/2), 2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);
	sceGuDepthRange(65535, 0);
	sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_CLIP_PLANES);
	sceGuEnable(GU_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuFinish();
	sceGuSync(0,0);
 
	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);

	int base_code = 0;
	SceCtrlData pad;
	
	while(running) 	{
		sceGuStart(GU_DIRECT, list);

		sceGumMatrixMode(GU_PROJECTION);
		sceGumLoadIdentity();
		sceGumPerspective( 75.0f, 16.0f/9.0f, 0.5f, 1000.0f);
 
        sceGumMatrixMode(GU_VIEW);
		sceGumLoadIdentity();

		sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
		
		sceGuClearColor(BLACK);
		sceGuClearDepth(0);
		sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

        // Draw various text
        int y = 18, i, j;
		        
		for(j=0; j<0x10; j++){
			char text[8];
			sprintf(text, "%X", j);
			intraFontPrint(ltn0, 64 + j*22, y, text);
		}
		y += 20;
		for(i=0; i<12; i++, y+= 20){
			char text[8];
			unsigned short code = base_code + i*0x10;
			sprintf(text, "%04X", code);
			intraFontPrint(ltn0, 0, y, text);
			for(j=0; j<0x10; j++){
				unsigned short ucs2_all[] = { code + j, 0};
				intraFontPrintUCS2(ltn0, 64 + j*22, y, ucs2_all);      //print chars from all fonts (using alternative fonts, which were set after font loading)
				
			}
		}
        
        // End drawing
		sceGuFinish();
		sceGuSync(0,0);
		
		// Swap buffers (waiting for vsync)
		sceDisplayWaitVblankStart();
		sceGuSwapBuffers();
		
		
		sceCtrlReadBufferPositive(&pad, 1);
		if(pad.Buttons & PSP_CTRL_DOWN){
			base_code += 0x10;
			if(base_code + 0xB0 > 0xFFFF)
				base_code = 0xFFF0 - 0xB0;
			sceKernelDelayThread(200*1000);
		} else if(pad.Buttons & PSP_CTRL_UP){
			base_code -= 0x10;
			if(base_code < 0)
				base_code = 0;
			sceKernelDelayThread(200*1000);
		} else if(pad.Buttons & PSP_CTRL_RIGHT){
			base_code += 0x100;
			if(base_code + 0xB0 > 0xFFFF)
				base_code = 0xFFF0 - 0xB0;
			sceKernelDelayThread(200*1000);
		} else if(pad.Buttons & PSP_CTRL_LEFT){
			base_code -= 0x100;
			if(base_code < 0)
				base_code = 0;
			sceKernelDelayThread(200*1000);
		} else if(pad.Buttons & PSP_CTRL_RTRIGGER){
			base_code += 0x1000;
			if(base_code + 0xB0 > 0xFFFF)
				base_code = 0xFFF0 - 0xB0;
			sceKernelDelayThread(200*1000);
		} else if(pad.Buttons & PSP_CTRL_LTRIGGER){
			base_code -= 0x1000;
			if(base_code < 0)
				base_code = 0;
			sceKernelDelayThread(200*1000);
		} else if(pad.Buttons & (PSP_CTRL_SELECT | PSP_CTRL_START)){
			break;
		}
		sceKernelDelayThread(100*1000);
	}
	
    intraFontUnload(ltn0);
    intraFontUnload(jpn0);
	intraFontUnload(kr0);
    intraFontUnload(arib);
	intraFontUnload(chn);

	// Shutdown font library
	intraFontShutdown();

	sceKernelExitGame();

	return 0;
}
