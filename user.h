#ifndef USER_H
#define USER_H
#include <glib.h>

struct users;
struct user
{
	char *username;
	char quote[2048];
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

struct users *users_new(GKeyFile *file);
struct user *users_get_user(struct users *users, const char *user);
void free_users(struct users *users);
gint user_compare(gconstpointer a, gconstpointer b);
GSList *users_get_all(struct users *users);

#endif /* USER_H */
