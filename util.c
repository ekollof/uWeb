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
 * util.c
 *
 */


#include "webserver.h"

/* Miscellaneous routines go here */

int
lowercase(buf)
	char *buf;
{
        char           *ptr = buf;
        int             len;
        char            c;
        len = strlen(buf) + 1;  /* adjust for \0 */
        while (--len > 0) {
                c = *ptr++;
                *buf++ = tolower(c);
        }
        *buf++ = '\0';
        return;
}

void
stripchar(buf, strip)
	char *buf;
	int strip;
{
        char           *ptr = buf;
        int             len;
        char            c;
        len = strlen(buf) + 1;  /* adjust for \0 */
        while (--len > 0) {
                c = *ptr++;
                if (c != strip)
                        *buf++ = c;
        }
        *buf++ = '\0';
        return;
}

int
chomp(buf)
	char *buf;
{
	stripchar(buf, '\n');
	stripchar(buf, '\r');
}

char *
strdup(buf) 
	char *buf;
{
	size_t len;
	char *ret;

	if (!buf) return(NULL);

	len = strlen(buf)+1;

	ret = (char *)malloc(len);

	if (!ret) 
		return(NULL);

	memcpy(ret,buf,len);
        return(ret);
}

int 
raw_read(rfd, wfd)
        register int rfd; /* fd to read from */
	register int wfd; /* fd to write to */
{
        register int off;
        ssize_t nr, nw;
        size_t bsize;
        static char *buf = NULL;
	struct stat sbuf;

        if (buf == NULL) {
		if (fstat(wfd, &sbuf))
			err(1, "problem with statting fd");
                bsize = MAX(sbuf.st_blksize, 1024);
                if ((buf = (char *) malloc(bsize)) == NULL)
                        err(1, "buffer");
        }
        while ((nr = read(rfd, buf, bsize)) > 0)
                for (off = 0; nr; nr -= nw, off += nw)
                        if ((nw = write(wfd, buf + off, (size_t)nr)) < 0)
                                err(1, "out");

        if (nr < 0) {
                warn("filedescriptor problem");
		return(0);
        }
}

