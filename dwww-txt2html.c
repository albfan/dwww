/* vim:ts=4
 * dwww-txt2html.c
 * "@(#)dwww:$Id: dwww-txt2html.c,v 1.20 2006-05-07 18:30:06 robert Exp $"
 *
 * A very simple converter from formatted manual pages to HTML. Handles
  * backspace characters. Converts `<', `>', and `&' properly. Does _NOT_ add
 * <head> and <body> tags -- caller must do that. _Does_ add <pre>
 * and </pre> tags. Converts manual page references to anchors.
 *
 * Bug: because of the static line length limit, anchor generation
 * can fail if the manual page reference happens to fall on buffer
 * limit. This is rather unlikely, though, if we make the buffer big
 * enough. (I'm lazy.)
 *
 * Bug: if the manual page reference is divided on more than two lines, only
 * the part on the second (two) line(s) is recognized.
 *
 * Part of dwww.
 * Lars Wirzenius.
 *
 * Robert Luberda, Jan 2002: also generates links for  http, ftp, mail, closes: and /usr/ dirs
 */


#include <assert.h>
#include <limits.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <publib.h>
#include <values.h>

#include "utils.h"

#define UNDERLINE	(0x01 << CHAR_BIT)
#define BOLD		(0x02 << CHAR_BIT)

#define FLAGMASK	((~0) << CHAR_BIT)
#define CHARMASK	(~FLAGMASK)

#define FL_MANY		0x01
#define FL_NOCASE	0x02


#define BUF_SIZE 512

#define ispagename(c)	((c) == '_' || (c) == '-' || (c) == ':' || (c) == '+' \
			  || (c) == '.' || isalnum(c))
#define isdirname(c)	((c) == '_' || (c) == '-' || (c) == ':' || (c) == '+' \
			  || (c) == '.' || (c) == '/' || isalnum(c))
#define ismailname(c)	((c) == '_' || (c) == '-' || (c) == '.' || (c) == '+' || isalnum(c))
#define iswwwname(c)	((c) == '_' || (c) == '-' || (c) == ':' \
			  || (c) == '.' || (c) == '/' || isalnum(c) || (c) == '?' || (c) == '&' \
			  || (c) == '%' || (c) == '=' || (c) == '#' || (c) == '~' \
			  || (c) == '+' || (c) == '@')
#define stoppoint(c)  ((c) == '.' || (c) == ',' || (c) == '?' || (c) == ')')

static int manual_page;	/* are we doing a manual page? */

/* Functions */
static int flush(int *, int *, int *);
static int add(int *, int *, int *, int);
static int txt2html(FILE *, char *, void *); 
static int find_uris(int *, int);

static int check_dir_uri(char * buf, char * url, int uri_no, int loc, int * begin, int * end);
static int check_www_uri(char * buf, char * url, int uri_no, int loc, int * begin, int * end);
static int check_man_uri(char * buf, char * url, int uri_no, int loc, int * begin, int * end);
static int check_bug_uri(char * buf, char * url, int uri_no, int loc, int * begin, int * end);
static int check_mail_uri(char * buf, char * url, int uri_no, int loc, int * begin, int * end);
static int check_cve_uri(char * buf, char * url, int uri_no, int loc, int * begin, int * end);

		
enum { U_HTTP, U_FTP, U_HTTP2, U_FTP2, U_MAILTO, U_MAN, U_DIR, U_BUG, U_CVE, U_CAN};
static const struct { 
		int type;
		char * pattern;
		char * prefix;
		int flags;
		int (* srchf)(char *, char *, int, int, int *, int *);
} uris[] = {
		{	U_HTTP, 
			"http",  /* http://XXXXX https://XXXXXX */ 
			"",
			0,
		   	check_www_uri
		},
		{	U_FTP,
			"ftp", /*  ftp://XXXXXX  ftps://XXXXXX ftp.XXXXXX */
			"",
			0,
		   	check_www_uri
		},
		{	U_HTTP2,
			"www.", /* www.XXXXX */
			"",
			0,
		   	check_www_uri
		},
		{	U_DIR,
			"/usr/",
			"/cgi-bin/dwww/",
			0,
		   	check_dir_uri
		},
		{	U_BUG,
			"closes:",
			"http://bugs.debian.org/",
			FL_MANY | FL_NOCASE,
		   	check_bug_uri
		},
		{	U_MAN,
			"(",
			"/cgi-bin/dwww?type=runman&amp;location=",
			0,
		   	check_man_uri
		},			
		{	U_MAILTO,
			"@",
			"mailto:",
			0,
		   	check_mail_uri
		},
		{	U_CVE, /* CVE-dddd-dddd*/
			"cve-",
			"http://cve.mitre.org/cgi-bin/cvename.cgi?name=",
			0,
		   	check_cve_uri
		},
		{	U_CAN, /* CAN-dddd-dddd */
			"can-",
			"http://cve.mitre.org/cgi-bin/cvename.cgi?name=",
			0,
		   	check_cve_uri
		}		
};

static const int nuris = (int) sizeof(uris)/sizeof(*uris);

static struct {
		int begin;
		int end;
		char * prefix;
		char url[BUF_SIZE];
} anchors[128];

static int flush(int *buf, int *i, int *n) {
	int j, m, nmen;
	int c, prev, this;
	
	nmen = find_uris(buf, *n);

	prev = 0;
	m = 0;
	for (j = 0; j < *n; ++j) {
		c = buf[j] & CHARMASK;
		if (c == '\n' || j + 1 == *n)
			this = 0;
		else
			this = buf[j] & FLAGMASK;
		if (this != prev) {
			if ((this & UNDERLINE) != 0 && (this & BOLD) != 0) {
				if (prev == UNDERLINE)
					this = BOLD;
				else
					this = UNDERLINE;
			}
			if (prev == UNDERLINE)
				(void) printf("</em>");
			else if (prev == BOLD)
				(void) printf("</strong>");
		}
		if (m < nmen && j == anchors[m].end + 1) {
			(void) printf("</A>");
			++m;
		}
		if (m < nmen && j == anchors[m].begin) {
			printf("<A HREF=\"%s%s\">", anchors[m].prefix, anchors[m].url);
		}
		if (this != prev)
		{	
			if (this == UNDERLINE)
				(void) printf("<em>");
			else if (this == BOLD)
				(void) printf("<strong>");
		}
		prev = this;
		switch (c) {
		case '<':
			(void) printf("&lt;"); break;
		case '>':
			(void) printf("&gt;"); break;
		case '&':	
			(void) printf("&amp;"); break;
		case 173:	
			(void) printf("&shy;"); break;
		case 180:	
			(void) printf("&acute;"); break;
		case 183:	
			(void) printf("&middot;"); break;
		case 215:	
			(void) printf("&times;"); break;
		case '\n':
			(void) printf("\n"); break;
		default:
			(void) printf("%c", c); break;
		}
	}
	
	if (m < nmen && j == anchors[m].end + 1) {
		(void) printf("</A>");
		++m;
	}

	*i = *n = 0;
	if (ferror(stdout))
		return -1;
	return 0;
}

static int add(int *buf, int *i, int *n, int c) {
	if (*i == *n && *n >= BUF_SIZE - 1)
		if (flush(buf, i, n) == -1)
			return -1;
	assert(*i < *n || *n < BUF_SIZE);
	if (*i < *n) {
		if (c == '_')
			buf[*i] |= UNDERLINE;
		else if (buf[*i] == '_') {
			buf[*i] |= UNDERLINE;
			buf[*i] = (buf[*i] & FLAGMASK) | (c & CHARMASK);
		} else if (c == buf[*i])
			buf[*i] |= BOLD;
		else
			buf[*i] = c & CHARMASK;
	} else {
		buf[*i] = c & CHARMASK;
		++*n;
	}
	++*i;
	return 0;
}

static int txt2html(FILE *f, char *filename, void *dummy) {
	int buf[BUF_SIZE];
	int c, i, j, n, late_flush;
	int prev_i;
	char prev;

	(void) printf("<pre%s>", manual_page? " class=\"man\"" : "" );

	prev   = 0;
	prev_i = -1;
	buf[0] = 0;
	i = n = late_flush = 0;
	
	while ((c = getc(f)) != EOF) {
		switch (c) {
		case '\n':
			if (manual_page && !i && !prev_i)
					break;
			prev_i = i;
			if (add(buf, &i, &n, '\n') == -1)
				return -1;
			late_flush = manual_page && (prev == '-');
			if (!late_flush && flush(buf, &i, &n) == -1)
				return -1;
			break;
		case '\b':
			if (i > 0)
				--i;
			break;
		case '\t':
			for (j = 8-(i%8); j > 0; --j)
				if (add(buf, &i, &n, ' ') == -1)
					return -1;
			break;
		case ' ':	
			if (add(buf, &i, &n, ' ') == -1)
				return -1;
			if (late_flush && prev != ' ' && prev != '\n' && prev != '\t')
			{
					late_flush = 0;
					if (flush(buf, &i, &n) == -1)
						return -1;
			}
			break;
		case 0255: /* continuation hyphen */
			c = '-';
			/* !!! NO BREAK !!! */	
		default:
			if (add(buf, &i, &n, c) == -1)
				return -1;
			break;
		}
		prev = c;
	}

	/* flush buffer in case of no ending "\n" in file */
	if (i > 0 && flush(buf, &i, &n) == -1)
		return -1;

	(void) printf("</pre>\n");

	if (ferror(stdout))
		return -1;
	return 0;
}

int main(int argc, char **argv) {
	if (argc == 1 || strcmp(argv[1], "--man") != 0)
		manual_page = 0;
	else {
		manual_page = 1;
		--argc;
		++argv;
	}

	dwww_initialize("dwww-txt2html");
	if (main_filter(argc-1, argv+1, txt2html, NULL) == -1)
		return EXIT_FAILURE;
	return 0;
}



static char * urlenc(char c)
{
		static char buf[10];

		if ((isalnum(c)) || c == '/' || c == '.')
		{
				buf[0] = c;
				buf[1] = 0;
		}
		else
				sprintf(buf, "%%%02x", c);
		return buf;
}


static int find_uris(int * buf, int last)
{
	signed int  minp[20];
	char tmpbuf[BUF_SIZE];
	char lowbuf[BUF_SIZE];
	char * tmp;
	int  current_pos;
	int  i, j,  min;
	int  furis;
	int  begin, end;
	int  skip_pos;
	int  skip_cnt;
	int  prev_minp;

	
	skip_pos = manual_page ? -1 : MAXINT;
	skip_cnt = 0;
	for (i=0, j=0; i<last; i++)
	{
			char c;
			
			c = buf[i] & CHARMASK; 
			if (skip_pos < 0)
			{
				if (c == '\n' && j > 0 && tmpbuf[j-1] == '-')
				{
					skip_pos = --j;
					skip_cnt = 2; /* "-\n" */
					continue;
				}
			} 
			else if ( c == ' ' && j == skip_pos)
			{
				skip_cnt++;
				continue;
			}
							
			tmpbuf[j] = c;
			lowbuf[j] = tolower(tmpbuf[j]);
			j++;
	}

	assert (j < (int) sizeof(tmpbuf));	
	
	tmpbuf[j] = 0;
	lowbuf[j] = 0;


	for(i=0; i<nuris; minp[i++] = -1)
		;
	
	min 	    = -1;
	current_pos = 0;
	furis       = 0;
	prev_minp   = MAXINT;
		
	while (current_pos < last) 
	{

		for(i=0; i<nuris; i++)
			if (minp[i] <= prev_minp)
			{
				tmp = (i == min) ? lowbuf + prev_minp + 1 : lowbuf + current_pos;	
				//printf("\nXXXX: %s\n", tmp);
				if ((tmp = strstr(tmp, uris[i].pattern)))
					minp[i] = tmp - lowbuf;
				else 
					minp[i] = MAXINT;
			}	
	
		/* find new minimum */
		for (i=0, min=0; i < nuris; i++)
			if (minp[i] < minp[min])
				min = i;
		prev_minp = minp[min];
					
		if (minp[min]  == MAXINT)
				return furis;

		if (uris[min].flags & FL_NOCASE)
				tmp = lowbuf;
		else
				tmp = tmpbuf;
	
		if (uris[min].srchf(tmp + current_pos, anchors[furis].url, min, minp[min] - current_pos, &begin, &end))
		{
				do 
				{
					int a,b;
					
					a = current_pos + begin;
					b = current_pos + end;

					if (a < skip_pos && b > skip_pos)
					{
						/* URI is divided into 2 lines; we need to split it
						 * into 2 uris
						 */
						anchors[furis].begin  = a;
						anchors[furis].end    = skip_pos;
						anchors[furis].prefix = uris[min].prefix;

						furis++;
						anchors[furis].begin  = skip_pos + skip_cnt;
						anchors[furis].end    = b + skip_cnt;
						anchors[furis].prefix = uris[min].prefix;
						strcpy(anchors[furis].url, anchors[furis-1].url);
					}
					else
					{	
						# define do_skip(x) ((x) < skip_pos) ? (x) : ((x) + skip_cnt)
						anchors[furis].begin  = do_skip(a);
						anchors[furis].end    = do_skip(b);
						anchors[furis].prefix = uris[min].prefix;
					}
					
					furis++;
					current_pos += end + 1;
					
				} while ((uris[min].flags & FL_MANY) && uris[min].srchf(tmp + current_pos, anchors[furis].url, min, 0, &begin, &end));
		}	
	}
	
	return furis;
	
}


static const struct {
	   char * dir;
	   size_t  size;
}  dirs[] = {
	{ "/usr/share/doc/",			sizeof("/usr/share/doc/") - 1 				},
	{ "/usr/share/man/",			sizeof("/usr/share/man/") - 1 				},
	{ "/usr/share/common-licenses/",sizeof("/usr/share/common-licenses/") - 1	},
	{ "/usr/doc/",					sizeof("/usr/doc/") - 1 					},
	{ "/usr/man/",					sizeof("/usr/man/") - 1						},
	{ "/usr/X11R6/man/",			sizeof("/usr/X11R6/man/") - 1 				},
	{ "/usr/local/man/",			sizeof("/usr/local/man/") - 1 				},
	{ "/usr/local/doc/",			sizeof("/usr/local/doc/") - 1 				}
};

static const int ndirs = (int) sizeof(dirs)/sizeof(*dirs);

static int check_dir_uri(char * buf, char * url, int uri_no, int loc, int * begin, int * end)
{
		int i;

		url[0] = 0;

		if (loc > 0 && isalnum(buf[loc - 1]))
				return 0;
		
		for (i=0; i < ndirs; i++)
		{
				if (!strncmp(dirs[i].dir, buf + loc, dirs[i].size)) 
				{
						*begin = loc;
						strcpy(url, dirs[i].dir);
						*end = loc + dirs[i].size;
						while (isdirname(buf[*end]))
							(*end)++;
						(*end)--;
						if (stoppoint(buf[*end]))
							(*end)--;
						
						for (i = *begin + dirs[i].size; i <= *end; i++)
							strcat(url, urlenc(buf[i]));
						
						return 1;
				}
		}
		return 0;
}

static int check_www_uri(char * buf, char * url, int uri_no, int loc, int * begin, int * end)
{
		
		int i;
		char * tmp;
		char * prefix;
		int ftp_colon = 0; /* should we skip colon in addreses like ftp.X.com:/dir/ */
		
		
		if (loc > 0 && isalnum(buf[loc - 1]))
				return 0;

		*begin = loc;
		*end   = loc + strlen(uris[uri_no].pattern);
		tmp    = buf + *end;
		prefix = "";
		
		switch (uris[uri_no].type)
		{
				case U_HTTP2: /* "www." */
						prefix = "http://";
						break;
				case U_FTP:   /* "ftp" */
						if (*tmp == '.') 
						{
								tmp++;
								ftp_colon = 1;
								prefix = "ftp://";
								break;
						}
						/* !!! NOBREAK !!! */
				case U_HTTP:   /* "http" */
						if (*tmp == 's')   tmp++;
						if (*tmp++ != ':') return 0;
						if (*tmp++ != '/') return 0;
						if (*tmp++ != '/') return 0;
						break;
				default:
						return 0;	
		}
		
		while (iswwwname(*tmp))
				tmp++;
		tmp--;
		if (stoppoint(*tmp))
			tmp--;

		if ((tmp - buf) - *end < 3)
				return 0;
		*end = tmp - buf;	

		
		i = (int) strlen(prefix);

		if (*end - *begin >= BUF_SIZE -i)
				return 0;

		strcpy(url, prefix);
		tmp = url + i;
		
		for (i = *begin; i <= *end; i++)
		{
			if (ftp_colon && buf[i] == '/' && *(tmp-1) == ':')	
			{
					ftp_colon = 0;
					tmp--;
			}		
			*(tmp++) = buf[i];
		}	

		*tmp = '\0';
		
		return 1;
}


static int check_man_uri(char * buf, char * url, int uri_no, int loc, int * begin, int * end)
{
		int i;
		char section[256];
		char * tmp;
		char * tmp_sec;
	
		url[0] = '\0';
		
		*begin = loc;
		*end   = loc;
		tmp = buf + loc;
		if ((*tmp) != '(' || loc == 0) 
				return 0;
		tmp++;
		if (!isdigit(*tmp) || *tmp == '0')
				return 0;
		tmp_sec = &section[0];
		while (isalnum(*tmp))
			*tmp_sec++ = *tmp++;
		if (*tmp != ')')
				return 0;
		*tmp_sec = '\0';
		*end = tmp - buf;
		
				
		tmp = buf + loc - 1;
		while (buf <= tmp && ispagename(*tmp))
				tmp--;
		*begin = tmp - buf + 1;


		for (i = *begin; i < loc; i++)
				strcat(url, urlenc(buf[i]));
		sprintf(url, "%s/%s", url, section);
								

		return (loc - *begin) && ((*end - loc) > 1) ;
}

static int check_bug_uri(char * buf, char * url, int uri_no, int loc, int * begin, int * end)
{
		
		int i;
		
		url[0] = '\0';
		
		if (!strncmp(buf + loc, "closes:", sizeof("closes:") - 1))
		{
			if (loc > 0 && isalnum(buf[loc-1]))
					return 0;
			loc += sizeof("closes:") - 1;
		}

		while (isspace(buf[loc]) || buf[loc]==',')
				loc++;
		
		*begin = loc;
		if (!strncmp(buf + loc, "bug#", sizeof("bug#") - 1))
				loc += sizeof("bug#") -1;
		else if (buf[loc] == '#')
				loc++;
		else 
				return 0;
		
		while (isspace(buf[loc]))
				loc++;

		i = 0;
		while (isdigit(buf[loc]))
			url[i++] = buf[loc++];
		url[i] = 0;

		if (i && !isalnum(buf[loc]))
		{
			*end = loc - 1;
			return 1;
		}

		return 0;	
}

static int check_mail_uri(char * buf, char * url, int uri_no, int loc, int * begin, int * end)
{
		int i;
		char * tmp;
		char * prevdot = NULL, * lastdot = NULL;

		if (buf[loc] != '@' || loc == 0) 
				return 0;

		tmp = buf + loc - 1;

		while (buf <= tmp && ismailname(*tmp))
				tmp--;
		*begin = tmp - buf + 1;
		
		if (*begin == loc)
			return 0;	
		
		tmp = buf + loc + 1;
		if (stoppoint(*tmp))
				return 0;
		while (ismailname(*tmp))
		{
				if (*tmp == '.') { 
						if (lastdot == tmp - 1)
								return 0;
						prevdot = lastdot;
						lastdot = tmp;
				}		
				tmp++;
		}	
		*end = tmp - buf - 1;
		
		if (*end == loc)
			return 0;
		

		if (stoppoint(buf[*begin]))
			(*begin)++;
						
		if (stoppoint(buf[*end]))
			(*end)--;

		tmp = buf + *end;
		if (lastdot > tmp)
				lastdot = prevdot;

		if (!lastdot || tmp - lastdot < 2 || tmp - lastdot > 5)
			return 0;

		for (i = *begin, tmp = url; i < BUF_SIZE && i <= *end;)
			*tmp++ = buf[i++];

		*tmp = 0;
		
		return 1;
		
}

static int check_cve_uri(char * buf, char * url, int uri_no, int loc, int * begin, int * end)
{
		
		int i, j;
		char * tmp;
		
		url[0] = '\0';
		
		if (!strncmp(buf + loc, "CVE-", sizeof("CVE-") - 1) ||
			!strncmp(buf + loc, "CAN-", sizeof("CAN-") - 1))
		{
			if (loc > 0 && isalnum(buf[loc-1]))
					return 0;
		    *begin = loc;
			loc += sizeof("CVE-") - 1;
		} else 
				return 0;

		/* check if buf[loc] =~ /\d{4}-\d{4}/ */
		for (j = 0; j < 2; j++) {
			for (i = 0; i < 4; i++, loc++) {
					if (!(isdigit(buf[loc])))
							return 0;
			}
			if ((j == 0 && buf[loc++] != '-')
			   || (j == 1 && isalnum(buf[loc]) ) )
				return 0;
		}
		
		*end = loc - 1;

		for (i = *begin, tmp = url; i < BUF_SIZE && i <= *end;)
			*tmp++ = buf[i++];
		*tmp = 0;
		

		return 1;
}

