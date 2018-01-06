//////////////////////////////////////////////////////////////////////////////
// *
// * Predmetni projekat iz predmeta OAiS DSP 1
// * Godina: 2017
// *
// * Zadatak: Uklanjanje periodicnog suma upotrebom notch filtra
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
#include "print_number.h"
#include "math.h"
#include "Dsplib.h"
#include "window.h"


/* Frekvencija odabiranja */
#define SAMPLE_RATE 8000L

/* Velicina prozora za racunanje FFT-a */
#define FFT_SIZE 256

/* Niz za smestanje odbiraka ulaznog signala */
#pragma DATA_ALIGN(InputBufferL,4)
Int16 InputBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(InputBufferR,4)
Int16 InputBufferR[AUDIO_IO_SIZE];

#pragma DATA_ALIGN(inputCopyL, 4);
Int16 inputCopyL[FFT_SIZE];
#pragma DATA_ALIGN(inputCopyR, 4);
Int16 inputCopyR[FFT_SIZE];

#pragma DATA_ALIGN(fftSpectrumL, 4);
Int32 fftSpectrumL[FFT_SIZE/2];
#pragma DATA_ALIGN(fftSpectrumR, 4);
Int32 fftSpectrumR[FFT_SIZE/2];

void main( void )
{
	int i = 0;
	int j = 0;
	int k = 0;

	/* Inicijalizaija razvojne ploce */
	EZDSP5535_init( );

    /* Inicijalizacija LCD kontrolera */
	initPrintNumber();

	printf("\n Uklanjanje sinusoidalnog suma iz signala \n");

    /* Podesavanje ulazne i izlazne datoteke za simulaciju AD/DA konvertora */
	aic3204_set_input_filename("../noise1070Hz.pcm");
	aic3204_set_output_filename("../output1.pcm");

    /* Inicijalizacija veze sa AIC3204 kodekom (AD/DA) */
    aic3204_hardware_init();

    /* Inicijalizacija AIC3204 kodeka */
	aic3204_init();

    aic3204_dma_init();

    /* Postavljanje vrednosti frekvencije odabiranja i pojacanja na kodeku */
    set_sampling_frequency_and_gain(SAMPLE_RATE, 0);

    for (i = 0; i < 500; i++)
	{
    	aic3204_read_block(InputBufferL, InputBufferR);

		for (j = 0; j < AUDIO_IO_SIZE; j++)
		{
			inputCopyL[j] = ((Int32)p_window[j] * InputBufferL[j]) >> 15 + 1;
			inputCopyR[j] = ((Int32)p_window[j] * InputBufferR[j]) >> 15 + 1;
		}

		rfft(inputCopyL, FFT_SIZE, SCALE);
		rfft(inputCopyR, FFT_SIZE, SCALE);

		for (k = 0; k < FFT_SIZE/2; k++)
		{
			fftSpectrumL[k] = (Int32)inputCopyL[2*k]*inputCopyL[2*k] + (Int32)inputCopyL[2*k+1]*inputCopyL[2*k+1];
			fftSpectrumR[k] = (Int32)inputCopyR[2*k]*inputCopyR[2*k] + (Int32)inputCopyR[2*k+1]*inputCopyR[2*k+1];
		}

		aic3204_write_block(InputBufferL, InputBufferR);
	}

	/* Prekid veze sa AIC3204 kodekom */
    aic3204_disable();

    printf( "\n***Kraj programa***\n" );
	SW_BREAKPOINT;
}
