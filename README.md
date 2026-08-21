# Matt_daemon

Simple UNIX daemon (C++17) that accepts TCP clients and logs activity.

What it is
- A small daemon program that listens on TCP port 4242 and accepts up to 3 simultaneous clients.
- Logs all actions to `/var/log/matt_daemon/matt_daemon.log` with timestamps.
- Ensures a single running instance using `/var/lock/matt_daemon.lock` and `flock()`.

Key behaviors
- Runs only as root (main checks `geteuid()`).
- Daemonizes (double-fork, `setsid()`, chdir to `/`, redirect std fds).
- Client messages are logged; a client sending the exact string `quit` causes the daemon to stop.
- Signals are captured and logged; the daemon exits cleanly when a signal is received.

Build
```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

Run (required root)
1. Run the daemon as root:

```bash
sudo ./build/Matt_daemon
```

Testing
- Connect from another terminal using `telnet`:

```bash
telnet localhost 4242
```
- Type text and press Enter; messages are recorded in `/var/log/matt_daemon/matt_daemon.log`.
- Send `quit` (then Enter) to stop the daemon.

Files of interest
- `CMakeLists.txt` — build target `Matt_daemon` (C++17).
- `src/Daemon.cpp` — lockfile, daemonization, session management.
- `src/Server.cpp` — socket handling, client management, message processing.
- `src/Tintin_reporter.cpp` — logging implementation and timestamp formatting.

Notes & tips
- The lock file is created at `/var/lock/matt_daemon.lock` and is used to prevent multiple instances
- The server currently accepts-and-closes extra connections when at capacity.
- Ensure `/var/log/matt_daemon` is writable by the account running the daemon (the code creates it when started from `main` if not present).
