/* vim:ft=c:cindent:ts=4:sts=4:sw=4:et:fdm=marker
 * 
 * File:    dwww-cache.c
 * Purpose: Manage the dwww cache of converted documents.
 * Author:  Lars Wirzenius
 * Version: "(#)dwww:$Id: dwww-cache.c 516 2009-01-15 19:51:36Z robert $"
 * Description: See the manual page for how to use this program.
 *
 *      Basically, what we do is read in a file from stdin,
 *      store it somewhere, and then retrieve it when asked.
 *      The original name of the file is given on the command
 *      line.  We must be quick, so we keep a mapping between
 *      between the original name and the new name in a file.
 *
 *      The current implementation uses a simple "database"
 *      for the mapping that was written specifically for this
 *      program.  It's relatively fast as long as the database
 *      is not too big -- the whole database is read and written
 *      instead of just a record.  This may have to change
 *      in a future version.  On the other hand, as long as
 *      the database can be kept small, this is quite fast.
 *
 *      One way to make it smaller than it is is to compress
 *      the filenames, since they contain a lot of common
 *      strings ("/usr/man/" for example).  We'll see.
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <publib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <utime.h>
#include <sys/file.h>
#include <libgen.h>

#include "utils.h"

#define BUF_SIZE 1024

/*
 * Filename prefixes (for simple compression). At most 256 elements!
 */
static const char *prefs[] = {/* prefs[] {{{*/
    "",     /* MUST BE EMPTY STRING */
    "/usr/lib/",
    "/usr/share/doc/",
    "/usr/doc/",
    "/usr/local/doc/",
    "/usr/share/info/",
    "/usr/share/common-licenses/",
    "/usr/info/",
    "/usr/local/info/",
    "/usr/share/man/man1/",
    "/usr/share/man/man2/",
    "/usr/share/man/man3/",
    "/usr/share/man/man4/",
    "/usr/share/man/man5/",
    "/usr/share/man/man6/",
    "/usr/share/man/man7/",
    "/usr/share/man/man8/",
    "/usr/share/",
    "/usr/X11R6/man/man1/",
    "/usr/X11R6/man/man2/",
    "/usr/X11R6/man/man3/",
    "/usr/X11R6/man/man4/",
    "/usr/X11R6/man/man5/",
    "/usr/X11R6/man/man6/",
    "/usr/X11R6/man/man7/",
    "/usr/X11R6/man/man8/",
    "/usr/local/man/man1/",
    "/usr/local/man/man2/",
    "/usr/local/man/man3/",
    "/usr/local/man/man4/",
    "/usr/local/man/man5/",
    "/usr/local/man/man6/",
    "/usr/local/man/man7/",
    "/usr/local/man/man8/",
    "/usr/man/man1/",
    "/usr/man/man2/",
    "/usr/man/man3/",
    "/usr/man/man4/",
    "/usr/man/man5/",
    "/usr/man/man6/",
    "/usr/man/man7/",
    "/usr/man/man8/",
};/*}}}*/
static int nprefs = sizeof(prefs) / sizeof(*prefs);


/*
 * Information about one file in in-memory format.
 */
struct cache_entry {/*{{{*/
    char type;
    char permanent;
    int pref;       /* full name if prefs[pref]+original */
    char *original;
    char *converted;
};/*}}}*/

/*
 * The whole database.
 */
struct dochash {
    char *db;
    size_t db_size;
    struct dynarr tab;
};

/*
 * Where we store the files and the database.
 */
#define SPOOL_DIR "/var/cache/dwww/db/"

/*
 * The name of the database in SPOOL_DIR.
 */
#define CACHE_DB SPOOL_DIR ".cache_db"


/*
 * What does the user want us to do?
 */
enum {
    action_error,
    action_help,
    action_store,
    action_lookup,
    action_list,
    action_list_all,
    action_clean
};
static int action = action_error;

/*
 * Options; for GNU getopt_long.
 */
static struct option options[] = {/*{{{*/
    { "help", 0, &action, action_help },
    { "store", 0, &action, action_store },
    { "lookup", 0, &action, action_lookup },
    { "list", 0, &action, action_list },
    { "list-all", 0, &action, action_list_all },
    { "clean", 0, &action, action_clean },
    { NULL, 0, NULL, 0 },
};/*}}}*/


static int parse_options(int, char **, struct option *, int *);
static int store(char *, char *);
static int lookup(char *, char *);
static int list(char *, char *);
static int list_all(char *, char *);
static int clean(char *, char *);

static int help(char *type UNUSED, char *location UNUSED) { /*{{{*/
    fprintf(stdout, "Usage: %s [--lookup|--store|--list] type location\n"
                    "       %s --list-all|--clean|--help\n",
                    get_progname(), get_progname() );
    exit(0);
}   /*}}}*/

static int open_db_reading(void);
static int open_db_writing(void);
static int read_db(int, struct dochash *);
static int compare_entry(const void *, const void *);
static void sort_db(struct dochash *);
static int lookup_db(struct dochash *, char *);
static int insert_db(struct dochash *, struct cache_entry *);
static int write_db(int, struct dochash *);
static int close_db(int);
static int storage_size(struct cache_entry *);
static int output_file(char *);
static void init_entry(struct cache_entry *, char *, char *, char *, int);
static int create_tmp_file(char *, char *, size_t);
static int get_new_filename(char *, char *, char *, size_t);
static int copy_fd_to_fd(int, int, int);
static char *int_to_type(int);
static int type_to_int(char *);
static int rebuild_db(struct dochash *);
static time_t mtime(char *);
static int find_pref(const char *);
static int set_modtime(const char *, time_t); 

int main(int argc, char **argv) {/*{{{*/
    static struct {
        int action;
        int need_args;
        int (*func)(char *, char *);
    } actions[] = {
        { action_help, 0, help },
        { action_lookup, 2, lookup },
        { action_store, 2, store },
        { action_list, 2, list },
        { action_list_all, 0, list_all },
        { action_clean, 0, clean },
        { action_error, 0, NULL },
    };
    int i, first_nonopt;

    dwww_initialize( "dwww-cache");


    if (parse_options(argc, argv, options, &first_nonopt) == -1)
        return EXIT_FAILURE;
        
    for (i = 0; actions[i].action != action_error; ++i)
        if (actions[i].action == action)
            break;

    if (actions[i].action == action_error)
        errormsg(1, 0, "error: no action specified");

    if (actions[i].need_args != argc - first_nonopt)
        errormsg(1, 0, "error: wrong number of args");

    if (actions[i].func(argv[first_nonopt], argv[first_nonopt+1]) == -1)
        return EXIT_FAILURE;

    if (fflush(stdout) == EOF || ferror(stdout)) {
        errormsg(0, -1, "error writing to stdout");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}/*}}}*/


static int parse_options(int argc, char **argv, struct option *opt, int *ind) {/*{{{*/
    int o;

    while ((o = getopt_long(argc, argv, "", opt, ind)) != EOF)
        if (o == '?' || o == ':')
            return -1;
    *ind = optind;
    return 0;
}/*}}}*/



static int list(char *type UNUSED, char *location) {/*{{{*/
    struct dochash hash;
    struct cache_entry *data;
    int i, db;
    char buf[BUF_SIZE];
    char orig[BUF_SIZE];

    db = open_db_reading();
    if (db == -1)
        return -1;
    if (read_db(db, &hash) == -1)
        return -1;
    i = lookup_db(&hash, location);
    if (i == -1) {
#if 0
        errormsg(0, 0, "document not in cache");
#endif
        return -1;
    }
    
    data = hash.tab.data;
    snprintf(orig, sizeof(orig), "%s%s", prefs[data[i].pref], data[i].original);
    snprintf(buf,  sizeof(buf),  "%s%.20s", SPOOL_DIR, data[i].converted);
    if (mtime(orig) != mtime(buf)) {
        (void) close_db(db);
        (void) truncate(buf, 0);
        return -1;
    }
    printf("%s %s %s %s\n", int_to_type(data[i].type),
        orig, data[i].converted,
        data[i].permanent ? "y" : "n");
    if (close_db(db) == -1)
        return -1;
    return 0;
}/*}}}*/



static int list_all(char *type UNUSED, char *location UNUSED) {/*{{{*/
    struct dochash hash;
    struct cache_entry *data;
    size_t i;
    int    db;
    char buf[BUF_SIZE];
    char orig[BUF_SIZE];

    db = open_db_reading();
    if (db == -1)
        return -1;
    if (read_db(db, &hash) == -1)
        return -1;
    data = hash.tab.data;
    
    for (i = 0; i < hash.tab.used; ++i) {
        snprintf(orig, sizeof(orig), "%s%s", prefs[data[i].pref], data[i].original);
        snprintf(buf,  sizeof(buf),  "%s%.20s", SPOOL_DIR, data[i].converted);
        if (mtime(orig) != mtime(buf)) 
            (void) truncate(buf, 0);
        else
            printf("%s %s %s %s\n", int_to_type(data[i].type),
                orig, data[i].converted,
                data[i].permanent ? "y" : "n");
    }
    
    if (close_db(db) == -1)
        return -1;
    
    return 0;
}/*}}}*/



static int lookup(char *type UNUSED, char *location) {/*{{{*/
    struct dochash hash;
    struct cache_entry *data;
    int i, db;
    char buf[BUF_SIZE];
    char orig[BUF_SIZE];

    db = open_db_reading();
    if (db == -1)
        return -1;
    if (read_db(db, &hash) == -1)
        return -1;
    i = lookup_db(&hash, location);
    if (i == -1)
        return -1;
    data = hash.tab.data;
    snprintf(orig, sizeof(orig), "%s%s", prefs[data[i].pref], data[i].original);
    snprintf(buf,  sizeof(buf),  "%s%.20s", SPOOL_DIR, data[i].converted);
    if (mtime(orig) != mtime(buf)) {
        (void) close_db(db);
        (void) truncate(buf, 0);
        return -1;
    }
    if (output_file(buf) == -1) {
        (void) close_db(db);
        return -1;
    }
    if (close_db(db) == -1)
        return -1;
    return 0;
}/*}}}*/



static int store(char *type, char *location) {/*{{{*/
    int i, db, fd;
    struct dochash hash;
    struct cache_entry new, *data;
    char filename[BUF_SIZE];
    char buf[BUF_SIZE];
    char tmp_file[BUF_SIZE];

    fd = create_tmp_file(SPOOL_DIR, tmp_file, sizeof(tmp_file));
    if (fd == -1) {
        errormsg(0, -1, "can't create temporary cache file");
        fd = STDOUT_FILENO;
    }
    
    if (copy_fd_to_fd(STDIN_FILENO, fd, STDOUT_FILENO) == -1) {
        errormsg(0, -1, "can't copy stdin to %s", tmp_file);
        (void) unlink(tmp_file);
        return -1;
    }
    
    if (fd == STDOUT_FILENO)
        return -1;
    
    close(fd);

    db = open_db_writing();
    if (db == -1) {
        (void) unlink(tmp_file);
        return -1;
    }
        

    if (read_db(db, &hash) == -1) {
        (void) close_db(db);
        (void) unlink (tmp_file);
        return -1;
    }   
        
    i = lookup_db(&hash, location);
    if (i != -1) {
        data =  hash.tab.data;
        snprintf(filename, sizeof(filename), "%s%s", SPOOL_DIR, data[i].converted);
        (void) unlink(filename);
    } else {
        if (get_new_filename(location, SPOOL_DIR, buf, sizeof(buf)) == -1) {
            errormsg(0, -1, "can't get new cache filename");
            (void) unlink(tmp_file);
            (void) close_db(db);
            return -1;
        }
        snprintf(filename, sizeof(filename), "%s%s", SPOOL_DIR, buf);
        
        init_entry(&new, type, location, buf, 0);

        if (insert_db(&hash, &new) == -1)
        { 
            (void) unlink (tmp_file);
            (void) close_db(db);
            return -1;
        }
    }

    if (rename(tmp_file, filename) == -1)
    {
            errormsg(0, -1, "can't rename temporary file to %s", filename);
            (void) unlink(tmp_file);
            (void) close_db(db);
            return -1;
    }

    (void) set_modtime(filename, mtime(location));
    
    if ((write_db(db, &hash) == -1)  || (close_db(db) == -1)){
            (void) truncate(filename, 0);
            (void) close_db(db);
            return -1;
    }
            
    return 0;
}/*}}}*/



static int clean(char *type UNUSED, char *location UNUSED) {/*{{{*/
    struct  dochash hash;
    struct  cache_entry *data;
    char    buf[BUF_SIZE];
    char    orig[BUF_SIZE];
    size_t  i, j;
    int     db;

    db = open_db_writing();
    if (db == -1 || read_db(db, &hash) == -1)
        return -1;
        
    data = hash.tab.data;
    j = 0;
    for (i = 0; i < hash.tab.used; ++i) {
        snprintf(orig, sizeof(orig), "%s%s", prefs[data[i].pref], data[i].original);
        snprintf(buf,  sizeof(buf),  "%s%.20s", SPOOL_DIR, data[i].converted);
        if (mtime(orig) == mtime(buf)) 
            data[j++] = data[i];
        else
            (void) unlink(buf);
    }
    hash.tab.used = j;
    
    if (rebuild_db(&hash) == -1 ||
        write_db(db, &hash) == -1 ||
        close_db(db) == -1)
        return -1;

    return 0;
}/*}}}*/



static int open_db_reading(void) {/*{{{*/
    int db;
    int tries = 5;

    db = open(CACHE_DB, O_RDONLY, 0644);
    if (db == -1) {
            if (errno != ENOENT)
                errormsg(0, -1, "can't check cache database");
        return -1;
    }               

    while (--tries && flock(db, LOCK_SH | LOCK_NB) < 0) 
            sleep(1);

    if (!tries)
    {
        (void) close(db);
        errormsg(0, -1, "can't create shared lock on cache database");
        return -1;
    }
        
    return db;
}/*}}}*/


static int open_db_writing(void) {/*{{{*/
    int db;
    int tries = 5;

    db = open(CACHE_DB, O_RDWR | O_CREAT, 0664);
    if (db == -1)
    {
        errormsg(0, -1, "can't update cache database");
        return -1;
    }       
                    
    while (--tries && flock(db, LOCK_EX | LOCK_NB) < 0) 
            sleep(1);

    if (!tries)
    {
        (void) close(db);
        errormsg(0, -1, "can't create exlusive lock on cache database");
        return -1;
    }

    return db;
}/*}}}*/


static int read_db(int db, struct dochash *hash) {/*{{{*/
    struct stat st;
    char *p, *end;
    struct cache_entry *data;

    if (fstat(db, &st) == -1) {
        errormsg(0, -1, "can't find database size");
        return -1;
    }

    p = malloc((size_t)st.st_size);
    if (p == NULL) {
        errormsg(0, -1, "out of memory");
        return -1;
    }
    
    if (read(db, p, (size_t)st.st_size) != st.st_size) {
        errormsg(0, -1, "couldn't read database");
        return -1;
    }
    
    hash->db = p;
    hash->db_size = st.st_size;
    dynarr_init(&hash->tab, sizeof(struct cache_entry));

    end = p + st.st_size;
    while (p < end) {
        if (dynarr_resize(&hash->tab, hash->tab.used + 1) == -1) {
            errormsg(0, -1, "out of memory");
            return -1;
        }
        data = hash->tab.data;
        data += hash->tab.used;
        data->type = *p++;
        data->permanent = *p++;
        data->pref = *p++;
        data->original = p;
        p += strlen(p) + 1;
        data->converted = p;
        p += strlen(p) + 1;
        ++hash->tab.used;
    }

    sort_db(hash);
    
    return 0;
}/*}}}*/


static int compare_entry(const void *a, const void *b) {/*{{{*/
    const struct cache_entry *aa = a;
    const struct cache_entry *bb = b;
    int i;
    
    i = aa->pref - bb->pref;
    if (i == 0)
        i = strcmp(aa->original, bb->original);
    return i;
}/*}}}*/


static void sort_db(struct dochash *hash) {/*{{{*/
    qsort(hash->tab.data, hash->tab.used, sizeof(struct cache_entry),
        compare_entry);
}/*}}}*/


static int lookup_db(struct dochash *hash, char *original) {/*{{{*/
    struct cache_entry dummy, *p, *data;

    init_entry(&dummy, "", original, 0, 0);
    data = hash->tab.data;
    p = bsearch(&dummy, data, hash->tab.used,
        sizeof(struct cache_entry), compare_entry);
    if (p == NULL)
        return -1;
    return p - data;
}/*}}}*/


static int insert_db(struct dochash *hash, struct cache_entry *new) {/*{{{*/
    struct cache_entry *data;
    int i;

    i = lookup_db(hash, new->original);
    if (i == -1) {
        if (dynarr_resize(&hash->tab, hash->tab.used + 1) == -1) {
            errormsg(0, -1, "out of memory");
            return -1;
        }
        i = hash->tab.used;
        ++hash->tab.used;
    }

    data = hash->tab.data;
    data[i] = *new;
    return rebuild_db(hash);
}/*}}}*/


static int rebuild_db(struct dochash *hash) {/*{{{*/
    struct  cache_entry *data;
    size_t  i, n, qsize;
    char    *q, *qq;

    data = hash->tab.data;
    qsize = 0;
    for (i = 0; i < hash->tab.used; ++i)
        qsize += storage_size(data + i);

    q = malloc(qsize + 1);
    if (q == NULL) {
        errormsg(0, -1, "out of memory");
        return -1;
    }
    
    qq = q;
    for (i = 0; i < hash->tab.used; ++i) {
        *qq++ = data[i].type;
        *qq++ = data[i].permanent;
        *qq++ = data[i].pref;
        n = strlen(data[i].original) + 1;
        memcpy(qq, data[i].original, n);
        qq += n;
        n = strlen(data[i].converted) + 1;
        memcpy(qq, data[i].converted, n);
        qq += n;
    }
    
    free(hash->db);
    hash->db = q;
    hash->db_size = qsize;

    return 0;
}/*}}}*/


static int write_db(int db, struct dochash *hash) {/*{{{*/
    if (lseek(db, 0, SEEK_SET) == -1) {
        errormsg(0, -1, "can't update cache database");
        return -1;
    }
    
    if (ftruncate(db, 0) == -1) {
        errormsg(0, -1, "can't truncate cache database");
        return -1;
    }
    
    if (write(db, hash->db, hash->db_size) == -1) {
        errormsg(0, -1, "can't update cache database");
        return -1;
    }
    
    return 0;
}/*}}}*/


static int close_db(int db) {/*{{{*/
    if (flock(db, LOCK_UN) == -1) {
        close(db);
        errormsg(0, -1, "can't unlock cache database");
        return -1;
    }
    if (close(db) == -1) {
        errormsg(0, -1, "error closing database");
        return -1;
    }
    return 0;
}/*}}}*/


static int storage_size(struct cache_entry *p) {/*{{{*/
    return 2 + 1 + strlen(p->original) + 1 + strlen(p->converted) + 1;
}/*}}}*/


static void init_entry(struct cache_entry *p, char *type, char *orig,/*{{{*/
char *conv, int perm)
{
    p->type = type_to_int(type);
    p->pref = find_pref(orig);
    p->original = orig + strlen(prefs[p->pref]);
    p->converted = conv;
    p->permanent = perm;
}/*}}}*/



static int output_file(char *name) {/*{{{*/
    int fd;
    
    fd = open(name, O_RDONLY);
    if (fd == -1) {
        errormsg(0, -1, "can't read file %s", name);
        return -1;
    }
    
    if (copy_fd_to_fd(fd, 1, 1) == -1) {
        (void) close(fd);
        return -1;
    }
    
    close(fd);
    return 0;
}/*}}}*/


static int copy_fd_to_fd(int from, int to_prim, int to_sec) {/*{{{*/
    char    buf[BUF_SIZE];
    ssize_t n;
    ssize_t n_prim = 0;
    ssize_t n_sec  = 0;

    
    if (to_prim == to_sec)
        n_sec = -100; /* *must* be less then 0 */
    
    while ((n = read(from, buf, sizeof(buf))) > 0) {
        if (n_prim >= 0)
            n_prim = write(to_prim, buf, n) - n;
        if (n_sec >= 0)
            n_sec = write(to_sec, buf, n) - n;
        if (n_prim < 0 && n_sec < 0)
        {   
            errormsg(0, -1, "error writing");
            return -1;
        }
    }
    
    if (n_prim < 0)
    {   
        errormsg(0, -1, "error writing");
        return -1;
    }
    
    if (n < 0) {
        errormsg(0, -1, "error reading");
        return -1;
    }
    
    return 0;
}/*}}}*/


static int create_tmp_file(char * dir, char * full, size_t size) {/*{{{*/
    int fd;

    snprintf(full, size,  "%s/tmp%d.%ld", dir, (int) getpid(), (long int)time(NULL));
    fd = open(full, O_CREAT | O_EXCL | O_RDWR, 0664);
    
    return fd;
}/*}}}*/

static int get_new_filename(char * location, char * dir, char * buf, size_t buf_size)/*{{{*/
{
        char * tmp;
        char full[BUF_SIZE];
        int  i = 0;
        char c = '_';

        tmp = basename(location);
        if (tmp != NULL) 
                c = *tmp;
        
        snprintf(full, sizeof(full), "%s/%c", dir, c);
        if (mkdir(full, 0775) == -1 && errno != EEXIST)
        {
                errormsg(0, -1, "can't create directory %d", full);
                return -1;
        }
        for (i = 0; i < INT_MAX; i++)
        {
                snprintf(full, sizeof(full), "%s/%c/%d", dir, c, i);
                errno = 0;
                if (access(full, F_OK) == -1 && errno == ENOENT)
                {
                        snprintf(buf, buf_size, "%c/%d", c, i);
                        return 0;
                }
                else if (errno)
                        return -1;
        }
        return -1;
}/*}}}*/
        

static const struct {/*typetab[] {{{*/
    char *str;
    int ch;
} typetab[] = {
    { "man", 'm' },
    { "runman", 'r' },
    { "info", 'i' },
    { "text", 't' },
    { "dir", 'd' },
    { "html", 'h' },
    { "text/html", 'h' },
    { "file", 'f' },
    { NULL, '?' },
};/*}}}*/


static int type_to_int(char *str) {/*{{{*/
    int i;

    for (i = 0; typetab[i].str != NULL; ++i)
        if (strcmp(typetab[i].str, str) == 0)
            break;
    return typetab[i].ch;
}/*}}}*/


static char *int_to_type(int ch) {/*{{{*/
    int i;

    for (i = 0; typetab[i].str != NULL; ++i)
        if (typetab[i].ch == ch)
            return typetab[i].str;
    return "unknown";
}/*}}}*/


static time_t mtime(char *name) {/*{{{*/
    struct stat st;
    if (stat(name, &st) == -1)
        return 0;
    if (!st.st_size) /* file is empty ??? */
        return 0;
    return st.st_mtime;
}/*}}}*/


static int find_pref(const char *s) {/*{{{*/
    int i;
    for (i = 1; i < nprefs; ++i)
        if (strncmp(s, prefs[i], strlen(prefs[i])) == 0)
            return i;
    return 0;
}/*}}}*/


static int set_modtime(const char * file, time_t modtime) {/*{{{*/
    struct utimbuf ut;

    ut.modtime = modtime;
    ut.actime  = time(NULL);

    return utime(file, &ut);            
}/*}}}*/
