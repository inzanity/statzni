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
