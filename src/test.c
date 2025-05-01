#include "test.h"

#include "mem_stats.h"
#include "platform.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#if defined(C_WIN)
	#define vsscanf vsscanf_s
#endif

#define BYTE_TO_BIN_PATTERN "%c%c%c%c%c%c%c%c"

#define TEST_PREFIX "test_"

// clang-format off
#define BYTE_TO_BIN(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')
// clang-format on

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

static tdata_t s_data;

tdata_t t_get_data()
{
	return s_data;
}

void t_set_data(tdata_t data)
{
	s_data = data;
}

static size_t t_printv(const char *fmt, va_list args)
{
	size_t off = s_data.dst.off;
	s_data.dst.off += dputv(s_data.dst, fmt, args);
	return s_data.dst.off - off;
}

static size_t t_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	size_t ret = t_printv(fmt, args);
	va_end(args);
	return ret;
}

static size_t t_wprintv(const wchar_t *fmt, va_list args)
{
	size_t off = s_data.wdst.off;
	s_data.wdst.off += wdputv(s_data.wdst, fmt, args);
	return s_data.wdst.off - off;
}

static size_t t_wprintf(const wchar_t *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	size_t ret = t_wprintv(fmt, args);
	va_end(args);
	return ret;
}

void t_set_priv(void *priv)
{
	s_data.priv = priv;
}

void t_setup(setup_fn setup)
{
	s_data.setup = setup;
}

void t_teardown(teardown_fn teardown)
{
	s_data.teardown = teardown;
}

dst_t t_set_dst(dst_t dst)
{
	dst_t cur  = s_data.dst;
	s_data.dst = dst;
	return cur;
}

wdst_t t_set_wdst(wdst_t dst)
{
	wdst_t cur  = s_data.wdst;
	s_data.wdst = dst;
	return cur;
}

void *t_get_priv()
{
	return s_data.priv;
}

static inline int pur()
{
	t_printf("└─");
	return 2;
}

static inline int pv()
{
	t_printf("│ ");
	return 2;
}

static inline int pvr()
{
	t_printf("├─");
	return 2;
}

void t_init()
{
	s_data.dst  = DST_STD();
	s_data.wdst = WDST_STD();

	s_data.passed = 0;
	s_data.failed = 0;
	s_data.depth  = -1;

	s_data.buf_size = 256;
	s_data.buf_len	= 0;

	s_data.buf = malloc(s_data.buf_size);

	mem_stats_set(&s_data.mem_stats);
}

int t_finish()
{
	if (s_data.failed == 0) {
		t_printf("\033[0;32mPASS %llu %s\033[0m\n", s_data.passed, s_data.passed == 1 ? "TEST" : "TESTS");
	} else {
		t_printf("\033[0;31mFAIL %llu/%llu %s\033[0m\n",
			 s_data.failed,
			 s_data.failed + s_data.passed,
			 s_data.failed == 1 ? "TEST" : "TESTS");
	}

	free(s_data.buf);

	return (int)s_data.failed;
}

int t_run(test_fn fn, int print)
{
	dst_t dst   = {0};
	wdst_t wdst = {0};

	if (print == 0) {
		dst  = t_set_dst(DST_NONE());
		wdst = t_set_wdst(WDST_NONE());
	}

	int ret = fn();

	if (print == 0) {
		t_set_dst(dst);
		t_set_wdst(wdst);
	}

	return ret;
}

void t_start()
{
	s_data.mem = s_data.mem_stats.mem;

	if (s_data.setup) {
		s_data.setup(s_data.priv);
	}
}

int t_end(int passed, const char *file, const char *func, int line)
{
	if (s_data.teardown) {
		s_data.teardown(s_data.priv);
	}

	if (!passed) {
		s_data.failed++;
		return 1;
	}

	for (int i = 0; i < s_data.depth; i++) {
		pv();
	}
	pvr();

	if (s_data.mem != s_data.mem_stats.mem) {
		t_printf("\033[0;31mLEAK %s\033[0m\n", func + sizeof(TEST_PREFIX) - 1);

		for (int i = 0; i < s_data.depth; i++) {
			pv();
		}
		pv();

		t_printf("\033[0;31m%s:%d: %d B\033[0m\n", file, line, s_data.mem_stats.mem - s_data.mem);

		s_data.failed++;
		return 1;
	}

	t_printf("\033[0;32mPASS %s\033[0m\n", func + sizeof(TEST_PREFIX) - 1);

	s_data.passed++;
	return 0;
}

void t_cstart()
{
}

int t_cend(int passed, const char *func)
{
	(void)func;

	if (!passed) {
		s_data.failed++;
		return 1;
	}

	s_data.passed++;
	return 0;
}

void t_sstart(const char *func)
{
	for (int i = 0; i < s_data.depth; i++) {
		pv();
	}
	if (s_data.depth >= 0) {
		pvr();
	}

	t_printf("%s\n", func + sizeof(TEST_PREFIX) - 1);
	s_data.depth++;
}

int t_send(int passed, int failed)
{
	for (int i = 0; i < s_data.depth; i++) {
		pv();
	}
	pur();
	if (failed == 0) {
		t_printf("\033[0;32mPASS %d %s\033[0m\n", passed, passed == 1 ? "TEST" : "TESTS");
	} else {
		t_printf("\033[0;31mFAIL %d/%d %s\033[0m\n", failed, failed + passed, failed == 1 ? "TEST" : "TESTS");
	}
	s_data.depth--;
	return failed > 0;
}

int t_scan(const char *str, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	const int ret = vsscanf(str, fmt, args);
	va_end(args);
	return ret;
}

static size_t t_strlen(const char *str)
{
	size_t len = 0;
	while (*str++) {
		len++;
	}
	return len;
}

int t_strcmp(const char *act, const char *exp)
{
	if (act == NULL && exp == NULL) {
		return 0;
	}
	if (act == NULL || exp == NULL) {
		return 1;
	}

	size_t act_len = t_strlen(act);
	size_t exp_len = t_strlen(exp);

	if (act_len != exp_len) {
		return 1;
	}

	return memcmp(act, exp, exp_len) == 0 ? 0 : 1;
}

int t_strncmp(const char *act, const char *exp, size_t len)
{
	if (act == NULL && exp == NULL && len == 0) {
		return 0;
	}
	if (act == NULL || exp == NULL || t_strlen(exp) != len) {
		return 1;
	}

	return memcmp(act, exp, len) == 0 ? 0 : 1;
}

int t_wstrcmp(const wchar_t *act, const wchar_t *exp)
{
	if (act == NULL && exp == NULL) {
		return 0;
	}
	if (act == NULL || exp == NULL) {
		return 1;
	}

	size_t act_len = wcslen(act);
	size_t exp_len = wcslen(exp);

	if (act_len != exp_len) {
		return 1;
	}

	return memcmp(act, exp, exp_len) == 0 ? 0 : 1;
}

int t_wstrncmp(const wchar_t *act, const wchar_t *exp, size_t len)
{
	if (act == NULL && exp == NULL && len == 0) {
		return 0;
	}
	if (act == NULL || exp == NULL || wcslen(exp) != len) {
		return 1;
	}
	return memcmp(act, exp, len) == 0 ? 0 : 1;
}

static void print_header(int passed, const char *file, const char *func, int line)
{
	if (passed && func) {
		int len = 0;
		for (int i = 0; i < s_data.depth; i++) {
			len += pv();
		}
		len += pvr();
		t_printf("\033[0;31mFAIL %s\033[0m\n", func + sizeof(TEST_PREFIX) - 1);
	}

	for (int i = 0; i < s_data.depth; i++) {
		pv();
	}
	pv();

	t_printf("\033[0;31m");

	if (file == NULL) {
		return;
	}

	t_printf("%s:%d: ", file, line);
}

// clang-format off
#define get_char(_size, _args) \
	(char)va_arg(_args, int)

#define get_short(_size, _args)                  \
	_size == 1 ? (short)va_arg(_args, int) : \
	_size == 2 ? (short)va_arg(_args, int) : \
		     0

#define get_int(_size, _args)                  \
	_size == 1 ? (int)va_arg(_args, int) : \
	_size == 2 ? (int)va_arg(_args, int) : \
	_size == 4 ? (int)va_arg(_args, int) : \
		     0
// clang-format on

#define get_long(_size, _args)                                                                                                             \
	_size == 1   ? (long long)va_arg(_args, int)                                                                                       \
	: _size == 2 ? (long long)va_arg(_args, int)                                                                                       \
	: _size == 4 ? (long long)va_arg(_args, int)                                                                                       \
	: _size == 8 ? (long long)va_arg(_args, long long)                                                                                 \
		     : 0

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void print_values(int passed, const char *file, const char *func, int line, const char *act, size_t act_size, const char *exp,
			 size_t exp_size, const char *cond, va_list args)
{
	print_header(passed, file, func, line);
	t_printf("%s %s %s (", act, cond, exp);

	int max_size = MAX((int)act_size, (int)exp_size);

	switch (max_size) {
	case 0: {
		const unsigned char a = get_char(act_size, args);
		const unsigned char b = get_char(exp_size, args);

		t_printf("%c %s %c", a ? '1' : '0', cond, b ? '1' : '0');
		break;
	}
	case 1: {
		const unsigned char a = get_char(act_size, args);
		const unsigned char b = get_char(exp_size, args);

		t_printf(BYTE_TO_BIN_PATTERN " %s " BYTE_TO_BIN_PATTERN, BYTE_TO_BIN(a), cond, BYTE_TO_BIN(b));
		break;
	}
	case 2: {
		const unsigned short a = get_short(act_size, args);
		const unsigned short b = get_short(exp_size, args);

		t_printf("%04X %s %04X", a, cond, b);
		break;
	}
	case 4: {
		const int a = get_int(act_size, args);
		const int b = get_int(exp_size, args);

		t_printf("%08X %s %08X", a, cond, b);
		break;
	}
	case 8: {
		const long long a = get_long(act_size, args);
		const long long b = get_long(exp_size, args);

		t_printf("%016llX %s %016llX", a, cond, b);
		break;
	}
	default:
		t_printf("Unsupported type of size: %zu", act_size);
		break;
	}

	t_printf(")");
}

void t_expect_ch(int passed, const char *file, const char *func, int line, const char *check)
{
	print_header(passed, file, func, line);
	t_printf("%s\033[0m\n", check);
}

void t_expect_g(int passed, const char *file, const char *func, int line, const char *act, size_t act_size, const char *exp,
		size_t exp_size, const char *cond, ...)
{
	va_list args;
	va_start(args, cond);
	print_values(passed, file, func, line, act, act_size, exp, exp_size, cond, args);
	va_end(args);

	t_printf("\033[0m\n");
}

void t_expect_m(int passed, const char *file, const char *func, int line, const char *act, size_t act_size, const char *exp,
		size_t exp_size, const char *cond, unsigned char mask, ...)
{
	va_list args;
	va_start(args, mask);
	print_values(passed, file, func, line, act, act_size, exp, exp_size, cond, args);
	va_end(args);

	t_printf(" & " BYTE_TO_BIN_PATTERN "\033[0m\n", BYTE_TO_BIN(mask));
}

static int print_line(int passed, const char *h, const char *str, size_t ln, size_t col, size_t line_start, size_t line_end, size_t *h_len)
{
	print_header(passed, NULL, NULL, 0);

	int app = 0;
	*h_len	= t_printf("%s:%zu: ", h, ln);
	for (size_t i = 0; i < line_end - line_start; i++) {
		char c = str[line_start + i];
		// clang-format off
		switch (c) {
		case '\n': t_printf("\\n"); app += (i <= col ? 1 : 0); break;
		case '\r': t_printf("\\r"); app += (i <= col ? 1 : 0); break;
		case '\t': t_printf("\\t"); app += (i <= col ? 1 : 0); break;
		case '\033': t_printf("\\033"); app += (i <= col ? 3 : 0); break;
		default: t_printf("%c", c); break;
		}
		// clang-format on
	}

	t_printf("\033[0m\n");

	return app;
}

static void print_str(int passed, const char *file, const char *func, int line, const char *act_str, const char *exp_str, size_t act_len,
		      size_t exp_len)
{
	size_t ln	    = 0;
	size_t col	    = 0;
	size_t line_start   = 0;
	size_t exp_line_end = 0;
	size_t act_line_end = 0;
	int diff	    = 0;

	for (size_t i = 0; i < MAX(exp_len, act_len); i++) {
		char exp = i < exp_len ? exp_str[i] : '\0';
		char act = i < act_len ? act_str[i] : '\0';

		diff |= act != exp;

		if (diff) {
			if (exp == '\n' && exp_line_end == 0) {
				exp_line_end = i + 1;
			}
			if (act == '\n' && act_line_end == 0) {
				act_line_end = i + 1;
			}
			if (exp_line_end != 0 && act_line_end != 0) {
				break;
			}
		} else if (exp == '\n') {
			col = 0;
			ln++;
			line_start = i + 1;
		} else {
			col++;
		}
	}

	print_header(passed, file, func, line);
	t_printf("\033[0m\n");

	size_t h_len;
	int exp_app = print_line(passed, "exp", exp_str, ln, col, line_start, exp_line_end == 0 ? exp_len : exp_line_end, &h_len);
	int act_app = print_line(passed, "act", act_str, ln, col, line_start, act_line_end == 0 ? act_len : act_line_end, &h_len);

	print_header(passed, NULL, NULL, 0);
	t_printf("%*s^\033[0m\n", (int)h_len + MIN(act_app, exp_app) + col, "");
}

static int print_wline(int passed, const char *h, const wchar_t *str, size_t ln, size_t col, size_t line_start, size_t line_end,
		       size_t *h_len)
{
	print_header(passed, NULL, NULL, 0);

	int app = 0;
	*h_len	= t_printf("%s:%zu: ", h, ln);
	c_startw(stdout);
	for (size_t i = 0; i < line_end - line_start; i++) {
		wchar_t c = str[line_start + i];
		// clang-format off
		switch (c) {
		case L'\n': t_wprintf(L"\\n"); app += (i <= col ? 1 : 0); break;
		case L'\r': t_wprintf(L"\\r"); app += (i <= col ? 1 : 0); break;
		case L'\t': t_wprintf(L"\\t"); app += (i <= col ? 1 : 0); break;
		case L'\033': t_wprintf(L"\\033"); app += (i <= col ? 3 : 0); break;
		default: t_wprintf(L"%c", c); break;
		}
		// clang-format on
	}
	c_endw(stdout);

	t_printf("\033[0m\n");

	return app;
}

static void print_wstr(int passed, const char *file, const char *func, int line, const wchar_t *act_str, const wchar_t *exp_str,
		       size_t act_len, size_t exp_len)
{
	size_t ln	    = 0;
	size_t col	    = 0;
	size_t line_start   = 0;
	size_t exp_line_end = 0;
	size_t act_line_end = 0;
	int diff	    = 0;

	for (size_t i = 0; i < MAX(exp_len, act_len); i++) {
		wchar_t exp = i < exp_len ? exp_str[i] : '\0';
		wchar_t act = i < act_len ? act_str[i] : '\0';

		diff |= act != exp;

		if (diff) {
			if (exp == '\n' && exp_line_end == 0) {
				exp_line_end = i + 1;
			}
			if (act == '\n' && act_line_end == 0) {
				act_line_end = i + 1;
			}
			if (exp_line_end != 0 && act_line_end != 0) {
				break;
			}
		} else if (exp == '\n') {
			col = 0;
			ln++;
			line_start = i + 1;
		} else {
			col++;
		}
	}

	print_header(passed, file, func, line);
	t_printf("\033[0m\n");

	size_t h_len;
	int exp_app = print_wline(passed, "exp", exp_str, ln, col, line_start, exp_line_end == 0 ? exp_len : exp_line_end, &h_len);
	int act_app = print_wline(passed, "act", act_str, ln, col, line_start, act_line_end == 0 ? act_len : act_line_end, &h_len);

	print_header(passed, NULL, NULL, 0);
	t_printf("%*s^\033[0m\n", (int)h_len + MIN(act_app, exp_app) + col, "");
}

void t_expect_str(int passed, const char *file, const char *func, int line, const char *act, const char *exp)
{
	print_str(passed, file, func, line, act, exp, act == NULL ? 0 : t_strlen(act), exp == NULL ? 0 : t_strlen(exp));
}

void t_expect_strn(int passed, const char *file, const char *func, int line, const char *act, const char *exp, size_t len)
{
	print_str(passed, file, func, line, act, exp, MIN(len, act == NULL ? 0 : t_strlen(act)), exp == NULL ? 0 : t_strlen(exp));
}

void t_expect_fmt(int passed, const char *file, const char *func, int line, const char *act, unsigned int cnt, ...)
{
	va_list args;
	va_start(args, cnt);
	const char *exp = va_arg(args, const char *);
	va_end(args);

	print_str(passed, file, func, line, act, exp, t_strlen(act), t_strlen(exp));
}

void t_expect_wstr(int passed, const char *file, const char *func, int line, const wchar_t *act, const wchar_t *exp)
{
	print_wstr(passed, file, func, line, act, exp, act == NULL ? 0 : wcslen(act), exp == NULL ? 0 : wcslen(exp));
}

void t_expect_wstrn(int passed, const char *file, const char *func, int line, const wchar_t *act, const wchar_t *exp, size_t len)
{
	print_wstr(passed, file, func, line, act, exp, MIN(len, act == NULL ? 0 : wcslen(act)), exp == NULL ? 0 : wcslen(exp));
}

void t_expect_fail(int passed, const char *fmt, ...)
{
	print_header(passed, NULL, NULL, 0);

	va_list args;
	va_start(args, fmt);
	t_printv(fmt, args);
	va_end(args);

	t_printf("\033[0m\n");
}

int t_fprintf(void *priv, const char *fmt, ...)
{
	(void)priv;

	va_list args;
	va_start(args, fmt);
	int ret = vsnprintf(s_data.buf + s_data.buf_len, s_data.buf_size - s_data.buf_len, fmt, args);
	va_end(args);

	s_data.buf_len += ret;

	return ret;
}

void t_expect_fstr_start(const char *exp, size_t len)
{
	s_data.exp     = exp;
	s_data.exp_len = len;

	if (s_data.buf_size < len + 1) {
		void *buf = realloc(s_data.buf, len + 1);
		if (buf != NULL) {
			s_data.buf_size = len + 1;
			s_data.buf	= buf;
		}
	}

	memset(s_data.buf, 0, s_data.buf_size);
	s_data.buf_len = 0;
}

int t_expect_fstr_end(int passed, const char *file, const char *func, int line)
{
	const int ret = t_strcmp(s_data.buf, s_data.exp);

	if (ret) {
		print_str(passed, file, func, line, s_data.buf, s_data.exp, s_data.buf_len, s_data.exp_len);
	}

	return ret;
}
