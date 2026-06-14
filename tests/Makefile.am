TESTS = \
    discovery/test_discovery_header \
    discovery/test_discovery_crc \
    discovery/test_discovery_records \
    discovery/test_discovery_iteration \
    discovery/test_discovery_parser_minimal \
    discovery/test_duplicate_handle \
    discovery/test_missing_end \
    discovery/test_invalid_length \
    discovery/test_unknown_record \
    capability/test_capability_validation \
    capability/test_capability_ownership \
    capability/test_capability_ring_boundary \
    capability/test_capability_delegation \
    capability/test_capability_revocation \
    mailbox/test_mailbox_crc \
    mailbox/test_mailbox_sequence \
    mailbox/test_mailbox_validation

check_PROGRAMS = $(TESTS)

LDADD = ../src/libatarix.a
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/tests/discovery -I$(top_srcdir)/tests/capability -I$(top_srcdir)/tests/mailbox

discovery_test_discovery_header_SOURCES = discovery/test_discovery_header.c
discovery_test_discovery_crc_SOURCES = discovery/test_discovery_crc.c
discovery_test_discovery_records_SOURCES = discovery/test_discovery_records.c
discovery_test_discovery_iteration_SOURCES = discovery/test_discovery_iteration.c
discovery_test_discovery_parser_minimal_SOURCES = discovery/test_discovery_parser_minimal.c
discovery_test_duplicate_handle_SOURCES = discovery/test_duplicate_handle.c
discovery_test_missing_end_SOURCES = discovery/test_missing_end.c
discovery_test_invalid_length_SOURCES = discovery/test_invalid_length.c
discovery_test_unknown_record_SOURCES = discovery/test_unknown_record.c

capability_test_capability_validation_SOURCES = capability/test_capability_validation.c
capability_test_capability_ownership_SOURCES = capability/test_capability_ownership.c
capability_test_capability_ring_boundary_SOURCES = capability/test_capability_ring_boundary.c
capability_test_capability_delegation_SOURCES = capability/test_capability_delegation.c
capability_test_capability_revocation_SOURCES = capability/test_capability_revocation.c

mailbox_test_mailbox_crc_SOURCES = mailbox/test_mailbox_crc.c
mailbox_test_mailbox_sequence_SOURCES = mailbox/test_mailbox_sequence.c
mailbox_test_mailbox_validation_SOURCES = mailbox/test_mailbox_validation.c
