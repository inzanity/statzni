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
#include "state.h"
#include "user.h"

struct state
{
	GKeyFile *state;
	gchar *statefile;
};

static gchar *_key_escape(const gchar *key)
{
	GString *str = g_string_sized_new(strlen(key));

	for (; *key; key++) {
		if (g_ascii_isalnum(*key) || *key == '-' || *key == '/' ||
		    *key == '+' || *key == '_')
			g_string_append_c(str, *key);
		else
			g_string_append_printf(str, ".%02x", *key);
	}

	return g_string_free(str, FALSE);
}

#define HEXVAL(x) ((x) <= '9' ? (x) - '0' : (x) - 'W')

static void _key_unescape(gchar *key)
{
	gchar *to;

	for (to = key; *key; key++, to++) {
		if (*key == '.') {
			*to = HEXVAL(key[1]) << 4 | HEXVAL(key[2]);
			key += 2;
		}
		else
			*to = *key;
	}
	*to = *key;
}

struct state *state_new(GKeyFile *settings)
{
	gchar *statefile = g_key_file_get_string(settings,
						 "settings",
						 "state",
						 NULL);
	struct state *state;

	if (!statefile) {
		return NULL;
	}

	state = malloc(sizeof(struct state));
	state->state = g_key_file_new();
	state->statefile = statefile;
	g_key_file_load_from_file(state->state, statefile, 0, NULL);

	return state;
}

void state_load(struct state *state, struct users *users, int times[24])
{
	gchar **nicks;
	gsize i;
	gint *tms;

	if (!state)
		return;

	tms = g_key_file_get_integer_list(state->state,
					  "global",
					  "times",
					  &i,
					  NULL);
	if (tms) {
		if (i == 24)
			memcpy(times, tms, 24 * sizeof(int));
		g_free(tms);
	}

	nicks = g_key_file_get_keys(state->state,
				    "users",
				    NULL,
				    NULL);

	if (nicks) {
		for (i = 0; nicks[i]; i++) {
			struct user *user;
			gsize len;
			gchar *quote;
			int *ints;
			
			_key_unescape(nicks[i]);
			user = users_get_user(users, nicks[i]);
			ints = g_key_file_get_integer_list(state->state,
								"users",
								nicks[i],
								&len,
								NULL);
			if (!ints)
				continue;

			if (len == 12) {
				gsize j;
				user->lines += ints[0];
				for (j = 0; j < 4; j++)
					user->timelines[j] += ints[j + 1];
				user->words += ints[5];
				user->chars += ints[6];
				user->questions += ints[7];
				user->exclamations += ints[8];
				user->happy += ints[9];
				user->sad += ints[10];
				user->tsup += ints[11];
			}
			g_free(ints);

			if (!*user->quote) {
				quote = g_key_file_get_string(state->state,
							      "quotes",
							      nicks[i],
							      NULL);
				if (quote) {
					strcpy(user->quote, quote);
					g_free(quote);
				}
			}
		}
		g_strfreev(nicks);
	}
}

void state_save(struct state *state, struct users *users, int times[24])
{
	GSList *usrs;
	gchar *data;
	gsize len;

	if (!state)
		return;
	g_key_file_remove_group(state->state,
				"users",
				NULL);
	g_key_file_remove_group(state->state,
				"quotes",
				NULL);
	g_key_file_set_integer_list(state->state,
				    "global",
				    "times",
				    times,
				    24);

	usrs = users_get_all(users);
	while (usrs) {
		struct user *user = usrs->data;
		gchar *key = _key_escape(user->username);
		int ints[12];
		gsize j;
		ints[0] = user->lines;
		for (j = 0; j < 4; j++)
			ints[j + 1] = user->timelines[j];
		ints[5] = user->words;
		ints[6] = user->chars;
		ints[7] = user->questions;
		ints[8] = user->exclamations;
		ints[9] = user->happy;
		ints[10] = user->sad;
		ints[11] = user->tsup;
		g_key_file_set_integer_list(state->state,
					    "users",
					    key,
					    ints,
					    12);
		g_key_file_set_string(state->state,
				      "quotes",
				      key,
				      user->quote);
		g_free(key);
		usrs = g_slist_delete_link(usrs, usrs);
	}

	data = g_key_file_to_data(state->state,
				  &len,
				  NULL);
	g_file_set_contents(state->statefile,
			    data,
			    len,
			    NULL);
	g_free(data);
}

void state_free(struct state *state)
{
	g_key_file_free(state->state);
	g_free(state->statefile);
	free(state);
}
