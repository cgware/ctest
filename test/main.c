#include "mem_stats.h"
#include "test.h"
#include "type.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct tdata_s {
	void *priv;
	setup_fn setup;
	setup_fn teardown;
	dst_t dst;
	wdst_t wdst;
	long long passed;
	long long failed;
	int depth;
	char *buf;
	size_t buf_size;
	size_t buf_len;
	const char *exp;
	size_t exp_len;
	size_t mem;
	mem_stats_t mem_stats;
} tdata_t;

extern tdata_t t_get_data();
extern void t_set_data(tdata_t data);

#define CW "\033[0m"
#define CR "\033[0;31m"
#define CG "\033[0;32m"

TEST(t_init_finish)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();

	t_init();

	tdata_t tmp = {0};
	tmp.dst	    = DST_BUF(buf);

	t_set_data(tmp);
	EXPECT_EQ(t_finish(), 0);
	t_set_data(data);
	EXPECT_STR(buf, CG "PASS 0 TESTS" CW "\n");

	t_init();

	tmp.failed = 1;
	t_set_data(tmp);
	EXPECT_EQ(t_finish(), 1);
	t_set_data(data);
	EXPECT_STR(buf, CR "FAIL 1/1 TEST" CW "\n");

	END;
}

static int empty_test()
{
	return 1;
}

TEST(t_run)
{
	START;

	tdata_t data = t_get_data();

	EXPECT_EQ(t_run(empty_test, 0), 1);
	EXPECT_EQ(t_run(empty_test, 1), 1);

	tdata_t tmp = t_get_data();

	t_set_data(data);
	EXPECT_EQ(data.dst.putv, tmp.dst.putv);
	EXPECT_EQ(data.wdst.wputv, tmp.wdst.wputv);

	END;
}

TEST(t_priv)
{
	START;

	tdata_t data = t_get_data();

	int a = 0;
	t_set_priv(&a);
	EXPECT_EQ(t_get_priv(), &a);

	t_set_data(data);

	END;
}

static int setup(void *priv)
{
	(void)priv;
	return 0;
}

static int teardown(void *priv)
{
	(void)priv;
	return 0;
}

TEST(t_setup_teardown)
{
	START;

	tdata_t data = t_get_data();

	tdata_t tmp = {0};
	t_set_data(tmp);

	t_setup(setup);
	t_teardown(teardown);

	t_start();
	t_end(0, "", "", 0);

	t_set_data(data);

	END;
}

TEST(t_start_end)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	int res;

	tmp.dst.off = 0;
	t_set_data(tmp);
	t_start();
	res = t_end(1, "file", "test_func", 0);
	t_set_data(data);
	EXPECT_EQ(res, 0);
	EXPECT_STR(buf, "├─" CG "PASS func" CW "\n");

	END;
}

TEST(t_end_leak)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = data;
	tmp.dst	     = DST_BUF(buf);
	tmp.depth    = 1;
	tmp.mem -= 1;
	t_set_data(tmp);

	int res;

	res = t_end(1, "file", "test_func", 0);

	t_set_data(data);
	EXPECT_EQ(res, 1);
	EXPECT_STR(buf,
		   "│ ├─" CR "LEAK func" CW "\n"
		   "│ │ " CR "file:0: 1 B" CW "\n");

	END;
}

TEST(t_cstart_cend)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	int res;

	t_set_data(tmp);
	t_cstart();
	res = t_cend(1, "func");
	t_set_data(data);
	EXPECT_EQ(res, 0);
	EXPECT_STR(buf, "");

	t_set_data(tmp);
	t_cstart();
	res = t_cend(0, "func");
	t_set_data(data);
	EXPECT_EQ(res, 1);
	EXPECT_STR(buf, "");

	END;
}

TEST(t_sstart)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);
	tmp.depth    = 1;

	t_set_data(tmp);
	t_sstart("test_func");
	t_set_data(data);

	EXPECT_STR(buf, "│ ├─func\n");

	END;
}

TEST(t_send)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	int res;

	t_set_data(tmp);
	res = t_send(0, 1);
	t_set_data(data);
	EXPECT_EQ(res, 1);
	EXPECT_STR(buf, "└─" CR "FAIL 1/1 TEST" CW "\n");

	t_set_data(tmp);
	res = t_send(1, 2);
	t_set_data(data);
	EXPECT_EQ(res, 1);
	EXPECT_STR(buf, "└─" CR "FAIL 2/3 TESTS" CW "\n");

	t_set_data(tmp);
	res = t_send(1, 0);
	t_set_data(data);
	EXPECT_EQ(res, 0);
	EXPECT_STR(buf, "└─" CG "PASS 1 TEST" CW "\n");

	t_set_data(tmp);
	res = t_send(2, 0);
	t_set_data(data);
	EXPECT_EQ(res, 0);
	EXPECT_STR(buf, "└─" CG "PASS 2 TESTS" CW "\n");

	END;
}

TEST(t_expect)
{
	START;

	tdata_t tdata = t_get_data();

	t_set_dst(DST_NONE());
	t_set_wdst(WDST_NONE());

	t_expect_g(1, __FILE__, __func__, __LINE__, "", 2, "", 0, "==");
	t_expect_g(1, __FILE__, __func__, __LINE__, "", 4, "", 3, "==");
	t_expect_g(1, __FILE__, __func__, __LINE__, "", 8, "", 7, "==");
	t_expect_g(1, __FILE__, __func__, __LINE__, "", 7, "", 7, "==");

	t_set_data(tdata);

	END;
}

TEST(t_set_print)
{
	START;

	tdata_t tdata = t_get_data();

	t_set_dst(DST_NONE());
	t_set_wdst(WDST_NONE());

	t_set_data(tdata);
	_passed = 1;

	END;
}

TEST(t_finish)
{
	START;

	tdata_t tdata = t_get_data();
	tdata_t tmp   = tdata;

	tmp.dst	 = DST_NONE();
	tmp.wdst = WDST_NONE();

	tmp.failed = 1;
	tmp.buf	   = malloc(tmp.buf_size);
	t_set_data(tmp);
	t_finish();

	tmp.failed = 0;
	tmp.buf	   = malloc(tmp.buf_size);
	t_set_data(tmp);
	t_finish();

	t_set_data(tdata);

	END;
}

TEST(t_scan)
{
	START;

	unsigned int u = 0;

	EXPECT_EQ(t_scan("123", "%3u", &u), 1);
	EXPECT_EQ(t_scan("aaa", "%3u", &u), 0);

	END;
}

TEST(t_strcmp)
{
	START;
	EXPECT_EQ(t_strcmp(NULL, NULL), 0);
	EXPECT_EQ(t_strcmp("", NULL), 1);
	EXPECT_EQ(t_strcmp(NULL, ""), 1);
	EXPECT_EQ(t_strcmp("", ""), 0);
	EXPECT_EQ(t_strcmp("", " "), 1);
	EXPECT_EQ(t_strcmp(" ", ""), 1);
	EXPECT_EQ(t_strcmp("a", "a"), 0);
	EXPECT_EQ(t_strcmp("a", "b"), 1);
	EXPECT_EQ(t_strcmp("b", "a"), 1);

	END;
}

TEST(t_strncmp)
{
	START;
	EXPECT_EQ(t_strncmp(NULL, NULL, 0), 0);
	EXPECT_EQ(t_strncmp(NULL, NULL, 1), 1);
	EXPECT_EQ(t_strncmp("", NULL, 0), 1);
	EXPECT_EQ(t_strncmp(NULL, "", 0), 1);
	EXPECT_EQ(t_strncmp("", "", 0), 0);
	EXPECT_EQ(t_strncmp("", " ", 1), 1);
	EXPECT_EQ(t_strncmp(" ", "", 0), 0);
	EXPECT_EQ(t_strncmp("a", "a", 1), 0);
	EXPECT_EQ(t_strncmp("a", "b", 1), 1);
	EXPECT_EQ(t_strncmp("b", "a", 1), 1);

	END;
}

TEST(t_wstrcmp)
{
	START;
	EXPECT_EQ(t_wstrcmp(NULL, NULL), 0);
	EXPECT_EQ(t_wstrcmp(L"", NULL), 1);
	EXPECT_EQ(t_wstrcmp(NULL, L""), 1);
	EXPECT_EQ(t_wstrcmp(L"", L""), 0);
	EXPECT_EQ(t_wstrcmp(L"", L" "), 1);
	EXPECT_EQ(t_wstrcmp(L" ", L""), 1);
	EXPECT_EQ(t_wstrcmp(L"a", L"a"), 0);
	EXPECT_EQ(t_wstrcmp(L"a", L"b"), 1);
	EXPECT_EQ(t_wstrcmp(L"b", L"a"), 1);

	END;
}

TEST(t_wstrncmp)
{
	START;
	EXPECT_EQ(t_wstrncmp(NULL, NULL, 0), 0);
	EXPECT_EQ(t_wstrncmp(NULL, NULL, 1), 1);
	EXPECT_EQ(t_wstrncmp(L"", NULL, 0), 1);
	EXPECT_EQ(t_wstrncmp(NULL, L"", 0), 1);
	EXPECT_EQ(t_wstrncmp(L"", L"", 0), 0);
	EXPECT_EQ(t_wstrncmp(L"", L" ", 1), 1);
	EXPECT_EQ(t_wstrncmp(L" ", L"", 0), 0);
	EXPECT_EQ(t_wstrncmp(L"a", L"a", 1), 0);
	EXPECT_EQ(t_wstrncmp(L"a", L"b", 1), 1);
	EXPECT_EQ(t_wstrncmp(L"b", L"a", 1), 1);

	END;
}

TEST(check)
{
	SSTART;
	RUN(t_scan);
	RUN(t_strcmp);
	RUN(t_strncmp);
	RUN(t_wstrcmp);
	RUN(t_wstrncmp);
	SEND;
}

TEST(t_expect_ch)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_ch(0, NULL, NULL, 0, "check");
	t_set_data(data);
	EXPECT_STR(buf, "│ " CR "check" CW "\n");
	END;
}

TEST(t_expect_g)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_g(0, NULL, NULL, 0, NULL, 0, NULL, 0, "==", 0, 1);
	t_set_data(data);
	EXPECT_STR(buf, "│ " CR "(null) == (null) (0 == 1)" CW "\n");

	t_set_data(tmp);
	t_expect_g(0, NULL, NULL, 0, NULL, 1, NULL, 1, "==", U8_MIN, U8_MAX);
	t_set_data(data);
	EXPECT_STR(buf, "│ " CR "(null) == (null) (00000000 == 11111111)" CW "\n");

	t_set_data(tmp);
	t_expect_g(0, NULL, NULL, 0, NULL, 2, NULL, 2, "==", U16_MIN, U16_MAX);
	t_set_data(data);
	EXPECT_STR(buf, "│ " CR "(null) == (null) (0000 == FFFF)" CW "\n");

	t_set_data(tmp);
	t_expect_g(0, NULL, NULL, 0, NULL, 3, NULL, 3, "==", 0, 0);
	t_set_data(data);
	EXPECT_STR(buf, "│ " CR "(null) == (null) (Unsupported type of size: 3)" CW "\n");

	t_set_data(tmp);
	t_expect_g(0, NULL, NULL, 0, NULL, 4, NULL, 4, "==", U32_MIN, U32_MAX);
	t_set_data(data);
	EXPECT_STR(buf, "│ " CR "(null) == (null) (00000000 == FFFFFFFF)" CW "\n");

	t_set_data(tmp);
	t_expect_g(0, NULL, NULL, 0, NULL, 8, NULL, 8, "==", U64_MIN, U64_MAX);
	t_set_data(data);
	EXPECT_STR(buf, "│ " CR "(null) == (null) (0000000000000000 == FFFFFFFFFFFFFFFF)" CW "\n");

	END;
}

TEST(t_expect_m)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_m(0, NULL, NULL, 0, NULL, 0, NULL, 0, "==", 0, 0, 1);
	t_set_data(data);
	EXPECT_STR(buf, "│ " CR "(null) == (null) (0 == 1) & 00000000" CW "\n");

	END;
}

TEST(t_expect_str_null)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_str(0, NULL, NULL, 0, NULL, NULL);
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: " CW "\n"
		   "│ " CR "act:0: " CW "\n"
		   "│ " CR "       ^" CW "\n");
	END;
}

TEST(t_expect_str_passed)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);
	tmp.depth    = 1;

	t_set_data(tmp);
	t_expect_str(1, "file", "test_func", 0, NULL, NULL);
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ ├─" CR "FAIL func" CW "\n"
		   "│ │ " CR "file:0: " CW "\n"
		   "│ │ " CR "exp:0: " CW "\n"
		   "│ │ " CR "act:0: " CW "\n"
		   "│ │ " CR "       ^" CW "\n");
	END;
}

TEST(t_expect_str_empty)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_str(0, NULL, NULL, 0, "", "");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: " CW "\n"
		   "│ " CR "act:0: " CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_str_same)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_str(0, NULL, NULL, 0, "a", "a");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: a" CW "\n"
		   "│ " CR "act:0: a" CW "\n"
		   "│ " CR "        ^" CW "\n");

	END;
}

TEST(t_expect_str_same_nl)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_str(0, NULL, NULL, 0, "\n", "\n");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:1: " CW "\n"
		   "│ " CR "act:1: " CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_str_diff)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_str(0, NULL, NULL, 0, "a", "b");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: b" CW "\n"
		   "│ " CR "act:0: a" CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_str_diff_nl)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_str(0, NULL, NULL, 0, "a\na", "b\na");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: b\\n" CW "\n"
		   "│ " CR "act:0: a\\n" CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_str_diff_not_print)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_str(0, NULL, NULL, 0, "\r\t\033a", "\r\t\033b");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: \\r\\t\\033b" CW "\n"
		   "│ " CR "act:0: \\r\\t\\033a" CW "\n"
		   "│ " CR "               ^" CW "\n");

	END;
}

TEST(t_expect_str_exp_nl)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_str(0, NULL, NULL, 0, "a", "b\na");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: b\\n" CW "\n"
		   "│ " CR "act:0: a" CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_str_act_nl)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_str(0, NULL, NULL, 0, "b\na", "a");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: a" CW "\n"
		   "│ " CR "act:0: b\\n" CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_strn_null)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_strn(0, NULL, NULL, 0, NULL, NULL, 0);
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: " CW "\n"
		   "│ " CR "act:0: " CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_fmt_null)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	unsigned int u = 0;

	t_set_data(tmp);
	t_expect_fmt(0, NULL, NULL, 0, "%u", 1, &u);
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: " CW "\n"
		   "│ " CR "act:0: %u" CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_str)
{
	SSTART;
	RUN(t_expect_str_null);
	RUN(t_expect_str_passed);
	RUN(t_expect_str_empty);
	RUN(t_expect_str_same);
	RUN(t_expect_str_same_nl);
	RUN(t_expect_str_diff);
	RUN(t_expect_str_diff_nl);
	RUN(t_expect_str_diff_not_print);
	RUN(t_expect_str_exp_nl);
	RUN(t_expect_str_act_nl);
	RUN(t_expect_strn_null);
	RUN(t_expect_fmt_null);
	SEND;
}

TEST(t_expect_wstr_null)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_wstr(0, NULL, NULL, 0, NULL, NULL);
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: " CW "\n"
		   "│ " CR "act:0: " CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_wstr_passed)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_wstr(1, "file", "test_func", 0, NULL, NULL);
	t_set_data(data);
	EXPECT_STR(buf,
		   "├─" CR "FAIL func" CW "\n"
		   "│ " CR "file:0: " CW "\n"
		   "│ " CR "exp:0: " CW "\n"
		   "│ " CR "act:0: " CW "\n"
		   "│ " CR "       ^" CW "\n");
	END;
}

TEST(t_expect_wstr_empty)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_wstr(0, NULL, NULL, 0, L"", L"");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: " CW "\n"
		   "│ " CR "act:0: " CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_wstr_same)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_wstr(0, NULL, NULL, 0, L"a", L"a");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: " CW "\n"
		   "│ " CR "act:0: " CW "\n"
		   "│ " CR "        ^" CW "\n");

	END;
}

TEST(t_expect_wstr_same_nl)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_wstr(0, NULL, NULL, 0, L"\n", L"\n");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:1: " CW "\n"
		   "│ " CR "act:1: " CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_wstr_diff)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_wstr(0, NULL, NULL, 0, L"a", L"b");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: " CW "\n"
		   "│ " CR "act:0: " CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_wstr_diff_nl)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_wstr(0, NULL, NULL, 0, L"a\na", L"b\na");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: " CW "\n"
		   "│ " CR "act:0: " CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_wstr_diff_not_print)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_wstr(0, NULL, NULL, 0, L"\r\t\033a", L"\r\t\033b");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: " CW "\n"
		   "│ " CR "act:0: " CW "\n"
		   "│ " CR "               ^" CW "\n");

	END;
}

TEST(t_expect_wstr_exp_nl)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_wstr(0, NULL, NULL, 0, L"a", L"\n");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: " CW "\n"
		   "│ " CR "act:0: " CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_wstr_act_nl)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_wstr(0, NULL, NULL, 0, L"\n", L"a");
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: " CW "\n"
		   "│ " CR "act:0: " CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_wstrn_null)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_wstrn(0, NULL, NULL, 0, NULL, NULL, 0);
	t_set_data(data);
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: " CW "\n"
		   "│ " CR "act:0: " CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(t_expect_wstr)
{
	SSTART;
	RUN(t_expect_wstr_null);
	RUN(t_expect_wstr_passed);
	RUN(t_expect_wstr_empty);
	RUN(t_expect_wstr_same);
	RUN(t_expect_wstr_same_nl);
	RUN(t_expect_wstr_diff);
	RUN(t_expect_wstr_diff_nl);
	RUN(t_expect_wstr_diff_not_print);
	RUN(t_expect_wstr_exp_nl);
	RUN(t_expect_wstr_act_nl);
	RUN(t_expect_wstrn_null);
	SEND;
}

TEST(t_expect_fail)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);

	t_set_data(tmp);
	t_expect_fail(0, "FAIL");
	t_set_data(data);
	EXPECT_STR(buf, "│ " CR "FAIL" CW "\n");

	END;
}

TEST(t_expect_fstr)
{
	START;

	char buf[1024] = {0};

	tdata_t data = t_get_data();
	tdata_t tmp  = {0};
	tmp.dst	     = DST_BUF(buf);
	tmp.buf	     = malloc(1);
	tmp.buf_size = 1;
	tmp.buf_len  = 0;

	t_set_data(tmp);
	t_expect_fstr_start("aa", 2);
	EXPECT_EQ(t_fprintf(NULL, "%s", "bb"), 2);
	int res = t_expect_fstr_end(0, NULL, NULL, 0);

	tmp = t_get_data();
	free(tmp.buf);

	t_set_data(data);
	EXPECT_EQ(res, 1)
	EXPECT_STR(buf,
		   "│ " CR CW "\n"
		   "│ " CR "exp:0: aa" CW "\n"
		   "│ " CR "act:0: bb" CW "\n"
		   "│ " CR "       ^" CW "\n");

	END;
}

TEST(expect)
{
	SSTART;
	RUN(t_expect_ch);
	RUN(t_expect_g);
	RUN(t_expect_m);
	RUN(t_expect_str);
	RUN(t_expect_wstr);
	RUN(t_expect_fail);
	RUN(t_expect_fstr);
	SEND;
}

TEST(ctest)
{
	SSTART;

	RUN(t_init_finish);
	RUN(t_run);
	RUN(t_priv);
	RUN(t_setup_teardown);
	RUN(t_start_end);
	RUN(t_end_leak);
	RUN(t_cstart_cend);
	RUN(t_sstart);
	RUN(t_send);
	RUN(t_expect);
	RUN(t_set_print);
	RUN(t_send);
	RUN(t_finish);
	RUN(check);
	RUN(expect);

	SEND;
}

int main()
{
	c_print_init();
	t_init();
	t_run(test_ctest, 1);
	return t_finish();
}
