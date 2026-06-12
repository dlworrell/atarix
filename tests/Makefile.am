TESTS = \
    discovery/test_discovery_header \
    discovery/test_discovery_crc \
    discovery/test_discovery_records \
    discovery/test_discovery_iteration \
    discovery/test_discovery_parser_minimal \
    discovery/test_duplicate_handle \
    discovery/test_missing_end \
    discovery/test_invalid_length \
    discovery/test_unknown_record

check_PROGRAMS = $(TESTS)

LDADD = ../src/libatarix.a
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/tests/discovery

discovery_test_discovery_header_SOURCES = discovery/test_discovery_header.c
discovery_test_discovery_crc_SOURCES = discovery/test_discovery_crc.c
discovery_test_discovery_records_SOURCES = discovery/test_discovery_records.c
discovery_test_discovery_iteration_SOURCES = discovery/test_discovery_iteration.c
discovery_test_discovery_parser_minimal_SOURCES = discovery/test_discovery_parser_minimal.c
discovery_test_duplicate_handle_SOURCES = discovery/test_duplicate_handle.c
discovery_test_missing_end_SOURCES = discovery/test_missing_end.c
discovery_test_invalid_length_SOURCES = discovery/test_invalid_length.c
discovery_test_unknown_record_SOURCES = discovery/test_unknown_record.c
