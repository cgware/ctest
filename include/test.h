#ifndef TEST_H
#define TEST_H

#include "dst.h"
#include "print.h"
#include "wdst.h"
#include "wprint.h"

void t_init(void);
int t_finish(void);

typedef int (*test_fn)(void);
int t_run(test_fn fn, int print);

typedef int (*setup_fn)(void *priv);
typedef int (*teardown_fn)(void *priv);

void t_set_priv(void *priv);
void t_setup(setup_fn setup);
void t_teardown(teardown_fn teardown);

dst_t t_set_dst(dst_t dst);
wdst_t t_set_wdst(wdst_t dst);

void *t_get_priv(void);

void t_start(void);
int t_end(int passed, const char *file, const char *func, int line);

void t_cstart(void);
int t_cend(int passed, const char *func);

void t_sstart(const char *func);
int t_send(int passed, int failed);

int t_scan(const char *str, const char *fmt, ...);
int t_strcmp(const char *act, const char *exp);
int t_strncmp(const char *act, const char *exp, size_t len);

int t_wstrcmp(const wchar_t *act, const wchar_t *exp);
int t_wstrncmp(const wchar_t *act, const wchar_t *exp, size_t len);

void t_expect_ch(int passed, const char *file, const char *func, int line, const char *check);

void t_expect_g(int passed, const char *file, const char *func, int line, const char *act, size_t act_size, const char *exp,
		size_t exp_size, const char *cond, ...);
void t_expect_m(int passed, const char *file, const char *func, int line, const char *act, size_t act_size, const char *exp,
		size_t exp_size, unsigned char mask, const char *cond, ...);
void t_expect_p(int passed, const char *file, const char *func, int line, const char *act, const char *exp, const char *cond,
		const void *act_ptr, const void *exp_ptr);

void t_expect_str(int passed, const char *file, const char *func, int line, const char *act, const char *exp);
void t_expect_strn(int passed, const char *file, const char *func, int line, const char *act, const char *exp, size_t len);
void t_expect_fmt(int passed, const char *file, const char *func, int line, const char *act, unsigned int cnt, ...);

void t_expect_wstr(int passed, const char *file, const char *func, int line, const wchar_t *act, const wchar_t *exp);
void t_expect_wstrn(int passed, const char *file, const char *func, int line, const wchar_t *act, const wchar_t *exp, size_t len);

void t_expect_fail(int passed, const char *fmt, ...);

int t_fprintf(void *priv, const char *fmt, ...);
void t_expect_fstr_start(const char *exp, size_t len);
int t_expect_fstr_end(int passed, const char *file, const char *func, int line);

// Declare subtest
#define STEST(_name)	   int test_##_name(void)
#define STESTP(_name, ...) int test_##_name(__VA_ARGS__)

// Declare test
#define TEST(_name)	  static inline int test_##_name(void)
#define TESTP(_name, ...) static inline int test_##_name(__VA_ARGS__)

// Test start
#define START                                                                                                                              \
	int _passed = 1;                                                                                                                   \
	t_start()

// Test result
#define RES t_end(_passed, __FILE__, __func__, __LINE__)

// Test end
#define END return RES

// Callback start
#define CSTART                                                                                                                             \
	int _passed = 1;                                                                                                                   \
	t_cstart();

// Callback end
#define CEND t_cend(_passed, __func__)

// Subtests start
#define SSTART                                                                                                                             \
	int _spassed = 0;                                                                                                                  \
	int _sfailed = 0;                                                                                                                  \
	t_sstart(__func__)

// Run test
#define RUN(_fn)                                                                                                                           \
	if (test_##_fn()) {                                                                                                                \
		_sfailed++;                                                                                                                \
	} else {                                                                                                                           \
		_spassed++;                                                                                                                \
	}

#define RUNP(_fn, ...)                                                                                                                     \
	if (test_##_fn(__VA_ARGS__)) {                                                                                                     \
		_sfailed++;                                                                                                                \
	} else {                                                                                                                           \
		_spassed++;                                                                                                                \
	}

// Subtests end
#define SEND return t_send(_spassed, _sfailed)

#define EXPECT(_check)                                                                                                                     \
	if (!(_check)) {                                                                                                                   \
		t_expect_ch(_passed, __FILE__, __func__, __LINE__, #_check);                                                               \
		_passed = 0;                                                                                                               \
	}

#define T_EXPECT_G(_actual, _expected, _cond, _failed)                                                                                     \
	do {                                                                                                                               \
		__extension__ __typeof__(1 ? (_actual) : (_actual)) _t_actual	    = (_actual);                                           \
		__extension__ __typeof__(1 ? (_expected) : (_expected)) _t_expected = (_expected);                                         \
		if (_failed) {                                                                                                             \
			t_expect_g(_passed,                                                                                                \
				   __FILE__,                                                                                               \
				   __func__,                                                                                               \
				   __LINE__,                                                                                               \
				   #_actual,                                                                                               \
				   sizeof(_t_actual),                                                                                      \
				   #_expected,                                                                                             \
				   sizeof(_t_expected),                                                                                    \
				   _cond,                                                                                                  \
				   _t_actual,                                                                                              \
				   _t_expected);                                                                                           \
			_passed = 0;                                                                                                       \
		}                                                                                                                          \
	} while (0)

#define T_EXPECT_M(_actual, _expected, _mask, _cond, _failed)                                                                              \
	do {                                                                                                                               \
		__extension__ __typeof__(1 ? (_actual) : (_actual)) _t_actual	    = (_actual);                                           \
		__extension__ __typeof__(1 ? (_expected) : (_expected)) _t_expected = (_expected);                                         \
		__extension__ __typeof__(1 ? (_mask) : (_mask)) _t_mask		    = (_mask);                                             \
		if (_failed) {                                                                                                             \
			t_expect_m(_passed,                                                                                                \
				   __FILE__,                                                                                               \
				   __func__,                                                                                               \
				   __LINE__,                                                                                               \
				   #_actual,                                                                                               \
				   sizeof(_t_actual),                                                                                      \
				   #_expected,                                                                                             \
				   sizeof(_t_expected),                                                                                    \
				   _t_mask,                                                                                                \
				   _cond,                                                                                                  \
				   _t_actual,                                                                                              \
				   _t_expected);                                                                                           \
			_passed = 0;                                                                                                       \
		}                                                                                                                          \
	} while (0)

#define EXPECT_PTR(_actual, _expected)                                                                                                     \
	do {                                                                                                                               \
		const void *_t_actual	= (_actual);                                                                                       \
		const void *_t_expected = (_expected);                                                                                     \
		if (_t_actual != _t_expected) {                                                                                            \
			t_expect_p(_passed, __FILE__, __func__, __LINE__, #_actual, #_expected, "==", _t_actual, _t_expected);             \
			_passed = 0;                                                                                                       \
		}                                                                                                                          \
	} while (0)

#define EXPECT_PTR_NE(_actual, _expected)                                                                                                  \
	do {                                                                                                                               \
		const void *_t_actual	= (_actual);                                                                                       \
		const void *_t_expected = (_expected);                                                                                     \
		if (_t_actual == _t_expected) {                                                                                            \
			t_expect_p(_passed, __FILE__, __func__, __LINE__, #_actual, #_expected, "!=", _t_actual, _t_expected);             \
			_passed = 0;                                                                                                       \
		}                                                                                                                          \
	} while (0)

#define EXPECT_EQ(_actual, _expected)	      T_EXPECT_G(_actual, _expected, "==", _t_actual != _t_expected)
#define EXPECT_EQM(_actual, _expected, _mask) T_EXPECT_M(_actual, _expected, _mask, "==", ((_t_actual) ^ (_t_expected)) & (_t_mask))
#define EXPECT_NE(_actual, _expected)	      T_EXPECT_G(_actual, _expected, "!=", _t_actual == _t_expected)
#define EXPECT_NEM(_actual, _expected, _mask) T_EXPECT_M(_actual, _expected, _mask, "!=", !(((_t_actual) ^ (_t_expected)) & (_t_mask)))
#define EXPECT_NULL(_actual)		      EXPECT_PTR(_actual, NULL)
#define EXPECT_NOT_NULL(_actual)	      EXPECT_PTR_NE(_actual, NULL)
#define EXPECT_GT(_actual, _expected)	      T_EXPECT_G(_actual, _expected, "> ", _t_actual <= _t_expected)
#define EXPECT_GE(_actual, _expected)	      T_EXPECT_G(_actual, _expected, ">=", _t_actual < _t_expected)
#define EXPECT_LT(_actual, _expected)	      T_EXPECT_G(_actual, _expected, "< ", _t_actual >= _t_expected)
#define EXPECT_LE(_actual, _expected)	      T_EXPECT_G(_actual, _expected, "<=", _t_actual > _t_expected)

#define EXPECT_EQB(_actual, _expected)                                                                                                     \
	if ((_actual) != (_expected)) {                                                                                                    \
		t_expect_g(_passed, __FILE__, __func__, __LINE__, #_actual, 0, #_expected, 0, "==", _actual, _expected);                   \
		_passed = 0;                                                                                                               \
	}

#define EXPECT_NEB(_actual, _expected)                                                                                                     \
	if ((_actual) == (_expected)) {                                                                                                    \
		t_expect_g(_passed, __FILE__, __func__, __LINE__, #_actual, 0, #_expected, 0, "!=", _actual, _expected);                   \
		_passed = 0;                                                                                                               \
	}

#define EXPECT_STR(_actual, _expected)                                                                                                     \
	if (t_strcmp(_actual, _expected)) {                                                                                                \
		t_expect_str(_passed, __FILE__, __func__, __LINE__, _actual, _expected);                                                   \
		_passed = 0;                                                                                                               \
	}

#define EXPECT_STRN(_actual, _expected, _len)                                                                                              \
	if (t_strncmp(_actual, _expected, _len)) {                                                                                         \
		t_expect_strn(_passed, __FILE__, __func__, __LINE__, _actual, _expected, _len);                                            \
		_passed = 0;                                                                                                               \
	}

#define EXPECT_FMT(_str, _cnt, ...)                                                                                                        \
	if ((_cnt) != t_scan(_str, __VA_ARGS__)) {                                                                                         \
		t_expect_fmt(_passed, __FILE__, __func__, __LINE__, _str, _cnt, __VA_ARGS__);                                              \
		_passed = 0;                                                                                                               \
	}

#define EXPECT_WSTR(_actual, _expected)                                                                                                    \
	if (t_wstrcmp(_actual, _expected)) {                                                                                               \
		t_expect_wstr(_passed, __FILE__, __func__, __LINE__, _actual, _expected);                                                  \
		_passed = 0;                                                                                                               \
	}

#define EXPECT_WSTRN(_actual, _expected, _len)                                                                                             \
	if (t_wstrncmp(_actual, _expected, _len)) {                                                                                        \
		t_expect_wstrn(_passed, __FILE__, __func__, __LINE__, _actual, _expected, _len);                                           \
		_passed = 0;                                                                                                               \
	}

#define EXPECT_FAIL(_fmt, ...)                                                                                                             \
	t_expect_fail(_passed, _fmt, __VA_ARGS__);                                                                                         \
	_passed = 0;

#define EXPECT_FSTR(_print, _expected, _len)                                                                                               \
	t_expect_fstr_start(_expected, _len);                                                                                              \
	_print;                                                                                                                            \
	if (t_expect_fstr_end(_passed, __FILE__, __func__, __LINE__) != 0) {                                                               \
		_passed = 0;                                                                                                               \
	}

#endif
