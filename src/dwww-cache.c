/* vim:ft=c:cindent:ts=4:sts=4:sw=4:et:fdm=marker
 *
 * File:    dwww-cache.c
 * Purpose: Manage the dwww cache of converted documents.
 * Author:  Lars Wirzenius
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
 *      strings ("/usr/share/man/" for example).  We'll see.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <publib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <utime.h>
#include <sys/file.h>
#include <libgen.h>
#include <stdbool.h>

#include "utils.h"

#define BUF_SIZE 1024

/*
 * Filename prefixes (for simple compression). At most 256 elements!
 */
static const char *prefs[] = {/* prefs[] {{{*/
    "",     /* MUST BE EMPTY STRING */
    "/usr/lib/",
    "/usr/share/doc/",
    "/usr/share/info/",
    "/usr/share/common-licenses/",
    "/usr/share/man/man1/",
    "/usr/share/man/man2/",
    "/usr/share/man/man3/",
    "/usr/share/man/man4/",
    "/usr/share/man/man5/",
    "/usr/share/man/man6/",
    "/usr/share/man/man7/",
    "/usr/share/man/man8/",
    "/usr/share/",
    "/usr/local/share/doc/",
    "/usr/local/share/info/",
    "/usr/local/share/man/man1/",
    "/usr/local/share/man/man2/",
    "/usr/local/share/man/man3/",
    "/usr/local/share/man/man4/",
    "/usr/local/share/man/man5/",
    "/usr/local/share/man/man6/",
    "/usr/local/share/man/man7/",
    "/usr/local/share/man/man8/",
    "/usr/local/share/"
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
static bool store(char *, char *);
static bool lookup(char *, char *);
static bool list(char *, char *);
static bool list_all(char *, char *);
static bool clean(char *, char *);

static bool help(char *type UNUSED, char *location UNUSED) { /*{{{*/
    fprintf(stdout, "Usage: %s [--lookup|--store|--list] type location\n"
                    "       %s --list-all|--clean|--help\n",
                    get_progname(), get_progname() );
    exit(0);
}   /*}}}*/

static int open_db_reading(void);
static int open_db_writing(void);
static bool read_db(int, struct dochash *);
static int compare_entry(const void *, const void *);
static void sort_db(struct dochash *);
static int lookup_db(struct dochash *, char *);
static int insert_db(struct dochash *, struct cache_entry *);
static int write_db(int, struct dochash *);
static bool close_db(const int, const bool);
static size_t storage_size(struct cache_entry *);
static int output_file(char *);
static void init_entry(struct cache_entry *, char *, char *, char *, char);
static int create_tmp_file(char *, char *, size_t);
static int get_new_filename(char *, char *, char *, size_t);
static int copy_fd_to_fd(int, int, int);
static char *int_to_type(int);
static char type_to_int(char *);
static int rebuild_db(struct dochash *);
static bool check_mtimes(const char * const origname,
                        const char * const cachedname);
static int find_pref(const char *);
static int set_modtime_from_orig(const char * const , const char * const);

int main(int argc, char **argv) {/*{{{*/
    static struct {
        int action;
        int need_args;
        bool (*func)(char *, char *);
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

    if (!actions[i].func(argv[first_nonopt], argv[first_nonopt+1]))
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



static bool list(char *type UNUSED, char *location) {/*{{{*/
    struct dochash hash;
    struct cache_entry *data;
    int i, db;
    char buf[BUF_SIZE];
    char orig[BUF_SIZE];

    db = open_db_reading();
    if (db == -1)
        return false;
    if (!read_db(db, &hash))
        return close_db(db, false);
    i = lookup_db(&hash, location);
    if (i == -1)
        return close_db(db, false);

    data = hash.tab.data;
    snprintf(orig, sizeof(orig), "%s%s", prefs[data[i].pref], data[i].original);
    snprintf(buf,  sizeof(buf),  "%s%.20s", SPOOL_DIR, data[i].converted);
    if (!check_mtimes(orig, buf)) {
        return close_db(db, false);
    }
    printf("%s %s %s %s\n", int_to_type(data[i].type),
        orig, data[i].converted,
        data[i].permanent ? "y" : "n");

    return close_db(db, true);
}/*}}}*/



static bool list_all(char *type UNUSED, char *location UNUSED) {/*{{{*/
    struct dochash hash;
    struct cache_entry *data;
    size_t i;
    int    db;
    char buf[BUF_SIZE];
    char orig[BUF_SIZE];

    db = open_db_reading();
    if (db == -1)
        return false;
    if (!read_db(db, &hash))
        return close_db(db, false);
    data = hash.tab.data;

    for (i = 0; i < hash.tab.used; ++i) {
        snprintf(orig, sizeof(orig), "%s%s", prefs[data[i].pref], data[i].original);
        snprintf(buf,  sizeof(buf),  "%s%.20s", SPOOL_DIR, data[i].converted);
        if (check_mtimes(orig, buf))
            printf("%s %s %s %s\n", int_to_type(data[i].type),
                orig, data[i].converted,
                data[i].permanent ? "y" : "n");
    }

    return close_db(db, true);
}/*}}}*/



static bool lookup(char *type UNUSED, char *location) {/*{{{*/
    struct dochash hash;
    struct cache_entry *data;
    int i, db;
    char buf[BUF_SIZE];
    char orig[BUF_SIZE];

    db = open_db_reading();
    if (db == -1)
        return false;
    if (!read_db(db, &hash))
        return close_db(db, false);
    i = lookup_db(&hash, location);
    if (i == -1)
        return close_db(db, false);

    data = hash.tab.data;
    snprintf(orig, sizeof(orig), "%s%s", prefs[data[i].pref], data[i].original);
    snprintf(buf,  sizeof(buf),  "%s%.20s", SPOOL_DIR, data[i].converted);
    if (!check_mtimes(orig, buf)) {
        return close_db(db, false);
    }
    if (output_file(buf) == -1) {
        return close_db(db, false);
    }
    return close_db(db, true);
}/*}}}*/


static bool internal_store(char * type,
                           char * location,
                           char * tmp_file,
                           const int fd,
                           const int db) {/*{{{*/
    int i;
    struct dochash hash;
    struct cache_entry new, *data;
    char filename[BUF_SIZE];
    char buf[BUF_SIZE];


    if (copy_fd_to_fd(STDIN_FILENO, fd, STDOUT_FILENO) == -1) {
        errormsg(0, -1, "can't copy stdin to %s", *tmp_file ? tmp_file : "stdout");
        return false;
    }

    if (!*tmp_file)
        return false;


    if (!read_db(db, &hash)) {
        return false;
    }

    i = lookup_db(&hash, location);
    if (i != -1) {
        data =  hash.tab.data;
        snprintf(filename, sizeof(filename), "%s%s", SPOOL_DIR, data[i].converted);
    } else {
        if (get_new_filename(location, SPOOL_DIR, buf, sizeof(buf)) == -1) {
            errormsg(0, -1, "can't get new cache filename");
            return false;
        }
        snprintf(filename, sizeof(filename), "%s%s", SPOOL_DIR, buf);

        init_entry(&new, type, location, buf, 0);

        if (insert_db(&hash, &new) == -1)
        {
            return false;
        }
    }

    if (rename(tmp_file, filename) == -1)
    {
        errormsg(0, -1, "can't rename temporary file to %s", filename);
        return false;
    }
    *tmp_file = 0; // tmp_file no longer exists...

    if (set_modtime_from_orig(filename, location) < 0
        || write_db(db, &hash) == -1)
    {
        (void) unlink(filename);
        return false;
    }

    return true;
}/*}}}*/


static bool store(char *type, char *location) {/*{{{*/
    int  db, fd;
    bool retval;
    char tmp_file[BUF_SIZE];

    fd = -1;

    db = open_db_writing();
    if (db >= 0)
    {
        fd = create_tmp_file(SPOOL_DIR, tmp_file, sizeof(tmp_file));
        if (fd == -1) {
            errormsg(0, -1, "can't create temporary cache file");
        }
    }

    if (fd < 0)
    {
        /* failed to open database or create temporary file, thus
           output to stdout only & don't cache */
        *tmp_file = 0;
        fd = STDOUT_FILENO;
    }

    retval = internal_store(type,
                            location,
                            tmp_file,
                            fd,
                            db);

    if (*tmp_file && unlink(tmp_file) < 0)
    {
        errormsg(0, -1, "can't remove temporary file %s", tmp_file);
        retval = false;
    }

    if (fd != STDOUT_FILENO && close(fd) < 0)
    {
        errormsg(0, -1, "can't close file");
        retval = false;
    }

    if (db >= 0)
        return close_db(db, retval);

    return retval;
}/*}}}*/



static bool clean(char *type UNUSED, char *location UNUSED) {/*{{{*/
    struct  dochash hash;
    struct  cache_entry *data;
    char    buf[BUF_SIZE];
    char    orig[BUF_SIZE];
    size_t  i, j;
    int     db;

    db = open_db_writing();
    if (db == -1 || !read_db(db, &hash))
        return false;

    data = hash.tab.data;
    j = 0;
    for (i = 0; i < hash.tab.used; ++i) {
        snprintf(orig, sizeof(orig), "%s%s", prefs[data[i].pref], data[i].original);
        snprintf(buf,  sizeof(buf),  "%s%.20s", SPOOL_DIR, data[i].converted);
        if (check_mtimes(orig, buf))
            data[j++] = data[i];
        else
            (void) unlink(buf);
    }
    hash.tab.used = j;

    if (!rebuild_db(&hash)   ||
        !write_db(db, &hash))
        return close_db(db, false);

    return close_db(db, true);
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


static bool read_db(int db, struct dochash *hash) {/*{{{*/
    struct stat st;
    char *p, *end;
    struct cache_entry *data;

    if (fstat(db, &st) == -1) {
        errormsg(0, -1, "can't find database size");
        return false;
    }

    p = malloc((size_t)st.st_size);
    if (p == NULL) {
        errormsg(0, -1, "out of memory");
        return false;
    }

    if (read(db, p, (size_t)st.st_size) != st.st_size) {
        free(p);
        errormsg(0, -1, "couldn't read database");
        return false;
    }

    hash->db = p;
    hash->db_size = (size_t) st.st_size;
    dynarr_init(&hash->tab, sizeof(struct cache_entry));

    end = p + st.st_size;
    while (p < end) {
        if (dynarr_resize(&hash->tab, hash->tab.used + 1) == -1) {
            errormsg(0, -1, "out of memory");
            return false;
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

    return true;
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
        i = (int) hash->tab.used;
        ++hash->tab.used;
    }

    data = hash->tab.data;
    data[i] = *new;
    return rebuild_db(hash);
}/*}}}*/


static int rebuild_db(struct dochash *hash) {/*{{{*/
    struct  cache_entry *data;
    size_t  i, n;
    size_t   qsize;
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
        *qq++ = (char)data[i].pref;
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


static bool close_db(int db, const bool retVal) {/*{{{*/
    if (flock(db, LOCK_UN) == -1) {
        close(db);
        errormsg(0, -1, "can't unlock cache database");
        return false;
    }
    if (close(db) == -1) {
        errormsg(0, -1, "error closing database");
        return false;
    }
    return retVal;
}/*}}}*/


static size_t storage_size(struct cache_entry *p) {/*{{{*/
    return 2 + 1 + strlen(p->original) + 1 + strlen(p->converted) + 1;
}/*}}}*/


static void init_entry(struct cache_entry *p, char *type, char *orig,/*{{{*/
char *conv, char perm)
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
            n_prim = write(to_prim, buf, (size_t)n) - n;
        if (n_sec >= 0)
            n_sec = write(to_sec, buf, (size_t)n) - n;
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
    char ch;
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


static char type_to_int(char *str) {/*{{{*/
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


static time_t mtime(const char * const name) {/*{{{*/
    struct stat st;
    if (stat(name, &st) == -1)
        return 0;
    if (!st.st_size) /* file is empty, possibly truncated before */
        return 0;
    return st.st_mtime;
}/*}}}*/

static bool check_mtimes(const char * const origname,
                        const char * const cachedname) { /*{{{*/

    const time_t cachedmtime = mtime(cachedname);
    if (!cachedmtime)
    {
      /* cached file does not exist or is empty  (or some bad gay intentionally set its mtime to 0...) */
        return false;
    }

    if (mtime(origname) == cachedmtime)
        return true;

    /* Don't care if truncate fails or not... It could fail mostly because of EPERM, but it's OK
       Normally I would write
          (void) truncate(cachedname, 0);
       but compilers tries to be too smart, see
       http://launchpadlibrarian.net/28344578/buildlog_ubuntu-karmic-amd64.dwww_1.11.1ubuntu1_FULLYBUILT.txt.gz
   */
    if (truncate(cachedname, 0))
        return false;
    return false;
}/*}}}*/



static int find_pref(const char *s) {/*{{{*/
    int i;
    for (i = 1; i < nprefs; ++i)
        if (strncmp(s, prefs[i], strlen(prefs[i])) == 0)
            return i;
    return 0;
}/*}}}*/


static int set_modtime_from_orig(const char * const cachedname,
                                 const char * const origname) {/*{{{*/
    struct utimbuf ut;

    ut.modtime = mtime(origname);
    if (!ut.modtime)
        return -1;
    ut.actime  = time(NULL);

    return utime(cachedname, &ut);
}/*}}}*/
