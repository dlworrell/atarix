bin_PROGRAMS = atarix-mkdiscovery atarix-bibdump

atarix_mkdiscovery_SOURCES = mkdiscovery/mkdiscovery.c
atarix_mkdiscovery_CPPFLAGS = -I$(top_srcdir)/include

atarix_bibdump_SOURCES = bibdump/bibdump.c
atarix_bibdump_CPPFLAGS = -I$(top_srcdir)/include
atarix_bibdump_LDADD = $(top_builddir)/src/libatarix.a
