BUILDDIR := $(CURDIR)/
SRCDIR := $(CURDIR)/

TCC := $(CC)

ARCH := x64
ifeq ($(ARCH), x64)
BITS := 64
endif
ifeq ($(ARCH), x86)
BITS := 32
endif

CONFIG := Release
ifeq ($(CONFIG), Debug)
CFLAGS := -Wall -Wextra -Werror -pedantic -O0 -ggdb -coverage
LDFLAGS := -coverage
endif
ifeq ($(CONFIG), Release)
CFLAGS := -Wall -Wextra -Werror -pedantic
LDFLAGS :=
endif

OUTDIR := $(BUILDDIR)bin/$(ARCH)-$(CONFIG)
INTDIR := $(OUTDIR)/int
LIBSDIR := $(OUTDIR)/libs
TESTSDIR := $(OUTDIR)/tests
EXESDIR := $(OUTDIR)/exes

PKGDIR = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
PKGSRCDIR = $(PKGDIR)src/
PKGINCDIR = $(PKGDIR)include/
PKGTESTDIR = $(PKGDIR)test/

PKGSRC_C = $(shell find $(PKGSRCDIR) -type f -name '*.c')
PKGSRC_H = $(shell find $(PKGSRCDIR) -type f -name '*.h')
PKGTEST_C = $(shell find $(PKGTESTDIR) -type f -name '*.c')
PKGTEST_H = $(shell find $(PKGTESTDIR) -type f -name '*.h')
PKGINC_H = $(shell find $(PKGINCDIR) -type f -name '*.h')

INTSRCDIR = $(INTDIR)/$(PKGNAME)/src/
INTTESTDIR = $(INTDIR)/$(PKGNAME)/test/

PKGSRC_OBJ = $(patsubst $(PKGSRCDIR)%.c,$(INTSRCDIR)%.o,$(PKGSRC_C))
PKGSRC_GCDA = $(patsubst %.o,%.gcda,$(PKGSRC_OBJ))
PKGTEST_OBJ = $(patsubst $(PKGTESTDIR)%.c,$(INTTESTDIR)%.o,$(PKGTEST_C))
PKGTEST_GCDA = $(patsubst %.o,%.gcda,$(PKGTEST_OBJ))

PKGEXE = $(EXESDIR)/$(PKGNAME)
PKGLIB = $(LIBSDIR)/$(PKGNAME).a
PKGTEST = $(TESTSDIR)/$(PKGNAME)

$(SRCDIR)deps/cbase/cbase.mk:
ifeq ($(DEV), true)
	git clone git@github.com:cgware/cbase.git $(SRCDIR)deps/cbase
else
	git clone https://github.com/cgware/cbase.git $(SRCDIR)deps/cbase
endif

include $(SRCDIR)deps/cbase/cbase.mk

include $(SRCDIR)ctest.mk

.PHONY: test
test: ctest_test

.PHONY: coverage
coverage: test
	@lcov -q -c -o $(BUILDDIR)/bin/lcov.info -d $(INTDIR)/ctest
ifeq ($(SHOW), true)
	@genhtml -q -o $(BUILDDIR)/report/coverage $(BUILDDIR)/bin/lcov.info 
	@open $(BUILDDIR)/report/coverage/index.html
endif
