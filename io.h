#ifndef IO_H
# define IO_H
struct iofile;
struct iofile *io_open(const char *filename);
char *io_getline(struct iofile *file);
void io_close(struct iofile *file);
unsigned int io_tell(struct iofile *file);
void io_seek(struct iofile *file, unsigned int pos);

#endif /* IO_H */
