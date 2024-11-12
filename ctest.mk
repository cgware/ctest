PROJDIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CTEST_SRC := $(wildcard $(PROJDIR)src/*.c)
CTEST_INC := $(wildcard include/*.h)
CTEST_INC += $(wildcard $(PROJDIR)src/*.h)

CTEST_OUTDIR := $(BUILDDIR)bin/ctest/$(ARCH)-$(CONFIG)/
CTEST_INTDIR := $(CTEST_OUTDIR)int/
CTEST_OBJ := $(patsubst $(PROJDIR)%.c,$(CTEST_INTDIR)%.o,$(CTEST_SRC))
CTEST_GCDA := $(patsubst %.o,%.gcda,$(CTEST_OBJ))

CTEST_INCLUDES := -I$(PROJDIR)src/ -I$(PROJDIR)include/ -I$(SRCDIR)deps/cbase/include/

CTEST_NAME := ctest.a
CTEST := $(CTEST_OUTDIR)$(CTEST_NAME)

.PHONY: ctest
ctest: $(CTEST)

$(CTEST): $(CBASE) $(CTEST_OBJ)
	@mkdir -p $(@D)
	@ar rcs $@ $(CTEST_OBJ)

$(CTEST_INTDIR)%.o: $(PROJDIR)%.c $(CTEST_INC)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c $(CTEST_INCLUDES) $(CFLAGS) -o $@ $<

CTEST_TEST_SRC := $(wildcard $(PROJDIR)test/*.c)
CTEST_TEST_INC := $(wildcard include/*.h)
CTEST_TEST_INC += $(wildcard $(PROJDIR)test/*.h)

CTEST_TEST_OUTDIR := $(BUILDDIR)bin/ctest_test/$(ARCH)-$(CONFIG)/
CTEST_TEST_INTDIR := $(CTEST_TEST_OUTDIR)int/
CTEST_TEST_OBJ := $(patsubst $(PROJDIR)%.c,$(CTEST_TEST_INTDIR)%.o,$(CTEST_TEST_SRC))
CTEST_TEST_GCDA := $(patsubst %.o,%.gcda,$(CTEST_TEST_OBJ))

CTEST_TEST_INCLUDES := -I$(PROJDIR)test/ -I$(PROJDIR)include/ -I$(SRCDIR)deps/cbase/include/
CTEST_TEST_LIBS := -L$(CTEST_OUTDIR) -l:$(CTEST_NAME) -L$(CBASE_OUTDIR) -l:$(CBASE_NAME)

CTEST_TEST := $(CTEST_TEST_OUTDIR)ctest_test

.PHONY: ctest_test
ctest_test: $(CTEST_TEST)
	@rm -rf $(CTEST_GCDA) $(CTEST_TEST_GCDA)
	@$(CTEST_TEST)

$(CTEST_TEST): $(CTEST) $(CTEST_TEST_OBJ)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) $(CTEST_TEST_OBJ) $(CTEST_TEST_LIBS) $(LDFLAGS) -o $@

$(CTEST_TEST_INTDIR)%.o: $(PROJDIR)%.c $(CTEST_TEST_INC)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c $(CTEST_TEST_INCLUDES) $(CFLAGS) -o $@ $<
