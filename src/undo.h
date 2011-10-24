void UNDO_cleanup(void);
void UNDO_Reset(void);
void UNDO_addTransform(struct FFTSound *fftsound);
void UNDO_do(struct FFTSound *fftsound);
void UNDO_redo(struct FFTSound *fftsound);
int UNDO_getDoUndo(void);
void UNDO_setDoUndo(int dasdoundo);
bool UNDO_allowedUndo(void);
bool UNDO_allowedRedo(void);

