/**
 * Copyright (c) 2012 William Light <wrl@illest.net>
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <lo/lo.h>

#define OSC_HANDLER_FUNC(x)\
	static int x(const char *path, const char *types,\
				 lo_arg **argv, int argc,\
				 lo_message data, void *user_data)

OSC_HANDLER_FUNC(enum_reply)
{
	int i;

	/* we receive a reply at the path "#reply". it will have 1 or more
	   string arguments, with the first being the path we sent in the
	   original message. we iterate along the rest of the arguments
	   (which are the sub-paths) and print them out. */
	for (i = 1; i < argc; i++)
		printf("%s%s\n", &argv[0]->s, &argv[i]->s);

	return 0;
}

static void usage(char *prog)
{
	printf(
		"usage: %s <host> <port> <path>\n",
		"    enumerate OSC methods at <host>:<port> under <path>\n",
		prog);
}

static char *get_enum_path(char *path)
{
	char *buf;
	int len = strlen(path);

	/* if there's no trailing slash, add one. */
	if (path[len - 1] != '/') {
		buf = malloc(len + 1);
		strncpy(buf, path, len);

		buf[len] = '/';
		buf[len + 1] = '\0';
	} else
		buf = strdup(path);

	return buf;
}

int main(int argc, char **argv)
{
	lo_address out;
	lo_server srv;
	char *path;
	int l;

	if (argc < 4) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	srv = lo_server_new(NULL, NULL);
	if (!(out = lo_address_new(argv[1], argv[2]))) {
		fprintf(stderr, "could not allocate lo_address for %s:%s\n",
				argv[1], argv[2]);
		return EXIT_FAILURE;
	}

	lo_server_add_method(srv, NULL, NULL, enum_reply, NULL);

	path = get_enum_path(argv[3]);
	lo_send_from(out, srv, LO_TT_IMMEDIATE, path, "");

	/* wait 1 second for a reply from the server. if we don't get one, then
	   either there's no server at this host:port combination, or the server
	   doesn't support this enumeration call. */
	if (!lo_server_recv_noblock(srv, 1000))
		fprintf(stderr, "no response from the OSC server.\n");

	free(path);

	lo_address_free(out);
	lo_server_free(srv);

	return EXIT_SUCCESS;
}
