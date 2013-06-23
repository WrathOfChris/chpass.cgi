/* $Gateweaver: output.c,v 1.3 2005/10/25 22:47:48 cmaxwell Exp $ */
/* From: $UNDEADLY: undeadly.c,v 1.16 2005/07/11 18:30:38 dhartmei Exp $ */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chpass.h"

static char *html_esc(const char *, char *, size_t, int)
	__attribute__ ((__bounded__(__buffer__, 2, 3)));

void
render_error(const char *fmt, ...)
{
	va_list ap;
	char s[8192], e[8192];

	va_start(ap, fmt);
	vsnprintf(s, sizeof(s), fmt, ap);
	va_end(ap);
	printf("%s\n\n", conf.ct_html);
	printf("<html><head><title>Error</title></head><body>\n");
	printf("<h2>Error</h2><p><b>%s</b><p>\n", s);
	if (q != NULL) {
		printf("Request: <b>%s</b><br>\n",
		    html_esc(q->query_string, e, sizeof(e), 0));
		printf("Address: <b>%s</b><br>\n",
		    html_esc(q->remote_addr, e, sizeof(e), 0));
		if (q->user_agent != NULL)
			printf("User agent: <b>%s</b><br>\n",
			    html_esc(q->user_agent, e, sizeof(e), 0));
	}
	printf("<p>Please send reports with instructions about how to "
	    "reproduce to <a href=\"mailto:%s\"><b>%s</b></a><p>\n",
	    conf.mailaddr, conf.mailaddr);
	printf("</body></html>\n");
}

/*
 * Safely escape HTML.  (s) is source, (d) is destination.
 * Set (allownl) to transform \n|\r to <br>
 */
static char *
html_esc(const char *s, char *d, size_t len, int allownl)
{
	char *p;

	for (p = d; *s && p < (d + len - 1); ++s)
		switch (*s) {
			case '&':
				if (strlcat(p, "&amp;", (d + len - p)) >= 5)
					goto overflow;
				break;
			case '\"':
				if (strlcat(p, "&quot;", (d + len - p)) >= 6)
					goto overflow;
				break;
			case '<':
				if (strlcat(p, "&lt;", (d + len - p)) >= 4)
					goto overflow;
				break;
			case '>':
				if (strlcat(p, "&rt;", (d + len - p)) >= 4)
					goto overflow;
				break;
			case '\r':
			case '\n':
				if (!allownl) {
					/* skip */
					break;
				} else if (allownl > 1 && *s == '\r') {
					if (strlcat(p, "<br>", (d + len - p)) >= 4)
						goto overflow;
					break;
				}
				/* FALLTHROUGH */
			default:
				*p++ = *s;
		}
	*p = '\0';
	return d;

overflow:
	return "(internal overflow)";
}

int
render_html(const char *html_fn, render_cb r)
{
	FILE *f;
	char s[8192];

	if ((f = fopen(html_fn, "r")) == NULL) {
		printf("ERROR: fopen: %s: %s<br>\n", html_fn, strerror(errno));
		return (1);
	}
	while (fgets(s, sizeof(s), f)) {
		char *a, *b;

		for (a = s; (b = strstr(a, "%%")) != NULL;) {
			*b = 0;
			printf("%s", a);
			a = b + 2;
			if ((b = strstr(a, "%%")) != NULL) {
				*b = 0;
				if (!strcmp(a, "BASEURL"))
					printf("%s", conf.baseurl);
				else if (!strcmp(a, "TITLE"))
					printf("%s", conf.title);
				else if (!strcmp(a, "REMOTEUSER"))
					printf("%s", getenv("REMOTE_USER"));
#if 0
				else if (!strcmp(a, "RCSID"))
					printf("%s", rcsid);
#endif
				else if (r != NULL)
					(*r)(a);
				a = b + 2;
			}
		}
		printf("%s", a);
	}
	fclose(f);
	return 0;
}

void
render_front(const char *m)
{
	char fn[1024];

	if (!strcmp(m, "HEADER")) {
		snprintf(fn, sizeof(fn), "%s/header.html", conf.htmldir);
		render_html(fn, NULL);
	} else if (!strcmp(m, "FOOTER")) {
		snprintf(fn, sizeof(fn), "%s/footer.html", conf.htmldir);
		render_html(fn, NULL);
	} else
		printf("render_front: unknown macro '%s'<br>\n", m);
}
