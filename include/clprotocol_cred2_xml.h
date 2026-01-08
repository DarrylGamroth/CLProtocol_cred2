#ifndef CLPROTOCOL_CRED2_XML_H
#define CLPROTOCOL_CRED2_XML_H

static const char k_embedded_cred2_xml[] = R"CLPXML(
<?xml version="1.0" encoding="UTF-8"?>
<RegisterDescription
  ModelName="C-RED2"
  VendorName="First Light Imaging"
  ToolTip="C-RED2 GenApi draft mapping"
  SchemaVersion="1.1">

  <Category Name="Root">
    <pFeature>DeviceControl</pFeature>
    <pFeature>ImageFormatControl</pFeature>
    <pFeature>AcquisitionControl</pFeature>
    <pFeature>AnalogControl</pFeature>
    <pFeature>DeviceTemperature</pFeature>
    <pFeature>UserSetControl</pFeature>
    <pFeature>EventsControl</pFeature>
    <pFeature>C-RED2</pFeature>
  </Category>

  <Port Name="Device">
    <DisplayName>Device Port</DisplayName>
    <AccessMode>RW</AccessMode>
  </Port>

  <Category Name="DeviceControl">
    <pFeature>DeviceModelName</pFeature>
    <pFeature>DeviceSerialNumber</pFeature>
    <pFeature>DeviceFirmwareVersion</pFeature>
    <pFeature>DeviceFirmwareVersionDetailed</pFeature>
    <pFeature>DeviceFirmwareBuild</pFeature>
    <pFeature>DeviceFpgaVersion</pFeature>
    <pFeature>DeviceHardwareVersion</pFeature>
    <pFeature>DeviceStatus</pFeature>
    <pFeature>DeviceStatusDetailed</pFeature>
    <pFeature>DeviceShutdown</pFeature>
    <pFeature>ContinueAfterError</pFeature>
    <pFeature>DeviceIndicatorSelector</pFeature>
    <pFeature>DeviceIndicatorMode</pFeature>
    <pFeature>DeviceFactoryReset</pFeature>
  </Category>

  <StringReg Name="DeviceModelNameReg">
    <Address>0x0000</Address>
    <Length>64</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="DeviceModelName">
    <Description>CLI: cameratype raw</Description>
    <pValue>DeviceModelNameReg</pValue>
  </String>

  <StringReg Name="DeviceSerialNumberReg">
    <Address>0x0040</Address>
    <Length>64</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="DeviceSerialNumber">
    <Description>CLI: hwuid raw</Description>
    <pValue>DeviceSerialNumberReg</pValue>
  </String>

  <StringReg Name="DeviceFirmwareVersionReg">
    <Address>0x0080</Address>
    <Length>64</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="DeviceFirmwareVersion">
    <Description>CLI: version firmware raw</Description>
    <pValue>DeviceFirmwareVersionReg</pValue>
  </String>

  <StringReg Name="DeviceFirmwareVersionDetailedReg">
    <Address>0x00C0</Address>
    <Length>64</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="DeviceFirmwareVersionDetailed">
    <Description>CLI: version firmware detailed raw</Description>
    <pValue>DeviceFirmwareVersionDetailedReg</pValue>
  </String>

  <StringReg Name="DeviceFirmwareBuildReg">
    <Address>0x0100</Address>
    <Length>64</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="DeviceFirmwareBuild">
    <Description>CLI: version firmware build raw</Description>
    <pValue>DeviceFirmwareBuildReg</pValue>
  </String>

  <StringReg Name="DeviceFpgaVersionReg">
    <Address>0x0140</Address>
    <Length>64</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="DeviceFpgaVersion">
    <Description>CLI: version fpga raw</Description>
    <pValue>DeviceFpgaVersionReg</pValue>
  </String>

  <StringReg Name="DeviceHardwareVersionReg">
    <Address>0x0180</Address>
    <Length>64</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="DeviceHardwareVersion">
    <Description>CLI: version hardware raw</Description>
    <pValue>DeviceHardwareVersionReg</pValue>
  </String>

  <StringReg Name="DeviceStatusReg">
    <Address>0x01C0</Address>
    <Length>128</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="DeviceStatus">
    <Description>CLI: status raw</Description>
    <pValue>DeviceStatusReg</pValue>
  </String>

  <StringReg Name="DeviceStatusDetailedReg">
    <Address>0x0240</Address>
    <Length>128</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="DeviceStatusDetailed">
    <Description>CLI: status detailed raw</Description>
    <pValue>DeviceStatusDetailedReg</pValue>
  </String>

  <IntReg Name="DeviceShutdownReg">
    <Address>0x0300</Address>
    <Length>4</Length>
    <AccessMode>WO</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Command Name="DeviceShutdown">
    <Description>CLI: shutdown</Description>
    <pValue>DeviceShutdownReg</pValue>
  </Command>

  <IntReg Name="ContinueAfterErrorReg">
    <Address>0x0304</Address>
    <Length>4</Length>
    <AccessMode>WO</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Command Name="ContinueAfterError">
    <Description>CLI: continue</Description>
    <pValue>ContinueAfterErrorReg</pValue>
  </Command>

  <IntReg Name="DeviceFactoryResetReg">
    <Address>0x0308</Address>
    <Length>4</Length>
    <AccessMode>WO</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Command Name="DeviceFactoryReset">
    <Description>CLI: restorefactory</Description>
    <pValue>DeviceFactoryResetReg</pValue>
  </Command>

  <IntReg Name="DeviceIndicatorSelectorReg">
    <Address>0x0310</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Enumeration Name="DeviceIndicatorSelector">
    <pValue>DeviceIndicatorSelectorReg</pValue>
    <EnumEntry Name="Status">
      <Value>0</Value>
    </EnumEntry>
  </Enumeration>

  <IntReg Name="DeviceIndicatorModeReg">
    <Address>0x0314</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Enumeration Name="DeviceIndicatorMode">
    <Description>CLI: led on|off</Description>
    <pValue>DeviceIndicatorModeReg</pValue>
    <EnumEntry Name="Off"><Value>0</Value></EnumEntry>
    <EnumEntry Name="On"><Value>1</Value></EnumEntry>
  </Enumeration>

  <Category Name="AcquisitionControl">
    <pFeature>AcquisitionFrameRate</pFeature>
    <pFeature>AcquisitionFrameRateMin</pFeature>
    <pFeature>AcquisitionFrameRateMax</pFeature>
    <pFeature>ExposureTime</pFeature>
    <pFeature>ExposureTimeMin</pFeature>
    <pFeature>ExposureTimeMax</pFeature>
    <pFeature>ExposureTimeMaxNoOverlap</pFeature>
    <pFeature>ExposureTimeGranularityEnable</pFeature>
    <pFeature>TriggerMode</pFeature>
    <pFeature>TriggerDelay</pFeature>
    <pFeature>TriggerSourceFormat</pFeature>
  </Category>

  <FloatReg Name="AcquisitionFrameRateReg">
    <Address>0x1000</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </FloatReg>
  <Float Name="AcquisitionFrameRate">
    <Description>CLI: fps raw / set fps</Description>
    <Unit>Hz</Unit>
    <pValue>AcquisitionFrameRateReg</pValue>
  </Float>

  <FloatReg Name="AcquisitionFrameRateMinReg">
    <Address>0x1004</Address>
    <Length>4</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </FloatReg>
  <Float Name="AcquisitionFrameRateMin">
    <Description>CLI: minfps raw</Description>
    <Unit>Hz</Unit>
    <pValue>AcquisitionFrameRateMinReg</pValue>
  </Float>

  <FloatReg Name="AcquisitionFrameRateMaxReg">
    <Address>0x1008</Address>
    <Length>4</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </FloatReg>
  <Float Name="AcquisitionFrameRateMax">
    <Description>CLI: maxfps raw (Camera Link) or maxfpsusb raw (USB)</Description>
    <Unit>Hz</Unit>
    <pValue>AcquisitionFrameRateMaxReg</pValue>
  </Float>

  <FloatReg Name="ExposureTimeReg">
    <Address>0x1010</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </FloatReg>
  <Float Name="ExposureTime">
    <Description>CLI: tint raw / set tint</Description>
    <Unit>us</Unit>
    <pValue>ExposureTimeReg</pValue>
  </Float>

  <FloatReg Name="ExposureTimeMinReg">
    <Address>0x1014</Address>
    <Length>4</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </FloatReg>
  <Float Name="ExposureTimeMin">
    <Description>CLI: mintint raw</Description>
    <Unit>us</Unit>
    <pValue>ExposureTimeMinReg</pValue>
  </Float>

  <FloatReg Name="ExposureTimeMaxReg">
    <Address>0x1018</Address>
    <Length>4</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </FloatReg>
  <Float Name="ExposureTimeMax">
    <Description>CLI: maxtint raw</Description>
    <Unit>us</Unit>
    <pValue>ExposureTimeMaxReg</pValue>
  </Float>

  <FloatReg Name="ExposureTimeMaxNoOverlapReg">
    <Address>0x101C</Address>
    <Length>4</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </FloatReg>
  <Float Name="ExposureTimeMaxNoOverlap">
    <Description>CLI: maxtintitr raw</Description>
    <Unit>us</Unit>
    <pValue>ExposureTimeMaxNoOverlapReg</pValue>
  </Float>

  <IntReg Name="ExposureTimeGranularityEnableReg">
    <Address>0x1020</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Boolean Name="ExposureTimeGranularityEnable">
    <Description>CLI: tintgranularity on|off</Description>
    <pValue>ExposureTimeGranularityEnableReg</pValue>
  </Boolean>

  <IntReg Name="TriggerModeReg">
    <Address>0x1030</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Enumeration Name="TriggerMode">
    <Description>CLI: extsynchro on|off</Description>
    <pValue>TriggerModeReg</pValue>
    <EnumEntry Name="Off"><Value>0</Value></EnumEntry>
    <EnumEntry Name="On"><Value>1</Value></EnumEntry>
  </Enumeration>

  <FloatReg Name="TriggerDelayReg">
    <Address>0x1034</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </FloatReg>
  <Float Name="TriggerDelay">
    <Description>CLI: tlsydel raw / Set tlsydel</Description>
    <Unit>us</Unit>
    <pValue>TriggerDelayReg</pValue>
  </Float>

  <IntReg Name="TriggerSourceFormatReg">
    <Address>0x1038</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Enumeration Name="TriggerSourceFormat">
    <Description>CLI: synchronization lvds|cmos</Description>
    <pValue>TriggerSourceFormatReg</pValue>
    <EnumEntry Name="LVDS"><Value>0</Value></EnumEntry>
    <EnumEntry Name="CMOS"><Value>1</Value></EnumEntry>
  </Enumeration>

  <Category Name="AnalogControl">
    <pFeature>VrefAdjustEnable</pFeature>
    <pFeature>TcdsAdjustEnable</pFeature>
    <pFeature>SensitivityMode</pFeature>
  </Category>

  <IntReg Name="VrefAdjustEnableReg">
    <Address>0x1100</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Boolean Name="VrefAdjustEnable">
    <Description>CLI: vrefadjust on|off</Description>
    <pValue>VrefAdjustEnableReg</pValue>
  </Boolean>

  <IntReg Name="TcdsAdjustEnableReg">
    <Address>0x1104</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Boolean Name="TcdsAdjustEnable">
    <Description>CLI: tcdsadjust on|off</Description>
    <pValue>TcdsAdjustEnableReg</pValue>
  </Boolean>

  <IntReg Name="SensitivityModeReg">
    <Address>0x1108</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Enumeration Name="SensitivityMode">
    <Description>CLI: sensibility low|medium|high</Description>
    <pValue>SensitivityModeReg</pValue>
    <EnumEntry Name="Low"><Value>0</Value></EnumEntry>
    <EnumEntry Name="Medium"><Value>1</Value></EnumEntry>
    <EnumEntry Name="High"><Value>2</Value></EnumEntry>
  </Enumeration>

  <Category Name="ImageFormatControl">
    <pFeature>CropEnable</pFeature>
    <pFeature>OffsetX</pFeature>
    <pFeature>OffsetY</pFeature>
    <pFeature>Width</pFeature>
    <pFeature>Height</pFeature>
    <pFeature>RawImagesEnable</pFeature>
    <pFeature>ImroReadBetweenReset</pFeature>
    <pFeature>BiasCorrectionEnable</pFeature>
    <pFeature>FlatCorrectionEnable</pFeature>
    <pFeature>BadPixelCorrectionEnable</pFeature>
    <pFeature>ChunkModeActive</pFeature>
  </Category>

  <IntReg Name="CropEnableReg">
    <Address>0x1200</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Boolean Name="CropEnable">
    <Description>CLI: cropping on|off</Description>
    <pValue>CropEnableReg</pValue>
  </Boolean>

  <IntReg Name="OffsetXReg">
    <Address>0x1204</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Integer Name="OffsetX">
    <Description>CLI: cropping columns (0-639, step 32)</Description>
    <Unit>px</Unit>
    <Min>0</Min>
    <Max>639</Max>
    <Inc>32</Inc>
    <pValue>OffsetXReg</pValue>
  </Integer>

  <IntReg Name="OffsetYReg">
    <Address>0x1208</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Integer Name="OffsetY">
    <Description>CLI: cropping rows (0-511, step 4)</Description>
    <Unit>px</Unit>
    <Min>0</Min>
    <Max>511</Max>
    <Inc>4</Inc>
    <pValue>OffsetYReg</pValue>
  </Integer>

  <IntReg Name="WidthReg">
    <Address>0x120C</Address>
    <Length>4</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Integer Name="Width">
    <Description>Derived from cropping or fixed sensor width</Description>
    <Unit>px</Unit>
    <Min>1</Min>
    <Max>640</Max>
    <pValue>WidthReg</pValue>
  </Integer>

  <IntReg Name="HeightReg">
    <Address>0x1210</Address>
    <Length>4</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Integer Name="Height">
    <Description>Derived from cropping or fixed sensor height</Description>
    <Unit>px</Unit>
    <Min>1</Min>
    <Max>512</Max>
    <pValue>HeightReg</pValue>
  </Integer>

  <IntReg Name="RawImagesEnableReg">
    <Address>0x1214</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Boolean Name="RawImagesEnable">
    <Description>CLI: rawimages on|off</Description>
    <pValue>RawImagesEnableReg</pValue>
  </Boolean>

  <IntReg Name="ImroReadBetweenResetReg">
    <Address>0x1218</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Integer Name="ImroReadBetweenReset">
    <Description>CLI: nbreadworeset / set nbreadworeset</Description>
    <pValue>ImroReadBetweenResetReg</pValue>
  </Integer>

  <IntReg Name="BiasCorrectionEnableReg">
    <Address>0x1220</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Boolean Name="BiasCorrectionEnable">
    <Description>CLI: bias on|off</Description>
    <pValue>BiasCorrectionEnableReg</pValue>
  </Boolean>

  <IntReg Name="FlatCorrectionEnableReg">
    <Address>0x1224</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Boolean Name="FlatCorrectionEnable">
    <Description>CLI: flat on|off</Description>
    <pValue>FlatCorrectionEnableReg</pValue>
  </Boolean>

  <IntReg Name="BadPixelCorrectionEnableReg">
    <Address>0x1228</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Boolean Name="BadPixelCorrectionEnable">
    <Description>CLI: badpixel on|off</Description>
    <pValue>BadPixelCorrectionEnableReg</pValue>
  </Boolean>

  <IntReg Name="ChunkModeActiveReg">
    <Address>0x1230</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Boolean Name="ChunkModeActive">
    <Description>CLI: imagetags on|off</Description>
    <pValue>ChunkModeActiveReg</pValue>
  </Boolean>

  <Category Name="DeviceTemperature">
    <pFeature>DeviceTemperatureSelector</pFeature>
    <pFeature>DeviceTemperature</pFeature>
  </Category>

  <IntReg Name="DeviceTemperatureSelectorReg">
    <Address>0x2000</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Enumeration Name="DeviceTemperatureSelector">
    <pValue>DeviceTemperatureSelectorReg</pValue>
    <EnumEntry Name="Motherboard"><Value>0</Value></EnumEntry>
    <EnumEntry Name="FrontEnd"><Value>1</Value></EnumEntry>
    <EnumEntry Name="PowerBoard"><Value>2</Value></EnumEntry>
    <EnumEntry Name="Sensor"><Value>3</Value></EnumEntry>
    <EnumEntry Name="SensorSetpoint"><Value>4</Value></EnumEntry>
    <EnumEntry Name="Peltier"><Value>5</Value></EnumEntry>
    <EnumEntry Name="Heatsink"><Value>6</Value></EnumEntry>
  </Enumeration>

  <FloatReg Name="DeviceTemperatureReg">
    <Address>0x2004</Address>
    <Length>4</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </FloatReg>
  <Float Name="DeviceTemperature">
    <Description>CLI: temperatures &lt;selector&gt; raw</Description>
    <Unit>degC</Unit>
    <pValue>DeviceTemperatureReg</pValue>
  </Float>

  <Category Name="UserSetControl">
    <pFeature>UserSetSelector</pFeature>
    <pFeature>UserSetLoad</pFeature>
    <pFeature>UserSetSave</pFeature>
  </Category>

  <IntReg Name="UserSetSelectorReg">
    <Address>0x2100</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Enumeration Name="UserSetSelector">
    <Description>CLI: preset / set preset [0-9]</Description>
    <pValue>UserSetSelectorReg</pValue>
    <EnumEntry Name="UserSet0"><Value>0</Value></EnumEntry>
    <EnumEntry Name="UserSet1"><Value>1</Value></EnumEntry>
    <EnumEntry Name="UserSet2"><Value>2</Value></EnumEntry>
    <EnumEntry Name="UserSet3"><Value>3</Value></EnumEntry>
    <EnumEntry Name="UserSet4"><Value>4</Value></EnumEntry>
    <EnumEntry Name="UserSet5"><Value>5</Value></EnumEntry>
    <EnumEntry Name="UserSet6"><Value>6</Value></EnumEntry>
    <EnumEntry Name="UserSet7"><Value>7</Value></EnumEntry>
    <EnumEntry Name="UserSet8"><Value>8</Value></EnumEntry>
    <EnumEntry Name="UserSet9"><Value>9</Value></EnumEntry>
  </Enumeration>

  <IntReg Name="UserSetLoadReg">
    <Address>0x2104</Address>
    <Length>4</Length>
    <AccessMode>WO</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Command Name="UserSetLoad">
    <Description>CLI: set preset [0-9]</Description>
    <pValue>UserSetLoadReg</pValue>
  </Command>

  <IntReg Name="UserSetSaveReg">
    <Address>0x2108</Address>
    <Length>4</Length>
    <AccessMode>WO</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Command Name="UserSetSave">
    <Description>CLI: save</Description>
    <pValue>UserSetSaveReg</pValue>
  </Command>

  <Category Name="EventsControl">
    <pFeature>EventEnable</pFeature>
  </Category>

  <IntReg Name="EventEnableReg">
    <Address>0x2200</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Boolean Name="EventEnable">
    <Description>CLI: events on|off</Description>
    <pValue>EventEnableReg</pValue>
  </Boolean>

  <Category Name="C-RED2">
    <pFeature>DevicePowerSelector</pFeature>
    <pFeature>DevicePowerConsumption</pFeature>
    <pFeature>DeviceFanMode</pFeature>
    <pFeature>DeviceFanSpeed</pFeature>
    <pFeature>VrefVoltage</pFeature>
    <pFeature>VrefVoltageTarget</pFeature>
    <pFeature>IpAddress</pFeature>
    <pFeature>IpNetmask</pFeature>
    <pFeature>IpGateway</pFeature>
    <pFeature>IpDns</pFeature>
    <pFeature>IpAlternateDns</pFeature>
    <pFeature>IpMode</pFeature>
    <pFeature>TelnetEnable</pFeature>
    <pFeature>RemoteMaintenanceEnable</pFeature>
    <pFeature>AccessPassword</pFeature>
    <pFeature>LicenseList</pFeature>
  </Category>

  <IntReg Name="DevicePowerSelectorReg">
    <Address>0x3000</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Enumeration Name="DevicePowerSelector">
    <Description>CLI: power, power snake, power peltier</Description>
    <pValue>DevicePowerSelectorReg</pValue>
    <EnumEntry Name="Total"><Value>0</Value></EnumEntry>
    <EnumEntry Name="Snake"><Value>1</Value></EnumEntry>
    <EnumEntry Name="Peltier"><Value>2</Value></EnumEntry>
  </Enumeration>

  <FloatReg Name="DevicePowerConsumptionReg">
    <Address>0x3004</Address>
    <Length>4</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </FloatReg>
  <Float Name="DevicePowerConsumption">
    <Unit>W</Unit>
    <pValue>DevicePowerConsumptionReg</pValue>
  </Float>

  <IntReg Name="DeviceFanModeReg">
    <Address>0x3010</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Enumeration Name="DeviceFanMode">
    <Description>CLI: fan mode automatic|manual</Description>
    <pValue>DeviceFanModeReg</pValue>
    <EnumEntry Name="Automatic"><Value>0</Value></EnumEntry>
    <EnumEntry Name="Manual"><Value>1</Value></EnumEntry>
  </Enumeration>

  <IntReg Name="DeviceFanSpeedReg">
    <Address>0x3014</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Integer Name="DeviceFanSpeed">
    <Description>CLI: fan speed &lt;value&gt;</Description>
    <pValue>DeviceFanSpeedReg</pValue>
  </Integer>

  <FloatReg Name="VrefVoltageReg">
    <Address>0x3020</Address>
    <Length>4</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </FloatReg>
  <Float Name="VrefVoltage">
    <Description>CLI: voltage vref raw</Description>
    <Unit>V</Unit>
    <pValue>VrefVoltageReg</pValue>
  </Float>

  <FloatReg Name="VrefVoltageTargetReg">
    <Address>0x3024</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </FloatReg>
  <Float Name="VrefVoltageTarget">
    <Description>CLI: set voltage vref &lt;vrefValue&gt;</Description>
    <Unit>V</Unit>
    <pValue>VrefVoltageTargetReg</pValue>
  </Float>

  <StringReg Name="IpAddressReg">
    <Address>0x3100</Address>
    <Length>32</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="IpAddress">
    <Description>CLI: ipaddress raw / set ip address</Description>
    <pValue>IpAddressReg</pValue>
  </String>

  <StringReg Name="IpNetmaskReg">
    <Address>0x3110</Address>
    <Length>32</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="IpNetmask">
    <Description>CLI: set ip netmask</Description>
    <pValue>IpNetmaskReg</pValue>
  </String>

  <StringReg Name="IpGatewayReg">
    <Address>0x3120</Address>
    <Length>32</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="IpGateway">
    <Description>CLI: set ip gateway</Description>
    <pValue>IpGatewayReg</pValue>
  </String>

  <StringReg Name="IpDnsReg">
    <Address>0x3130</Address>
    <Length>32</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="IpDns">
    <Description>CLI: set ip dns</Description>
    <pValue>IpDnsReg</pValue>
  </String>

  <StringReg Name="IpAlternateDnsReg">
    <Address>0x3140</Address>
    <Length>32</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="IpAlternateDns">
    <Description>CLI: set ip alternate-dns</Description>
    <pValue>IpAlternateDnsReg</pValue>
  </String>

  <IntReg Name="IpModeReg">
    <Address>0x3150</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Enumeration Name="IpMode">
    <Description>CLI: set ip mode manual|automatic</Description>
    <pValue>IpModeReg</pValue>
    <EnumEntry Name="Manual"><Value>0</Value></EnumEntry>
    <EnumEntry Name="Automatic"><Value>1</Value></EnumEntry>
  </Enumeration>

  <IntReg Name="TelnetEnableReg">
    <Address>0x3160</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Boolean Name="TelnetEnable">
    <Description>CLI: telnet / set telnet enable|disable</Description>
    <pValue>TelnetEnableReg</pValue>
  </Boolean>

  <IntReg Name="RemoteMaintenanceEnableReg">
    <Address>0x3164</Address>
    <Length>4</Length>
    <AccessMode>RW</AccessMode>
    <pPort>Device</pPort>
  </IntReg>
  <Boolean Name="RemoteMaintenanceEnable">
    <Description>CLI: remotemaintenance / set remotemaintenance on|off</Description>
    <pValue>RemoteMaintenanceEnableReg</pValue>
  </Boolean>

  <StringReg Name="AccessPasswordReg">
    <Address>0x3170</Address>
    <Length>64</Length>
    <AccessMode>WO</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="AccessPassword">
    <Description>CLI: set password</Description>
    <pValue>AccessPasswordReg</pValue>
  </String>

  <StringReg Name="LicenseListReg">
    <Address>0x3180</Address>
    <Length>256</Length>
    <AccessMode>RO</AccessMode>
    <pPort>Device</pPort>
  </StringReg>
  <String Name="LicenseList">
    <Description>CLI: licenses</Description>
    <pValue>LicenseListReg</pValue>
  </String>

</RegisterDescription>
)CLPXML";

#endif
