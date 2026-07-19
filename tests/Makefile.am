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
    capability/test_capability_respects_ring_boundary \
    capability/test_delegation_cannot_cross_ring_boundary \
    capability/test_revoked_capability_still_denied_after_ring_check \
    mailbox/test_mailbox_crc \
    mailbox/test_mailbox_sequence \
    mailbox/test_mailbox_validation \
    mailbox/test_mailbox_duplicate \
    mailbox/test_mailbox_out_of_order \
    mailbox/test_mailbox_unknown_type \
    mailbox/test_mailbox_max_payload \
    mailbox/test_mailbox_ring_restricted \
    mailbox/test_mailbox_supervisor_endpoint_denied \
    mailbox/test_mailbox_quarantine_isolation \
    sim/test_sim_node \
    sim/test_sim_ping_pong \
    sim/test_sim_version_query \
    sim/test_sim_reject_invalid \
    sim/test_sim_fabric_attach \
    sim/test_sim_fabric_duplicate \
    sim/test_sim_fabric_capacity \
    sim/test_sim_fabric_lookup_missing \
    ring/test_ring_self_access \
    ring/test_ring_upward_denied \
    ring/test_ring_downward_allowed \
    ring/test_ring_quarantine_denied \
    ring/test_ring_supervisor_override \
    memory/test_memory_descriptor \
    bib/test_bib_vectors

check_PROGRAMS = $(TESTS)

LDADD = ../src/libatarix.a
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/tests/discovery -I$(top_srcdir)/tests/capability -I$(top_srcdir)/tests/mailbox -I$(top_srcdir)/tests/sim -I$(top_srcdir)/tests/ring -I$(top_srcdir)/tests/memory -I$(top_srcdir)/tests/bib

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
capability_test_capability_respects_ring_boundary_SOURCES = capability/test_capability_respects_ring_boundary.c
capability_test_delegation_cannot_cross_ring_boundary_SOURCES = capability/test_delegation_cannot_cross_ring_boundary.c
capability_test_revoked_capability_still_denied_after_ring_check_SOURCES = capability/test_revoked_capability_still_denied_after_ring_check.c

mailbox_test_mailbox_crc_SOURCES = mailbox/test_mailbox_crc.c
mailbox_test_mailbox_sequence_SOURCES = mailbox/test_mailbox_sequence.c
mailbox_test_mailbox_validation_SOURCES = mailbox/test_mailbox_validation.c
mailbox_test_mailbox_duplicate_SOURCES = mailbox/test_mailbox_duplicate.c
mailbox_test_mailbox_out_of_order_SOURCES = mailbox/test_mailbox_out_of_order.c
mailbox_test_mailbox_unknown_type_SOURCES = mailbox/test_mailbox_unknown_type.c
mailbox_test_mailbox_max_payload_SOURCES = mailbox/test_mailbox_max_payload.c
mailbox_test_mailbox_ring_restricted_SOURCES = mailbox/test_mailbox_ring_restricted.c
mailbox_test_mailbox_supervisor_endpoint_denied_SOURCES = mailbox/test_mailbox_supervisor_endpoint_denied.c
mailbox_test_mailbox_quarantine_isolation_SOURCES = mailbox/test_mailbox_quarantine_isolation.c

sim_test_sim_node_SOURCES = sim/test_sim_node.c
sim_test_sim_ping_pong_SOURCES = sim/test_sim_ping_pong.c
sim_test_sim_version_query_SOURCES = sim/test_sim_version_query.c
sim_test_sim_reject_invalid_SOURCES = sim/test_sim_reject_invalid.c
sim_test_sim_fabric_attach_SOURCES = sim/test_sim_fabric_attach.c
sim_test_sim_fabric_duplicate_SOURCES = sim/test_sim_fabric_duplicate.c
sim_test_sim_fabric_capacity_SOURCES = sim/test_sim_fabric_capacity.c
sim_test_sim_fabric_lookup_missing_SOURCES = sim/test_sim_fabric_lookup_missing.c

ring_test_ring_self_access_SOURCES = ring/test_ring_self_access.c
ring_test_ring_upward_denied_SOURCES = ring/test_ring_upward_denied.c
ring_test_ring_downward_allowed_SOURCES = ring/test_ring_downward_allowed.c
ring_test_ring_quarantine_denied_SOURCES = ring/test_ring_quarantine_denied.c
ring_test_ring_supervisor_override_SOURCES = ring/test_ring_supervisor_override.c

memory_test_memory_descriptor_SOURCES = memory/test_memory_descriptor.c

bib_test_bib_vectors_SOURCES = bib/test_bib_vectors.c
