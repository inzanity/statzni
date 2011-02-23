/*
 * statzni - simple irc log analyzer
 * Copyright (C) 2010-11 Santtu Lakkala
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <string.h>
#include "parser.h"

struct IrssiParser {
	unsigned int basetime;
};

static void *_irssi_init(void) {
	struct IrssiParser *data = malloc(sizeof(struct IrssiParser));
	data->basetime = 0;

	return data;
}

static void _irssi_deinit(void *data)
{
	free(data);
}

static inline unsigned int _irssi_parse_month(const char *month)
{
	switch (*month) {
	case 'F':
		return 2;
	case 'M':
		return (month[1] && month[2] == 'y') ? 5 : 3;
	case 'A':
		return month[1] == 'u' ? 8 : 4;
	case 'J':
		return (month[1] == 'a' ? 1 : month[1] && month[2] == 'l') ? 7 : 6;
	case 'S':
		return 9;
	case 'O':
		return 10;
	case 'N':
		return 11;
	case 'D':
		return 12;
	default:
		return 0;
	}
}

static inline unsigned int _irssi_parse_date(const char *date, int hastime)
{
	unsigned int rv = 0;
	const char *spc = date;

#define PROGRESS(x) x = strchr(x, ' '); if (!(x++) || !*x) return rv
	PROGRESS(spc);
	rv |= _irssi_parse_month(spc) << MONTH_SHIFT;

	PROGRESS(spc);
	rv |= atoi(spc) << DAY_SHIFT;

	PROGRESS(spc);
	if (hastime)
		PROGRESS(spc);

	rv |= atoi(spc) << YEAR_SHIFT;
	
	return rv;
#undef PROGRESS
}

#define IGNIFNUL(x) while (!(x)) { details->type = ACTION_IGNORE; return; }
static void _irssi_parse_line(void *pdata,
			      struct action_details *details,
			      char *line)
{
	struct IrssiParser *data = pdata;
	char *spc;
	char *enick;
	(void)data;

	if (*line == '-') {
		spc = strstr(line, "opened ");
		if (spc || strstr(line, "Day changed "))
			data->basetime = _irssi_parse_date(line + 7,
							   !!spc);
		details->type = ACTION_IGNORE;
		return; 
	}

	spc = strchr(line, ' ');
	IGNIFNUL(spc);

	*spc = '\0';
	details->time = ((line[0] - '0') * 10 + (line[1] - '0')) << HOUR_SHIFT |
		((line[3] - '0') * 10 + line[4] - '0');
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
	{ "irssi", { _irssi_init, _irssi_parse_line, _irssi_deinit } }
};


struct parser *get_parser(const char *format)
{
	unsigned int i;
	for (i = 0; i < sizeof(parsers)/sizeof(*parsers); i++)
		if (!strcmp(format, parsers[i].format))
			return &parsers[i].funcs;
	return NULL;
}
