void CONFIG_show(void);

bool CONFIG_getBool(char *confname);
int CONFIG_getInt(char *confname);
double CONFIG_getDouble(char *confname);
char *CONFIG_getChar(char *confname);

struct FFTSound;
int CONFIG_getMaxPreviewLength(struct FFTSound *fftsound);
