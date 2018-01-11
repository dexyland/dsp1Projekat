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
#include "iir.h"
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

/* Niz za smestanje kopija odbiraka ulaznog signala */
#pragma DATA_ALIGN(inputCopyL, 4);
Int16 inputCopyL[FFT_SIZE];
#pragma DATA_ALIGN(inputCopyR, 4);
Int16 inputCopyR[FFT_SIZE];

/* Niz za smestanje fft koeficijenata */
#pragma DATA_ALIGN(fftSpectrumL, 4);
Int32 fftSpectrumL[FFT_SIZE/2];
#pragma DATA_ALIGN(fftSpectrumR, 4);
Int32 fftSpectrumR[FFT_SIZE/2];

/* Koeficijenti notch filtra - 720Hz */
#pragma DATA_ALIGN(coefficients720, 4);
Int16 coefficients720[6] = { 29646, -23984, 29646, 0, -21208, 20972 };
//Int16 coefficients720[6] = { 29958, -24237, 29958, 0, -23064, 24802 };
//Int16 coefficients720[6] = { 31111, -25169, 31111, 0, -24919, 28954 };
//Int16 coefficients720[6] = { 32449, -26252, 32449, 0, -26245, 32116 };

/* Koeficijenti notch filtra - 1070Hz */
#pragma DATA_ALIGN(coefficients1070, 4);
Int16 coefficients1070[6] = { 28184, -18804, 28184, 0, -17490, 20972 };
//Int16 coefficients1070[6] = { 29340, -19575, 29340, 0, -19020, 24802 };
//Int16 coefficients1070[6] = { 30979, -20669, 30979, 0, -20551, 28954 };
//Int16 coefficients1070[6] = { 32445, -21647, 32445, 0, -21644, 32116 };

/* Koeficijenti notch filtra - 1580Hz */
#pragma DATA_ALIGN(coefficients1580, 4);
Int16 coefficients1580[6] = { 27184, -8805, 27184, 0, -8491, 20972 };
//Int16 coefficients1070[6] = { 28918, -9367, 28918, 0, -9234, 24802 };
//Int16 coefficients1070[6] = { 30889, -10006, 30889, 0, -9977, 28954 };
//Int16 coefficients1070[6] = { 32443, -10509, 32443, 0, -10508, 32116 };

/* Koeficijenti notch filtra - 2680Hz */
#pragma DATA_ALIGN(coefficients2680, 4);
Int16 coefficients2680[6] = { 26649, 13565, 26649, 0, 13344, 20972 };
//Int16 coefficients1070[6] = { 28692, 14605, 28692, 0, 14512, 24802 };
//Int16 coefficients1070[6] = { 30841, 15699, 30841, 0, 15679, 28954 };
//Int16 coefficients1070[6] = { 32441, 16514, 32441, 0, 16513, 32116 };

/* Niz za pamcenje prethodnih odbiraka */
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

    /* Inicijalizacija veze sa AIC3204 kodekom (AD/DA) */
    aic3204_hardware_init();
	
    /* Inicijalizacija AIC3204 kodeka */
	aic3204_init();

    aic3204_dma_init();

    /* Postavljanje vrednosti frekvencije odabiranja i pojacanja na kodeku */
    set_sampling_frequency_and_gain(SAMPLE_RATE, 0);

    while(1)
	{
		aic3204_read_block(InputBufferL, InputBufferR);

		/* Kopiranje ulaznih odbiraka i prozoriranje */
		for (j = 0; j < AUDIO_IO_SIZE; j++)
		{
			inputCopyL[j] = ((Int32)p_window[j] * InputBufferL[j]) >> 15 + 1;
			inputCopyR[j] = ((Int32)p_window[j] * InputBufferR[j]) >> 15 + 1;
		}

		/* FFT i racunanje koeficijenata spektra */
		rfft(inputCopyL, FFT_SIZE, SCALE);
		rfft(inputCopyR, FFT_SIZE, SCALE);

		for (k = 0; k < FFT_SIZE/2; k++)
		{
			fftSpectrumL[k] = (Int32)inputCopyL[2*k]*inputCopyL[2*k] + (Int32)inputCopyL[2*k+1]*inputCopyL[2*k+1];
			fftSpectrumR[k] = (Int32)inputCopyR[2*k]*inputCopyR[2*k] + (Int32)inputCopyR[2*k+1]*inputCopyR[2*k+1];
		}

		/* Trazenje najveceg koeficijenta */
		maxCoeff = fftSpectrumL[0];
		maxCoeffInd = 0;

		for (k = 1; k < FFT_SIZE/2; k++)
		{
			if (fftSpectrumL[k] > maxCoeff)
			{
				maxCoeff = fftSpectrumL[k];
				maxCoeffInd = k;
			}
		}

		if (maxCoeffInd >= 22 && maxCoeffInd <= 24)
		{
			//printf("Detected 720Hz noise!\n");

			for (k = 0; k < AUDIO_IO_SIZE; k++)
			{
				InputBufferL[k] = 2 * second_order_IIR(InputBufferL[k], coefficients720, inputHistoryL, outputHistoryL);
				InputBufferR[k] = 2 * second_order_IIR(InputBufferR[k], coefficients720, inputHistoryR, outputHistoryR);
			}
		}
		else if (maxCoeffInd == 34)
		{
			//printf("Detected 1070Hz noise!\n");

			for (k = 0; k < AUDIO_IO_SIZE; k++)
			{
				InputBufferL[k] = 2 * second_order_IIR(InputBufferL[k], coefficients1070, inputHistoryL, outputHistoryL);
				InputBufferR[k] = 2 * second_order_IIR(InputBufferR[k], coefficients1070, inputHistoryR, outputHistoryR);
			}
		}
		else if (maxCoeffInd >= 50 && maxCoeffInd <= 51)
		{
			//printf("Detected 1580Hz noise!\n");

			for (k = 0; k < AUDIO_IO_SIZE; k++)
			{
				InputBufferL[k] = 2 * second_order_IIR(InputBufferL[k], coefficients1580, inputHistoryL, outputHistoryL);
				InputBufferR[k] = 2 * second_order_IIR(InputBufferR[k], coefficients1580, inputHistoryR, outputHistoryR);
			}
		}
		else if (maxCoeffInd == 86)
		{
			//printf("Detected 2680Hz noise!\n");

			for (k = 0; k < AUDIO_IO_SIZE; k++)
			{
				InputBufferL[k] = 2 * second_order_IIR(InputBufferL[k], coefficients2680, inputHistoryL, outputHistoryL);
				InputBufferR[k] = 2 * second_order_IIR(InputBufferR[k], coefficients2680, inputHistoryR, outputHistoryR);
			}
		}
		else
		{
			//printf("No noise!\n");
		}

		aic3204_write_block(InputBufferL, InputBufferR);
	}
    	
	/* Prekid veze sa AIC3204 kodekom */
    aic3204_disable();

    printf( "\n***Kraj programa***\n" );
	SW_BREAKPOINT;
}
