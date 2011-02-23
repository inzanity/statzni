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
#include <stdio.h>
#include <stdlib.h>
#include "output.h"
#include "user.h"

struct formatter
{
	FILE *output;
};

struct formatter *formatter_new(GKeyFile *settings)
{
	gchar *path;
	struct formatter *fmt;
	FILE *output;

	path = g_key_file_get_string(settings, "settings", "output", NULL);

	if (!path)
		output = stdout;
	else {
		output = fopen(path, "wb");
		g_free(path);
	}

	if (!output)
		return NULL;
	fmt = malloc(sizeof(struct formatter));
	fmt->output = output;

	return fmt;
}

static const char *classes[] = {
	"night",
	"morning",
	"day",
	"evening"
};

void formatter_timeline(struct formatter *fmt, int times[24])
{
	int total = 0;
	int max = 0;
	int i;

	for (i = 0; i < 24; i++) {
		total += times[i];

		if (times[i] > max)
			max = times[i];
	}

	fprintf(fmt->output, "<div id=\"timeline\">\n");
	for (i = 0; i < 24; i++)
		fprintf(fmt->output,
			" <div class=\"%s\" style=\"height: %d%%\"><span>%.1f%%</span></div>\n",
			classes[i / 6],
			times[i] * 100 / max,
			times[i] * 100.0 / total);
	fprintf(fmt->output, "</div>\n");
}

void formatter_users(struct formatter *fmt, GSList *users)
{
	int n = 1;

	fprintf(fmt->output, "<ol id=\"userlist\">\n");
	for (; users && n <= 25; users = users->next, n++) {
		gchar *quote;
		int i;
		struct user *user = users->data;
		if (user->lines == 0) {
			n--;
			continue;
		}
		fprintf(fmt->output, "<li><span class=\"nick\">%s</span><span class=\"lines\">%d</span><span class=\"act\">",
			user->username,
			user->lines);

		for (i = 0; i < 4; i++)
			fprintf(fmt->output, "<div class=\"%s\" style=\"width: %d%%\"><span>%.1f%%</span></div>",
				classes[i],
				user->timelines[i] * 100 / user->lines,
				user->timelines[i] * 100.0 / user->lines);
		fprintf(fmt->output, "</span><span class=\"words\">%d</span><span class=\"wpl\">%.2f</span><span class=\"cpw\">%.2f</span>",
			user->words,
			(double)user->words / user->lines,
			(double)user->chars / user->words);
		quote = g_markup_escape_text(user->quote, -1);
		fprintf(fmt->output, "<span class=\"quote\">%s</span></li>\n",
			quote);
		g_free(quote);
	}
	fprintf(fmt->output, "</ol>");
}

void formatter_trivia(struct formatter *fmt, GSList *users)
{
	struct user *user;
	if (!users)
		return;

	fprintf(fmt->output, "<ol id=\"trivia\">\n");

	users = g_slist_sort_with_data(users, user_comparer,
				       GINT_TO_POINTER(COMPARE_QUESTIONS |
						       COMPARE_RELATIVE));
	user = users->data;
	fprintf(fmt->output, "<p>%s kyseli julmetusti, %.2f%% ajasta.</p>",
		user->username, (gdouble)user->questions * 100 / user->lines);

	users = g_slist_sort_with_data(users, user_comparer,
				       GINT_TO_POINTER(COMPARE_EXCLAMATIONS |
						       COMPARE_RELATIVE));
	user = users->data;
	fprintf(fmt->output,
		"<p>%s huusi kuin viimeistä päivää, %.1f%% ajasta.</p>",
		user->username,
		(gdouble)user->exclamations * 100 / user->lines);

	users = g_slist_sort_with_data(users, user_comparer,
				       GINT_TO_POINTER(COMPARE_TSUP));
	user = users->data;
	fprintf(fmt->output,
		"<p>%s *tsup*paili eniten, %u kertaa.</p>",
		user->username,
		user->tsup);

	fprintf(fmt->output, "</div>");
}

void formatter_free(struct formatter *fmt)
{
	if (fmt->output != stdout)
		fclose(fmt->output);
	free(fmt);
}
