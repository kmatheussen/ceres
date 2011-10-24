
struct TempFile{
  struct TempFile *next;
  FILE *file;
  char *name;
};

void TF_freezePath(void);
void TF_unfreezePath(void);
void TF_delete(struct TempFile *tf);
void TF_cleanup(void);
void TF_cleanup_exit(void);
struct TempFile *TF_new(char *firstname);
struct TempFile *TF_makeCopy(char *firstname,struct TempFile *from);
