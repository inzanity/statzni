enum action {
	ACTION_MESSAGE,
	ACTION_JOIN,
	ACTION_LEAVE,
	ACTION_QUIT,
	ACTION_TOPIC,
/*	ACTION_MODE, */
	ACTION_IGNORE
};

struct action_details {
	enum action type;
	const char *nick;
	const char *value;
	int time;
};

struct parser {
	void *(*init)();
	void (*parse_line)(void *data,
			   struct action_details *details, char *line);
	void (*deinit)(void *data);
};

struct parser *get_parser(const char *format);
