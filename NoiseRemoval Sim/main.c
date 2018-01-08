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
#include "iir.h"


/* Frekvencija odabiranja */
#define SAMPLE_RATE 8000L

/* Velicina prozora za racunanje FFT-a */
#define FFT_SIZE 256

/* Niz za smestanje odbiraka ulaznog signala */
#pragma DATA_ALIGN(InputBufferL,4)
Int16 InputBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(InputBufferR,4)
Int16 InputBufferR[AUDIO_IO_SIZE];

/* Niz za smestanje odbiraka izlaznog signala */
#pragma DATA_ALIGN(OutputBufferL,4)
Int16 OutputBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(OutputBufferR,4)
Int16 OutputBufferR[AUDIO_IO_SIZE];

#pragma DATA_ALIGN(inputCopyL, 4);
Int16 inputCopyL[FFT_SIZE];
#pragma DATA_ALIGN(inputCopyR, 4);
Int16 inputCopyR[FFT_SIZE];

#pragma DATA_ALIGN(fftSpectrumL, 4);
Int32 fftSpectrumL[FFT_SIZE/2];
#pragma DATA_ALIGN(fftSpectrumR, 4);
Int32 fftSpectrumR[FFT_SIZE/2];

#pragma DATA_ALIGN(coefficients1070, 4);
Int16 coefficients1070[6] = { 28184, -18804, 28184, 0, -17490, 20972 };
//Int16 coefficients1070[6] = { 29340, -19575, 29340, 0, -19020, 24802 };
//Int16 coefficients1070[6] = { 30979, -20669, 30979, 0, -20551, 28954 };
//Int16 coefficients1070[6] = { 32445, -21647, 32445, 0, -21644, 32116 };

#pragma DATA_ALIGN(coefficients720, 4);
Int16 coefficients720[6] = { 29000, -22984, 29000, 0, -19500, 16900 };
//Int16 coefficients720[6] = { 29646, -23984, 29646, 0, -21208, 20972 };
//Int16 coefficients720[6] = { 29958, -24237, 29958, 0, -23064, 24802 };
//Int16 coefficients720[6] = { 31111, -25169, 31111, 0, -24919, 28954 };
//Int16 coefficients720[6] = { 32449, -26252, 32449, 0, -26245, 32116 };


#pragma DATA_ALIGN(inputHistoryL, 4);
Int16 inputHistoryL[2];
#pragma DATA_ALIGN(outputHistoryL, 4);
Int16 outputHistoryL[2];

#pragma DATA_ALIGN(inputHistoryR, 4);
Int16 inputHistoryR[2];
#pragma DATA_ALIGN(outputHistoryR, 4);
Int16 outputHistoryR[2];


void main( void )
{
	int i = 0;
	int j = 0;
	int k = 0;
	Int32 maxCoeff = 0;
	int maxCoeffInd = 0;

	/* Inicijalizaija razvojne ploce */
	EZDSP5535_init( );

    /* Inicijalizacija LCD kontrolera */
	initPrintNumber();

	printf("\n Uklanjanje sinusoidalnog suma iz signala \n");

    /* Podesavanje ulazne i izlazne datoteke za simulaciju AD/DA konvertora */
	aic3204_set_input_filename("../Female2Noise1070.pcm");
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
			inputCopyL[j] = InputBufferL[j];
			inputCopyR[j] = InputBufferR[j];
    	}

		for (j = 0; j < AUDIO_IO_SIZE; j++)
		{
			inputCopyL[j] = ((Int32)p_window[j] * inputCopyL[j]) >> 15 + 1;
			inputCopyR[j] = ((Int32)p_window[j] * inputCopyR[j]) >> 15 + 1;
		}

		rfft(inputCopyL, FFT_SIZE, SCALE);
		rfft(inputCopyR, FFT_SIZE, SCALE);

		for (k = 0; k < FFT_SIZE/2; k++)
		{
			fftSpectrumL[k] = (Int32)inputCopyL[2*k]*inputCopyL[2*k] + (Int32)inputCopyL[2*k+1]*inputCopyL[2*k+1];
			fftSpectrumR[k] = (Int32)inputCopyR[2*k]*inputCopyR[2*k] + (Int32)inputCopyR[2*k+1]*inputCopyR[2*k+1];
		}

		maxCoeff = fftSpectrumL[0];
		maxCoeffInd = 0;

		for (k = 1; k < FFT_SIZE/2; k++)
		{
			if (fftSpectrumL[k] > maxCoeff)
			{
				//printf("Nasao veci! k = %d\n", k);
				maxCoeff = fftSpectrumL[k];
				maxCoeffInd = k;
			}
		}

		//printf("Najveci koeficijent: %ld; index: %d\n", maxCoeff, maxCoeffInd);

		for (k = 0; k < AUDIO_IO_SIZE; k++)
		{
			OutputBufferL[k] = 2 * second_order_IIR(InputBufferL[k], coefficients1070, inputHistoryL, outputHistoryL);
			OutputBufferR[k] = 2 * second_order_IIR(InputBufferR[k], coefficients1070, inputHistoryR, outputHistoryR);
		}

		aic3204_write_block(OutputBufferL, OutputBufferR);
	}

	/* Prekid veze sa AIC3204 kodekom */
    aic3204_disable();

    printf( "\n***Kraj programa***\n" );
	SW_BREAKPOINT;
}
