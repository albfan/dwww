/* vim:ts=4
 * utils.c
 * "@(#)dwww:$Id: utils.c,v 1.3 2002-11-24 23:17:22 robert Exp $"
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <syslog.h>
#include <publib.h>
#include <stdlib.h>

#include "utils.h"

const char * version = VERSION;

void dwww_initialize(char * progname)
{
	int fd;

	set_progname(progname, "dwww");

	umask((077 & umask(022)) | 022);
	

	fd = open("/dev/null", O_RDONLY, 0644);
	if (fd < 0)
	{
		openlog(get_progname(), LOG_NDELAY | LOG_PID, LOG_DAEMON);
		syslog(LOG_ERR, "can't open /dev/null: %m");
		closelog();
		exit(-1);
	}
	while (fd > -1 && fd < 3)
		fd = dup(fd);
	if (fd < 0)
	{
		openlog(get_progname(), LOG_NDELAY | LOG_PID, LOG_DAEMON);
		syslog(LOG_ERR, "can't set standard file descriptors: %m");
		closelog();
		exit(-1);
	}
	close(fd);
		
}


#if 0
void main(void)
{
	set_progname("a", "a");

	umask(0143);
	umask(077 & umask(022) | 022);
	printf("%d:0%3o\n", getpid(), umask(022));
//	close(1);
//	close(2);

	dwww_initialize();
	sleep(1000);
}
#endif
