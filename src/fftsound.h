
struct FFTSound *makeFFTSoundCopy(struct FFTSound *fftsound,int start,int end);
void copyFFTSound(struct FFTSound *from,struct FFTSound *to,int fromstart);
void freeFFTSound(struct FFTSound *fftsound);

