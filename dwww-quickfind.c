/*
 * File:	dwww-quickfind.c
 * Purpose:	Find quickly which package a program belongs to.
 * Author:	Lars Wirzenius <liw@iki.fi>
 * Version:	"@(#)dwww:$Id: dwww-quickfind.c,v 1.3 2002/05/08 06:51:17 robert Exp $"
 * Description:	Builds a database (--build):
 *			line pairs
 *			first is filename (reversed: /bin/ls -> sl/nib/)
 *			second is package name
 */


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <publib.h>

#include <sys/stat.h>
#include <unistd.h>
#include <search.h>

#include "utils.h"

#define DEFAULT_DBFILE "/var/lib/dwww/quickfind.dat"
#define SAME_CHR       "."	


struct file {
	char *package;
	char *file;
};


static int file_cmp(const void *a, const void *b) {
	const struct file *aa = a;
	const struct file *bb = b;
	return strcmp(aa->file, bb->file);
}

static int pfile_cmp(const void *a, const void *b) {
	const struct file *aa = a;
	const struct file *bb = b;
	int  res;
	res = strcmp(aa->file, bb->file);
	if (!res)
		res = strcmp(aa->package, bb->package);
	return res;
		
}

static FILE *f;
static void write_action(const void * nodep, const VISIT which, const int depth)
{
	struct file * datap;

	switch (which) {
		case postorder:
			/* !!! NOBREAK !!! */
		case leaf:
			datap = *(struct file **) nodep;
			fprintf(f, "%s\n%s\n", datap->file, datap->package);
			break;
		default:
			break;
	}
}
			
static void write_db(void ** t_root, char *dbfile) {

	if (!t_root)
		return;
	
	f = fopen(dbfile, "w");
	if (f == NULL)
		errormsg(1, -1, "couldn't create %s", dbfile);
	
	twalk(*t_root, write_action);
	
	if (ferror(f))
		errormsg(1, -1, "error writing to %s", dbfile);
	fclose(f);
}


static void add_file(void ** t_root, char *package, char *file) {
	struct file *f;

	f = (struct file*) malloc(sizeof(*f));
	if (!f)
		errormsg(1, 0, "insufficient memory");

	f->file    = file;
	if (!strcmp(file, package))
		f->package = SAME_CHR;
	else
		f->package = package;
	tsearch(f, t_root,  pfile_cmp);
}


static void read_db(void ** t_root, char *dbfile) {
	FILE *f;
	char *file, *pkg;

	f = fopen(dbfile, "r");
	if (f == NULL)
		errormsg(1, -1, "couldn't open %s", dbfile);
	while ((file = getaline(f)) != NULL && (pkg = getaline(f)) != NULL)
		add_file(t_root, pkg, file);
	if (ferror(f))
		errormsg(1, -1, "error reading %s", dbfile);
	fclose(f);
}


/* kludge */
static int name_is_ok(const char *p) {
	static char *tab[] = {
		"/bin/",
		"/sbin/",
		"/usr/games/",
	};
	static int n = sizeof(tab) / sizeof(*tab);
	int i;
	
	for (i = 0; i < n; ++i)
		if (strstr(p, tab[i]) != NULL)
			return 1;
	return 0;
}


static void build(char *dbfile) {
	char *p, *tmp, *line, *package;
	struct stat st;
	void * t_root = NULL;
	
	while ((line = getaline(stdin)) != NULL) {
		p = strchr(line, ':');
		if (p == NULL)
#if 0
			errormsg(1, 0, "syntax error in input: no colon");
#else
			continue;
#endif
		*p++ = '\0';
		strtrim(line);
		strtrim(p);
		
		/* package name should not contain some characters... */
		if (!(strpbrk(line, " \t\n;,"))) {
			package = strdup(line);
			if (name_is_ok(p) && stat(p, &st) != -1) {
				if (S_ISREG(st.st_mode) && (st.st_mode & 0111) != 0) {
					if ((tmp = strrstr(p, "/")))
							p = strdup(tmp + 1);
					add_file(&t_root, package, p);
				}
			} 
			add_file(&t_root, SAME_CHR, package); /* add package */
		}
		free(line);
		
	}
	write_db(&t_root, dbfile);
}

static void find(char *program, char *dbfile) {
	struct file  key, **p;
	void * t_root = NULL;
	
	read_db(&t_root, dbfile);
	key.file = program;
	do {
		p = tfind(&key, &t_root, file_cmp);
		if (p != NULL)
		{
			if (!strcmp(SAME_CHR,(*p)->package))
				printf("%s\n", (*p)->file);
			else
				printf("%s\n", (*p)->package);
			tdelete(*p, &t_root, pfile_cmp);
		}
	} while (p != NULL);
}


int main(int argc, char **argv) {
	char * dbfile = NULL;
	
	dwww_initialize("dwww-quickfind");

	if (argc == 2)
		dbfile = DEFAULT_DBFILE;
	else if (argc == 3)
		dbfile = argv[2];
	else 
		errormsg(1, 0, "Error: wrong number of arguments");
		
	if (strcmp(argv[1], "--build") == 0)
		build(dbfile);
	else
		find(argv[1], dbfile);
		
	return 0;
}
