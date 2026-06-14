# Fabric Simulator

The Fabric simulator initially represents node `0x0001`.

Initial responsibilities:

```text
Receive PING
Reply PONG
Receive VERSION_QUERY
Reply VERSION_REPLY
```

No Discovery, Directory, Capability transport, resource model, or device model is implemented in the skeleton.

The Fabric simulator must use mailbox validation and must not expose direct address/register shortcuts to callers.
