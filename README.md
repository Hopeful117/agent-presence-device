# Agent Presence Device

## ESP-IDF CLI Environment

This project uses the existing ESP-IDF 6.1 installation used by CLion:

- ESP-IDF: `/home/ludo/.espressif/v6.1/esp-idf`
- Python environment: `/home/ludo/.espressif/python_env/idf6.1_py3.12_env`
- Tools: `/home/ludo/.espressif/tools`

In a new Bash terminal, put the system Python ahead of incompatible Python versions before sourcing the official ESP-IDF environment:

```bash
export PATH="/usr/bin:/bin:$PATH"
source /home/ludo/.espressif/v6.1/esp-idf/export.sh
cd /home/ludo/dev/embedded/agent-presence-device
```

The Python environment was repaired with the ESP-IDF `install-python-env` mechanism. Do not install ESP-IDF packages into system or Conda Python.

## Build

```bash
idf.py build
```

The project is already configured for `esp32s3`. Use `idf.py set-target esp32s3` only when creating a new build configuration.

This checkout contains an existing tracked `build/` directory from an older CLion configuration. If it is not a valid clean CMake build directory, use a separate generated directory for the first CLI build:

```bash
idf.py -B /tmp/agent-presence-build build
```

On a fresh checkout, or after the existing generated directory is removed during normal repository maintenance, the standard `idf.py build` command above is sufficient.

## Flash And Monitor

Discover the serial port before using it:

```bash
ls -l /dev/serial/by-id/* /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

Then flash and monitor the identified device:

```bash
idf.py -p /dev/<detected-port> flash
idf.py -p /dev/<detected-port> monitor
```

The user must have access to the serial device, normally through the `dialout` group. Do not use broad permissions such as `chmod 777`.

## HTTP Endpoint

The firmware exposes `POST /message` on port `80`:

```bash
curl -X POST http://<device-ip>/message \
  -H 'Content-Type: application/json' \
  -d '{"source":"DEVLOG","title":"Environment ready","body":"CLI device workflow works.","level":"INFO"}'
```

DevLog's backend uses its `DEVICE_BASE_URL` runtime configuration to reach this endpoint. Host-to-device reachability does not imply that the backend container can reach the device; validate the container path separately.

New notifications remain visible until the user interacts with them. A short press starts or continues reading and advances to the next page; a long press acknowledges immediately. Once reading has started, inactivity returns the device to idle after the ten-second reading timeout. The final page does not wrap to the first page.

## Generated Files

Do not stage or commit generated files under `build/` or `cmake-build-debug/`. Existing generated build files are already present in repository history and are outside this operational workflow.
