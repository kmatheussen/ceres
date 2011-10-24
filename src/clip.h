

struct Clip{
  struct TempFile *megampfile;
  struct TempFile *megfreqfile;

  int area_start;
  int area_end;
  int area_y1;
  int area_y2;
};

void CLIP_paste(
		struct Clip *clip,
		struct FFTSound *fftsound,
		int to_area_start,int to_area_end
		);

void CLIP_delete(struct Clip *clip);

struct Clip *CLIP_new(
		      struct FFTSound *fftsound,
		      int area_start,int area_end,
		      int area_y1,int area_y2
		      );


