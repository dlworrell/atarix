# Atarix Directory Service Architecture

## Purpose

The Directory Service provides name-to-object resolution for Atarix.

Conceptually:

```text
human-readable name
  -> directory binding
  -> object identity
  -> object resolution
```

The Directory Service is not the object system.

The Directory Service is not the capability system.

The Directory Service is not the mailbox system.

It resolves names to object identities.

## Responsibilities

The Directory Service is responsible for:

- Registering name bindings.
- Looking up names.
- Removing stale bindings.
- Managing aliases.
- Enumerating namespace entries.
- Supporting service discovery.
- Supporting namespace watch notifications.
- Supporting future distributed directory caches.

## Non-Responsibilities

The Directory Service does not:

- Grant permissions.
- Manage capabilities.
- Schedule tasks.
- Route messages.
- Validate ring security.
- Create objects.
- Destroy objects.

Security and authority checks remain the responsibility of the Security, Authority, Capability, and Object models.

## Registration

Registration binds a name to an object identity.

Example:

```text
/services/logger -> OID
```

Conceptual API:

```c
atx_directory_register("/services/logger", logger_oid);
```

Directory registration does not create the object.

The object must already exist or be in a valid publishable state.

## Lookup

Lookup resolves a name to an object identity.

Example:

```text
lookup("/services/logger") -> OID
```

Lookup does not return raw pointers, table slots, physical addresses, or mailbox numbers.

Lookup success does not imply access.

## Alias Management

Multiple names may resolve to the same object identity.

Example:

```text
/services/storage.boot
/services/boot-storage
/services/primary-storage
```

may all resolve to the same OID.

Aliases do not create new objects.

## Enumeration

The directory may support enumeration of namespace entries.

Example:

```text
list("/services")
```

may return:

```text
storage.boot
network.fabric
scheduler
logger
```

Enumeration reveals namespace structure, not authority to use listed objects.

## Service Discovery

The directory may support discovery by object type or service class.

Example:

```text
find(type=storage)
```

may return a list of object identities or names associated with storage services.

Applications should not be required to know every service name in advance.

## Namespace Layout

The initial namespace should use the layout defined by the Namespace Model:

```text
/
  services/
  nodes/
  users/
  devices/
  system/
  fabric/
```

## Binding Lifetime

When an object is destroyed, any directory bindings to that object become stale and must be removed or marked stale.

The directory shall not intentionally preserve dangling bindings to destroyed objects.

Destroyed object identities shall not be reused to satisfy stale bindings.

## Watch Notifications

The directory should support watch notifications for namespace changes.

Example:

```text
watch("/services")
```

may notify clients when:

- A service is added.
- A service is removed.
- A service is rebound.
- A service becomes stale.

Watch notifications prevent polling and support distributed systems.

## Versioning

Service versions should be modeled explicitly rather than encoded by ad hoc names.

Example:

```text
/versions/storage/1
/versions/storage/2
/services/storage
```

`/services/storage` may bind to the preferred current version.

This supports live upgrades and compatibility negotiation.

## Distributed Fabric Model

Every node may maintain a local directory cache.

The authoritative directory state may be local, remote, replicated, or partitioned in later implementations.

The public contract remains name-to-OID resolution.

Distributed implementation details must not alter the namespace abstraction.

## Failure Semantics

Directory operations shall return stable errors rather than raw null pointers.

Initial conceptual errors:

```text
ATX_OK
ATX_NOT_FOUND
ATX_STALE
ATX_ACCESS_DENIED
ATX_TIMEOUT
ATX_INVALID_NAME
ATX_ALREADY_EXISTS
```

The exact error model shall be finalized in the Error Model.

## Security Separation

Names are not capabilities.

Knowing a name only permits lookup.

Access still requires:

- Ring validation
- Capability validation
- Object state validation

The Directory Service must not become an implicit authority mechanism.

## Acceptance Criteria

Before the Directory Service is considered architecturally complete, it must define:

- Hierarchical namespace behavior.
- OID binding behavior.
- Alias behavior.
- Enumeration behavior.
- Service discovery behavior.
- Watch notification behavior.
- Versioning behavior.
- Distributed cache behavior.
- Failure semantics.
- Security separation.
