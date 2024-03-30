// main.c
 #include <string.h>
 #include <stdio.h>
 #include <math.h>
#include <stdlib.h>
 
 struct soundhdr
 {
   char riff[4];           /* "RIFF"                                  */
   int32_t flength;        /* file length in bytes                    */
   char wave[4];           /* "WAVE"                                  */
   char fmt[4];            /* "fmt "                                  */
   int32_t chunk_size;     /* size of FMT chunk in bytes (usually 16) */
   int16_t format_tag;     /* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM */
   int16_t num_chans;      /* 1=mono, 2=stereo                        */
   int32_t srate;          /* Sampling rate in samples per second     */
   int32_t bytes_per_sec;  /* bytes per second = srate*bytes_per_samp */
   int16_t bytes_per_samp; /* 2=16-bit mono, 4=16-bit stereo          */
   int16_t bits_per_samp;  /* Number of bits per sample               */
   char data[4];           /* "data"                                  */
   int32_t dlength;        /* data length in bytes (filelength - 44)  */
 };
 
 struct soundhdr wavh;
 
 // https://pages.mtu.edu/~suits/notefreqs.html
 const float D2 = 73.42;
 const float G2 = 98.00;
 const float A2 = 110.00;
 const float B2 = 123.47;
 const float D3 = 146.83;
 const float G3 = 196.00;
 const float A3 = 220.00;
 const float B3 = 246.94;
 const float Cs4 = 277.18;
 const float D4 = 293.66;
 const float E4 = 329.63;
 const float Fs4 = 369.99;
 const float G4 = 392.00;
 const float A4 = 440.00;
 const float B4 = 493.88;
 
 const int num_measures_to_play = 4 * 4;
 const int sample_rate = 16000;
 const int beats_per_measure = 4;
 const int ms_per_beat = 500;
 const int samples_per_beat = (sample_rate * ms_per_beat) / 1000;
 const int buffer_size = num_measures_to_play * beats_per_measure * samples_per_beat;
 buffer = (short int *) malloc((buffer_size+1) * sizeof(short int));
 int i = 0;
 while (buffer[i]) {buffer[i++] = 0;}
 //short int buffer[buffer_size] = {};
 
 void play(float freq, float duration, int measure, float beat)
 {
   float current_beat = beats_per_measure * measure + beat;
   int start_index = current_beat * samples_per_beat;
   int end_index = start_index + duration * samples_per_beat;
 
   for (int i = start_index; i < end_index; i++)
   {
     float percent_through_note = ((float)i - start_index) / (end_index - start_index);
     float amplitude_multiplier = 1.0f;
 
     if (percent_through_note < 0.25)
     {
       amplitude_multiplier = (percent_through_note / 0.25);
     }
     else if (percent_through_note < 0.5f)
     {
       amplitude_multiplier = (1 - ((percent_through_note - 0.25) / 0.25)) * 0.8 + 0.2;
     }
     else
     {
       amplitude_multiplier = 1 - percent_through_note;
     }
 
     buffer[i] += (short int)((cos((2 * M_PI * freq * i) / sample_rate) * 3000) * amplitude_multiplier);
   }
 }
 
 // DMajor
 void DM(float duration, int measure, float beat) {
   play(D4, duration, measure, beat);
   play(Fs4, duration, measure, beat);
   play(A4, duration, measure, beat);
 }
 
 // A Major 1st Inversion
 void AM1st(float duration, int measure, float beat) {
   play(Cs4, duration, measure, beat);
   play(E4, duration, measure, beat);
   play(A4, duration, measure, beat);
 }
 
 // B Minor 1st Inversion
 void Bm1st(float duration, int measure, float beat) {
   play(D4, duration, measure, beat);
   play(Fs4, duration, measure, beat);
   play(B4, duration, measure, beat);
 }
 
 // G Major 2nd Inversion
 void GM2nd(float duration, int measure, float beat) {
   play(D4, duration, measure, beat);
   play(G4, duration, measure, beat);
   play(B4, duration, measure, beat);
 }
 
 int main(void)
 {
   strncpy(wavh.riff, "RIFF", 4);
   strncpy(wavh.wave, "WAVE", 4);
   strncpy(wavh.fmt, "fmt ", 4);
   strncpy(wavh.data, "data", 4);
 
   wavh.chunk_size = 16;
   wavh.format_tag = 1;
   wavh.num_chans = 1;
   wavh.srate = sample_rate;
   wavh.bits_per_samp = 16;
 
   int sample_index_in_beat = 0;
   int beat_index = 0;
 
   int measure = 0;
   while (measure < num_measures_to_play) {
     play(D2, 4, measure, 0);
     play(D3, 4, measure, 0);
     DM(1, measure, 0);
     DM(1, measure, 1);
     DM(1, measure, 2);
     DM(1, measure, 3);
 
     measure++;
     play(A3, 4, measure, 0);
     play(A2, 4, measure, 0);
     AM1st(1, measure, 0);
     AM1st(1, measure, 1);
     AM1st(1, measure, 2);
     AM1st(1, measure, 3);
 
     measure++;
     play(B2, 4, measure, 0);
     play(B3, 4, measure, 0);
     Bm1st(1, measure, 0);
     Bm1st(1, measure, 1);
     Bm1st(1, measure, 2);
     Bm1st(1, measure, 3);
 
     measure++;
     play(G2, 4, measure, 0);
     play(G3, 4, measure, 0);
     GM2nd(1, measure, 0);
     GM2nd(1, measure, 1);
     GM2nd(1, measure, 2);
     GM2nd(1, measure, 3);
 
     measure++;
   }
 
   wavh.flength = buffer_size + 44;
   const int num_bytes = 2;
   wavh.dlength = buffer_size * num_bytes;
 
   FILE *fp = fopen("test.wav", "w");
   fwrite(&wavh, 1, 44, fp);
   fwrite(buffer, 2, buffer_size, fp);
 }
