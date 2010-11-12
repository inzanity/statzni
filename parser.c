#include <stdlib.h>
#include <string.h>
#include "parser.h"

static void *_noop_init() {
	return NULL;
}

static void _noop_deinit(void *data)
{
	(void)data;
}

#define IGNIFNUL(x) while (!(x)) { details->type = ACTION_IGNORE; return; }
static void _irssi_parse_line(void *data,
			      struct action_details *details,
			      char *line)
{
	char *spc;
	char *enick;
	(void)data;

	if (*line == '-') {
		details->type = ACTION_IGNORE;
		return; 
	}

	spc = strchr(line, ' ');
	IGNIFNUL(spc);

	*spc = '\0';
	details->time = (line[0] - '0') * 600 + (line[1] - '0') * 60 +
		(line[3] - '0') * 10 + line[4] - '0';
	spc++;
	if (*spc == '<') {
		IGNIFNUL(*++spc);
		spc++;
		IGNIFNUL(enick = strchr(spc, '>'));

		details->nick = spc;

		*enick = '\0';
		enick += 2;

		details->type = ACTION_MESSAGE;
		details->value = enick;

		return;
	}

	IGNIFNUL(spc = strchr(spc, ' '));
	spc++;

	IGNIFNUL(enick = strchr(spc, ' '));
	*enick = '\0';

	if (strchr(spc, '/') || strchr(spc, ':')) {
		details->type = ACTION_IGNORE;
		return;
	}

	details->nick = spc;
	enick++;

	if (*enick == '[') {
		IGNIFNUL(spc = strchr(enick, ' '));
		IGNIFNUL(spc = strchr(spc + 1, ' '));
		spc++;

		if (*spc == 'j') {
			details->type = ACTION_JOIN;
			details->value = NULL;
			return;
		}
		details->type = *spc == 'l' ? ACTION_LEAVE : ACTION_QUIT;
		IGNIFNUL(spc = strchr(spc, '['));
		spc++;
		details->value = spc;
		spc[strlen(spc) - 1] = '\0';

		return;
	}

	IGNIFNUL(spc = strchr(enick, ':'));
	IGNIFNUL(*(++spc));
	details->type = ACTION_TOPIC;
	spc++;

	details->value = spc;
	return;
}

struct {
	const char *format;
	struct parser funcs;
} parsers[] = {
	{ "irssi", { _noop_init, _irssi_parse_line, _noop_deinit } }
};


struct parser *get_parser(const char *format)
{
	unsigned int i;
	for (i = 0; i < sizeof(parsers)/sizeof(*parsers); i++)
		if (!strcmp(format, parsers[i].format))
			return &parsers[i].funcs;
	return NULL;
}
