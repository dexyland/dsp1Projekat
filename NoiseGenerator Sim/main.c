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

/* Niz za smestanje odbiraka ulaznog signala */
#pragma DATA_ALIGN(bufferL,4)
Int16 bufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(bufferR,4)
Int16 bufferR[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(bufferL,4)
Int16 outBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(bufferR,4)
Int16 outBufferR[AUDIO_IO_SIZE];

#pragma DATA_ALIGN(noiseBuffer, 4);
Int16 noiseBuffer[AUDIO_IO_SIZE];


#define PI 3.14159265

void main( void )
{
	int i;
	int j;
	float ph720 = 3.2672534;
	float ph1070 = 0.7539773;
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

    /* Podesavanje ulazne i izlazne datoteke za simulaciju AD/DA konvertora */
	aic3204_set_input_filename("../Female2.pcm");
	aic3204_set_output_filename("../output1.pcm");

    /* Inicijalizacija veze sa AIC3204 kodekom (AD/DA) */
    aic3204_hardware_init();

    /* Inicijalizacija AIC3204 kodeka */
	aic3204_init();

    aic3204_dma_init();

    /* Postavljanje vrednosti frekvencije odabiranja i pojacanja na kodeku */
    set_sampling_frequency_and_gain(SAMPLE_RATE, 0);

    ph720 = fmod(2*128*PI*1580.0/8000, 2*PI);

    printf("Fazni pomeraj: %10.8f\n", ph720);

    /* 500 x 128 = 8000odbiraka x 8sec */
	for (j = 0; j < 500; j++)
	{

		aic3204_read_block(bufferL, bufferR);

		phase = fmod(phase, 2*PI);

	    gen_sinus_table(AUDIO_IO_SIZE, 1, 1580.0/8000, phase, noiseBuffer);

	    for (i = 0; i < AUDIO_IO_SIZE; i++)
	    {
	    	outBufferL[i] = noiseBuffer[i]/2 + bufferL[i]/2;
	    	outBufferR[i] = noiseBuffer[i]/2 + bufferR[i]/2;
	    	//outBufferL[i] = noiseBuffer[i];
	    	//outBufferR[i] = noiseBuffer[i];
	    }

	    if (j == 99)
	    {
	    	printf("stop!\n");
	    }
	    phase += ph720;

		aic3204_write_block(outBufferL, outBufferR);

	}

	/* Prekid veze sa AIC3204 kodekom */
    aic3204_disable();

    printf( "\n***Kraj programa***\n" );
	SW_BREAKPOINT;
}


