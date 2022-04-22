#define _USE_BSD
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <stdarg.h>
#include <bits/stdc++.h>
#include <sqlite3.h>
#include <sys/stat.h>

using namespace std;

/*------------------------------------------------------------------------
 * reaper - clean up zombie children
 *------------------------------------------------------------------------
 */
void reaper(int sig) {
	int	status;
	while (waitpid(-1, &status, WNOHANG) >= 0)
		/* empty */;
}

int errexit(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}

void send_msg(int fd, string & msg){
    int cmd_errno;
    if (cmd_errno=write(fd, msg.c_str(), strlen(msg.c_str())) < 0){
        errexit("echo write: %s\n", strerror(cmd_errno));
    }
    return;
}