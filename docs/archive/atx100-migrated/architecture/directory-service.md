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

The Directory Service is not the object system, the capability system, or the mailbox system.

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

The Directory Service does not grant permissions, manage capabilities, schedule tasks, route messages, validate ring security, create objects, or destroy objects.

Security and authority checks remain the responsibility of the Security, Authority, Capability, and Object models.

## Security Rule

Names are not capabilities.

Lookup success does not imply access.

Access still requires ring validation, capability validation, and object-state validation.

## Binding Lifetime

When an object is destroyed, directory bindings to that object become stale and must be removed or marked stale.

Destroyed object identities shall not be reused to satisfy stale bindings.
