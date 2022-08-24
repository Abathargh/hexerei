#ifndef HEXEREI_RECORD_H
#define HEXEREI_RECORD_H

#include "hexerei.h"

hexerei_err_e
hexerei_record_parse(FILE *f, hexerei_record_t *rec);

hexerei_err_e
hexerei_record_write(hexerei_record_t *record,
										 int start,
										 const char *data,
										 size_t length);

hexerei_err_e
hexerei_record_read(hexerei_record_t *record,
                    char *output,
                    size_t length);

#endif
