#include "tistdtypes.h"
#include "gen_sinus.h"
#include "sine_table.h"
#include "math.h"

#define PI 3.14159265359

/********************************************************************
 * Generisanje sinusnog signala upotrebom tabele pretrazivanja
 *   n - broj odbiraka
 *   a - amplituda (u opsegu 0 - 1.0)
 *   f - frekvencija (normalizovana)
 *   ph - fazni pomeraj
 *   buffer - niz u kome ce biti smesten izlazni signal
 *********************************************************************/

void gen_sinus_table(Int16 n, Int16 a, float f, float ph, Int16 buffer[])
{
	int i = 0;
	int delta = f * 4 * SINE_TABLE_SIZE;
	int k = ph*4*SINE_TABLE_SIZE/(2*PI);
	int mask = 4*SINE_TABLE_SIZE - 1;
	int index = 0;

	for (i = 0; i < n; i++)
	{
		k = k & mask;

		if (k < SINE_TABLE_SIZE)
		{
			buffer[i] = a*p_sine_table[k];
		}
		else if (k < 2*SINE_TABLE_SIZE)
		{
			index = 2*SINE_TABLE_SIZE - k;

			if (index >= SINE_TABLE_SIZE)
			{
				index = 511;
			}

			buffer[i] = a*p_sine_table[index];
		}
		else if (k < 3*SINE_TABLE_SIZE)
		{
			index = k - 2*SINE_TABLE_SIZE;

			if (index >= SINE_TABLE_SIZE)
			{
			  index = 511;
			}

			buffer[i] = -a*p_sine_table[index];
		}
		else
		{
			index = 4*SINE_TABLE_SIZE - k;

			if (index >= SINE_TABLE_SIZE)
			{
			  index = 511;
			}

			buffer[i] = -a*p_sine_table[index];
		}

		k += delta;
	}
}
