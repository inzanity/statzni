#include <glib.h>
#include "user.h"

struct state;
struct state *state_new(GKeyFile *settings);
void state_load(struct state *state, struct users *users, int times[24]);
void state_save(struct state *state, struct users *users, int times[24]);
