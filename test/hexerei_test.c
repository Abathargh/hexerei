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

	for(size_t i = 0; i < sizeof(test_cases)/sizeof(struct hex_test); i++) {
		struct hex_test test_case = test_cases[i];
		FILE *f = fmemopen((void*)test_case.input, strlen(test_case.input), "r");
		hexerei_hex_file_t *hfile;
		hexerei_err_e rerr = hexerei_hex_readall(f, &hfile);
		fclose(f);

		FMT_MSG("Failed on test case #%ld, input: %s", i, test_case.input);
		TEST_ASSERT_MESSAGE(hfile != NULL, assert_buf);
		
		if(rerr != NO_ERR) {
			TEST_ASSERT_EQUAL_MESSAGE(test_case.err, rerr, assert_buf);
			continue;
		}

		TEST_ASSERT_EQUAL_INT_MESSAGE(test_case.record_num, hfile->records->length, assert_buf);
		tokenizer_t *tokens = tokenize(test_case.input, "\r\n");
		TEST_ASSERT_MESSAGE(tokens != NULL, assert_buf);

		TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(tokens->items[i], hfile->records->items[i]->data, tokens->length, assert_buf);
		tokenizer_free(tokens);
		hexerei_hex_free(hfile);
	}
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
					tokenizer_free(out);
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
	return UNITY_END();
}

