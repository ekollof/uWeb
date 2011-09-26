/*
 * Copyright (c) 2003, Emiel Kollof <coolvibe@hackerheaven.org> All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution. Neither the name of
 * the organisation nor the names of its contributors may be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 * network.c
 *
 */

#include "webserver.h"

/* network code goes here */

int
net_readln(fd, bufptr, len)
	int fd;
	char *bufptr;
	size_t len;
{
        char           *bufx = bufptr;
        static char    *bp;
        static int      cnt = 0;
        static char     b[1500];
        char            c;

        while (--len > 0) {
                if (--cnt <= 0) {
                        cnt = recv(fd, b, sizeof(b), 0);
                        if (cnt < 0) {
                                if (errno == EINTR) {
                                        len++;
                                        continue;
                                }
                                return -1;
                        }
                        if (cnt == 0)
                                return 0;
                        bp = b;
                }
                c = *bp++;
                *bufptr++ = c;
                if (c == '\n') {
                        *bufptr = '\0';
                        return bufptr - bufx;
                }
        }
        set_errno(EMSGSIZE);
        return -1;
}

int net_send(fd, buf)
	int fd;
	char *buf;
{
	int             bout;
        bout = 0;
	fflush(stdout);
	printf("sock: %d sent: %s", fd, buf);
        if ((bout = send(fd, buf, strlen(buf), 0)) < 0) {
                if (bout == 0) {
                        fprintf(stderr, "netsend: socket disconnected.\n");
                        close(fd);
                        exit(0);
                } else {
			perror("netsend");
			exit(1);
                }
        }
        return (bout);
}

int
net_printf(va_alist)
	va_dcl
{
	va_list argp;
	char *fmt;
	int fd;
	char buf[1500];

	va_start(argp);
	fd = va_arg(argp, int);
	fmt = va_arg(argp, char *);
	vsprintf(buf, fmt, argp);

	va_end(argp);

	net_send(fd, buf);
	return(0);

}
 
int
net_setaddr(hname, sname, sap)
	char *hname;
	char *sname;
	struct sockaddr_in *sap;
{
	struct servent *sp;
	struct hostent *hp;
	char *endptr;
	short port;

	memset(sap, '0', sizeof(*sap));
	sap->sin_family = PF_INET;
	if (hname != NULL) {
		sap->sin_addr.s_addr = inet_addr(hname);
		if (sap->sin_addr.s_addr == -1) {
			hp = gethostbyname(hname);
			if (hp == NULL) {
				perror("gethostbyname");
				exit(1);	
			}
			sap->sin_addr = *(struct in_addr *) hp->h_addr;
		}	
	} else {
		printf("hname was NULL, using INADDR_ANY.\n");
		sap->sin_addr.s_addr = htonl(INADDR_ANY);
	}
		
	port = atoi(sname);

	printf("Will listen on port %l\n", port);
	sap->sin_port = htons(port);

	return(0);
}

int 
net_server(hname, sname) 
	char *hname;
	char *sname;
{
	struct sockaddr_in sap;
	int on = 1;
	int s;

	net_setaddr(hname, sname, &sap);
	s = socket(PF_INET, SOCK_STREAM, 0);
	if (!s) {
		perror("socket");
		exit(1);
	}
	if (setsockopt( s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
		perror("setsockopt");
		exit(1);
	}	
	if ( bind( s, (struct sockaddr *) &sap, sizeof(sap))) {
		perror("bind");
		exit(1);
	}	 
	if (listen( s, NLISTEN)) {
		perror("listen");
		exit(1);
	}
	return s;
}


int
net_mainloop(hname, sname)
	char *hname;
	char *sname;
{
	int len;

	sock = net_server(hname, sname);
	printf("Socket fd returned: %d\n", sock);

	/* We have a socket, and it's ready to rock. */
	/* here begins the main accept loopt */	

	do {

		int pid;
		int client;
		struct web_request request;
		
		len = sizeof(struct sockaddr);		
		request.client_fd = accept(sock, (struct sockaddr *) &request.peer, &len);
		client = request.client_fd;
		if (!client) {
			perror("accept");
			exit(1);
		}	
		printf("\nAccepted connection. Forking\n");
		
		switch (pid = fork()) {
			case -1:
				/* error */
				perror("fork");
				exit(1);
			case 0:
				/* child */
				printf("child fd: %d\n", client);
				web_process(&request);
				printf("child exit\n");
				exit(0);
			default:
				/* parent */
				close(client);
				printf("forked child with pid %d\n", pid);
				break;
		}

	} while(1);

	return 0;
}
