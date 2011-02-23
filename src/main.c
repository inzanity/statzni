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
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io.h"
#include "user.h"
#include "parser.h"
#include "output.h"
#include "state.h"

static void user_handle_line(struct user *user, const char *line)
{
	const char *i = line;
	char p = '\0';
	char exl = 0;
	char que = 0;

	++user->lines;
	++user->chars;
	
	if (*user->quote) {
		if (rand() > (RAND_MAX/100*99))
			strcpy(user->quote, line);
	} else {
			strcpy(user->quote, line);
	}

	while (*i) {
		switch (*i) {
		case '?':
			if (!que)
				++user->questions;
			que = 1;
			break;
		case '!':
			if (!exl)
				++user->exclamations;
			exl = 1;
			break;
		case ' ':
			if (p != ' ')
				++user->words;
			break;
		case ':':
		case ';':
		case '8':
			if (i[1] == ')' || i[1] == 'D')
				++user->happy;
			else if (i[1] == '(')
				++user->sad;
			break;
		case '*':
			if (strncmp(i, "*tsup*", 6))
				++user->tsup;
		default:
			++user->chars;
		}

		p = *i;
		i = g_utf8_next_char(i);
	}

	if (p == ' ')
		--user->words;
}

int main(int argc, char **argv)
{
	GKeyFile *settings;
	GSList *result;
	struct action_details details;
	struct users *users;
	struct parser *p = get_parser("irssi");
	struct formatter *f;
	struct state *s;
	void *d = p->init();
	char *line;
	int i;
	int times[24] = { 0 };
	
	srand(time(NULL));

	if (argc < 3) {
		fprintf(stderr, "Usage: %s <configuration file> <logfiles>\n",
			argv[0]);
		exit(1);
	}

	settings = g_key_file_new();
	g_key_file_load_from_file(settings, argv[1], 0, NULL);
	users = users_new();
	users_load(users, settings);
	f = formatter_new(settings);
	s = state_new(settings);
	g_key_file_free(settings);

	state_load(s, users, times);

	for (i = 2; i < argc; i++) {
		struct iofile *file = io_open(argv[i]);
		while ((line = io_getline(file))) {
			p->parse_line(d, &details, line);
			if (details.type == ACTION_MESSAGE) {
				struct user *user = users_get_user(
						users,
						details.nick);
				times[(details.time & HOUR_MASK) >> HOUR_SHIFT]++;
				user->timelines[((details.time & HOUR_MASK) >> HOUR_SHIFT) / 6]++;
				user_handle_line(user, details.value);
			}
		}
		io_close(file);
	}

	p->deinit(d);

	state_save(s, users, times);

	formatter_timeline(f, times);

	result = users_get_all(users);
	formatter_users(f, result);
	formatter_trivia(f, result);
	g_slist_free(result);

	formatter_free(f);
	free_users(users);
	state_free(s);

	return 0;
}
