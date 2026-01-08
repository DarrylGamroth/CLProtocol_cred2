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

Manual command:

```sh
g++ -fPIC -shared -o libCLProtocol_cred2.so \
  -I"./include" -I"./include/genicam_include" \
  src/clprotocol_cred2.cpp
```

## Usage

Set the environment variable so the CTI can load it:

```sh
export EURESYS_CLPROTOCOL64_PATH=/path/to/libCLProtocol.so
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

To regenerate the embedded XML header after editing the XML:

```sh
make embed
```

To run the test harness:

```sh
make test
```

## TODO

- Implement the serial transport and map CLI commands to register
  accesses.
