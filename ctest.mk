CTEST_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CTEST_SRC := $(CTEST_DIR)src/
CTEST_C := $(wildcard $(CTEST_SRC)*.c)
CTEST_H := $(wildcard $(CTEST_SRC)*.h)
CTEST_HEADERS := $(wildcard $(CTEST_DIR)include/*.h)
CTEST_HEADERS += $(CBASE_HEADERS)

CTEST_OUTDIR := $(BUILDDIR)bin/$(ARCH)-$(CONFIG)/ctest
CTEST_INTDIR := $(CTEST_OUTDIR)/int/
CTEST_OBJ := $(patsubst $(CTEST_SRC)%.c,$(CTEST_INTDIR)%.o,$(CTEST_C))
CTEST_GCDA := $(patsubst %.o,%.gcda,$(CTEST_OBJ))

CTEST_INCLUDES := -I$(CTEST_DIR)include/ $(CBASE_INCLUDES)

CTEST := $(CTEST_OUTDIR)/ctest.a

.PHONY: ctest
ctest: $(CTEST)

$(CTEST): $(CTEST_OBJ)
	@mkdir -p $(@D)
	@ar rcs $@ $(CTEST_OBJ)

$(CTEST_INTDIR)%.o: $(CTEST_SRC)%.c $(CTEST_H) $(CTEST_HEADERS)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c -I$(CTEST_SRC) $(CTEST_INCLUDES) $(CFLAGS) -o $@ $<

CTEST_TEST_SRC := $(CTEST_DIR)test/
CTEST_TEST_C := $(wildcard $(CTEST_TEST_SRC)*.c)
CTEST_TEST_H := $(wildcard $(CTEST_TEST_SRC)*.h)
CTEST_TEST_HEADERS := $(CTEST_HEADERS)

CTEST_TEST_OUTDIR := $(BUILDDIR)bin/$(ARCH)-$(CONFIG)/ctest_test
CTEST_TEST_INTDIR := $(CTEST_TEST_OUTDIR)/int/
CTEST_TEST_OBJ := $(patsubst $(CTEST_TEST_SRC)%.c,$(CTEST_TEST_INTDIR)%.o,$(CTEST_TEST_C))
CTEST_TEST_GCDA := $(patsubst %.o,%.gcda,$(CTEST_TEST_OBJ))

CTEST_TEST_INCLUDES := -I$(CTEST_DIR)include/ $(CTEST_INCLUDES)
CTEST_TEST_LIBS := $(CTEST) $(CBASE)

CTEST_TEST := $(CTEST_TEST_OUTDIR)/ctest_test

.PHONY: ctest_test
ctest_test: $(CTEST_TEST)
	@rm -rf $(CTEST_GCDA) $(CTEST_TEST_GCDA)
	@$(CTEST_TEST)

$(CTEST_TEST): $(CTEST_TEST_OBJ) $(CTEST_TEST_LIBS)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) $(LDFLAGS) -o $@ $(CTEST_TEST_OBJ) $(patsubst %,-L%,$(dir $(CTEST_TEST_LIBS))) $(patsubst %,-l:%,$(notdir $(CTEST_TEST_LIBS)))

$(CTEST_TEST_INTDIR)%.o: $(CTEST_TEST_SRC)%.c $(CTEST_TEST_H) $(CTEST_TEST_HEADERS)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c -I$(CTEST_TEST_SRC) $(CTEST_TEST_INCLUDES) $(CFLAGS) -o $@ $<
