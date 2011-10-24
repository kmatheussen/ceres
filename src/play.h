
#include <pthread.h>


struct PlayStruct{
  void *playdata;
  struct FFTSound *fftsound;  
  bool usejack;
};

struct DoPlayStruct{
  struct FFTSound *fftsound;
  int start;
  int end;
  bool usejack;
  bool keep_formants;
};

void PlayWaveConsumer(
		      struct FFTSound *fftsound,
		      void *pointer,
		      double **samples,
		      int num_samples
		      );

Boolean InitPlay(void);

void *OpenPlay(struct FFTSound *fftsound); // Returns NULL if Play couldnt open, else port.
void *JackOpenPlay(struct FFTSound *fftsound);

void WritePlay(
		      struct FFTSound *fftsound,
		      void *port,double **samples,int size
		      );
void JackWritePlay(
	       struct FFTSound *fftsound,
	       void *port,double **samples,int num_samples
	       );

void JackClosePlay(void *port);
void ClosePlay(void *port);

void *DoPlay(void *pointer);
void Play(struct FFTSound *fftsound,int start,int end);
void PlayStop(void);
void PlayStopHard(void);

