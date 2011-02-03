#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include "user.h"

#ifdef USE_STRIDX
#include "stridx.h"
#endif

struct users
{
	GArray *users;
#ifdef USE_STRIDX
	struct stridx *cache;
#else
	GHashTable *cache;
#endif
	gsize user_size;
};

static void _regexify_strv(gchar **strv)
{
	gsize i;
	static const char *chars = "[-A-}0-9]";
	static const gsize charslen = 9;
	for (i = 0; strv[i]; i++) {
		gsize newlen = 0;
		gchar *j;
		gchar *k;
		gchar *news;

		for (j = strv[i]; *j; ++j, newlen++)
			if (*j == '|') newlen++;
			else if (*j == '*') newlen += charslen;
		news = malloc(newlen + 1);

		for (j = strv[i], k = news; *j; ++j, ++k) {
			if (*j == '|')
				*(k++) = '\\';
			else if (*j == '*') {
				memcpy(k, chars, charslen);
				k += charslen;
			}
			*k = *j;
		}
		*k = *j;

		g_free(strv[i]);
		strv[i] = news;
	}
}

struct users *users_new()
{
	struct users *users = malloc(sizeof(struct users));
#if USE_STRIDX
	users->cache = index_new();
#else
	users->cache = g_hash_table_new_full(g_str_hash,
					     g_str_equal,
					     g_free,
					     NULL);
#endif
	users->user_size = sizeof(struct user);

	return users;
}

gsize users_add_data(struct users *users, gsize size)
{
	g_debug("%u + %u", users->user_size, size);
	users->user_size += G_MEM_ALIGN - 
		((users->user_size - 1) % G_MEM_ALIGN) - 1 + size;
	g_debug("%u", users->user_size);
	return users->user_size - size;
}

void users_load(struct users *users, GKeyFile *file)
{
	gsize len;
	gchar **keys = g_key_file_get_keys(file, "users", &len, NULL);

	gsize i;
	users->users = g_array_sized_new(FALSE, TRUE,
					 users->user_size, len + 5);
	g_array_set_size(users->users, len);

	for (i = 0; i < len; i++) {
		struct user *user = &g_array_index(users->users,
						   struct user,
						   i);
		gchar **masks;
		gchar *joined;
		gchar *pattern;
		GError *error = NULL;
		user->username = keys[i];
		masks = g_key_file_get_string_list(file, "users", keys[i],
						   NULL, NULL);
		_regexify_strv(masks);
		joined = g_strjoinv("|", masks);
		pattern = g_strconcat("^(?:", joined, ")$", NULL);
		user->matcher = g_regex_new(pattern,
					    G_REGEX_CASELESS |
					    G_REGEX_DOLLAR_ENDONLY |
					    G_REGEX_OPTIMIZE,
					    0,
					    &error);
		if (!user->matcher) {
			g_warning("%s", error->message);
			g_error_free(error);
		}
#ifdef USE_STRIDX
		add_index(users->cache, keys[i], i);
#else
		g_hash_table_insert(users->cache,
				    g_strdup(keys[i]),
				    GUINT_TO_POINTER(i));
#endif
		g_free(joined);
		g_free(pattern);
		g_strfreev(masks);
	}
	g_free(keys);
}

struct user *users_get_user(struct users *users, const char *name)
{
	gsize i;
	struct user *user;

#ifdef USE_STRIDX
	if (get_index(users->cache, name, &i))
		return &g_array_index(users->users, struct user,
				      i);
#else
	gpointer idxp;
	if (g_hash_table_lookup_extended(users->cache,
					 name,
					 NULL, &idxp))
		return &g_array_index(users->users, struct user,
				      GPOINTER_TO_UINT(idxp));
#endif

	for (i = 0; i < users->users->len; i++) {
		user = &g_array_index(users->users,
				      struct user,
				      i);
		if (!user->matcher) break;
		if (g_regex_match(user->matcher, name, 0, NULL)) {
#ifdef USE_STRIDX
			add_index(users->cache, name, i);
#else
			g_hash_table_insert(users->cache,
					    g_strdup(name),
					    GUINT_TO_POINTER(i));
#endif
			return user;
		}
	}

	g_array_set_size(users->users, users->users->len + 1);
	user = &g_array_index(users->users,
			      struct user,
			      users->users->len - 1);

	user->username = g_strdup(name);
#ifdef USE_STRIDX
	add_index(users->cache, name, users->users->len - 1);
#else
	g_hash_table_insert(users->cache,
			    g_strdup(name),
			    GUINT_TO_POINTER(users->users->len - 1));
#endif

	return user;
}

void free_users(struct users *users)
{
	gsize i;
#ifdef USE_STRIDX
	index_free(users->cache);
#else
	g_hash_table_destroy(users->cache);
#endif
	for (i = 0; i < users->users->len; i++) {
		struct user *user = &g_array_index(users->users,
						   struct user,
						   i);
		g_free(user->username);
		if (user->matcher)
			g_regex_unref(user->matcher);
	}
	g_array_free(users->users, TRUE);
	g_free(users);
}

gint user_compare(gconstpointer a, gconstpointer b)
{
	const struct user *au = a;
	const struct user *bu = b;

	if (au->lines != bu->lines)
		return bu->lines - au->lines;
	if (au->words != bu->words)
		return bu->words - au->words;
	if (au->chars != bu->chars)
		return bu->chars - au->chars;
	return 0;
}

GSList *users_get_all(struct users *users)
{
	GSList *r = NULL;
	gsize i;

	for (i = 0; i < users->users->len; i++) {
		r = g_slist_insert_sorted(r,
					  &g_array_index(users->users,
							 struct user,
							 i),
					  user_compare);
	}

	return r;
}

gint user_comparer(gconstpointer a, gconstpointer b, gpointer user_data)
{
	enum user_compare_field fld = GPOINTER_TO_INT(user_data);
	const struct user *au = a;
	const struct user *bu = b;
	guint av;
	guint bv;

	switch (fld & 0x7) {
	case COMPARE_LINES:
		av = au->lines;
		bv = bu->lines;
		break;
	case COMPARE_WORDS:
		av = au->words;
		bv = bu->words;
		break;
	case COMPARE_CHARS:
		av = au->chars;
		bv = bu->chars;
		break;
	case COMPARE_QUESTIONS:
		av = au->questions;
		bv = bu->questions;
		break;
	case COMPARE_EXCLAMATIONS:
		av = au->exclamations;
		bv = bu->exclamations;
		break;
	case COMPARE_HAPPY:
		av = au->happy;
		bv = bu->happy;
		break;
	case COMPARE_SAD:
		av = au->sad;
		bv = bu->sad;
		break;
	case COMPARE_TSUP:
		av = au->tsup;
		bv = bu->tsup;
		break;
	}

	if (fld & COMPARE_RELATIVE) {
		gdouble ad = (gdouble)av / au->lines;
		gdouble bd = (gdouble)bv / bu->lines;

		if (ad != bd)
			return bd > ad ? 1 : -1;
	}

	return bv - av;
}
