# Supervisor Simulator

The Supervisor simulator represents node `0x0000`.

Initial responsibilities:

```text
Generate PING
Generate VERSION_QUERY
Receive PONG
Receive VERSION_REPLY
```

The Supervisor simulator must use mailbox validation and must not directly mutate Fabric state.
