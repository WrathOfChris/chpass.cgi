/*	$Gateweaver: cgi.h,v 1.2 2005/10/25 22:47:48 cmaxwell Exp $ */
/*	$UNDEADLY: cgi.h,v 1.4 2004/08/21 13:57:28 dhartmei Exp $ */
/*
 * Copyright (c) 2013 Christopher Maxwell
 * Copyright (c) 2004 Daniel Hartmeier
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

#ifndef _CGI_H_
#define _CGI_H_

#include <sys/types.h>

struct query_param;

struct query {
	char			*request_method;
	char			*query_string;
	char			*remote_addr;
	char			*user_agent;
	struct query_param	*params;
};

struct query	*get_query(void);
void		 tokenize_query_params(char *q, struct query_param **p);
const char	*get_query_param(const struct query *q, const char *key);
void		 free_query(struct query *q);
unsigned	 url_decode(const char *s, char *d, size_t l)
	__attribute__ ((__bounded__(__buffer__, 2, 3)));
unsigned	 url_encode(const char *s, char *d, size_t l)
	__attribute__ ((__bounded__(__buffer__, 2, 3)));

#endif
