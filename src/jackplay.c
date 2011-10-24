
/************************************************/
/*
  Jackplay.c. This should be a fine example on
  how to add jack-support for a program that is
  not callback-based, I think. (Well, except
  perhaps for the usleep call) -Kjetil M.
 */						 
/***********************************************/

#include <stdio.h>

#include "ceres.h"

char *outportnames[MAX_SAMPS_PER_FRAME]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

/*
void create_jack(struct FFTSound *fftsound){
  JackOpenPlay(fftsound);
  atexit(jackClosePlay);
}
*/

#ifndef USEJACK

/* Dummies, never called. */

void *JackOpenPlay(struct FFTSound *fftsound){
  return NULL;
}
void JackWritePlay(
	       struct FFTSound *fftsound,
	       void *port,double **samples,int num_samples
	       )
{
  return;
}
void JackClosePlay(void *port){
  return;
}

#else 

#include <sys/time.h>

#include <jack/jack.h>


#include "play.h"

typedef jack_default_audio_sample_t sample_t;
typedef jack_nframes_t nframes_t;


#include <unistd.h>


//#include <glib.h>
//#include <jack/port.h>

#define BUFFERSIZE (1024*4)

struct JackPlayChannel{
  jack_port_t *output_port;
  sample_t buffer[BUFFERSIZE];
};

struct Jackplay{
  struct JackPlayChannel jpc[MAX_SAMPS_PER_FRAME];
  jack_client_t *client;
  struct FFTSound *fftsound;
  int writeplace;
  int readplace;
  int buffersize;
  int unread;
};


static jack_client_t *globalclient=NULL; // Hack to get around a bug in jack.
static char clientname[500];

/* Consumer */

int jackprocess (nframes_t nframes, void *arg){
  int ch,i;
  struct Jackplay *jackplay=(struct Jackplay *)arg;
  int numch=jackplay->fftsound->samps_per_frame;
  sample_t *out[numch];

  for(ch=0;ch<numch;ch++){
    out[ch]= (sample_t *) jack_port_get_buffer (jackplay->jpc[ch].output_port, nframes);
    memset(out[ch],0.0f,nframes*sizeof(sample_t));
  }

  for(i=0;i<nframes;i++){
    if(jackplay->unread==0) break;

    for(ch=0;ch<numch;ch++){
      out[ch][i]=jackplay->jpc[ch].buffer[jackplay->readplace];
    }
    jackplay->unread--;

    jackplay->readplace++;
    if(jackplay->readplace==jackplay->buffersize){
      jackplay->readplace=0;
    }
  } 
 
  return 0;
}


/* Providor */

void JackWritePlay(
		   struct FFTSound *fftsound,
		   void *port,double **samples,int num_samples
		   )
{
  struct Jackplay *jackplay=(struct Jackplay *)port;

  int i,ch;

//  fprintf(stderr,"Writing\n");
  for (i=0; i<num_samples; i++) {
    while(jackplay->unread==jackplay->buffersize){
      usleep(128);
    }
    for (ch=0; ch<fftsound->samps_per_frame; ch++){
      jackplay->jpc[ch].buffer[jackplay->writeplace]=(sample_t)samples[ch][i];
    }
    jackplay->unread++;
    jackplay->writeplace++;
    if(jackplay->writeplace==jackplay->buffersize){
      jackplay->writeplace=0;
    }
  }
}


void jackCleanUp(void){
  // jack_client_close(globalclient); (Nah, didnt work)
}

void *JackOpenPlay(struct FFTSound *fftsound){
  int lokke;
  static char *outnames[MAX_SAMPS_PER_FRAME]={"output1","output2","output3","output4","output5","output6","output7","output8"};

  struct Jackplay *jackplay;
  jackplay=calloc(1,sizeof(struct Jackplay));

  jackplay->fftsound=fftsound;
  jackplay->buffersize=BUFFERSIZE;

  //  fprintf(stderr,"Open jack play.\n");

#if 1
  if(globalclient==NULL){ // Hack to get around a bug in jack.
    if (CONFIG_getBool("jack_client_name_is_ceres_plus_pid")==true){
      sprintf(clientname,"ceres-%d",getpid());
    }else{
      sprintf(clientname,"ceres");
    }
    if ((jackplay->client=globalclient = jack_client_new (clientname)) == 0) {
      fprintf(stderr,"Failed. Jack server not running?\n");
      goto failed;
    }
    atexit(jackCleanUp);
  }

  jackplay->client=globalclient;
#else
  if ((jackplay->client=globalclient=jack_client_new ("ceres")) == 0) {
    fprintf(stderr,"Failed. Jack server not running?\n");
    goto failed;
  }
#endif


  if(fftsound->R!=jack_get_sample_rate(jackplay->client)){
    fprintf(
	    stderr,
	    "Warning, sample rate is %d, while jacks sample rate is %lu.\n",
	    fftsound->R,
	    jack_get_sample_rate(jackplay->client)
	    );
  }
  jack_set_process_callback (jackplay->client, jackprocess, jackplay);

  for(lokke=0;lokke<fftsound->samps_per_frame;lokke++){
    jackplay->jpc[lokke].output_port = 
      jack_port_register(
			 jackplay->client,
			 outnames[lokke],
			 JACK_DEFAULT_AUDIO_TYPE,
			 JackPortIsOutput,
			 0
			 );
  }
  if (jack_activate (jackplay->client)) {
    fprintf (stderr, "Error. Cannot activate jack client.\n");
    goto failed_activate;
  }

  for(lokke=0;lokke<fftsound->samps_per_frame;lokke++){
    if (
	jack_connect(
		     jackplay->client,
		     jack_port_name(jackplay->jpc[lokke].output_port),
		     outportnames[lokke]
		     )
	)
      {
	fprintf (stderr, "Error. Cannot connect jack output port %d: \"%s\".\n",lokke,outportnames[lokke]);
	goto failed_connect;
      }
  }
  return jackplay;

 failed_connect:
  for(lokke=0;lokke<jackplay->fftsound->samps_per_frame;lokke++){
    jack_disconnect(
		    jackplay->client,
		    jack_port_name(jackplay->jpc[lokke].output_port),
		    outportnames[lokke]
		    );
    
    jack_port_unregister(
			 jackplay->client,jackplay->jpc[lokke].output_port
			 );
    
  }

 failed_activate:
  jack_deactivate(jackplay->client);
  //  jack_client_close (jackplay->client);
 failed:
  free(jackplay);
  return NULL;
}

void JackClosePlay(void *port){
  int lokke;
  struct Jackplay *jackplay=(struct Jackplay *)port;

  while(jackplay->unread>0){
    usleep(100);
  }

  for(lokke=0;lokke<jackplay->fftsound->samps_per_frame;lokke++){
    jack_disconnect(
		    jackplay->client,
		    jack_port_name(jackplay->jpc[lokke].output_port),
		    outportnames[lokke]
		    );
    
    jack_port_unregister(
			 jackplay->client,jackplay->jpc[lokke].output_port
			 );

  }

  jack_deactivate(jackplay->client);

  //  fprintf(stderr,"closing client\n");
  //  jack_client_close (jackplay->client);
  //  globalclient=NULL;
  free(jackplay);
}



#endif //ifdef USEJACK

