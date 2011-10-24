
#include <unistd.h>

#include "ceres.h"
#include "play.h"


static struct TempFile *tempfiles = NULL;

static int pathnum = 0;
static bool freezepath = false;



void TF_freezePath(void)
{
    freezepath = true;
}

void TF_unfreezePath(void)
{
    freezepath = false;
}


/* The return string should be freed after use. */
static char *TF_getPath(void)
{
    if (freezepath == false)
	pathnum++;
    switch (pathnum % 3) {
    case 0:
	return CONFIG_getChar("temporary_path_1");
    case 1:
	return CONFIG_getChar("temporary_path_2");
    case 2:
	return CONFIG_getChar("temporary_path_3");
    }
    return NULL;		// Never happens.
}

void TF_delete(struct TempFile *tf)
{
    struct TempFile *tempfile = tempfiles;
    struct TempFile *prev = NULL;

    while (tempfile != NULL) {
	if (tempfile == tf) {
	    if (prev == NULL) {
		tempfiles = tempfiles->next;
	    } else {
		prev->next = tempfile->next;
	    }
	    fclose(tf->file);
	    unlink(tf->name);
	    free(tf->name);
	    free(tf);
	    return;
	}
	prev = tempfile;
	tempfile = tempfile->next;
    }
    fprintf(stderr,
	    "Error in file tempfile.c function TF_delete: Could not find tempfile\n");
}


void TF_cleanup(void)
{
    PlayStopHard();

    while (tempfiles != NULL) {
	TF_delete(tempfiles);
    }
}

void TF_cleanup_exit(void)
{
    fprintf(stderr, "Cleaning up.\n");
    TF_cleanup();
}

struct TempFile *TF_new(char *firstname)
{
    char temp[500];
    struct TempFile *tf;
    FILE *file;
    char *dir = TF_getPath();

    sprintf(temp, "%sceres_tmp-%s-XXXXXX", dir, firstname);
    free(dir);

    mkstemp(temp);
    file = fopen(temp, "rwb+");
    if (file == NULL) {
	fprintf(stderr,
		"Could not open temporary file \"%s\" for writing.\n",
		temp);
	return NULL;
    }

    tf = malloc(sizeof(struct TempFile));

    tf->next = tempfiles;
    tf->name = malloc(strlen(temp) + 1);
    sprintf(tf->name, "%s", temp);
    tf->file = file;
    tempfiles = tf;

    return tf;
}

struct TempFile *TF_makeCopy(char *firstname, struct TempFile *from)
{
    struct TempFile *to = TF_new(firstname);
    char temp[1024];

    fclose(to->file);
    unlink(to->name);
    //  fclose(from->file);

    sprintf(temp, "cp %s %s", from->name, to->name);
    if (system(temp) != 0) {
	//    from->file=fopen(from->name,"rwb+");
	to->file = fopen(to->name, "rwb+");
	TF_delete(to);
	return NULL;
    }
    //  from->file=fopen(from->name,"rwb+");
    to->file = fopen(to->name, "rwb+");

    return to;
}


void create_tempfile(void)
{
    atexit(TF_cleanup_exit);
}
