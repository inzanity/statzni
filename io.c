#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>
#include <bzlib.h>
#include <glib.h>
#include <string.h>
#include <lzma.h>
#include "io.h"

typedef void *(*ioopen)(const char *path);
typedef int (*ioclose)(void *data);
typedef int (*ioread)(void *data, void *buf, size_t len);
typedef int (*ioeof)(void *data);
typedef void (*ioseek)(void *data, unsigned int pos);

struct io_module {
	const char *extension;
	ioopen open;
	ioclose close;
	ioread read;
	ioeof eof;
	ioseek seek;
};

static void *_gzopen(const char *path)
{
	return gzopen(path, "rb");
}

static void _gzseek(void *data, unsigned int pos)
{
	gzseek(data, pos, SEEK_CUR);
}

struct BzData
{
	FILE *file;
	BZFILE *bzFile;
	int error;
};

static void *_bzopen(const char *path)
{
	struct BzData *data = malloc(sizeof(struct BzData));

	data->file = fopen(path, "rb");
	data->bzFile = BZ2_bzReadOpen(&data->error, data->file, 0, 0, NULL, 0);

	if (data->error == BZ_OK)
		return data;

	if (data->file)
		fclose(data->file);
	free(data);
	return NULL;
}

static int _bzclose(void *p)
{
	struct BzData *data = p;
	BZ2_bzReadClose(&data->error, data->bzFile);

	fclose(data->file);
	free(data);

	return 0;
}

static int _bzread(void *p, void *buf, size_t len)
{
	struct BzData *data = p;

	return BZ2_bzRead(&data->error, data->bzFile, buf, (int)len);
}

static void _bzseek(void *p, unsigned int pos)
{
	struct BzData *data = p;
	char buffer[2048];

	while (pos && data->error != BZ_STREAM_END)
		pos -= BZ2_bzRead(&data->error, data->bzFile,
				  buffer, MIN(pos, 2048));
}

static int _bzeof(void *p)
{
	struct BzData *data = p;

	return data->error == BZ_STREAM_END;
}

struct LzmaData {
	FILE *file;
	lzma_stream strm;
	unsigned char bufin[2048];
	unsigned char bufout[2048];
};
static lzma_stream lzma_stream_init = LZMA_STREAM_INIT;

static void *_lzopen(const char *path)
{
	struct LzmaData *data = malloc(sizeof(struct LzmaData));
	lzma_ret r;
	data->strm = lzma_stream_init;
	data->file = fopen(path, "rb");

	r = lzma_auto_decoder(&data->strm, -1, 0);

	data->strm.avail_out = 2048;
	data->strm.next_out = data->bufout;

	return data;
}

static int _lzread(void *data, void *buf, size_t len)
{
	struct LzmaData *lz = data;

	for (;;) {
		if (lz->strm.avail_out < 2048) {
			if ((2048 - lz->strm.avail_out) < len)
				len = (2048 - lz->strm.avail_out);
			memcpy(buf, lz->strm.next_out, len);
			lz->strm.avail_out += len;
			lz->strm.next_out += len;

			return len;
		}

		if (lz->strm.avail_in) {
			lzma_ret r;
			lz->strm.next_out = lz->bufout;
			lz->strm.avail_out = 2048;
			r = lzma_code(&lz->strm, LZMA_RUN);
			lz->strm.next_out = lz->bufout;
			continue;
		}

		if (feof(lz->file))
			return 0;

		lz->strm.avail_in = fread(lz->bufin, 1, 2048, lz->file);
		lz->strm.next_in = lz->bufin;
	}
}

static void _lzseek(void *data, unsigned int pos)
{
	struct LzmaData *lz = data;
	unsigned int r = 0;

	for (; r + (2048 - lz->strm.avail_out) < pos;) {
		r += 2048 - lz->strm.avail_out;

		if (lz->strm.avail_in) {
			lzma_ret r;
			lz->strm.next_out = lz->bufout;
			lz->strm.avail_out = 2048;
			r = lzma_code(&lz->strm, LZMA_RUN);
			lz->strm.next_out = lz->bufout;
			continue;
		}

		if (feof(lz->file))
			return;

		lz->strm.avail_in = fread(lz->bufin, 1, 2048, lz->file);
		lz->strm.next_in = lz->bufin;
	}

	lz->strm.avail_out += pos - r;
	lz->strm.next_out += pos - r;
}

static int _lzeof(void *data)
{
	struct LzmaData *lz = data;
	return lz->strm.avail_out == 2048 && !lz->strm.avail_in &&
		feof(lz->file);
}

static int _lzclose(void *data)
{
	struct LzmaData *lz = data;
	lzma_end(&lz->strm);
	fclose(lz->file);
	free(data);

	return 0;
}

static void *_fopen(const char *path)
{
	return fopen(path, "rb");
}

static int _fread(void *data, void *buf, size_t len)
{
	return fread(buf, 1, len, data);
}

static void _fseek(void *data, unsigned int pos)
{
	fseek(data, pos, SEEK_CUR);
}

static struct io_module modules[] = {
	{ ".gz", _gzopen, gzclose, gzread, gzeof, _gzseek },
	{ ".bz2", _bzopen, _bzclose, _bzread, _bzeof, _bzseek },
	{ ".lzma", _lzopen, _lzclose, _lzread, _lzeof, _lzseek },
	{ ".xz", _lzopen, _lzclose, _lzread, _lzeof, _lzseek },
	{ "", _fopen, (ioclose)fclose, _fread, (ioeof)feof, _fseek },
};

struct iofile
{
	char buffer[2048];
	unsigned int buffer_used;
	unsigned int buffer_consumed;
	unsigned int module_idx;
	unsigned int position;
	void *module_data;
	char buftemp;
};

struct iofile *io_open(const char *filename)
{
	struct iofile *file = malloc(sizeof(struct iofile));
	file->buffer_used = 0;
	file->buffer_consumed = 0;
	for (file->module_idx = 0;; file->module_idx++) {
		if (g_str_has_suffix(filename,
				     modules[file->module_idx].extension))
			break;
	}
	file->module_data = modules[file->module_idx].open(filename);

	return file;
}

char *io_getline(struct iofile *file)
{
	char *nl;

	if (file->buffer_consumed) {
		memmove(file->buffer, file->buffer + file->buffer_consumed,
			file->buffer_used - file->buffer_consumed);
		file->buffer_used -= file->buffer_consumed;
		file->buffer_consumed = 0;
	}

	nl = memchr(file->buffer, '\n', file->buffer_used);
	if (nl) {
		*nl = '\0';
		file->buffer_consumed = nl - file->buffer + 1;

		file->position += nl - file->buffer + 1;

		return file->buffer;
	}

	while (!modules[file->module_idx].eof(file->module_data) &&
	       file->buffer_used < sizeof(file->buffer)) {
		file->buffer_used += modules[file->module_idx].read(
				file->module_data,
				file->buffer + file->buffer_used,
				sizeof(file->buffer) - file->buffer_used);

		nl = memchr(file->buffer, '\n', file->buffer_used);
		if (nl) {
			nl = memchr(file->buffer, '\n', file->buffer_used);
			*nl = '\0';
			file->buffer_consumed = nl - file->buffer + 1;
			file->position += nl - file->buffer + 1;

			return file->buffer;
		}
	}

	return NULL;
}

void io_close(struct iofile *file)
{
	modules[file->module_idx].close(file->module_data);
	free(file);
}

unsigned int io_tell(struct iofile *file)
{
	return file->position;
}

void io_seek(struct iofile *file, unsigned int pos)
{
	modules[file->module_idx].seek(file->module_data, pos);
	file->position = pos;
}
