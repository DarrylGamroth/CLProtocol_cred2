# CLProtocol v1.2 Traceability Matrix (C-RED2 Stub)

This matrix tracks key normative behaviors from GenICam CLProtocol v1.2 against implementation and verification evidence.

| Requirement ID | Requirement (summary) | Implementation | Verification | Status |
| --- | --- | --- | --- | --- |
| CLP-REQ-001 | `clpGetShortDeviceIDTemplates` returns short templates only (`Manufacturer[#Family[#Model]]`). | `src/clprotocol_cred2.cpp` (`clpGetShortDeviceIDTemplates`) | `src/clprotocol_cred2_test.cpp` probe call with short template | Covered |
| CLP-REQ-002 | `clpProbeDevice` accepts template/device-ID hints and returns a non-zero cookie + DeviceID. | `src/clprotocol_cred2.cpp` (`clp_matches_device_template`, `clpProbeDevice`) | `src/clprotocol_cred2_test.cpp` initial probe + reconnect probe using returned DeviceID | Covered |
| CLP-REQ-003 | Probe connection is established at 9600 baud (power-up/default probe rate). | `src/clprotocol_cred2.cpp` (`clpProbeDevice` sets `CL_BAUDRATE_9600`) | `src/clprotocol_cred2_test.cpp` checks `FakeSerial::set_baud_calls` after probe | Covered |
| CLP-REQ-004 | XML IDs are formatted as `SchemaVersion@<complete DeviceID>@XMLVersion` and tied to connected device. | `src/clprotocol_cred2.cpp` (`clp_xml_id_for_device`, `clpGetXMLIDs`) | `src/clprotocol_cred2_test.cpp` validates XML ID contains full DeviceID | Covered |
| CLP-REQ-005 | `clpGetXMLDescription` must reject unknown XML IDs with `CL_ERR_NO_XMLDESCRIPTION_FOUND`. | `src/clprotocol_cred2.cpp` (`clpGetXMLDescription`) | `src/clprotocol_cred2_test.cpp` unknown XML ID negative test | Covered |
| CLP-REQ-006 | Cookie lifetime is per-connection; multiple cookies can coexist and disconnect invalidates only that cookie. | `src/clprotocol_cred2.cpp` (`ConnectionState`, `g_connections`, `clpDisconnect`) | `src/clprotocol_cred2_test.cpp` dual-cookie flow + disconnect invalidation check | Covered |
| CLP-REQ-007 | `CLP_DEVICE_BAUDERATE` / `CLP_DEVICE_SUPPORTED_BAUDERATES` are cookie-scoped and validated against host-supported rates. | `src/clprotocol_cred2.cpp` (`clpGetParam`, `clpSetParam`) | `src/clprotocol_cred2_test.cpp` per-cookie baud get/set assertions | Covered |

## Verification command

```sh
make test
```
