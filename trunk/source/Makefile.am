
libnetserver_adir=$(top_srcdir)/source

PCH=stdafx.h.gch

INCLUDES=-I$(top_srcdir)/include

noinst_LIBRARIES = libnetserver.a

libnetserver_a_SOURCES = netserver.cpp worker.cpp
libnetserver_a_HEADERS =  typedefs.h stdafx.h

$(libnetserver_a_SOURCES) : $(PCH)

$(PCH):stdafx.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $<

