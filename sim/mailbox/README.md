# Simulator Mailbox

The simulator mailbox layer wraps the public ATARIX mailbox protocol.

It must validate:

```text
Header
Length
CRC
Sequence
Message type
```

before any virtual node receives a message.

The simulator mailbox layer is not allowed to provide direct delivery shortcuts that bypass protocol validation.
