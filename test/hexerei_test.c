#include "unity.h"
#include "hexerei.h"

#include <string.h>

#define LIST_T char*
#define LIST_NAME tokenizer
#define LIST_POINTER_TYPE
#include "list.h"

static char assert_buf[300];
#define FMT_MSG(m,...) snprintf(assert_buf, 300, m, __VA_ARGS__)

static tokenizer_t
*tokenize(const char *in, const char *token);

void setUp(void) {}
void tearDown(void) {}

void test_hexerei_hex_readall(void)
{
	struct hex_test {
		const char *input;
		size_t record_num;
		hexerei_err_e err;
	} test_cases[] = {
		{":020000021000EC\r\n"
		 ":10C20000E0A5E6F6FDFFE0AEE00FE6FCFDFFE6FD93\r\n"
		 ":10C21000FFFFF6F50EFE4B66F2FA0CFEF2F40EFE90\r\n"
		 ":10C22000F04EF05FF06CF07DCA0050C2F086F097DF\r\n"
		 ":10C23000F04AF054BCF5204830592D02E018BB03F9\r\n"
		 ":020000020000FC\r\n"
		 ":04000000FA00000200\r\n"
		 ":00000001FF\r\n", 8, NO_ERR},
		{":020000021000EC\r\n"
		 ":10C20000E0A5E6F6FDFFE0AEE00FE6FCFDFFE6FD93\r\n"
		 ":10C21000FFFFF6F50EFE4B66F2FA0CFEF2F40EFE90\r\n"
		 ":10C22000F04EF05FF06CF07DCA0050C2F086F097DF\r\n"
		 ":10C23000F04AF054BCF5204830592D02E018BB03F9\r\n"
		 ":020000020000FC\r\n"
		 ":04000000FA00000200\r\n", 0, NO_EOF_REC_ERR},
		{":020000021000EC\r\n"
		 ":10C20000E0A5E6F6FDFFE0AEE00FE6FCFDFFE6FD93\r\n"
		 ":10C21000FFFFF6F50EFE4B66F2FA0CFEF2F40EFE90\r\n"
		 ":10C22000F04EF05FF06CF07DCA0050C2F086F097DF\r\n"
		 ":10C23000F04AF054BCF5204830592D02E018BB03F9\r\n"
		 ":020000020000FC\r\n"
		 ":00000001FF\r\n"
		 ":04000000FA00000200\r\n"
		 ":00000001FF\r\n", 0, RECORD_AFTER_EOF_ERR}
	};


	hexerei_err_e null_err = hexerei_hex_readall(NULL, NULL);
	TEST_ASSERT(null_err == NULL_INPUT_ERR);

	const char mock[] = "mock";
	FILE *fmock = fmemopen((void*)mock, strlen(mock), "r");
	null_err = hexerei_hex_readall(fmock, NULL);
	TEST_ASSERT(null_err == NULL_INPUT_ERR);
	fclose(fmock);

	int test_num = sizeof(test_cases)/sizeof(struct hex_test);
	for(int i = 0; i < test_num; i++) {
		struct hex_test test_case = test_cases[i];
		FILE *f = fmemopen((void*)test_case.input, strlen(test_case.input), "r");
		hexerei_hex_file_t *hfile;
		hexerei_err_e rerr = hexerei_hex_readall(f, &hfile);
		fclose(f);

		FMT_MSG("Failed on test case #%d, input: %s", i, test_case.input);
		TEST_ASSERT_MESSAGE(hfile != NULL, assert_buf);
		
		if(rerr != NO_ERR) {
			TEST_ASSERT_EQUAL_MESSAGE(test_case.err, rerr, assert_buf);
			continue;
		}

		TEST_ASSERT_EQUAL_INT_MESSAGE(test_case.record_num, hfile->records->length, assert_buf);
		tokenizer_t *tokens = tokenize(test_case.input, "\r\n");
		TEST_ASSERT_MESSAGE(tokens != NULL, assert_buf);

		TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(tokens->items[i], hfile->records->items[i]->data, tokens->length, assert_buf);
		tokenizer_free_all(tokens);
		hexerei_hex_free(hfile);
	}
}

void test_hexerei_hex_write_at(void)
{
	const char
		*hex_input =
		":020000021000EC\r\n"
		":10C20000E0A5E6F6FDFFE0AEE00FE6FCFDFFE6FD93\r\n"
		":10C21000FFFFF6F50EFE4B66F2FA0CFEF2F40EFE90\r\n"
		":10C22000F04EF05FF06CF07DCA0050C2F086F097DF\r\n"
		":10C23000F04AF054BCF5204830592D02E018BB03F9\r\n"
		":020000022000DC\r\n"
		":04000000FA00000200\r\n"
		":00000001FF\r\n";

	struct hex_test {
		uint32_t position;
		size_t   size;
		const uint8_t input[32];
		hexerei_err_e err;
	} test_cases[] = {
		{0x1000*16 + 0xC200, 16, {0x00, 0xEE, 0xAE, 0xBC, 0x01, 0x02, 0x03, 0x04, 0xCC, 0x05, 0x60, 0x71, 0x44, 0x12, 0xF7, 0xA1}, NO_ERR},
		{0x1000*16 + 0xC200, 3, {0xAA, 0xBD, 0x1C}, NO_ERR},
		{0x1000*16 + 0xC200, 18,{0x00, 0xEE, 0xAE, 0xBC, 0x01, 0x02, 0x03, 0x04, 0xCC, 0x05, 0x60, 0x71, 0x44, 0x12, 0xF7, 0xA1, 0xFF, 0xFD}, NO_ERR},
		{0x1000*16 + 0xC202,	16, {0x00, 0xEE, 0xAE, 0xBC, 0x01, 0x02, 0x03, 0x04, 0xCC, 0x05, 0x60, 0x71, 0x44, 0x12, 0xF7, 0xA1}, NO_ERR},
		{0x1000*16 + 0xC202, 20,	{0x00, 0xEE, 0xAE, 0xBC, 0x01, 0x02, 0x03, 0x04, 0xCC, 0x05, 0x60, 0x71, 0x44, 0x12, 0xF7, 0xA1, 0x01, 0x09, 0x21, 0x23}, NO_ERR},
		{0x2000*16 - 2, 4, {0xAA, 0xBD, 0x1C, 0x2C}, OUT_OF_BOUNDS_ERR},
		{0x2000 * 16, 6, {0xAA, 0xBD, 0x1C, 0x2C, 0x00, 0xFE}, OUT_OF_BOUNDS_ERR},
		{0x2000 * 16, 4, {0xAA, 0xBD, 0x1C, 0x2C}, NO_ERR}
	};

	hexerei_hex_file_t *hexfile = &(hexerei_hex_file_t){0};
	FILE *f = fmemopen((void*)hex_input, strlen(hex_input), "r");
	hexerei_err_e herr = hexerei_hex_readall(f, &hexfile);
	fclose(f);

	TEST_ASSERT(herr == NO_ERR && hexfile != NULL);

	int test_num = sizeof(test_cases)/sizeof(struct hex_test);
	for(int i = 0; i < test_num; i++) {
		struct hex_test test_case = test_cases[i];
		FMT_MSG("Failed on test case #%d, pos: %d, len: %ld", i, test_case.position, test_case.size);
		hexerei_err_e werr = hexerei_hex_write_at(hexfile, test_case.position,test_case.input, test_case.size);

		if(werr != NO_ERR) {
			TEST_ASSERT_EQUAL_MESSAGE(test_case.err, werr, assert_buf);
			continue;
		}

		uint8_t *read_data = calloc(test_case.size, sizeof(uint8_t));
		hexerei_err_e rerr = hexerei_hex_read_at(hexfile,test_case.position,test_case.size,read_data);
		TEST_ASSERT_MESSAGE(rerr == NO_ERR, assert_buf);

		// TODO missing record validation

		TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(test_case.input, read_data, test_case.size, assert_buf);
		free(read_data);
	}
	hexerei_hex_free(hexfile);
}

void test_hexerei_hex_read_at(void)
{
	const char
	*hex_input =
		":020000021000EC\r\n"
		":10C20000E0A5E6F6FDFFE0AEE00FE6FCFDFFE6FD93\r\n"
		":10C21000FFFFF6F50EFE4B66F2FA0CFEF2F40EFE90\r\n"
		":10C22000F04EF05FF06CF07DCA0050C2F086F097DF\r\n"
		":10C23000F04AF054BCF5204830592D02E018BB03F9\r\n"
		":020000022000DC\r\n"
		":04000000FA00000200\r\n"
		":00000001FF\r\n";

	struct hex_test {
		uint32_t position;
		size_t size;
		hexerei_err_e err;
		const uint8_t expected[50];
	} test_cases[] = {
		{0, 10, OUT_OF_BOUNDS_ERR, {}},
		{0x1000*16 + 0xC200, 16, NO_ERR, {0xE0, 0xA5, 0xE6, 0xF6, 0xFD, 0xFF, 0xE0, 0xAE, 0xE0, 0x0F, 0xE6, 0xFC, 0xFD, 0xFF, 0xE6, 0xFD}},
		{0x1000*16 + 0xC200, 14, NO_ERR, {0xE0, 0xA5, 0xE6, 0xF6, 0xFD, 0xFF, 0xE0, 0xAE, 0xE0, 0x0F, 0xE6, 0xFC, 0xFD, 0xFF}},
		{0x1000*16 + 0xC200, 18, NO_ERR, {0xE0, 0xA5, 0xE6, 0xF6, 0xFD, 0xFF, 0xE0, 0xAE, 0xE0, 0x0F, 0xE6, 0xFC, 0xFD, 0xFF, 0xE6, 0xFD, 0xFF, 0xFF}},
		{0x1000*16 + 0xC202, 16, NO_ERR, {0xE6, 0xF6, 0xFD, 0xFF, 0xE0, 0xAE, 0xE0, 0x0F, 0xE6, 0xFC, 0xFD, 0xFF, 0xE6, 0xFD, 0xFF, 0xFF}},
		{0x2000*16 - 2, 4, OUT_OF_BOUNDS_ERR, {0}},
		{0x2000*16, 6, OUT_OF_BOUNDS_ERR, {0}},
		{0x2000*16, 4, NO_ERR, {0xFA, 0x00, 0x00, 0x02}},
	};

	hexerei_hex_file_t *hexfile = &(hexerei_hex_file_t){0};
	FILE *f = fmemopen((void*)hex_input, strlen(hex_input), "r");
	hexerei_err_e herr = hexerei_hex_readall(f, &hexfile);
	fclose(f);

	TEST_ASSERT(herr == NO_ERR && hexfile != NULL);

	int test_num = sizeof(test_cases)/sizeof(struct hex_test);
	for(int i = 0; i < test_num; i++) {
		struct hex_test test_case = test_cases[i];
		FMT_MSG("Failed on test case #%d, pos: %d, len: %ld", i, test_case.position, test_case.size);
		uint8_t *read_data = calloc(test_case.size, sizeof(uint8_t));
		hexerei_err_e rerr = hexerei_hex_read_at(hexfile,test_case.position,test_case.size,read_data);

		if(rerr != NO_ERR) {
			TEST_ASSERT_EQUAL_MESSAGE(test_case.err, rerr, assert_buf);
			free(read_data);
			continue;
		}
		TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(test_case.expected, read_data, test_case.size, assert_buf);
		free(read_data);
	}
	hexerei_hex_free(hexfile);
}


static tokenizer_t
*tokenize(const char *in, const char *token)
{
	tokenizer_t *out = tokenizer_init();
	if(out == NULL) return NULL;

	const char *tok_beg = in;
	while(*in != '\0') {
		if(*in == *token) {
			const char *ins = in;
			const char *sup = token;
			while(*ins == *sup && (*ins != '\0' && *sup != '\0')) {
				ins++, sup++;
			}

			if(*sup == '\0') {
				size_t tok_size = in - tok_beg + 1;
				char *buf = malloc(sizeof(char) * (tok_size+1));
				if(buf == NULL) {
					tokenizer_free_all(out);
					return NULL;
				}

				snprintf(buf, tok_size, "%s", tok_beg);
				list_err_e err = tokenizer_append(out, buf);
				if(err != LIST_NO_ERR) {
					tokenizer_free(out);
					return NULL;
				}
				in += strlen(token);
				tok_beg = in;
				continue;
			}
		}
		in++;
	}
	return out;
}


int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_hexerei_hex_readall);
	RUN_TEST(test_hexerei_hex_write_at);
	RUN_TEST(test_hexerei_hex_read_at);
	return UNITY_END();
}

