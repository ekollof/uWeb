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
 * web.c
 *
 */


#include "webserver.h"

/* web processing code goes here */

int
web_parse_browserinfo(request, buf)
	struct web_request *request;
	char *buf;	
{
	char *type;
	char *info;
	char *temp;

	temp = strdup(buf);
	type = strtok(temp, ":");
	info = strtok(NULL, "\0");

	/* skip over the extra space in the beginning */
	*info++;


	if (!strcmp(type, "User-Agent")) {
		printf("Type: %s\n", type);
		printf("Info: %s\n", info);

		strncpy(request->useragent, info, sizeof(request->useragent));	
	}

	free(temp);

	return(0);
}


int 
web_send_header(request, statuscode)
	struct web_request *request;
	char *statuscode;
{
	time_t now;
	int sock;

	sock = request->client_fd;

	net_printf(sock, "%s\n", statuscode);
	net_printf(sock, "Server: Coolvibe's minimalist PDP11 webserver\n");
}

int
web_get(request, document)
	struct web_request *request;
	char *document;
{
	int c;
	int fd;
	unsigned short bytesread;
	char buf[1];
	char path[1024];
	char filetype[32];

	/* build path to intended document */
	/* first check for a trailing slash, if it's there, nix it. */	
	
	c = (int) *(webroot+strlen(webroot)-1);
	if (c  == '/') {
		*(webroot+strlen(webroot)-1) = '\0';
	}

	/* Stop stupid pricks from doing stupid directory traversal stuff */
	if (strstr(document, "../") || strstr(document, "./" ) || 
	strstr(document, "/.." )) {
		web_send_header(request, WEB_BADRQ);	
		net_printf(request->client_fd, 
			"Content-Type: text/html\n\n");
		net_printf(request->client_fd, 
			"BAD request... go sit in a corner...");
		return(0);
	}	

	/* right, now let's figure out which document the browser wants */
	
	strncpy(path, webroot, sizeof(path));
	strcat(path, document, sizeof(path) - strlen(path));
	

	c = (int) *(path+strlen(path)-1);
	if (c == '/') {
		/* request ends in a traling slash, so add index */
		strcat(path, INDEX, sizeof(path) - strlen(path));
	}

	printf("User wants file: %s\n", path);
	strncpy(filetype, mime_get_type(path), sizeof(filetype));
	printf("File is type: %s\n", filetype);

	if ((fd = open(path, O_RDONLY)) == -1) {
		if (errno == ENOENT) {
			/* hmm, file doesn't exist, let's tell em */
			web_send_header(request, WEB_NOTFOUND);	
			net_printf(request->client_fd, 
				"Content-Type: text/html\n\n");
			net_printf(request->client_fd, 
				"404 man... this file ain't here.");
			return(0);
		}
        }	
	web_send_header(request, WEB_OK);	
	net_printf(request->client_fd,
                        "Content-Type: %s\n\n", filetype);
	
	raw_read(fd, request->client_fd);

	return(0);
}

int 
web_parse_request(request)
	struct web_request *request;
{
	
	char action[5];
	char document[1024];
	char junk[10];	

	sscanf(request->request, "%4s %1023s %9s", &action, &document, &junk);

	printf("Action: %s\n", action);
	printf("Wants: %s\n", document);
	printf("Junk: %s\n", junk);

	if (!strcmp(action, "HEAD")) {
		web_send_header(request, WEB_OK);
		net_printf(request->client_fd, 
			"Content-Type: text/html\n\n");
	}

	if (!strcmp(action, "GET")) {
		web_get(request, document);
	}

	return(0);
}

int 
web_process(request)
	struct web_request *request;
{
	int clsock;
	FILE *log;
	struct sockaddr_in sap;
	struct hostent *he;
	char buf[1000];
	char from[15];

	clsock = request->client_fd;
	sap = request->peer;

	strncpy(from, inet_ntoa(sap.sin_addr.s_addr), sizeof(from));
	he = gethostbyaddr(from, sizeof(from), PF_INET);

	setenv("REMOTE_ADDR", from, 1);

	printf("Request from: '%s' (%s)\n", he->h_name, from);

	/* first get the request */
	net_readln(clsock, buf, sizeof(buf));
	strncpy(request->request, buf, sizeof(request->request));

	/* Get information from browser until we get a empty line */
	while(strcmp(buf, "")) {
		net_readln(clsock, buf, sizeof(buf));
		chomp(buf);
		if (strcmp(buf, "")) {
			web_parse_browserinfo(request, buf);
		}
	}

	/* do request */
	web_parse_request(request);

	/* log it */
	log = fopen("logfile", "a");
	chomp(request->request);
	fprintf(log, "%s - %s - %s\n", from, 
					request->request, 
					request->useragent);
	fclose(log);

	close(clsock);
	return(0);
}


