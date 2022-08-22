/**
 *

APPENDIX: How to apply the Apache License to your work

To apply the Apache License to your work, attach the following boilerplate notice, with the fields enclosed by brackets "[]" replaced with your own identifying information. (Don't include the brackets!) The text should be enclosed in the appropriate comment syntax for the file format. We also recommend that a file or class name and description of purpose be included on the same "printed page" as the copyright notice for easier identification within third-party archives.

Copyright 2022 G. Marcello

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
 */
#ifndef HEXEREI_HEXEREI_H
#define HEXEREI_HEXEREI_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef enum{
  NO_MORE_RECORDS_ERR,
  NULL_INPUT_ERR,
  MISSING_START_CODE_ERR,
  WRONG_RECORD_FMT_ERR,
  INVALID_HEX_DIGIT,
  NO_ERR
} hexerei_err_e ;

typedef enum {
    DATA_REC,
    EOF_REC,
    EXTENDED_SEGMENT_ADDRESS_REC,
    START_SEGMENT_ADDRESS_REC,
    EXTENDED_LINEAR_ADDRESS_REC,
    START_LINEAR_ADDRESS_REC,
    INVALID_REC,
} hex_record_type_e;

typedef struct {
    size_t length;
    hex_record_type_e type;
    char data[32];
} hex_record_t;

typedef struct {
    size_t length;
    hex_record_t *records;
} hex_file_t;

hex_file_t     *hexerei_hex_readall(const char* fn);
hexerei_err_e  hexerei_hex_read_at(hex_file_t *hf, uint32_t pos, size_t size);

#endif
