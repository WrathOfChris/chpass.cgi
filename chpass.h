/* $Gateweaver: chpass.h,v 1.3 2005/10/25 22:47:48 cmaxwell Exp $ */
#ifndef CHPASS_H
#define CHPASS_H
#include <sys/param.h>
#include "cgi.h"
#include "conf.h"

#define CONTENT_TYPE	"Content-Type: text/html; charset=iso-8859-1"
#define CONF_FILE		"chpass.conf"

struct confinfo {
	char ct_html[MAXPATHLEN];
	char domain[MAXHOSTNAMELEN];
	char mailaddr[MAXPATHLEN];
	char htmldir[MAXPATHLEN];
	char baseurl[MAXPATHLEN];
	char title[MAXPATHLEN];
};

typedef	void (*render_cb)(const char *);

extern struct query *q;
extern struct confinfo conf;

void render_error(const char *, ...)
	__attribute__ ((__format__(printf, 1, 2)));
int render_html(const char *, render_cb);
void render_front(const char *);

#endif
