#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io.h"
#include "user.h"
#include "parser.h"
#include "output.h"
#include "state.h"

void user_handle_line(struct user *user, const char *line)
{
	const char *i = line;
	char p = '\0';

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
			if (p  != '?')
				++user->questions;
			break;
		case '!':
			if (p != '!')
				++user->exclamations;
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

	settings = g_key_file_new();
	g_key_file_load_from_file(settings, "conf.ini", 0, NULL);
	users = users_new(settings);
	f = formatter_new(settings);
	s = state_new(settings);
	g_key_file_free(settings);

	state_load(s, users, times);

	for (i = 1; i < argc; i++) {
		struct iofile *file = io_open(argv[i]);
		while ((line = io_getline(file))) {
			p->parse_line(d, &details, line);
			if (details.type == ACTION_MESSAGE) {
				struct user *user = users_get_user(
						users,
						details.nick);
				times[details.time / 60]++;
				user->timelines[details.time / 360]++;
				user_handle_line(user, details.value);
			}
		}
		io_close(file);
	}

	state_save(s, users, times);

	formatter_timeline(f, times);

	result = users_get_all(users);
	formatter_users(f, result);
	g_slist_free(result);

	free_users(users);

	return 0;
}
