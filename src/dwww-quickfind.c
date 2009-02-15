/* vim:ft=c:cindent:ts=4:sts=4:sw=4:et:fdm=marker
 *
 * File:    dwww-quickfind.c
 * Purpose: Find quickly which package a program belongs to.
 * Author:  Lars Wirzenius <liw@iki.fi>
 * Version: "@(#)dwww:$Id: dwww-quickfind.c 516 2009-01-15 19:51:36Z robert $"
 * Description: Builds a database (--build):
 *          line pairs
 *          first is filename (reversed: /bin/ls -> sl/nib/)
 *          second is package name
 */
#define _GNU_SOURCE 1
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <publib.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>
#include <search.h>

#include "utils.h"

#define DEFAULT_DBFILE "/var/cache/dwww/quickfind.dat"

#define FIRST_TYPE     TYPE_FILE
#define LAST_TYPE      TYPE_VIRTPKG


struct file {
    char * key;
    char * value;
    int type;
};

#define TYPE_FILE       1  /* key => file name ;  value => package */
#define TYPE_PACKAGE    2  /* key => pkg name : value => src pkg name */
#define TYPE_VIRTPKG    3  /* key => virt pkg name : value => pkg name */

static int key_cmp(const void *a, const void *b) {/*{{{*/
    const struct file *aa = a;
    const struct file *bb = b;
    int  res;
    res = strcmp(aa->key, bb->key);
    return res;
}/*}}}*/

static int key_type_cmp(const void *a, const void *b) {/*{{{*/
    const struct file *aa = a;
    const struct file *bb = b;
    int  res;
    res = strcmp(aa->key, bb->key);
    if (!res)
        res = aa->type - bb->type;
    return res;
}/*}}}*/

static int key_value_cmp(const void *a, const void *b) {/*{{{*/
    const struct file *aa = a;
    const struct file *bb = b;
    int  res;
    res = strcmp(aa->key, bb->key);
    if (!res)
        res = aa->type - bb->type;
    if (!res)
        res = strcmp(aa->value, bb->value);
    return res;

}/*}}}*/

static int value_cmp(const void *a, const void *b) {/*{{{*/
    const struct file *aa = a;
    const struct file *bb = b;
    int  res;
    res = strcmp(aa->value, bb->value);
    return res;
}/*}}}*/

static int value_key_cmp(const void *a, const void *b) {/*{{{*/
    const struct file *aa = a;
    const struct file *bb = b;
    int  res;
    res = strcmp(aa->value, bb->value);
    if (!res)
        res = aa->type - bb->type;
    if (!res)
        res = strcmp(aa->key, bb->key);
    return res;

}/*}}}*/

static FILE *f;
static void write_action(const void * const nodep, /*{{{*/
                         const VISIT which, 
                         const int depth UNUSED) {

    struct file * datap;

    switch (which) {
        case postorder:
            /* !!! NOBREAK !!! */
        case leaf:
            datap = *(struct file **) nodep;
            fprintf(f, "%s\n%s\n%d\n", datap->key, datap->value, datap->type);
            break;
        default:
            break;
    }
}/*}}}*/

static void write_db(void ** t_root, const char * const dbfile) {/*{{{*/
    if (!t_root)
        return;

    f = fopen(dbfile, "w");
    if (f == NULL)
        errormsg(1, -1, "couldn't create %s", dbfile);

    twalk(*t_root, write_action);

    if (ferror(f))
        errormsg(1, -1, "error writing to %s", dbfile);
    fclose(f);
}/*}}}*/

static void free_node(void * node) {/*{{{*/
    if (! node)
        return;
    free(((struct file*)node)->key);
    free(((struct file*)node)->value);
    free((struct file*)node);
}/*}}}*/

static void builddb_add_file(void ** t_root, char * package, char * const file, const int type) {/*{{{*/
    static struct file *f = NULL;
    struct file **q;
    int same_pkg;

    if (((same_pkg = !strcmp(file, package))) )
    {
        if (type == TYPE_FILE)
            return;
        same_pkg = type == TYPE_PACKAGE;
    }

    if (!f)
        f = (struct file*) malloc(sizeof(*f));
    if (!f)
        errormsg(1, 0, "insufficient memory");
    f->key      = file;
    f->value    = same_pkg ? "" : package;
    f->type     = type;

    if (!(q = tsearch(f, t_root,  key_value_cmp)))
        errormsg(1, 0, "insufficient memory");

    if (f == *q) {
        // new record
        f->key      = strdup(f->key);
        f->value    = strdup(f->value);
        f = NULL;
    }

}/*}}}*/

static void readdb_add_file(void ** t_root,/*{{{*/
                            void ** t_srcpkg_root,
                            char * const package,
                            char * const file,
                            const char * const type) {
    struct file *f;


    f = (struct file*) malloc(sizeof(*f));
    if (!f)
        errormsg(1, 0, "insufficient memory");
    f->key      = file;
    f->value    = *package ? package: file;
    f->type     = atoi(type);

    if (! tsearch(f, t_root,  key_value_cmp))
        errormsg(1, 0, "insufficient memory");

    // if src_package is not empty add to t_srcpkg_root
    if (f->type == TYPE_PACKAGE)
        if (! tsearch(f, t_srcpkg_root,  value_key_cmp))
            errormsg(1, 0, "insufficient memory");
}/*}}}*/

static void read_db(void ** t_root,/*{{{*/
                    void ** t_srcpkg_root,
                    const char * const dbfile) {
    FILE *f;
    char *file, *pkg, *type;

    f = fopen(dbfile, "r");
    if (f == NULL)
        errormsg(1, -1, "couldn't open %s", dbfile);
    while ((file = getaline(f)) != NULL
        && (pkg = getaline(f)) != NULL
        && (type = getaline(f)) != NULL)
        readdb_add_file(t_root, t_srcpkg_root, pkg, file, type);
    if (ferror(f))
        errormsg(1, -1, "error reading %s", dbfile);
    fclose(f);
}/*}}}*/

/* kludge */
static char * name_is_ok(char * p, int * type) {/*{{{*/
    static struct { const char * const path;
                    const int type;
                  } tab[] =
    {
        {"/bin/",       TYPE_FILE   },
        {"/sbin/",      TYPE_FILE   },
        {"/usr/games/",     TYPE_FILE   },
        {"PKG-",        TYPE_PACKAGE    },
        {"VRT-",        TYPE_VIRTPKG    }
    };
    static int const ntab = sizeof(tab) / sizeof(*tab);

    struct stat st;
    char * tmp;

    int i;

    for (i = 0; i < ntab; ++i) {
        if (strstr(p, tab[i].path) == NULL)
            continue;
        *type = tab[i].type;
        switch (*type) {
            case TYPE_FILE:
                if (stat(p, &st) != -1
                    && S_ISREG(st.st_mode)
                    && (st.st_mode & 0111) != 0)
                    return ( ((tmp = strrchr(p, '/'))) ? tmp + 1 : p);
                else
                    return NULL;
            /* NOT REACHED */

            case TYPE_PACKAGE:
            case TYPE_VIRTPKG:
                return p + strlen(tab[i].path);
            default:
                abort();
        }
    }

    *type = 0;
    return NULL;
}/*}}}*/

static void build(const char * const dbfile) {/*{{{*/
    char *p, *line, *package;
    void *t_root = NULL;
    int type = 0;

    while ((line = getaline(stdin)) != NULL) {
        p = strchr(line, ':');
        if (p == NULL) {
#if 0
            errormsg(1, 0, "syntax error in input: no colon");
#else
            free(line);
            continue;
        }
#endif
        *p++ = '\0';
        strtrim(line);
        strtrim(p);

        p = name_is_ok(p, &type);

        if (p) {
            package = strtok(line, ",");
            while (package) {
                strtrim(package);
                /* package name should not contain some characters... */
                if (strpbrk(package, " \t\n;"))
                    break;
                builddb_add_file(&t_root, package, p, type);
                package = strtok(NULL, ",");
            }
        }

        free(line);

    }
    write_db(&t_root, dbfile);
    tdestroy(t_root, free_node);
}/*}}}*/

static void srcpkg_find(void ** t_root,/*{{{*/
                        void ** t_srcpkg_root,
                        const struct file const * orig_key) {
    struct file key, **p;
    int count = 0;

    if (orig_key->type == TYPE_FILE) {
        key.key     = orig_key->value;
        key.value   = orig_key->key;
        key.type    = TYPE_PACKAGE;
        p = tfind(&key, t_root, key_type_cmp);
        if (!p) return;
        key.key     = (*p)->key;
        key.value   = (*p)->value;
    } else {
        key.key     = orig_key->key;
        key.value   = orig_key->value;
        key.type    = TYPE_PACKAGE;
    }

    p = tfind(&key, t_srcpkg_root, value_cmp);
    while (p != NULL) {
        switch ((*p)->type) {
            case TYPE_PACKAGE:
                if (strcmp((*p)->key, key.key))
                    printf("%s%s", count++ ? " " : ": ",  (*p)->key);
                break;
            }
        tdelete(*p, t_srcpkg_root, value_key_cmp);
        p = tfind(&key, t_srcpkg_root, value_cmp);
    }

}/*}}}*/

static void find(char * const program, const char * const dbfile) {/*{{{*/
    void * t_root = NULL;
    void * t_srcpkg_root = NULL;
    struct file key, **p;
    char * vtable[50];
    int    vcount = 0;



    read_db(&t_root, &t_srcpkg_root, dbfile);
    key.key    = program;
    key.value  = ""; // not needed
    key.type   = 0;  // not needed
    p = tfind(&key, &t_root, key_cmp);
    while (p != NULL) {
        switch ((*p)->type) {
            case TYPE_FILE:
                printf("%s", ((*p)->value));
                srcpkg_find(&t_root, &t_srcpkg_root, *p);
                putchar('\n');
                break;
            case TYPE_PACKAGE:
                printf("%s", ((*p)->key));
                srcpkg_find(&t_root, &t_srcpkg_root, *p);
                putchar('\n');
                break;
            case TYPE_VIRTPKG:
                if (vcount < (int)(sizeof(vtable)/sizeof(char*)))
                    vtable[vcount++] = (*p)->value;
                break;
            }

        tdelete(*p, &t_root, key_value_cmp);
        p = tfind(&key, &t_root, key_cmp);
    }
    if (vcount) {
        printf("VIRTUAL:");
        while (--vcount >= 0) printf(" %s", vtable[vcount]);
        putchar('\n');
    }

}/*}}}*/

int main(int argc, char **argv) {/*{{{*/
    char * dbfile = NULL;

    dwww_initialize("dwww-quickfind");

    if (argc > 1 && !strcmp(argv[1], "--")) {
        --argc;
        ++(argv);
    }

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
}/*}}}*/
