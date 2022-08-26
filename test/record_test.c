#include <string.h>

#include "unity.h"
#include "hexerei.h"

static char assert_buf[100];
#define FMT_MSG(m,...) snprintf(assert_buf, 100, m, __VA_ARGS__);

void setUp(void) {}
void tearDown(void) {}

void test_hexerei_record_parse(void)
{
	struct record_test {
		const char *input;
		hexerei_err_e err;
		hexerei_record_t *rec;
	}	test_cases[] = {
		{"", NO_MORE_RECORDS_ERR, NULL},
		{"\r\n", MISSING_START_CODE_ERR, NULL},
		{":\r\n", WRONG_RECORD_FMT_ERR, NULL},
		{":\r", WRONG_RECORD_FMT_ERR, NULL},
		{":\n", WRONG_RECORD_FMT_ERR, NULL},
		{":0001\r\n", WRONG_RECORD_FMT_ERR, NULL},
		{":00\r\n", WRONG_RECORD_FMT_ERR, NULL},
		{":020000021000EC\r\n", NO_ERR, &(hexerei_record_t) {
			15,
			EXT_SEG_ADDR_REC,
			{':', '0', '2', '0', '0', '0', '0', '0', '2', '1', '0', '0', '0', 'E', 'C'}}
		},
		{":0400000300000000F9\r\n", NO_ERR, &(hexerei_record_t){
			19,
			START_SEG_ADDR_REC,
			{':', '0', '4', '0', '0', '0', '0', '0', '3', '0', '0', '0', '0', '0', '0', '0', '0', 'F', '9'}}
		},
		{":06058000000A000000006B\r\n", NO_ERR, &(hexerei_record_t){
			23,
			DATA_REC,
			{':', '0', '6', '0', '5', '8', '0', '0', '0', '0', '0', '0', 'A', '0', '0', '0', '0', '0', '0', '0', '0', '6', 'B'}}
		},
		{":00000001FF\r\n", NO_ERR, &(hexerei_record_t){
			11,
			EOF_REC,
			{':', '0', '0', '0', '0', '0', '0', '0', '1', 'F', 'F'}}
		}
	};

	hexerei_err_e nerr = hexerei_record_parse(NULL, NULL);
	TEST_ASSERT(nerr == NULL_INPUT_ERR); 

	const char *mstr = "mock";
	FILE *mf = fmemopen((void*)mstr, strlen(mstr), "r");
	hexerei_err_e merr = hexerei_record_parse(mf, NULL);
	fclose(mf);
	TEST_ASSERT(merr == NULL_INPUT_ERR);

	int test_num = sizeof(test_cases)/sizeof(struct record_test);
	for(int i = 0; i < test_num; i++) {
		struct record_test test_case = test_cases[i];
		FILE *record_str = fmemopen((void*)test_case.input, strlen(test_case.input), "r");
		hexerei_record_t *record;
		hexerei_err_e err = hexerei_record_parse(record_str, &record);
		fclose(record_str);
		if(err != NO_ERR) {
			TEST_ASSERT_EQUAL_MESSAGE(test_case.err, err, test_case.input);
			continue;
		}
		TEST_ASSERT_EQUAL_INT(test_case.rec->length, record->length);
		TEST_ASSERT_MESSAGE(test_case.rec->length <= 32, "Max length = 32");
		TEST_ASSERT_EQUAL_CHAR_ARRAY(test_case.rec->data, record->data, test_case.rec->length);
		free(record);
	}
}

void test_hexerei_record_write(void)
{
	struct record_test {
		const char *input;
		const char wdata[32];
		int start;
		hexerei_err_e err;
		const char   exp[32];
	} test_cases[] = {
		{":020000021000EC\r\n", {}, -1, OUT_OF_BOUNDS_ERR, {}},
		{":020000021000EC\r\n", {}, 5, OUT_OF_BOUNDS_ERR, {}},
		{":020000021000EC\r\n", {'\n', ':'}, 0, INVALID_HEX_DIGIT, {}},
		{":020000021000EC\r\n", {'1', '0', '0', '0', '0'}, 0, OUT_OF_BOUNDS_ERR, {}},
		{":020000021000EC\r\n", {'3', '4', '5', '6'}, 0, NO_ERR, {':', '0', '2', '0', '0', '0', '0', '0', '2', '3', '4', '5', '6', '7', '2'}},
		{":06058000000A000000006B\r\n", {'A', 'E'}, 2, NO_ERR, {':', '0', '6', '0', '5', '8', '0', '0', '0', '0', '0', 'A', 'E', '0', '0', '0', '0', '0', '0', '0', '0', 'C', '7'}},
		{":00000001FF\r\n", {}, 0, NO_ERR,{':', '0', '0', '0', '0', '0', '0', '0', '1',  'F', 'F'}},
	};

	hexerei_err_e n_err = hexerei_record_write(NULL, 0, NULL, 0);
	TEST_ASSERT_EQUAL_MESSAGE(OUT_OF_BOUNDS_ERR, n_err, "Expected out of bounds on null rec");

	hexerei_record_t empty = {0};
	hexerei_err_e e_err = hexerei_record_write(&empty, 0, NULL, 0);
	TEST_ASSERT_EQUAL_MESSAGE(OUT_OF_BOUNDS_ERR, e_err, "Expected out of bounds on non-init rec");

	int test_num = sizeof(test_cases)/sizeof(struct record_test);
	for(int i = 0; i < test_num; i++) {
		struct record_test test_case = test_cases[i];
		FILE *record_str = fmemopen((void*)test_case.input, strlen(test_case.input), "r");
		hexerei_record_t *record;
		hexerei_err_e err = hexerei_record_parse(record_str, &record);
		fclose(record_str);

		TEST_ASSERT_MESSAGE(err == NO_ERR, "Unexpected parsing error in write tests");

		hexerei_err_e werr = hexerei_record_write(
			record,
			test_case.start,
			test_case.wdata,
			strlen(test_case.wdata)
		);

		FMT_MSG("Failed on test case #%d, input: %s", i, test_case.input);
		if(werr != NO_ERR) {
			TEST_ASSERT_EQUAL_MESSAGE(test_case.err, werr, assert_buf);
			free(record);
			continue;
		}
		TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(test_case.exp, record->data, record->length, assert_buf);
		free(record);
	}
}

void test_hexerei_record_read(void)
{
	struct record_test {
		const char *input;
		const char *exp;
	} test_cases[] = {
		{":020000021000EC\r\n", "1000"},
		{":06058000000A000000006B\r\n", "000A00000000"},
		{":00000001FF\r\n", ""}
	};

	hexerei_err_e n_err = hexerei_record_read(NULL, NULL, 0);
	TEST_ASSERT_MESSAGE(n_err == OUT_OF_BOUNDS_ERR, "Null record should err on out of bounds");

	hexerei_record_t *empty = &(hexerei_record_t){0};
	hexerei_err_e nbuf_err = hexerei_record_read(empty, NULL, 0);
	TEST_ASSERT_MESSAGE(nbuf_err == OUT_OF_BOUNDS_ERR, "Empty record should err on out of bounds");

	int test_num = sizeof(test_cases)/sizeof(struct record_test);
	for(int i = 0; i < test_num; i++) {
		struct record_test test_case = test_cases[i];
		FILE *record_str = fmemopen((void*)test_case.input, strlen(test_case.input), "r");
		hexerei_record_t *record;
		hexerei_err_e err = hexerei_record_parse(record_str, &record);

		TEST_ASSERT_MESSAGE(err == NO_ERR, "Unexpected parsing error in write tests");
		fclose(record_str);
		
		char buf[50];
		memset(buf, 0, sizeof(buf) / sizeof(char));
		hexerei_err_e rerr = hexerei_record_read(record, buf, sizeof(buf) / sizeof(char));
		FMT_MSG("Failed on test case #%d, input: %s", i, test_case.input);
		TEST_ASSERT_MESSAGE(rerr == NO_ERR, assert_buf);
		TEST_ASSERT_EQUAL_STRING_MESSAGE(test_case.exp, buf, assert_buf);
		free(record);
	}
}

int main (void) {
	UNITY_BEGIN();
	RUN_TEST(test_hexerei_record_parse);
	RUN_TEST(test_hexerei_record_write);
	RUN_TEST(test_hexerei_record_read);
	return UNITY_END();
}
