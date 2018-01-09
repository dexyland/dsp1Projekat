//////////////////////////////////////////////////////////////////////////////
// *
// * Predmetni projekat iz predmeta OAiS DSP 1
// * Godina: 2017
// *
// * Zadatak: Generator periodicnog suma
// * Autor:
// *                                                                          
// *                                                                          
//////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "ezdsp5535.h"
#include "ezdsp5535_i2c.h"
#include "aic3204.h"
#include "ezdsp5535_aic3204_dma.h"
#include "ezdsp5535_i2s.h"
#include "ezdsp5535_sar.h"
#include "print_number.h"
#include "gen_sinus.h"
#include "math.h"

/* Frekvencija odabiranja */
#define SAMPLE_RATE 8000L
#define PI 3.14159265359

/* Niz za smestanje odbiraka ulaznog signala */
#pragma DATA_ALIGN(bufferL,4)
Int16 bufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(bufferR,4)
Int16 bufferR[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(noiseBuffer,4);
Int16 noiseBuffer[AUDIO_IO_SIZE];

static void printFrequency(int freq);


void main( void )
{   
	int i;
	int currentFrequency = 1;
	Uint16 keyPressed;
	float ph0720 = 3.26725340;
	float ph1070 = 0.75397730;
	float ph1580 = 1.75929117;
	float ph2680 = 5.52921391;
	float phase = 0;

    /* Inicijalizaija razvojne ploce */
    EZDSP5535_init( );

    /* Inicijalizacija kontrolera za ocitavanje vrednosti pritisnutog dugmeta*/
    EZDSP5535_SAR_init();

    /* Inicijalizacija LCD kontrolera */
    initPrintNumber();

    printf("\n Dodavanje suma u signal \n");
		
    /* Inicijalizacija veze sa AIC3204 kodekom (AD/DA) */
    aic3204_hardware_init();
	
    /* Inicijalizacija AIC3204 kodeka */
	aic3204_init();

    aic3204_dma_init();
    
    /* Postavljanje vrednosti frekvencije odabiranja i pojacanja na kodeku */
    set_sampling_frequency_and_gain(SAMPLE_RATE, 0);

	while(1)
	{
		keyPressed = EZDSP5535_SAR_getKey;

		if (keyPressed == SW1)
		{
			if (++currentFrequency == 5)
			{
				currentFrequency = 1;
			}

			phase = 0.0;
		}
		else if (keyPressed == SW2)
		{
			printf("Exit key pressed!\n");
			break;
		}

		phase = fmod(phase, 2*PI);

		switch (currentFrequency)
		{
			case 1:
				gen_sinus_table(AUDIO_IO_SIZE, 1, 720.0/8000, phase, noiseBuffer);
			    phase += ph0720;
			    printFrequency(720);
			    break;
			case 2:
				gen_sinus_table(AUDIO_IO_SIZE, 1, 1070.0/8000, phase, noiseBuffer);
			    phase += ph1070;
			    printFrequency(1070);
			    break;
			case 3:
				gen_sinus_table(AUDIO_IO_SIZE, 1, 1580.0/8000, phase, noiseBuffer);
			    phase += ph1580;
			    printFrequency(1580);
			    break;
			case 4:
				gen_sinus_table(AUDIO_IO_SIZE, 1, 2680.0/8000, phase, noiseBuffer);
			    phase += ph2680;
			    printFrequency(2680);
		}

		aic3204_read_block(bufferL, bufferR);

	    for (i = 0; i < AUDIO_IO_SIZE; i++)
	    {
	    	bufferL[i] = noiseBuffer[i]/2 + bufferL[i]/2;
	    	bufferR[i] = noiseBuffer[i]/2 + bufferR[i]/2;
	    }

		aic3204_write_block(bufferL, bufferR);

	}
    	
	/* Prekid veze sa AIC3204 kodekom */
    aic3204_disable();

    printf( "\n***Kraj programa***\n" );
	SW_BREAKPOINT;
}

void printFrequency(int freq)
{
	if (freq == 720)
	{
		printChar(7);
		printChar(2);
		printChar(0);
		printChar(H);
		printChar(z);
	}
	else if (freq == 1070)
	{
		printChar(1);
		printChar(0);
		printChar(7);
		printChar(0);
		printChar(H);
		printChar(z);
	}
	else if (freq == 1580)
	{
		printChar(1);
		printChar(5);
		printChar(8);
		printChar(0);
		printChar(H);
		printChar(z);
	}
	else if (freq == 2680)
	{
		printChar(2);
		printChar(6);
		printChar(8);
		printChar(0);
		printChar(H);
		printChar(z);
	}
	else
	{
		printChar(N);
		printChar(o);
		printChar();
		printChar(N);
		printChar(o);
		printChar(i);
		printChar(s);
		printChar(e);
	}
}
