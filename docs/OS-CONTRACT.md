# Auxid OS Contract

The normative contract for Auxid's OS Foundation Layer modules (design record:
IAAppFramework `design/OS-FOUNDATION-LAYER.md`, ruling D-010). Semantics
promised here are enforced by conformance tests in the TestSuite (`tests/cpp/os/`)
across the full CI matrix. Where a platform cannot honestly express an
operation, the entry says so and the operation fails loudly — never a silent
no-op (the honest-absence rule).

Conventions (all modules):

- Strings are **UTF-8 at the API** on every platform; UTF-16 conversion is a
  Windows implementation detail. Paths are `filesystem::Path`.
- Fallible operations return `Result<T>`; OS failures carry
  `ErrorDomain::Os` (or a more specific domain) with the **raw** `errno` /
  `GetLastError` value in `Error::code`.
- Handles are owning, movable, non-copyable types.
- `<windows.h>` never appears in module-unit global fragments; Win32/CRT
  prototypes live in `src/hpp/auxid_win32.hpp` only.
- Module **interface** global fragments include as little as possible:
  heavy std headers (`<thread>`, `<chrono>`, `<filesystem>`) belong in
  implementation units. An interface GMF header that consumers also include
  textually invites include-vs-import collisions (observed on both MSVC and
  GCC). Prefer declarations-only interfaces with `AUXID_API` out-of-line
  definitions.
- mingw-gcc binaries containing rpmalloc's global allocator override must
  link libgcc/winpthread **dynamically**: GCC has no native PE TLS, so
  rpmalloc's thread-heap variable is emutls, and a statically linked
  emutls/pthread_once allocates through the overridden `calloc` — infinite
  recursion (SIGSEGV) or a pthread_once spin livelock. Ship the toolchain's
  runtime DLLs beside the executable for hermeticity (see the mingw
  toolchain file). MSVC/Clang-CL are unaffected (native TLS).

Module status: `auxid.time` SHIPPED · `auxid.env` SHIPPED · `auxid.ipc`
SHIPPED · `auxid.proc` PLANNED · `auxid.dl` PLANNED.

---

## auxid.time

| Op | Contract |
|----|----------|
| `monotonic_ns() -> u64` | Nanoseconds since an unspecified epoch. Never decreases within a process; unaffected by wall-clock adjustment; not comparable across processes or reboots. Cannot fail. |
| `wall_unix_ms() -> i64` | Milliseconds since the Unix epoch, UTC. May jump (NTP, manual adjustment); never use for durations. Cannot fail. |
| `sleep_ms(u32)` | Blocks the calling thread for **at least** the requested duration; scheduler latency may add more. `sleep_ms(0)` yields. |

Platform notes: all three are `std::chrono`/`std::this_thread` on every
platform (steady_clock is monotonic on all supported targets). No divergence.

## auxid.env

| Op | Contract |
|----|----------|
| `find(name) -> Option<String>` | Missing → none. Present → the value. **Divergence note:** an empty-valued variable is reported present on POSIX; on Windows, `SetEnvironmentVariableW(name, L"")` *deletes* the variable, so empty and missing are indistinguishable — do not design protocols around empty-valued variables. |
| `get_or(name, fallback) -> String` | `find` with a fallback; never fails. |
| `set(name, value) -> Result<void>` | Sets/overwrites. Errors carry the raw OS code. |
| `unset(name) -> Result<void>` | Removes. **Unsetting a missing variable is success** on every platform. |
| `executable_path() -> Result<Path>` | Absolute path of the running image, as reported by the OS (`GetModuleFileNameW` / `/proc/self/exe` / `_NSGetExecutablePath`); symlinks are not resolved beyond what the OS reports. **WASM: honest absence** — a wasm sandbox has no executable image; returns an Err stating so. |
| `standard_dir(dir, app) -> Result<Path>` | Conventional absolute path per the table below. **Does not create the directory.** `app` must be a filesystem-safe single component (unvalidated in v1). |

Thread-safety: the process environment is global mutable state. Concurrent
`set`/`unset` against reads from other threads is undefined on POSIX
(`getenv`/`setenv`). Mutate before spawning threads, or serialize at the
application level.

### StandardDir mapping (normative)

| Dir | Windows | Linux | macOS |
|-----|---------|-------|-------|
| Home | `%USERPROFILE%` | `$HOME` | `$HOME` |
| Temp | `%TEMP%` (else `%TMP%`) | `$TMPDIR` (else `/tmp`) | `$TMPDIR` (else `/tmp`) |
| Config | `%APPDATA%\<app>` | `$XDG_CONFIG_HOME` else `~/.config`, `/<app>` | `~/Library/Application Support/<app>` |
| Data | `%LOCALAPPDATA%\<app>` | `$XDG_DATA_HOME` else `~/.local/share`, `/<app>` | `~/Library/Application Support/<app>` |
| Cache | `%LOCALAPPDATA%\<app>\cache` | `$XDG_CACHE_HOME` else `~/.cache`, `/<app>` | `~/Library/Caches/<app>` |
| Runtime | `%LOCALAPPDATA%\<app>\runtime` | `$XDG_RUNTIME_DIR/<app>`, **honest fallback** `Temp/<app>` when unset | `Temp/<app>` |

Derived from environment variables by design (no shell32/KnownFolder
dependency); a missing required base variable (e.g. no `HOME`) is a Generic
error naming the variable.

## Portable error classification

Cross-platform failure conditions that products dispatch on are exposed as
**classifier functions over raw codes** — `ipc::is_closed(err)` (peer gone:
`ERROR_BROKEN_PIPE`/`ERROR_NO_DATA` vs `EPIPE`/`ECONNRESET`),
`ipc::is_already_held(err)` (`ERROR_ALREADY_EXISTS` vs
`EWOULDBLOCK`/`EACCES`). This refines the earlier "well-known constants"
plan: classifiers preserve the raw platform code in `Error::code` (which
constants would have overwritten) while still giving portable dispatch. Raw
codes remain the primary currency.

## auxid.ipc

Byte streams only — framing is the caller's business; product protocols
(and the planned IACore-style shared-memory ring fast path, which this API
deliberately does not preclude) layer on top. **WASM: honest absence** —
every operation returns an Err. All handles owning/movable;
`adopt_native`/`release_native` (Win32 HANDLE or fd, widened to i64) are the
low-level seam `auxid.proc` will use for child stdio.

| Op | Contract |
|----|----------|
| `pipe() -> Result<PipePair>` | Anonymous unidirectional pipe. `read` Ok(0) = all writers closed. |
| `PipeReader::read` / `PipeWriter::write` | Blocking; short writes possible; `write_all` loops. |
| `ChannelListener::bind(name)` | Realization: `\\.\pipe\auxid.<name>` (Windows) / `<XDG_RUNTIME_DIR else /tmp>/auxid.<name>.sock` chmod 0600 (POSIX). POSIX stale-socket recovery: a connect probe that is refused marks the node stale and it is unlinked; a live owner yields an Err (`EADDRINUSE`). Socket node is unlinked on listener close. |
| `ChannelListener::accept()` | Blocking; one `Channel` per client. Windows stages the next pipe instance before handing out the connected one; clients seeing `ERROR_PIPE_BUSY` wait-and-retry inside `connect`. |
| `Channel::connect(name)` / `read` / `write` | Bidirectional byte stream. `read` Ok(0) = peer closed; `write` to a gone peer is an Err with `is_closed()` true. POSIX writes are SIGPIPE-safe (`MSG_NOSIGNAL` / `SO_NOSIGPIPE`). Caution: writing to a closed **anonymous pipe** on POSIX still raises SIGPIPE (no per-write suppression exists for pipes) — supervise child exit before writing, or ignore SIGPIPE at the application level. |
| `InstanceLock::acquire(app_key)` | Named mutex `Local\auxid.<key>` (Windows) / `flock(LOCK_EX\|LOCK_NB)` on `<runtime>/auxid.<key>.lock` (POSIX). Losing to a live primary is an Err with `is_already_held()` true. The POSIX lock file is deliberately never unlinked (unlink-vs-flock races); the lock dies with the descriptor — including on crash, which is the point. |

Security note (v1): channel/lock nodes are created owner-only on POSIX;
Windows named pipes currently use default OS ACLs — a hardening pass
(explicit DACLs) is planned before any product ships multi-user.

## auxid.proc (PLANNED)

`spawn` with piped/inherited/null stdio, `LifetimeGroup` supervision
(job object / PDEATHSIG / pipe-EOF watcher — with the macOS guarantee
honestly weaker), `wait`/`try_status`/`terminate`. No graceful
`request_exit` in v1 (D-010): products send an IPC exit message and fall
back to `terminate()`.

## auxid.dl (PLANNED)

`Library::open/symbol/close`; destructor swallows unload errors, explicit
`close() -> Result<void>` reports them (D-010).
