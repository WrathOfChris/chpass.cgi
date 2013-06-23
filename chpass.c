/* $Gateweaver: chpass.c,v 1.6 2011/10/13 16:42:41 cmaxwell Exp $ */
/*
 * Copyright (c) 2013 Christopher Maxwell
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <kerberosV/krb5.h>
#include "chpass.h"

struct query *q = NULL;
struct confinfo conf;
struct conf confentries[] = {
	{ "ct_html", conf.ct_html },
	{ "domain", conf.domain },
	{ "mailaddr", conf.mailaddr },
	{ "htmldir", conf.htmldir },
	{ "baseurl", conf.baseurl },
	{ "title", conf.title },
	{ NULL, NULL }
};

int init_conf(void);

extern const char *__progname;

/*
 * Set defaults
 */
int
init_conf(void)
{
	char hostname[MAXHOSTNAMELEN], *h;

	bzero(&conf, sizeof(conf));

	/* ct_html */
	strlcpy(conf.ct_html, CONTENT_TYPE, sizeof(conf.ct_html));

	/* mailaddr */
	if (gethostname(hostname, sizeof(hostname)) == -1)
		return -1;
	if ((h = strchr(hostname, '.')) && strchr(h+1, '.'))
		h++;
	else
		h = hostname;
	snprintf(conf.mailaddr, sizeof(conf.mailaddr), "admin@%s", hostname);

	/* htmldir */
	snprintf(conf.htmldir, sizeof(conf.htmldir), ".");

	/* baseurl */
	snprintf(conf.baseurl, sizeof(conf.baseurl), "%s", __progname);

	/* title */
	snprintf(conf.title, sizeof(conf.title),
			"Kerberos Password Change Utility");

	return 0;
}

static int
change_password(char *oldpw, char *newpw)
{
	krb5_error_code ret = 0;
	krb5_context context = NULL;
	krb5_get_init_creds_opt *opts = NULL;
	krb5_creds creds;
	krb5_ccache id = NULL;
	const char *username;
	krb5_principal principal = NULL;
	krb5_data result_code_string, result_string;
	int result_code;

	if ((ret = krb5_init_context(&context)))
		goto out;

	bzero(&creds, sizeof(creds));

	if ((ret = krb5_get_init_creds_opt_alloc(context, &opts)))
		goto out;
	krb5_get_init_creds_opt_init(opts);
	krb5_get_init_creds_opt_set_tkt_life(opts, 300);
	krb5_get_init_creds_opt_set_renew_life(opts, 0);
	krb5_get_init_creds_opt_set_forwardable(opts, 0);
	krb5_get_init_creds_opt_set_proxiable(opts, 0);

	if ((ret = krb5_cc_gen_new(context, &krb5_mcc_ops, &id)))
		goto out;
	if ((username = getenv("REMOTE_USER")) == NULL) {
		ret = KRB5_NO_LOCALNAME;
		goto out;
	}
	if ((ret = krb5_parse_name(context, username, &principal)))
		goto out;

	if ((ret = krb5_get_init_creds_password(context,
					&creds,
					principal,
					oldpw,
					NULL,
					NULL,
					0,
					"kadmin/changepw",
					opts)))
		goto out;

	if ((ret = krb5_cc_initialize(context, id, principal)))
		goto out;
	if ((ret = krb5_cc_store_cred(context, id, &creds)))
		goto out;

	if ((ret = krb5_set_password_using_ccache(context, id, newpw,
					principal,
					&result_code,
					&result_code_string,
					&result_string)))
		goto out;

	if (result_code != 0) {
		render_error("Cannot change password: %s%s%.*s",
				krb5_passwd_result_to_string(context, result_code),
				result_string.length > 0 ? " : " : "",
				(int)result_string.length,
				result_string.length > 0 ? (char *)result_string.data : "");
		ret = -1;
		goto fail;
	}

	krb5_data_free(&result_code_string);
	krb5_data_free(&result_string);

out:
	if (ret != 0)
		render_error("changing password: %s",
				krb5_get_err_text(context, ret));
fail:
	if (opts)
		krb5_get_init_creds_opt_free(opts);
	if (id)
		krb5_cc_destroy(context, id);
	if (context)
		krb5_free_context(context);

	return ret;
}

int
main(int argc, char **argv)
{
	const char *action;

	umask(007);
	if (init_conf() == -1) {
		render_error("init_conf");
		goto done;
	}
	if (load_conf(CONF_FILE, confentries) == 1) {
		fprintf(stderr, "%s cannot load configuration file \"%s\": %s\n",
				__progname, CONF_FILE, strerror(errno));
	}
	if ((q = get_query()) == NULL) {
		render_error("get_query");
		goto done;
	}

	if ((action = get_query_param(q, "action")) == NULL)
		action = "front";

	/* main decision tree */
	if (strcmp(action, "front") == 0) {
		char fn[MAXPATHLEN];

		printf("%s\n\n", conf.ct_html);
		snprintf(fn, sizeof(fn), "%s/front.html", conf.htmldir);
		render_html(fn, &render_front);
	} else if (strcmp(action, "chpass") == 0) {
		char fn[MAXPATHLEN];
		char buf[BUFSIZ];
		unsigned pos = 0, r;
		struct query pq;
		char *oldpw;
		char *pw1;
		char *pw2;

		while (pos < sizeof(buf) - 1 && (r = fread(buf + pos, 1,
						sizeof(buf) - pos - 1, stdin)) > 0)
			pos += r;
		buf[pos] = '\0';

		bzero(&pq, sizeof(pq));
		tokenize_query_params(buf, &pq.params);
		oldpw = get_query_param(&pq, "oldpw");
		pw1 = get_query_param(&pq, "pw1");
		pw2 = get_query_param(&pq, "pw2");

		if (pw1 == NULL || pw2 == NULL) {
			render_error("must enter password twice (%s)",
					pw1 == NULL && pw2 ? "second" :
					pw2 == NULL && pw1 ? "first" :
					"both");
			goto done;
		}
		if (strcmp(pw1, pw2) != 0) {
			render_error("passwords do not match");
			goto done;
		}

		if (change_password(oldpw, pw1) != 0)
			goto done;

		printf("%s\n\n", conf.ct_html);
		snprintf(fn, sizeof(fn), "%s/success.html", conf.htmldir);
		render_html(fn, NULL);
	} else {
		render_error("invalid action");
	}

done:
	fflush(stdout);
	if (q != NULL)
		free_query(q);
	return 0;
}
