#ifndef STRIDX_H
#define STRIDX_H

struct stridx;

struct stridx *index_new(void);
void index_free(struct stridx *data);
int get_index(struct stridx *data, const char *str, unsigned int *idx);
void add_index(struct stridx *data, const char *str, unsigned int idx);

#endif /* STRIDX_H */
