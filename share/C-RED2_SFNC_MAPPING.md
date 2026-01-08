# C-RED2 CLI to SFNC Mapping (Draft)

This is a draft mapping from the C-RED2 CLI command set to SFNC-style GenApi nodes.
Source: C-RED2 User Manual_20180625 (commands list and CLI protocol details).

## Acquisition / Exposure
- `fps`, `set fps <value>` -> `AcquisitionFrameRate` (Float)
- `minfps`, `maxfps`, `maxfpsusb` -> limits for `AcquisitionFrameRate` (read-only metadata; or custom `AcquisitionFrameRateMin/Max`)
- `tint`, `set tint <value>` -> `ExposureTime` (Float; unit per camera convention)
- `mintint`, `maxtint` -> limits for `ExposureTime` (read-only metadata or custom nodes)
- `maxtintitr` -> custom `ExposureTimeMaxNoOverlap` (Float)
- `tintgranularity on|off` -> custom `ExposureTimeGranularityEnable` (Boolean)

## Trigger / Sync
- `extsynchro on|off` -> `TriggerMode` (Enum: Off/On)
- `synchronization lvds|cmos` -> custom `TriggerSourceFormat` or `LineFormat` (Enum: LVDS/CMOS)
- `synchronization` (get) -> same enum
- `tlsydel`, `Set tlsydel <value>` -> `TriggerDelay` (Float) or custom `LineDelay` (Float)

## ROI / Cropping (Offset + Size)
- `cropping on|off` -> `RegionSelector` + `RegionEnable` (or custom `CropEnable`)
- `cropping columns <0-639 step 32>` -> `OffsetX` (Integer)
- `cropping rows <0-511 step 4>` -> `OffsetY` (Integer)
- `Width`, `Height` -> derived from cropping or fixed sensor geometry (read-only)

## Temperature / Power
- `temperatures *` -> `DeviceTemperatureSelector` + `DeviceTemperature`
  - Selectors: `Motherboard`, `FrontEnd`, `PowerBoard`, `Sensor`, `SensorSetpoint`, `Peltier`, `Heatsink`
- `power`, `power snake`, `power peltier` -> custom `DevicePowerSelector` + `DevicePowerConsumption`

## Image Processing / Corrections
- `bias on|off` -> custom `BiasCorrectionEnable`
- `flat on|off` -> custom `FlatCorrectionEnable`
- `badpixel on|off` -> custom `BadPixelCorrectionEnable`
- `rawimages on|off` -> custom `ImroProcessingEnable` or `RawImagesEnable`
- `nbreadworeset <n>` / `nbreadworeset` -> custom `ImroReadBetweenReset`

## Metadata / Tags
- `imagetags on|off` -> `ChunkModeActive` (Boolean) and `ChunkSelector`/`ChunkEnable` (custom if needed)

## Device / Identification / Versions
- `version`, `version firmware` -> `DeviceFirmwareVersion`
- `version firmware detailed` -> custom `DeviceFirmwareVersionDetailed`
- `version firmware build` -> custom `DeviceFirmwareBuild`
- `version fpga` -> custom `DeviceFpgaVersion`
- `version hardware` -> `DeviceVersion` or custom `DeviceHardwareVersion`
- `hwuid` -> `DeviceID` or `DeviceSerialNumber`
- `cameratype` -> `DeviceModelName`

## Status / Control
- `status`, `status detailed` -> custom `DeviceStatus` / `DeviceStatusDetailed` (String)
- `continue` -> custom `ContinueAfterError` (Command)
- `shutdown` -> custom `DeviceShutdown` (Command)
- `save` -> `UserSetSave` (Command)
- `restorefactory` -> `UserSetSelector=Default` + `UserSetLoad` or custom `DeviceFactoryReset`
- `preset`, `set preset [0..9]` -> `UserSetSelector` + `UserSetLoad`

## Networking / Access
- `ipaddress`, `set ip ...`, `set ip mode ...`, `set ip dns ...` -> custom `Ip*` nodes (not SFNC unless GigE Vision)
- `telnet enable|disable`, `password`, `remotemaintenance` -> custom `RemoteAccess*` nodes

## Fan / Cooling / Sensitivity
- `fan mode automatic|manual` -> custom `DeviceFanMode`
- `fan speed <value>` -> custom `DeviceFanSpeed`
- `sensibility low|medium|high` -> `Gain` if it maps cleanly, else custom `SensitivityMode`

## LED / Events / Licenses / Files
- `led on|off` -> `DeviceIndicatorSelector` + `DeviceIndicatorMode` (or custom `StatusLedEnable`)
- `events on|off` -> `EventNotification` (per-event) or custom `EventEnable`
- `licenses`, `exec enablelicense`, `exec disablelicense` -> custom `LicenseList` + `LicenseControl`
- `sendfile`, `xsendfile`, `getflat`, `getbias`, `exec upgradefirmware`, `exec logs` -> custom maintenance commands

## CLI Protocol Notes
- ASCII commands terminated by `\n`
- No echo
- Each reply ends with `\r\nfli-cli>`
- Add `raw` to return value only (e.g., `fps raw` -> `400`)
