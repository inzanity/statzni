#ifndef OUTPUT_H
#define OUTPUT_H

#include <glib.h>

struct formatter;

struct formatter *formatter_new(GKeyFile *settings);
void formatter_timeline(struct formatter *fmt, int times[24]);
void formatter_users(struct formatter *fmt, GSList *users);

#endif /* OUTPUT_H */
