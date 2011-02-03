#include <stdlib.h>

static inline unsigned char _idx(char a)
{
	if (a == '-')
		return 43;
	if (a < 'A')
		return a - '0';
	if (a < 'a')
		return a - '7';
	if (a < '{')
		return a - 'W';
	return a - 'O';
}

struct node {
	struct node *subnodes[47];
	unsigned int idx;
};

struct stridx {
	struct node root;
};

struct stridx *index_new(void)
{
	struct stridx *rv = calloc(1, sizeof(struct stridx));
	return rv;
}

int get_index(struct stridx *data, const char *str, unsigned int *idx)
{
	struct node *i = &data->root;
	while (*str && i)
		i = i->subnodes[_idx(*(str++))];

	if (!i)
		return 0;

	*idx = i->idx;
	return 1;
}

void add_index(struct stridx *data, const char *str, unsigned int idx)
{
	struct node *i = &data->root;

	while (*str) {
		unsigned char id = _idx(*(str++));
		if (i->subnodes[id]) {
			i = i->subnodes[id];
			continue;
		}
		i = i->subnodes[id] = calloc(1, sizeof(struct node));
	}

	i->idx = idx;
}

static void _node_free(struct node *data)
{
	unsigned char i;

	if (!data)
		return;

	for (i = 0; i < 47; i++)
		_node_free(data->subnodes[i]);

	free(data);
}

void index_free(struct stridx *data)
{
	unsigned char i;

	if (!data)
		return;

	for (i = 0; i < 47; i++)
		_node_free(data->root.subnodes[i]);

	free(data);
}
