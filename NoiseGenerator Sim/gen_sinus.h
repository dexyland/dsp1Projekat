//////////////////////////////////////////////////////////////////////////////
// *
// * Predmetni projekat iz predmeta OAiS DSP 1
// * Godina: 2016
// *
// * Generisanje Sinusnog signala upotrebom tabele pretrazivanja
// *
//////////////////////////////////////////////////////////////////////////////

#ifndef DTMF_PREDAJNIK_GEN_SINUS_H_
#define DTMF_PREDAJNIK_GEN_SINUS_H_


/********************************************************************
 * Generisanje sinusnog signala upotrebom tabele pretrazivanja
 *   n - broj odbiraka
 *   a - amplituda (u opsegu 0 - 1.0)
 *   f - frekvencija (normalizovana)
 *   ph - fazni pomeraj
 *   buffer - niz u kome ce biti smesten izlazni signal
 *********************************************************************/
extern void gen_sinus_table(Int16 n, Int16 a, float f, float ph, Int16 buffer[]);

#endif /* DTMF_PREDAJNIK_GEN_SINUS_H_ */
