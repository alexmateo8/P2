#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "vad.h"
#include "pav_analysis.h"

const float FRAME_TIME = 10.0F; /* in ms. */
const int UNDECIDED_FRAMES = 5;
const int N_INIT = 3;
const float LLINDAR = 10; //llindar del power
const float LOW_ZERO_CROSSING = 2;
const float HIGH_ZERO_CROSSING = 1.5;



/* 
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT"//,"MAYBE_SILENCE", "MAYBE_VOICE"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
  float sampling_rate;
} Features;

/* 
 * TODO: Delete and use your own features!
 */

Features compute_features(const float *x, int N) {
  /*
   * Input: x[i] : i=0 .... N-1 
   * Ouput: computed features
   */

  Features feat;
  feat.p = compute_power(x,N);
  //feat.am = compute_am(x, N);
  feat.zcr = compute_zcr(x, N, 16000); //?
  return feat;
  
}

/* 
 * TODO: Init the values of vad_data
 */

VAD_DATA * vad_open(float rate) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;
  vad_data->ko = 0;
  vad_data->low_zero_crossing = 0;
  vad_data->high_zero_crossing = 0;
  vad_data->last_change = 0;
  vad_data->frame = 0;
  vad_data->last_state = ST_INIT;
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /* 
   * TODO: decide what to do with the last undecided frames
   */
  
  VAD_STATE state = vad_data->last_state; //se queda el ultimo valor (V o S) guardado


  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

/* 
 * TODO: Implement the Voice Activity Detection 
 * using a Finite State Automata
 */

VAD_STATE vad(VAD_DATA *vad_data, float *x) {

  /* 
   * TODO: You can change this, using your own features,
   * program finite state automaton, define conditions, etc.
   */

  Features f = compute_features(x, vad_data->frame_length);
  printf ("%f\n", f.zcr);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */

  switch (vad_data->state) {
   /* case ST_INIT:
    vad_data->state = ST_SILENCE;
    vad_data->zero_crossing = f.zcr;
    vad_data->ko = f.p + 15; //Cuantas tramas tomamos para establecer el llindar? 
    break;*/
    case ST_INIT:
    if (vad_data->frame<N_INIT){
        vad_data->ko = vad_data->ko + pow(10, f.p/10); //Cuantas tramas tomamos para establecer el llindar? 
        vad_data->low_zero_crossing = vad_data->low_zero_crossing + f.zcr;
        vad_data->high_zero_crossing = vad_data->high_zero_crossing + f.zcr;
    }else{
        vad_data->state = ST_SILENCE;
        vad_data->ko = 10*log10(vad_data->ko/N_INIT) + LLINDAR; //3 tramas para establecer el threshold
        vad_data->low_zero_crossing /= (LOW_ZERO_CROSSING*N_INIT); //dividimos entre 2 para establecer un buen umbral
        vad_data->high_zero_crossing = HIGH_ZERO_CROSSING*vad_data->high_zero_crossing/N_INIT;
        printf ("\t%f\n", vad_data->low_zero_crossing);
        printf ("\t%f\n", vad_data->high_zero_crossing);
         //printf ("%f\n", vad_data->zero_crossing);
    }
    break;
    

  case ST_SILENCE:
    if (f.p > vad_data->ko || (f.zcr < vad_data->low_zero_crossing || f.zcr > vad_data->high_zero_crossing))
      vad_data->state = ST_MAYBE_VOICE;
    vad_data->last_change = vad_data->frame;
    vad_data->last_state = ST_SILENCE; //utilizado para la ultima trama no definida, se quedará el ultimo valor escogido.
    break;

  case ST_VOICE:
    if (f.p < (vad_data->ko) && (f.zcr > vad_data->low_zero_crossing && f.zcr < vad_data->high_zero_crossing))
      vad_data->state = ST_MAYBE_SILENCE;
    vad_data->last_change = vad_data->frame;
    vad_data->last_state = ST_VOICE;
    break;

  case ST_MAYBE_SILENCE:
    if(f.p > vad_data->ko){ 
      vad_data->state = ST_VOICE;
    }
    else if ((f.p < (vad_data->ko) && (vad_data->frame - vad_data->last_change)==UNDECIDED_FRAMES)){
      vad_data->state = ST_SILENCE;
    }
    break;

  case ST_MAYBE_VOICE:
    if(f.p < vad_data->ko){
      vad_data->state = ST_SILENCE;
    }
    else if ((f.p > (vad_data->ko) || (vad_data->frame - vad_data->last_change)==UNDECIDED_FRAMES))  /*|| (f.zcr < vad_data->low_zero_crossing || f.zcr > vad_data->high_zero_crossing)*/{
      vad_data->state = ST_VOICE;
    }
    break;

  case ST_UNDEF:
    break;
  
  }
  
  
  vad_data->frame++;

  if (vad_data->state == ST_SILENCE || vad_data->state == ST_VOICE)
    return vad_data->state;
  else if (vad_data->state == ST_INIT)
    return ST_SILENCE;
  else
    return ST_UNDEF;

}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}


/*
Hipotesis sobre fitxer creat de audio mac, a fm=16kHz:
WE CONSIDER NOISE (silence) IF
-> zcr > 4000 
-> power < -50
-> amplitude < 0.01

WE CONSIDER VOWEL
-> zcr < 1200
-> power > -35
-> amplitude > 0.02

WE CONSIDER CONSONANT Ex: /s/
-> zcr > 4000
-> power > -35
-> amplitude > 0.02


*/