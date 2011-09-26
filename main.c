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
 * main.c
 *
 */

#include "webserver.h"

void 
usage() 
{
	printf("webserver usage:\n\n");
	printf("webserver [-f] [-p port] -d <webserver root>\n");
	printf("webserver -e (configuration in environment)\n\n");
	printf("With the -e argument, the following environment vars are read:\m\n");
	printf("WEBROOT - defines the webserver root dir\n");
	printf("PORT - defines the port to listen on\n");
	printf("DEBUG   - if this contains anything, debug mode\n");
	exit(0);
}

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	extern char *optarg;
	extern int optind;
	int readenv = 0;
	int ch, fd;
	char *buf;
	char *swp;       
	char port[10];

	if (argc < 2) {
		usage();
	}

	memset(webroot, 0, sizeof(webroot));
	memset(port, 0, sizeof(port));

	while ((ch = getopt(argc, argv, "efd:p:h")) != -1)
		switch (ch) {
			case 'f':
				foreground = 1;
				break;
			case 'd':
				strncpy(webroot, optarg, sizeof(webroot));
				break;
			case 'p':
				strncpy(port, optarg, sizeof(port));
				break;
			case 'e':
				readenv = 1;
				break;
			case 'h':
			default:
				usage();
		}
	argc -= optind;
	argv += optind;

	if (readenv) { 

		if (!getenv("WEBROOT")) {
			strncpy(webroot, getenv("WEBROOT"), sizeof(webroot));
		}
		if (!getenv("PORT")) {
			strncpy(port, getenv("PORT"), sizeof(port));
		}
		if (!getenv("DEBUG")) {
			strncpy(foreground, getenv("DEBUG"), sizeof(foreground));
		}
	}

	if (!strcmp(port, "")) {
		printf("defaulting to port 80\n");
		sprintf(port, "80");
	}
	if (!strcmp(webroot, "")) {
		printf("You must provide a web root.\n");
		usage();
	}

	/* check for leading period in webroot and expand it */
	
	if (*webroot == '.') {
      	
		int c;		
	
		printf("Web root contains a period. Expanding\n"); 

                /* replace period with current dir */
                buf = getwd();
		if (!buf) {
			perror("getwd");
			exit(1);
		}		
       
                /* dup webroot and nix leading period */
                swp = strdup(webroot);
		if (!swp) {
			perror("strdup");
			exit(1);
		}		

                *swp++;
		if (*swp = '.') { /* is there a second period? (as in ..) */
			*swp++;
		}

		c = (int) *(swp+strlen(swp)-1);
                strncpy(webroot, buf, sizeof(webroot));
		if (c != '/') {
			printf("trailing slash\n");
			strncat(webroot, "/", 
				sizeof(webroot) - strlen(webroot) -1);
		}
                strncat(webroot, swp, sizeof(webroot) - strlen(webroot) -1);
        }


	printf("asked port: %s webroot: %s\n", port, webroot);
	if (!foreground) {	
		daemon(1,0);
	}
	net_mainloop(NULL, port);	

	return (0);
}
