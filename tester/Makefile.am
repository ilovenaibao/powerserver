
CXXFLAGS+=-O0 -g3

INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/source


noinst_PROGRAMS=test


test_SOURCES=test_main.cpp
test_DEPENDENCIES = $(top_builddir)/source/libnetserver.a

test_LDADD=-lnetserver -lpthread

test_LDFLAGS=-L$(top_builddir)/source


