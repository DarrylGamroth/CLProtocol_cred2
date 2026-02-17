#include "clprotocol_cred2.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

#include <CLProtocol/ISerial.h>

namespace {

class FakeSerial : public ISerial {
 public:
  std::string last_write;
  std::queue<std::string> reads;
  bool fail_writes = false;
  CLUINT32 supported_baudrates = CL_BAUDRATE_9600 | CL_BAUDRATE_115200;
  std::vector<CLUINT32> set_baud_calls;

  CLINT32 CLPROTOCOL clSerialRead(CLINT8 *buffer, CLUINT32 *bufferSize, CLUINT32) override {
    if (!buffer || !bufferSize) {
      return CL_ERR_INVALID_PTR;
    }
    if (reads.empty()) {
      return CL_ERR_TIMEOUT;
    }
    std::string data = reads.front();
    reads.pop();
    const CLUINT32 to_copy = std::min(*bufferSize, static_cast<CLUINT32>(data.size()));
    memcpy(buffer, data.data(), to_copy);
    *bufferSize = to_copy;
    return CL_ERR_NO_ERR;
  }

  CLINT32 CLPROTOCOL clSerialWrite(CLINT8 *buffer, CLUINT32 *bufferSize, CLUINT32) override {
    if (!buffer || !bufferSize) {
      return CL_ERR_INVALID_PTR;
    }
    if (fail_writes) {
      return CL_ERR_TIMEOUT;
    }
    last_write.assign(reinterpret_cast<char *>(buffer), reinterpret_cast<char *>(buffer) + *bufferSize);
    return CL_ERR_NO_ERR;
  }

  CLINT32 CLPROTOCOL clGetSupportedBaudRates(CLUINT32 *baudRates) override {
    if (!baudRates) {
      return CL_ERR_INVALID_PTR;
    }
    *baudRates = supported_baudrates;
    return CL_ERR_NO_ERR;
  }

  CLINT32 CLPROTOCOL clSetBaudRate(CLUINT32 baudRate) override {
    set_baud_calls.push_back(baudRate);
    if ((supported_baudrates & baudRate) == 0) {
      return CL_ERR_BAUD_RATE_NOT_SUPPORTED;
    }
    return CL_ERR_NO_ERR;
  }
};

static float read_float_from_buf(const CLINT8 *buf) {
  uint32_t bits = static_cast<uint8_t>(buf[0]) |
                  (static_cast<uint8_t>(buf[1]) << 8) |
                  (static_cast<uint8_t>(buf[2]) << 16) |
                  (static_cast<uint8_t>(buf[3]) << 24);
  float value = 0.0f;
  memcpy(&value, &bits, sizeof(value));
  return value;
}

static int read_int_from_buf(const CLINT8 *buf) {
  uint32_t bits = static_cast<uint8_t>(buf[0]) |
                  (static_cast<uint8_t>(buf[1]) << 8) |
                  (static_cast<uint8_t>(buf[2]) << 16) |
                  (static_cast<uint8_t>(buf[3]) << 24);
  return static_cast<int>(bits);
}

static CLUINT32 read_u32_from_buf(const CLINT8 *buf) {
  return static_cast<CLUINT32>(static_cast<uint8_t>(buf[0]) |
                               (static_cast<uint8_t>(buf[1]) << 8) |
                               (static_cast<uint8_t>(buf[2]) << 16) |
                               (static_cast<uint8_t>(buf[3]) << 24));
}

}  // namespace

int main() {
  FakeSerial serial;
  FakeSerial serial2;

  CLINT8 buf[8] = {};
  CLINT8 str_buf[64] = {};
  CLUINT32 cookie = 0;
  CLINT8 device_id[256] = {};
  CLUINT32 device_id_size = sizeof(device_id);
  CLINT32 rc = clpProbeDevice(&serial,
                              reinterpret_cast<const CLINT8 *>("FirstLightImaging#CRED2#CRED2"),
                              device_id,
                              &device_id_size,
                              &cookie,
                              100);
  assert(rc == CL_ERR_NO_ERR);
  assert(cookie != 0);
  assert(!serial.set_baud_calls.empty());
  assert(serial.set_baud_calls.back() == CL_BAUDRATE_9600);

  const std::string device_id_str(device_id);
  assert(device_id_str.find("libCLProtocol_cred2.so#FirstLightImaging#CRED2#CRED2") != std::string::npos);
  assert(device_id_str.find("#Version_1_0_0#SN00000000") != std::string::npos);

  CLINT8 reconnect_device_id[256] = {};
  CLUINT32 reconnect_device_id_size = sizeof(reconnect_device_id);
  CLUINT32 reconnect_cookie = 0;
  rc = clpProbeDevice(&serial,
                      reinterpret_cast<const CLINT8 *>(device_id_str.c_str()),
                      reconnect_device_id,
                      &reconnect_device_id_size,
                      &reconnect_cookie,
                      100);
  assert(rc == CL_ERR_NO_ERR);
  assert(reconnect_cookie != 0);
  assert(reconnect_cookie != cookie);
  assert(std::string(reconnect_device_id) == device_id_str);
  rc = clpDisconnect(reconnect_cookie);
  assert(rc == CL_ERR_NO_ERR);

  CLINT8 xml_id_buf[512] = {};
  CLUINT32 xml_id_buf_size = sizeof(xml_id_buf);
  rc = clpGetXMLIDs(&serial, cookie, xml_id_buf, &xml_id_buf_size, 100);
  assert(rc == CL_ERR_NO_ERR);
  const std::string xml_id(xml_id_buf);
  assert(xml_id.find("SchemaVersion.1.1@") == 0);
  assert(xml_id.find("@" + device_id_str + "@") != std::string::npos);

  CLINT8 unknown_xml[] = "SchemaVersion.1.1@Unknown@XMLVersion.1.0.0";
  CLUINT32 xml_size = 0;
  rc = clpGetXMLDescription(&serial, cookie, unknown_xml, NULL, &xml_size, 100);
  assert(rc == CL_ERR_NO_XMLDESCRIPTION_FOUND);

  rc = clpGetXMLDescription(&serial,
                            cookie,
                            reinterpret_cast<const CLINT8 *>(xml_id.c_str()),
                            NULL,
                            &xml_size,
                            100);
  assert(rc == CL_ERR_BUFFER_TOO_SMALL);
  assert(xml_size > 0);

  std::vector<CLINT8> xml_data(xml_size);
  rc = clpGetXMLDescription(&serial,
                            cookie,
                            reinterpret_cast<const CLINT8 *>(xml_id.c_str()),
                            xml_data.data(),
                            &xml_size,
                            100);
  assert(rc == CL_ERR_NO_ERR);
  assert(xml_size == xml_data.size());

  CLINT8 device_id2[256] = {};
  CLUINT32 device_id2_size = sizeof(device_id2);
  CLUINT32 cookie2 = 0;
  rc = clpProbeDevice(&serial2,
                      reinterpret_cast<const CLINT8 *>("FirstLightImaging#CRED2#CRED2"),
                      device_id2,
                      &device_id2_size,
                      &cookie2,
                      100);
  assert(rc == CL_ERR_NO_ERR);
  assert(cookie2 != 0);
  assert(cookie2 != cookie);

  CLINT8 param_buf[sizeof(CLUINT32)] = {};
  CLUINT32 baudrate = CL_BAUDRATE_115200;
  memcpy(param_buf, &baudrate, sizeof(baudrate));
  rc = clpSetParam(&serial, CLP_DEVICE_BAUDERATE, cookie, param_buf, sizeof(param_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(!serial.set_baud_calls.empty());
  assert(serial.set_baud_calls.back() == CL_BAUDRATE_115200);

  memset(param_buf, 0, sizeof(param_buf));
  rc = clpGetParam(&serial, CLP_DEVICE_BAUDERATE, cookie, param_buf, sizeof(param_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(read_u32_from_buf(param_buf) == CL_BAUDRATE_115200);

  memset(param_buf, 0, sizeof(param_buf));
  rc = clpGetParam(&serial2, CLP_DEVICE_BAUDERATE, cookie2, param_buf, sizeof(param_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(read_u32_from_buf(param_buf) == CL_BAUDRATE_9600);

  serial.reads.push("123.0\r\nfli-cli>");
  rc = clpReadRegister(&serial, cookie, 0x1000, buf, sizeof(buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(read_float_from_buf(buf) == 123.0f);
  assert(serial.last_write == std::string("fps raw\n"));

  CLINT8 write_buf[4] = {};
  float fps = 100.0f;
  memcpy(write_buf, &fps, sizeof(fps));
  rc = clpWriteRegister(&serial, cookie, 0x1000, write_buf, sizeof(write_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(serial.last_write.find("set fps") == 0);

  serial.reads.push("on\r\nfli-cli>");
  rc = clpReadRegister(&serial, cookie, 0x1220, buf, sizeof(buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(read_int_from_buf(buf) == 1);

  int zero = 0;
  memcpy(write_buf, &zero, sizeof(zero));
  rc = clpWriteRegister(&serial, cookie, 0x1220, write_buf, sizeof(write_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(serial.last_write == std::string("set bias off\n"));

  serial.reads.push("enable\r\nfli-cli>");
  rc = clpReadRegister(&serial, cookie, 0x3160, buf, sizeof(buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(read_int_from_buf(buf) == 1);
  int one = 1;
  memcpy(write_buf, &one, sizeof(one));
  rc = clpWriteRegister(&serial, cookie, 0x3160, write_buf, sizeof(write_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(serial.last_write == std::string("set telnet enable\n"));

  serial.reads.push("off\r\nfli-cli>");
  rc = clpReadRegister(&serial, cookie, 0x3164, buf, sizeof(buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(read_int_from_buf(buf) == 0);
  rc = clpWriteRegister(&serial, cookie, 0x3164, write_buf, sizeof(write_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(serial.last_write == std::string("set remotemaintenance on\n"));

  const char *ip = "192.168.0.10";
  memset(str_buf, 0, sizeof(str_buf));
  memcpy(str_buf, ip, strlen(ip));
  rc = clpWriteRegister(&serial, cookie, 0x3100, str_buf, sizeof(str_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(serial.last_write == std::string("set ip address 192.168.0.10\n"));

  const char *mask = "255.255.255.0";
  memset(str_buf, 0, sizeof(str_buf));
  memcpy(str_buf, mask, strlen(mask));
  rc = clpWriteRegister(&serial, cookie, 0x3110, str_buf, sizeof(str_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(serial.last_write == std::string("set ip netmask 255.255.255.0\n"));

  const char *gw = "192.168.0.1";
  memset(str_buf, 0, sizeof(str_buf));
  memcpy(str_buf, gw, strlen(gw));
  rc = clpWriteRegister(&serial, cookie, 0x3120, str_buf, sizeof(str_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(serial.last_write == std::string("set ip gateway 192.168.0.1\n"));

  const char *dns = "8.8.8.8";
  memset(str_buf, 0, sizeof(str_buf));
  memcpy(str_buf, dns, strlen(dns));
  rc = clpWriteRegister(&serial, cookie, 0x3130, str_buf, sizeof(str_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(serial.last_write == std::string("set ip dns 8.8.8.8\n"));

  const char *alt_dns = "1.1.1.1";
  memset(str_buf, 0, sizeof(str_buf));
  memcpy(str_buf, alt_dns, strlen(alt_dns));
  rc = clpWriteRegister(&serial, cookie, 0x3140, str_buf, sizeof(str_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(serial.last_write == std::string("set ip alternate-dns 1.1.1.1\n"));

  int manual = 0;
  memcpy(write_buf, &manual, sizeof(manual));
  rc = clpWriteRegister(&serial, cookie, 0x3150, write_buf, sizeof(write_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(serial.last_write == std::string("set ip mode manual\n"));

  const char *pw = "secret";
  memset(str_buf, 0, sizeof(str_buf));
  memcpy(str_buf, pw, strlen(pw));
  rc = clpWriteRegister(&serial, cookie, 0x3170, str_buf, sizeof(str_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(serial.last_write == std::string("set password secret\n"));

  CLINT8 selector_buf[4] = {};
  int selector = 2;
  memcpy(selector_buf, &selector, sizeof(selector));
  rc = clpWriteRegister(&serial, cookie, 0x2000, selector_buf, sizeof(selector_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  serial.reads.push("42.5\r\nfli-cli>");
  rc = clpReadRegister(&serial, cookie, 0x2004, buf, sizeof(buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(read_float_from_buf(buf) == 42.5f);

  selector = 1;
  memcpy(selector_buf, &selector, sizeof(selector));
  rc = clpWriteRegister(&serial, cookie, 0x3000, selector_buf, sizeof(selector_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  serial.reads.push("12.75\r\nfli-cli>");
  rc = clpReadRegister(&serial, cookie, 0x3004, buf, sizeof(buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(read_float_from_buf(buf) == 12.75f);

  selector = 3;
  memcpy(selector_buf, &selector, sizeof(selector_buf));
  rc = clpWriteRegister(&serial, cookie, 0x2100, selector_buf, sizeof(selector_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  rc = clpReadRegister(&serial, cookie, 0x2100, buf, sizeof(buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(read_int_from_buf(buf) == 3);
  rc = clpReadRegister(&serial2, cookie2, 0x2100, buf, sizeof(buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(read_int_from_buf(buf) == 0);

  FakeSerial failing_serial;
  failing_serial.fail_writes = true;
  CLINT8 bad_buf[4] = {0};
  rc = clpWriteRegister(&failing_serial, cookie, 0x1000, bad_buf, sizeof(bad_buf), 100);
  assert(rc == CL_ERR_TIMEOUT);

  FakeSerial invalid_read;
  invalid_read.reads.push("not-a-number\r\nfli-cli>");
  rc = clpReadRegister(&invalid_read, cookie, 0x1000, buf, sizeof(buf), 100);
  assert(rc == CL_ERR_INVALID_REFERENCE);

  rc = clpWriteRegister(&serial, cookie, 0x0308, write_buf, sizeof(write_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(serial.last_write == std::string("restorefactory\n"));

  int automatic = 1;
  memcpy(write_buf, &automatic, sizeof(automatic));
  rc = clpWriteRegister(&serial, cookie, 0x3150, write_buf, sizeof(write_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(serial.last_write == std::string("set ip mode automatic\n"));

  CLINT8 lic_buf[256] = {};
  serial.reads.push("licenseA.lic\\nlicenseB.lic\\r\\nfli-cli>");
  rc = clpReadRegister(&serial, cookie, 0x3180, lic_buf, sizeof(lic_buf), 100);
  assert(rc == CL_ERR_NO_ERR);
  assert(std::string(reinterpret_cast<char *>(lic_buf)).find("licenseA.lic") != std::string::npos);

  rc = clpDisconnect(cookie2);
  assert(rc == CL_ERR_NO_ERR);
  rc = clpDisconnect(cookie);
  assert(rc == CL_ERR_NO_ERR);
  rc = clpDisconnect(cookie);
  assert(rc == CL_ERR_INVALID_COOKIE);

  std::cout << "clprotocol_cred2_test OK\n";
  return 0;
}
