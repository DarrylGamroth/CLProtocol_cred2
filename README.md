# CLProtocol Stub (C-RED2)

This is a minimal CLProtocol shared library skeleton for C-RED2.
It exports the CLProtocol `clp*` symbols and provides an XML description loader.

**Important:** This stub uses headers bundled in `include/genicam_include/`.

## Build

Using Make:

```sh
make
```

To change the library suffix:

```sh
make LIB_SUFFIX=cred2
```

Using CMake:

```sh
cmake -S . -B build
cmake --build build
```

To change the library suffix with CMake:

```sh
cmake -S . -B build -DLIB_SUFFIX=cred2
cmake --build build
```

End-to-end CMake example (Release build + install):

```sh
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DLIB_SUFFIX=cred2 \
  -DCMAKE_INSTALL_PREFIX=/opt/clprotocol_cred2
cmake --build build --parallel
cmake --install build
```

Manual command:

```sh
g++ -fPIC -shared -o libCLProtocol_cred2.so \
  -I"./include" -I"./include/genicam_include" \
  src/clprotocol_cred2.cpp
```

## Usage

Set the environment variable so the CTI can load it:

```sh
export EURESYS_CLPROTOCOL64_PATH=/path/to/libCLProtocol_cred2.so
```

If you install with `-DLIB_SUFFIX=cred2` and `CMAKE_INSTALL_PREFIX=/opt/clprotocol_cred2`, this is typically:

```sh
export EURESYS_CLPROTOCOL64_PATH=/opt/clprotocol_cred2/lib/libCLProtocol_cred2.so
```

By default, the stub uses an embedded copy of `share/C-RED2_GenApi.xml`.
Optionally point the stub at a GenApi XML file:

```sh
export CLP_XML_PATH=/path/to/C-RED2_GenApi.xml
```

Debug logging is available via:

```sh
export CLP_DEBUG=1
```

This logs CLI commands and trimmed responses to stderr.

To confirm that `grablink.cti` is loading this library and calling into it:

```sh
export EURESYS_DEFAULT_GENTL_PRODUCER=grablink
export EURESYS_CLPROTOCOL64_PATH=/path/to/libCLProtocol_cred2.so
export CLP_DEBUG=1
strace -f -o /tmp/gentl.strace -e trace=openat,access,getenv \
  gentl genapi \
    --cti=/opt/euresys/egrabber/lib/x86_64/grablink.cti \
    --if='PC1628 - Grablink Duo (1-camera) - GDU00185' \
    --dev=Device0 \
    --module=remote \
    --device-access=DEVICE_ACCESS_CUSTOM_SERIAL_COMMUNICATION \
    --dump Root
rg -n 'libCLProtocol|EURESYS_CLPROTOCOL64_PATH' /tmp/gentl.strace
```

`strace` confirms the library file is opened. `CLP_DEBUG=1` confirms function entry (for example,
`clpInitLib` prints `CLProtocol stub initialized` on stderr).

To regenerate the embedded XML header after editing the XML:

```sh
make embed
```

To run the test harness:

```sh
make test
```
