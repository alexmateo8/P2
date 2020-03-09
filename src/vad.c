#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "vad.h"
#include "pav_analysis.h"

const float FRAME_TIME = 10.0F; /* in ms. */
const int UNDECIDED_FRAMES = 5;
//float ko = 0;


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
} Features;

/* 
 * TODO: Delete and use your own features!
 */

Features compute_features(const float *x, int N) {
  /*
   * Input: x[i] : i=0 .... N-1 
   * Ouput: computed features
   */
  /* 
   * DELETE and include a call to your own functions
   *
   * For the moment, compute random value between 0 and 1 
   */
  Features feat;
  feat.p = compute_power(x,N);
  //feat.am = compute_am(x, N);
  //feat.zcr = compute_zcr(x, N, N/(FRAME_TIME * 1e-3)); //?
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
  //printf ("%f\n", f.p);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */

  switch (vad_data->state) {
    case ST_INIT:
    vad_data->state = ST_SILENCE;
    vad_data->ko = f.p; //Cuantas tramas tomamos para establecer el llindar? 
    break;
    /*case ST_INIT:
    if (vad_data->frame<3){
        vad_data->ko = vad_data->ko + pow(10, f.p/10); //Cuantas tramas tomamos para establecer el llindar? 
    }else{
        vad_data->state = ST_SILENCE;
        vad_data->ko = 10*log10(vad_data->ko/3); //3 tramas para establecer el threshold
    }
    break;
    */

  case ST_SILENCE:
    if (f.p > (vad_data->ko))
      vad_data->state = ST_MAYBE_VOICE;
    vad_data->last_change = vad_data->frame;
    vad_data->last_state = ST_SILENCE; //utilizado para la ultima trama no definida, se quedará el ultimo valor escogido.
    break;

  case ST_VOICE:
    if (f.p < (vad_data->ko))
      vad_data->state = ST_MAYBE_SILENCE;
    vad_data->last_change = vad_data->frame;
    vad_data->last_state = ST_VOICE;
    break;

  case ST_MAYBE_SILENCE:
    if(f.p > vad_data->ko){ //Le restamos 4 porque las fricativas sordas tienen poca potencia
      vad_data->state = ST_VOICE;
    }
    else if (f.p < (vad_data->ko) && (vad_data->frame - vad_data->last_change)==UNDECIDED_FRAMES){
      vad_data->state = ST_SILENCE;
    }
    break;

  case ST_MAYBE_VOICE:
    if(f.p < vad_data->ko){
      vad_data->state = ST_SILENCE;
    }
    else if (f.p > (vad_data->ko) && (vad_data->frame - vad_data->last_change)==UNDECIDED_FRAMES){
      vad_data->state = ST_VOICE;
    }
    break;

  case ST_UNDEF:
    break;
  }
  
  
  vad_data->frame++;

  if (vad_data->state == ST_SILENCE ||
      vad_data->state == ST_VOICE)
    return vad_data->state;
  else
    return ST_UNDEF;
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}
