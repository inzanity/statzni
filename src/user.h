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

#ifndef USER_H
#define USER_H
#include <glib.h>

#define USER_GET_DATA(x, o) ((void *)((char *)(x) + (o)))

struct users;
struct user
{
	char quote[2048];
	char *username;
	GRegex *matcher;
	unsigned int lines;
	unsigned int timelines[4];
	unsigned int words;
	unsigned int chars;
	unsigned int questions;
	unsigned int exclamations;
	unsigned int happy;
	unsigned int sad;
	unsigned int tsup;
};

enum user_compare_field
{
	COMPARE_LINES = 0,
	COMPARE_WORDS,
	COMPARE_CHARS,
	COMPARE_QUESTIONS,
	COMPARE_EXCLAMATIONS,
	COMPARE_HAPPY,
	COMPARE_SAD,
	COMPARE_TSUP,
	COMPARE_ABSOLUTE = 0,
	COMPARE_RELATIVE = 8
};

struct users *users_new(void);
gsize users_add_data(struct users *users, gsize size);
void users_load(struct users *users, GKeyFile *file);
struct user *users_get_user(struct users *users, const char *user);
void free_users(struct users *users);
gint user_compare(gconstpointer a, gconstpointer b);
GSList *users_get_all(struct users *users);
gint user_comparer(gconstpointer a, gconstpointer b, gpointer user_data);

#endif /* USER_H */
