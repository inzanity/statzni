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

#ifndef PARSER_H
#define PARSER_H
#define MINUTE_SHIFT 0
#define MINUTE_MASK 0x3f
#define HOUR_SHIFT 6
#define HOUR_MASK 0x7d0
#define DAY_SHIFT 11
#define DAY_MASK 0xf800
#define MONTH_SHIFT 16
#define MONTH_MASK 0xf0000
#define YEAR_SHIFT 20
#define YEAR_MASK 0xfff00000

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
	unsigned int time;
};

struct parser {
	void *(*init)(void);
	void (*parse_line)(void *data,
			   struct action_details *details, char *line);
	void (*deinit)(void *data);
};

struct parser *get_parser(const char *format);

#endif /* PARSER_H */
