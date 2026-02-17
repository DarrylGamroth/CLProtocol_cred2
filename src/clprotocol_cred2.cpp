#include "clprotocol_cred2.h"
#include "clprotocol_cred2_xml.h"

#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <CLProtocol/ISerial.h>

static std::string g_last_error = "not implemented";
static char *g_xml = NULL;
static CLUINT32 g_xml_len = 0;
static clp_logger_t g_logger = NULL;
static CLP_LOG_LEVEL_VALUE g_log_level = CLP_LOG_NOTSET;

#ifndef CLP_LIB_SUFFIX
#define CLP_LIB_SUFFIX "cred2"
#endif

#ifndef CLP_PLATFORM_SUBDIR
#if defined(_WIN64)
#define CLP_PLATFORM_SUBDIR "Win64_x64"
#elif defined(_WIN32)
#define CLP_PLATFORM_SUBDIR "Win32_i86"
#elif defined(__x86_64__) || defined(__aarch64__)
#define CLP_PLATFORM_SUBDIR "Linux64_x64"
#else
#define CLP_PLATFORM_SUBDIR "Linux32_i86"
#endif
#endif

static const CLINT8 k_short_device_template[] = "FirstLightImaging#CRED2#CRED2";
static const CLINT8 k_camera_manufacturer[] = "FirstLightImaging";
static const CLINT8 k_camera_family[] = "CRED2";
static const CLINT8 k_camera_model[] = "CRED2";
static const CLINT8 k_camera_version[] = "Version_1_0_0";
static const CLINT8 k_camera_serial[] = "SN00000000";
static const CLINT8 k_xml_schema_version[] = "SchemaVersion.1.1";
static const CLINT8 k_xml_version[] = "XMLVersion.1.0.0";

static bool g_initialized = false;
static CLUINT32 g_next_cookie = 1;
static bool g_stop_probe_requested = false;

struct DeviceState {
  CLUINT32 user_set_selector;
  CLUINT32 temperature_selector;
  CLUINT32 power_selector;
  CLUINT32 indicator_selector;
};

struct ConnectionState {
  CLUINT32 cookie;
  CLUINT32 device_baudrate;
  CLUINT32 supported_baudrates;
  DeviceState state;
  std::string device_id;
  std::string xml_id;
};

static std::vector<ConnectionState> g_connections;

static CLUINT32 clp_default_supported_baudrates(void) {
  return CL_BAUDRATE_9600 | CL_BAUDRATE_19200 | CL_BAUDRATE_38400 | CL_BAUDRATE_57600 |
         CL_BAUDRATE_115200 | CL_BAUDRATE_230400 | CL_BAUDRATE_460800 | CL_BAUDRATE_921600 |
         CL_BAUDRATE_AUTOMAX;
}

static std::string clp_join_path(const std::string &left, const std::string &right) {
  if (left.empty()) {
    return right;
  }
  if (right.empty()) {
    return left;
  }
  const char last = left[left.size() - 1];
  if (last == '/' || last == '\\') {
    return left + right;
  }
  return left + "/" + right;
}

static std::string clp_driver_directory_token(void) {
  const char *root = getenv("GENICAM_CLPROTOCOL");
  if (root && root[0] != '\0') {
    return clp_join_path(root, CLP_PLATFORM_SUBDIR);
  }
  return std::string("$(GENICAM_CLPROTOCOL)/") + CLP_PLATFORM_SUBDIR;
}

static std::string clp_driver_filename_token(void) {
#if defined(_WIN32)
  return std::string("CLProtocol_") + CLP_LIB_SUFFIX + ".dll";
#else
  return std::string("libCLProtocol_") + CLP_LIB_SUFFIX + ".so";
#endif
}

static std::string clp_short_device_id(void) {
  return std::string(reinterpret_cast<const char *>(k_camera_manufacturer)) + "#" +
         reinterpret_cast<const char *>(k_camera_family) + "#" +
         reinterpret_cast<const char *>(k_camera_model) + "#" +
         reinterpret_cast<const char *>(k_camera_version) + "#" +
         reinterpret_cast<const char *>(k_camera_serial);
}

static std::string clp_full_device_id(void) {
  return clp_driver_directory_token() + "#" + clp_driver_filename_token() + "#" + clp_short_device_id();
}

static std::string clp_xml_id_for_device(const std::string &device_id) {
  return std::string(reinterpret_cast<const char *>(k_xml_schema_version)) + "@" + device_id + "@" +
         reinterpret_cast<const char *>(k_xml_version);
}

static std::vector<std::string> clp_split_by_char(const std::string &value, char separator) {
  std::vector<std::string> parts;
  size_t offset = 0;
  while (offset <= value.size()) {
    const size_t pos = value.find(separator, offset);
    if (pos == std::string::npos) {
      parts.push_back(value.substr(offset));
      break;
    }
    parts.push_back(value.substr(offset, pos - offset));
    offset = pos + 1;
  }
  return parts;
}

static bool clp_is_token_prefix(const std::vector<std::string> &candidate,
                                const std::vector<std::string> &target) {
  if (candidate.empty() || candidate.size() > target.size()) {
    return false;
  }
  for (size_t index = 0; index < candidate.size(); ++index) {
    if (candidate[index] != target[index]) {
      return false;
    }
  }
  return true;
}

static bool clp_matches_device_template(const std::string &template_or_device_id,
                                        const std::string &full_device_id,
                                        const std::string &short_device_id) {
  const std::vector<std::string> candidates = clp_split_by_char(template_or_device_id, '\t');
  const std::vector<std::string> full_tokens = clp_split_by_char(full_device_id, '#');
  const std::vector<std::string> short_tokens = clp_split_by_char(short_device_id, '#');
  for (size_t idx = 0; idx < candidates.size(); ++idx) {
    if (candidates[idx].empty()) {
      continue;
    }
    const std::vector<std::string> candidate_tokens = clp_split_by_char(candidates[idx], '#');
    if (clp_is_token_prefix(candidate_tokens, full_tokens) ||
        clp_is_token_prefix(candidate_tokens, short_tokens)) {
      return true;
    }
  }
  return false;
}

static ConnectionState *clp_find_connection(const CLUINT32 cookie) {
  if (cookie == 0) {
    return NULL;
  }
  for (size_t idx = 0; idx < g_connections.size(); ++idx) {
    if (g_connections[idx].cookie == cookie) {
      return &g_connections[idx];
    }
  }
  return NULL;
}

static CLUINT32 clp_allocate_cookie(void) {
  for (;;) {
    if (g_next_cookie == 0) {
      g_next_cookie = 1;
    }
    const CLUINT32 cookie = g_next_cookie++;
    if (clp_find_connection(cookie) == NULL) {
      return cookie;
    }
  }
}

static CLINT32 clp_require_cookie(const CLUINT32 cookie, ConnectionState **connection = NULL) {
  ConnectionState *state = clp_find_connection(cookie);
  if (!state) {
    g_last_error = "invalid cookie";
    return CL_ERR_INVALID_COOKIE;
  }
  if (connection) {
    *connection = state;
  }
  return CL_ERR_NO_ERR;
}

static bool clp_is_valid_baudrate_value(const CLUINT32 baudrate) {
  return baudrate != 0 && (baudrate & (baudrate - 1)) == 0;
}

static bool clp_debug_enabled(void) {
  const char *val = getenv("CLP_DEBUG");
  if (!val || val[0] == '\0') {
    return false;
  }
  return strcmp(val, "0") != 0;
}

static void clp_debugf(const char *fmt, ...) {
  if (!clp_debug_enabled()) {
    return;
  }
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
}

static void clp_logf(CLP_LOG_LEVEL_VALUE level, const char *fmt, ...) {
  if (!g_logger || level > g_log_level) {
    return;
  }
  va_list args;
  va_start(args, fmt);
  g_logger(level, fmt, args);
  va_end(args);
}

static CLINT32 clp_load_embedded_xml(void) {
  if (g_xml) {
    return CL_ERR_NO_ERR;
  }

  const size_t len = strlen(k_embedded_cred2_xml);
  g_xml = (char *)malloc(len + 1);
  if (!g_xml) {
    g_last_error = "out of memory";
    return CL_ERR_OUT_OF_MEMORY;
  }

  memcpy(g_xml, k_embedded_cred2_xml, len + 1);
  g_xml_len = static_cast<CLUINT32>(len + 1);
  return CL_ERR_NO_ERR;
}

static CLINT32 clp_load_xml(void) {
  const char *path = getenv("CLP_XML_PATH");
  FILE *fp = NULL;
  long len = 0;

  if (g_xml) {
    return CL_ERR_NO_ERR;
  }

  if (!path || path[0] == '\0') {
    return clp_load_embedded_xml();
  }

  fp = fopen(path, "rb");
  if (!fp) {
    g_last_error = "failed to open CLP_XML_PATH";
    return CL_ERR_INVALID_PTR;
  }

  if (fseek(fp, 0, SEEK_END) != 0) {
    fclose(fp);
    g_last_error = "failed to seek XML file";
    return CL_ERR_INVALID_PTR;
  }

  len = ftell(fp);
  if (len <= 0) {
    fclose(fp);
    g_last_error = "empty XML file";
    return CL_ERR_INVALID_PTR;
  }

  if (fseek(fp, 0, SEEK_SET) != 0) {
    fclose(fp);
    g_last_error = "failed to rewind XML file";
    return CL_ERR_INVALID_PTR;
  }

  g_xml = (char *)malloc((size_t)len + 1);
  if (!g_xml) {
    fclose(fp);
    g_last_error = "out of memory";
    return CL_ERR_OUT_OF_MEMORY;
  }

  if (fread(g_xml, 1, (size_t)len, fp) != (size_t)len) {
    fclose(fp);
    free(g_xml);
    g_xml = NULL;
    g_last_error = "failed to read XML file";
    return CL_ERR_INVALID_PTR;
  }

  g_xml[len] = '\0';
  fclose(fp);
  g_xml_len = (CLUINT32)len + 1;
  return CL_ERR_NO_ERR;
}

static std::string clp_trim_response(const std::string &resp);

static CLINT32 clp_send_command(ISerial *serial, const std::string &cmd, CLUINT32 timeout,
                                std::string *response) {
  if (!serial) {
    g_last_error = "serial interface is NULL";
    return CL_ERR_INVALID_PTR;
  }

  clp_debugf("CLP send: %s", cmd.c_str());

  std::string payload = cmd;
  payload.push_back('\n');
  std::vector<CLINT8> write_buf(payload.begin(), payload.end());
  CLUINT32 write_size = static_cast<CLUINT32>(write_buf.size());
  CLINT32 rc = serial->clSerialWrite(write_buf.data(), &write_size, timeout);
  if (rc != CL_ERR_NO_ERR) {
    g_last_error = "serial write failed";
    return rc;
  }

  if (!response) {
    return CL_ERR_NO_ERR;
  }

  const char *prompt = "fli-cli>";
  std::string out;
  for (int attempt = 0; attempt < 32; ++attempt) {
    char buf[256];
    CLUINT32 read_size = sizeof(buf);
    rc = serial->clSerialRead(reinterpret_cast<CLINT8 *>(buf), &read_size, timeout);
    if (rc == CL_ERR_TIMEOUT) {
      if (out.find(prompt) != std::string::npos) {
        break;
      }
      continue;
    }
    if (rc != CL_ERR_NO_ERR) {
      g_last_error = "serial read failed";
      return rc;
    }
    if (read_size > 0) {
      out.append(buf, buf + read_size);
      if (out.find(prompt) != std::string::npos) {
        break;
      }
    }
  }

  *response = out;
  clp_debugf("CLP recv: %s", clp_trim_response(out).c_str());
  return CL_ERR_NO_ERR;
}

static std::string clp_trim_response(const std::string &resp) {
  const char *prompt = "fli-cli>";
  std::string out = resp;
  size_t prompt_pos = out.find(prompt);
  if (prompt_pos != std::string::npos) {
    out = out.substr(0, prompt_pos);
  }

  while (!out.empty() && (out.back() == '\r' || out.back() == '\n' || out.back() == ' ' || out.back() == '\t')) {
    out.pop_back();
  }
  size_t start = 0;
  while (start < out.size() &&
         (out[start] == '\r' || out[start] == '\n' || out[start] == ' ' || out[start] == '\t')) {
    ++start;
  }
  return out.substr(start);
}

static std::string clp_buffer_to_string(const CLINT8 *buffer, CLINT64 buffer_size) {
  if (!buffer || buffer_size <= 0) {
    return std::string();
  }
  const char *ptr = reinterpret_cast<const char *>(buffer);
  size_t len = strnlen(ptr, static_cast<size_t>(buffer_size));
  return std::string(ptr, len);
}

static CLINT32 clp_write_int32(CLINT8 *pBuffer, CLINT64 buffer_size, CLINT32 value) {
  if (!pBuffer || buffer_size < 4) {
    return CL_ERR_BUFFER_TOO_SMALL;
  }
  pBuffer[0] = static_cast<CLINT8>(value & 0xFF);
  pBuffer[1] = static_cast<CLINT8>((value >> 8) & 0xFF);
  pBuffer[2] = static_cast<CLINT8>((value >> 16) & 0xFF);
  pBuffer[3] = static_cast<CLINT8>((value >> 24) & 0xFF);
  clp_debugf("CLP write int32: %d", value);
  return CL_ERR_NO_ERR;
}

static CLINT32 clp_write_float32(CLINT8 *pBuffer, CLINT64 buffer_size, float value) {
  if (!pBuffer || buffer_size < 4) {
    return CL_ERR_BUFFER_TOO_SMALL;
  }
  uint32_t bits = 0;
  static_assert(sizeof(float) == sizeof(uint32_t), "float size unexpected");
  memcpy(&bits, &value, sizeof(bits));
  clp_debugf("CLP write float32: %.6f", value);
  return clp_write_int32(pBuffer, buffer_size, static_cast<CLINT32>(bits));
}

static CLINT32 clp_read_int32(const CLINT8 *pBuffer, CLINT64 buffer_size, CLINT32 *value) {
  if (!pBuffer || buffer_size < 4 || !value) {
    return CL_ERR_BUFFER_TOO_SMALL;
  }
  uint32_t v = static_cast<uint8_t>(pBuffer[0]) |
               (static_cast<uint8_t>(pBuffer[1]) << 8) |
               (static_cast<uint8_t>(pBuffer[2]) << 16) |
               (static_cast<uint8_t>(pBuffer[3]) << 24);
  *value = static_cast<CLINT32>(v);
  return CL_ERR_NO_ERR;
}

static CLINT32 clp_read_float32(const CLINT8 *pBuffer, CLINT64 buffer_size, float *value) {
  CLINT32 tmp = 0;
  CLINT32 rc = clp_read_int32(pBuffer, buffer_size, &tmp);
  if (rc != CL_ERR_NO_ERR) {
    return rc;
  }
  uint32_t bits = static_cast<uint32_t>(tmp);
  float f = 0.0f;
  memcpy(&f, &bits, sizeof(f));
  *value = f;
  return CL_ERR_NO_ERR;
}

static void clp_write_string(CLINT8 *pBuffer, CLINT64 buffer_size, const std::string &value) {
  if (!pBuffer || buffer_size <= 0) {
    return;
  }
  memset(pBuffer, 0, static_cast<size_t>(buffer_size));
  const size_t copy_len = std::min(static_cast<size_t>(buffer_size - 1), value.size());
  memcpy(pBuffer, value.data(), copy_len);
}

static CLINT32 clp_parse_bool(const std::string &value, CLINT32 *out) {
  if (!out) {
    return CL_ERR_INVALID_PTR;
  }
  if (value.empty()) {
    return CL_ERR_INVALID_REFERENCE;
  }
  if (value == "1" || value == "on" || value == "ON" || value == "On") {
    *out = 1;
    return CL_ERR_NO_ERR;
  }
  if (value == "0" || value == "off" || value == "OFF" || value == "Off") {
    *out = 0;
    return CL_ERR_NO_ERR;
  }
  if (value == "enable" || value == "ENABLE" || value == "Enable") {
    *out = 1;
    return CL_ERR_NO_ERR;
  }
  if (value == "disable" || value == "DISABLE" || value == "Disable") {
    *out = 0;
    return CL_ERR_NO_ERR;
  }
  char *end = NULL;
  long parsed = strtol(value.c_str(), &end, 10);
  if (end && *end == '\0') {
    *out = parsed ? 1 : 0;
    return CL_ERR_NO_ERR;
  }
  return CL_ERR_INVALID_REFERENCE;
}

static std::string clp_bool_to_cli(CLINT32 value) { return value ? "on" : "off"; }

static std::string clp_format_int(CLINT32 value) {
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%d", value);
  return std::string(buf);
}

static std::string clp_format_float(float value) {
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.6f", value);
  return std::string(buf);
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpInitLib(clp_logger_t logger, CLP_LOG_LEVEL_VALUE logLevel) {
  if (g_initialized) {
    g_last_error = "library already initialized";
    return CL_ERR_IN_USE;
  }
  g_initialized = true;
  g_logger = logger;
  g_log_level = logLevel;
  g_connections.clear();
  g_stop_probe_requested = false;
  clp_debugf("CLProtocol stub initialized");
  clp_logf(CLP_LOG_INFO, "%s", "CLProtocol stub initialized");
  return CL_ERR_NO_ERR;
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpCloseLib(void) {
  free(g_xml);
  g_xml = NULL;
  g_xml_len = 0;
  g_initialized = false;
  g_connections.clear();
  g_stop_probe_requested = false;
  return CL_ERR_NO_ERR;
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpGetShortDeviceIDTemplates(CLINT8 *pShortDeviceTemplates, CLUINT32 *pBufferSize) {
  const CLUINT32 needed = (CLUINT32)strlen((const char *)k_short_device_template) + 1;

  if (!pBufferSize) {
    g_last_error = "buffer size is NULL";
    return CL_ERR_INVALID_PTR;
  }

  if (!pShortDeviceTemplates || *pBufferSize < needed) {
    *pBufferSize = needed;
    return CL_ERR_BUFFER_TOO_SMALL;
  }

  memcpy(pShortDeviceTemplates, k_short_device_template, needed);
  *pBufferSize = needed;
  return CL_ERR_NO_ERR;
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpProbeDevice(ISerial *pSerial,
               const CLINT8 *pDeviceIDTemplate,
               CLINT8 *pDeviceID,
               CLUINT32 *pBufferSize,
               CLUINT32 *pCookie,
               const CLUINT32 TimeOut) {
  (void)TimeOut;

  if (!pSerial || !pDeviceIDTemplate || !pBufferSize || !pCookie) {
    g_last_error = "invalid probe arguments";
    return CL_ERR_INVALID_PTR;
  }

  if (g_stop_probe_requested) {
    g_stop_probe_requested = false;
    g_last_error = "probe stopped";
    return CL_ERR_TIMEOUT;
  }

  const std::string device_template = reinterpret_cast<const char *>(pDeviceIDTemplate);
  const std::string full_device_id = clp_full_device_id();
  const std::string short_device_id = clp_short_device_id();
  if (!clp_matches_device_template(device_template, full_device_id, short_device_id)) {
    g_last_error = "device template does not match";
    return CL_ERR_INVALID_DEVICEID;
  }

  const CLUINT32 needed = static_cast<CLUINT32>(full_device_id.size() + 1);
  if (!pDeviceID || *pBufferSize < needed) {
    *pBufferSize = needed;
    return CL_ERR_BUFFER_TOO_SMALL;
  }

  CLUINT32 supported = 0;
  CLINT32 rc = pSerial->clGetSupportedBaudRates(&supported);
  if (rc != CL_ERR_NO_ERR || supported == 0) {
    supported = clp_default_supported_baudrates();
  }

  if ((supported & CL_BAUDRATE_9600) == 0) {
    g_last_error = "9600 baud unsupported";
    return CL_ERR_BAUD_RATE_NOT_SUPPORTED;
  }

  rc = pSerial->clSetBaudRate(CL_BAUDRATE_9600);
  if (rc != CL_ERR_NO_ERR) {
    g_last_error = "failed to set baudrate";
    return rc;
  }

  ConnectionState state = {};
  state.cookie = clp_allocate_cookie();
  state.device_baudrate = CL_BAUDRATE_9600;
  state.supported_baudrates = supported;
  state.state = {0, 0, 0, 0};
  state.device_id = full_device_id;
  state.xml_id = clp_xml_id_for_device(full_device_id);
  g_connections.push_back(state);

  memcpy(pDeviceID, state.device_id.c_str(), needed);
  *pBufferSize = needed;
  *pCookie = state.cookie;
  return CL_ERR_NO_ERR;
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpGetXMLIDs(ISerial *pSerial,
             const CLUINT32 Cookie,
             CLINT8 *pXMLIDs,
             CLUINT32 *pBufferSize,
             const CLUINT32 TimeOut) {
  (void)pSerial;
  (void)TimeOut;

  ConnectionState *connection = NULL;
  CLINT32 rc = clp_require_cookie(Cookie, &connection);
  if (rc != CL_ERR_NO_ERR) {
    return rc;
  }

  if (!pBufferSize) {
    g_last_error = "buffer size is NULL";
    return CL_ERR_INVALID_PTR;
  }

  const CLUINT32 needed = static_cast<CLUINT32>(connection->xml_id.size() + 1);
  if (!pXMLIDs || *pBufferSize < needed) {
    *pBufferSize = needed;
    return CL_ERR_BUFFER_TOO_SMALL;
  }

  memcpy(pXMLIDs, connection->xml_id.c_str(), needed);
  *pBufferSize = needed;
  return CL_ERR_NO_ERR;
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpGetXMLDescription(ISerial *pSerial,
                     const CLUINT32 Cookie,
                     const CLINT8 *pXMLID,
                     CLINT8 *pXMLBuffer,
                     CLUINT32 *pBufferSize,
                     const CLUINT32 TimeOut) {
  (void)pSerial;
  (void)TimeOut;

  ConnectionState *connection = NULL;
  CLINT32 rc = clp_require_cookie(Cookie, &connection);
  if (rc != CL_ERR_NO_ERR) {
    return rc;
  }

  if (!pXMLID) {
    g_last_error = "XML ID is NULL";
    return CL_ERR_INVALID_PTR;
  }
  if (connection->xml_id != reinterpret_cast<const char *>(pXMLID)) {
    g_last_error = "XML ID does not match connected device";
    return CL_ERR_NO_XMLDESCRIPTION_FOUND;
  }

  if (!pBufferSize) {
    g_last_error = "buffer size is NULL";
    return CL_ERR_INVALID_PTR;
  }

  rc = clp_load_xml();
  if (rc != CL_ERR_NO_ERR) {
    return rc;
  }

  if (!pXMLBuffer || *pBufferSize < g_xml_len) {
    *pBufferSize = g_xml_len;
    return CL_ERR_BUFFER_TOO_SMALL;
  }

  memcpy(pXMLBuffer, g_xml, g_xml_len);
  *pBufferSize = g_xml_len;
  return CL_ERR_NO_ERR;
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpReadRegister(ISerial *pSerial,
                const CLUINT32 Cookie,
                const CLINT64 Address,
                CLINT8 *pBuffer,
                const CLINT64 BufferSize,
                const CLUINT32 TimeOut) {
  ConnectionState *connection = NULL;
  CLINT32 rc = clp_require_cookie(Cookie, &connection);
  if (rc != CL_ERR_NO_ERR) {
    return rc;
  }
  DeviceState &state = connection->state;
  if (!pSerial) {
    g_last_error = "serial interface is NULL";
    return CL_ERR_INVALID_PTR;
  }
  if (!pBuffer || BufferSize <= 0) {
    g_last_error = "buffer is NULL";
    return CL_ERR_INVALID_PTR;
  }

  auto read_text = [&](const std::string &cmd, std::string *out) -> CLINT32 {
    std::string resp;
    CLINT32 rc = clp_send_command(pSerial, cmd, TimeOut, &resp);
    if (rc != CL_ERR_NO_ERR) {
      return rc;
    }
    *out = clp_trim_response(resp);
    if (out->empty()) {
      g_last_error = "empty response";
      return CL_ERR_INVALID_REFERENCE;
    }
    return CL_ERR_NO_ERR;
  };

  auto read_int = [&](const std::string &cmd, CLINT32 *value) -> CLINT32 {
    std::string out;
    CLINT32 rc = read_text(cmd, &out);
    if (rc != CL_ERR_NO_ERR) {
      return rc;
    }
    char *end = NULL;
    long parsed = strtol(out.c_str(), &end, 10);
    if (!end || end == out.c_str()) {
      g_last_error = "failed to parse int";
      return CL_ERR_INVALID_REFERENCE;
    }
    *value = static_cast<CLINT32>(parsed);
    return CL_ERR_NO_ERR;
  };

  auto read_float = [&](const std::string &cmd, float *value) -> CLINT32 {
    std::string out;
    CLINT32 rc = read_text(cmd, &out);
    if (rc != CL_ERR_NO_ERR) {
      return rc;
    }
    char *end = NULL;
    double parsed = strtod(out.c_str(), &end);
    if (!end || end == out.c_str()) {
      g_last_error = "failed to parse float";
      return CL_ERR_INVALID_REFERENCE;
    }
    *value = static_cast<float>(parsed);
    return CL_ERR_NO_ERR;
  };

  switch (Address) {
    case 0x0000: {
      std::string out;
      CLINT32 rc = read_text("cameratype raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      clp_write_string(pBuffer, BufferSize, out);
      return CL_ERR_NO_ERR;
    }
    case 0x0040: {
      std::string out;
      CLINT32 rc = read_text("hwuid raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      clp_write_string(pBuffer, BufferSize, out);
      return CL_ERR_NO_ERR;
    }
    case 0x0080: {
      std::string out;
      CLINT32 rc = read_text("version firmware raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      clp_write_string(pBuffer, BufferSize, out);
      return CL_ERR_NO_ERR;
    }
    case 0x00C0: {
      std::string out;
      CLINT32 rc = read_text("version firmware detailed raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      clp_write_string(pBuffer, BufferSize, out);
      return CL_ERR_NO_ERR;
    }
    case 0x0100: {
      std::string out;
      CLINT32 rc = read_text("version firmware build raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      clp_write_string(pBuffer, BufferSize, out);
      return CL_ERR_NO_ERR;
    }
    case 0x0140: {
      std::string out;
      CLINT32 rc = read_text("version fpga raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      clp_write_string(pBuffer, BufferSize, out);
      return CL_ERR_NO_ERR;
    }
    case 0x0180: {
      std::string out;
      CLINT32 rc = read_text("version hardware raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      clp_write_string(pBuffer, BufferSize, out);
      return CL_ERR_NO_ERR;
    }
    case 0x01C0: {
      std::string out;
      CLINT32 rc = read_text("status raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      clp_write_string(pBuffer, BufferSize, out);
      return CL_ERR_NO_ERR;
    }
    case 0x0240: {
      std::string out;
      CLINT32 rc = read_text("status detailed raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      clp_write_string(pBuffer, BufferSize, out);
      return CL_ERR_NO_ERR;
    }
    case 0x0310:
      return clp_write_int32(pBuffer, BufferSize, static_cast<CLINT32>(state.indicator_selector));
    case 0x0314: {
      CLINT32 value = 0;
      std::string out;
      CLINT32 rc = read_text("led raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      rc = clp_parse_bool(out, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x1000: {
      float value = 0.0f;
      CLINT32 rc = read_float("fps raw", &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_float32(pBuffer, BufferSize, value);
    }
    case 0x1004: {
      float value = 0.0f;
      CLINT32 rc = read_float("minfps raw", &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_float32(pBuffer, BufferSize, value);
    }
    case 0x1008: {
      float value = 0.0f;
      CLINT32 rc = read_float("maxfps raw", &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_float32(pBuffer, BufferSize, value);
    }
    case 0x1010: {
      float value = 0.0f;
      CLINT32 rc = read_float("tint raw", &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_float32(pBuffer, BufferSize, value);
    }
    case 0x1014: {
      float value = 0.0f;
      CLINT32 rc = read_float("mintint raw", &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_float32(pBuffer, BufferSize, value);
    }
    case 0x1018: {
      float value = 0.0f;
      CLINT32 rc = read_float("maxtint raw", &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_float32(pBuffer, BufferSize, value);
    }
    case 0x101C: {
      float value = 0.0f;
      CLINT32 rc = read_float("maxtintitr raw", &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_float32(pBuffer, BufferSize, value);
    }
    case 0x1020: {
      CLINT32 value = 0;
      std::string out;
      CLINT32 rc = read_text("tintgranularity raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      rc = clp_parse_bool(out, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x1030: {
      CLINT32 value = 0;
      std::string out;
      CLINT32 rc = read_text("extsynchro raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      rc = clp_parse_bool(out, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x1034: {
      float value = 0.0f;
      CLINT32 rc = read_float("tlsydel raw", &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_float32(pBuffer, BufferSize, value);
    }
    case 0x1038: {
      std::string out;
      CLINT32 rc = read_text("synchronization raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      CLINT32 value = 0;
      if (out == "lvds" || out == "LVDS") {
        value = 0;
      } else if (out == "cmos" || out == "CMOS") {
        value = 1;
      } else {
        g_last_error = "unknown synchronization mode";
        return CL_ERR_INVALID_REFERENCE;
      }
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x1100: {
      CLINT32 value = 0;
      std::string out;
      CLINT32 rc = read_text("vrefadjust raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      rc = clp_parse_bool(out, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x1104: {
      CLINT32 value = 0;
      std::string out;
      CLINT32 rc = read_text("tcdsadjust raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      rc = clp_parse_bool(out, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x1108: {
      std::string out;
      CLINT32 rc = read_text("sensibility raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      CLINT32 value = 0;
      if (out == "low" || out == "LOW") {
        value = 0;
      } else if (out == "medium" || out == "MEDIUM") {
        value = 1;
      } else if (out == "high" || out == "HIGH") {
        value = 2;
      } else {
        g_last_error = "unknown sensibility mode";
        return CL_ERR_INVALID_REFERENCE;
      }
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x1200: {
      CLINT32 value = 0;
      std::string out;
      CLINT32 rc = read_text("cropping raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      rc = clp_parse_bool(out, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x1204: {
      CLINT32 value = 0;
      CLINT32 rc = read_int("cropping columns raw", &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x1208: {
      CLINT32 value = 0;
      CLINT32 rc = read_int("cropping rows raw", &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x120C:
      return clp_write_int32(pBuffer, BufferSize, 640);
    case 0x1210:
      return clp_write_int32(pBuffer, BufferSize, 512);
    case 0x1214: {
      CLINT32 value = 0;
      std::string out;
      CLINT32 rc = read_text("rawimages raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      rc = clp_parse_bool(out, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x1218: {
      CLINT32 value = 0;
      CLINT32 rc = read_int("nbreadworeset raw", &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x1220: {
      CLINT32 value = 0;
      std::string out;
      CLINT32 rc = read_text("bias raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      rc = clp_parse_bool(out, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x1224: {
      CLINT32 value = 0;
      std::string out;
      CLINT32 rc = read_text("flat raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      rc = clp_parse_bool(out, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x1228: {
      CLINT32 value = 0;
      std::string out;
      CLINT32 rc = read_text("badpixel raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      rc = clp_parse_bool(out, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x1230: {
      CLINT32 value = 0;
      std::string out;
      CLINT32 rc = read_text("imagetags raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      rc = clp_parse_bool(out, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x2000:
      return clp_write_int32(pBuffer, BufferSize, static_cast<CLINT32>(state.temperature_selector));
    case 0x2004: {
      float value = 0.0f;
      const char *cmd = "temperatures raw";
      switch (state.temperature_selector) {
        case 0:
          cmd = "temperatures motherboard raw";
          break;
        case 1:
          cmd = "temperatures frontend raw";
          break;
        case 2:
          cmd = "temperatures powerboard raw";
          break;
        case 3:
          cmd = "temperatures snake raw";
          break;
        case 4:
          cmd = "temperatures snake setpoint raw";
          break;
        case 5:
          cmd = "temperatures peltier raw";
          break;
        case 6:
          cmd = "temperatures heatsink raw";
          break;
        default:
          cmd = "temperatures raw";
          break;
      }
      CLINT32 rc = read_float(cmd, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_float32(pBuffer, BufferSize, value);
    }
    case 0x2100:
      return clp_write_int32(pBuffer, BufferSize, static_cast<CLINT32>(state.user_set_selector));
    case 0x2200: {
      CLINT32 value = 0;
      std::string out;
      CLINT32 rc = read_text("events raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      rc = clp_parse_bool(out, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x3000:
      return clp_write_int32(pBuffer, BufferSize, static_cast<CLINT32>(state.power_selector));
    case 0x3004: {
      float value = 0.0f;
      const char *cmd = "power raw";
      switch (state.power_selector) {
        case 0:
          cmd = "power raw";
          break;
        case 1:
          cmd = "power snake raw";
          break;
        case 2:
          cmd = "power peltier raw";
          break;
        default:
          cmd = "power raw";
          break;
      }
      CLINT32 rc = read_float(cmd, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_float32(pBuffer, BufferSize, value);
    }
    case 0x3010: {
      std::string out;
      CLINT32 rc = read_text("fan mode raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      CLINT32 value = 0;
      if (out == "automatic" || out == "Automatic" || out == "AUTOMATIC") {
        value = 0;
      } else if (out == "manual" || out == "Manual" || out == "MANUAL") {
        value = 1;
      } else {
        g_last_error = "unknown fan mode";
        return CL_ERR_INVALID_REFERENCE;
      }
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x3014: {
      CLINT32 value = 0;
      CLINT32 rc = read_int("fan speed raw", &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x3020: {
      float value = 0.0f;
      CLINT32 rc = read_float("voltage vref raw", &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_float32(pBuffer, BufferSize, value);
    }
    case 0x3024: {
      float value = 0.0f;
      CLINT32 rc = read_float("voltage vref raw", &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_float32(pBuffer, BufferSize, value);
    }
    case 0x3100: {
      std::string out;
      CLINT32 rc = read_text("ipaddress raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      clp_write_string(pBuffer, BufferSize, out);
      return CL_ERR_NO_ERR;
    }
    case 0x3110:
    case 0x3120:
    case 0x3130:
    case 0x3140:
      clp_write_string(pBuffer, BufferSize, "");
      return CL_ERR_NO_ERR;
    case 0x3150:
      return clp_write_int32(pBuffer, BufferSize, 0);
    case 0x3160: {
      CLINT32 value = 0;
      std::string out;
      CLINT32 rc = read_text("telnet raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      rc = clp_parse_bool(out, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x3164: {
      CLINT32 value = 0;
      std::string out;
      CLINT32 rc = read_text("remotemaintenance raw", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      rc = clp_parse_bool(out, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      return clp_write_int32(pBuffer, BufferSize, value);
    }
    case 0x3180: {
      std::string out;
      CLINT32 rc = read_text("licenses", &out);
      if (rc != CL_ERR_NO_ERR) return rc;
      clp_write_string(pBuffer, BufferSize, out);
      return CL_ERR_NO_ERR;
    }
    default:
      g_last_error = "unknown register address";
      return CL_ERR_INVALID_REFERENCE;
  }
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpWriteRegister(ISerial *pSerial,
                 const CLUINT32 Cookie,
                 const CLINT64 Address,
                 const CLINT8 *pBuffer,
                 const CLINT64 BufferSize,
                 const CLUINT32 TimeOut) {
  ConnectionState *connection = NULL;
  CLINT32 rc = clp_require_cookie(Cookie, &connection);
  if (rc != CL_ERR_NO_ERR) {
    return rc;
  }
  DeviceState &state = connection->state;
  if (!pSerial) {
    g_last_error = "serial interface is NULL";
    return CL_ERR_INVALID_PTR;
  }
  if (!pBuffer || BufferSize <= 0) {
    g_last_error = "buffer is NULL";
    return CL_ERR_INVALID_PTR;
  }

  auto send_cmd = [&](const std::string &cmd) -> CLINT32 {
    CLINT32 rc = clp_send_command(pSerial, cmd, TimeOut, NULL);
    if (rc != CL_ERR_NO_ERR) {
      return rc;
    }
    return CL_ERR_NO_ERR;
  };

  switch (Address) {
    case 0x0300:
      return send_cmd("shutdown");
    case 0x0304:
      return send_cmd("continue");
    case 0x0308:
      return send_cmd("restorefactory");
    case 0x0310: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      state.indicator_selector = static_cast<CLUINT32>(value);
      return CL_ERR_NO_ERR;
    }
    case 0x0314: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set led " + clp_bool_to_cli(value);
      return send_cmd(cmd);
    }
    case 0x1000: {
      float value = 0.0f;
      CLINT32 rc = clp_read_float32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set fps " + clp_format_float(value);
      return send_cmd(cmd);
    }
    case 0x1010: {
      float value = 0.0f;
      CLINT32 rc = clp_read_float32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set tint " + clp_format_float(value);
      return send_cmd(cmd);
    }
    case 0x1020: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set tintgranularity " + clp_bool_to_cli(value);
      return send_cmd(cmd);
    }
    case 0x1030: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set extsynchro " + clp_bool_to_cli(value);
      return send_cmd(cmd);
    }
    case 0x1034: {
      float value = 0.0f;
      CLINT32 rc = clp_read_float32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set tlsydel " + clp_format_float(value);
      return send_cmd(cmd);
    }
    case 0x1038: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string mode = (value == 0) ? "lvds" : "cmos";
      std::string cmd = "set synchronization " + mode;
      return send_cmd(cmd);
    }
    case 0x1100: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set vrefadjust " + clp_bool_to_cli(value);
      return send_cmd(cmd);
    }
    case 0x1104: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set tcdsadjust " + clp_bool_to_cli(value);
      return send_cmd(cmd);
    }
    case 0x1108: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string mode = "low";
      if (value == 1) mode = "medium";
      if (value >= 2) mode = "high";
      std::string cmd = "set sensibility " + mode;
      return send_cmd(cmd);
    }
    case 0x1200: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set cropping " + clp_bool_to_cli(value);
      return send_cmd(cmd);
    }
    case 0x1204: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set cropping columns " + clp_format_int(value);
      return send_cmd(cmd);
    }
    case 0x1208: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set cropping rows " + clp_format_int(value);
      return send_cmd(cmd);
    }
    case 0x1214: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set rawimages " + clp_bool_to_cli(value);
      return send_cmd(cmd);
    }
    case 0x1218: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set nbreadworeset " + clp_format_int(value);
      return send_cmd(cmd);
    }
    case 0x1220: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set bias " + clp_bool_to_cli(value);
      return send_cmd(cmd);
    }
    case 0x1224: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set flat " + clp_bool_to_cli(value);
      return send_cmd(cmd);
    }
    case 0x1228: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set badpixel " + clp_bool_to_cli(value);
      return send_cmd(cmd);
    }
    case 0x1230: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set imagetags " + clp_bool_to_cli(value);
      return send_cmd(cmd);
    }
    case 0x2000: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      state.temperature_selector = static_cast<CLUINT32>(value);
      return CL_ERR_NO_ERR;
    }
    case 0x2100: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      state.user_set_selector = static_cast<CLUINT32>(value);
      return CL_ERR_NO_ERR;
    }
    case 0x2104: {
      std::string cmd = "set preset " + clp_format_int(static_cast<CLINT32>(state.user_set_selector));
      return send_cmd(cmd);
    }
    case 0x2108:
      return send_cmd("save");
    case 0x2200: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set events " + clp_bool_to_cli(value);
      return send_cmd(cmd);
    }
    case 0x3000: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      state.power_selector = static_cast<CLUINT32>(value);
      return CL_ERR_NO_ERR;
    }
    case 0x3010: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string mode = (value == 0) ? "automatic" : "manual";
      std::string cmd = "set fan mode " + mode;
      return send_cmd(cmd);
    }
    case 0x3014: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set fan speed " + clp_format_int(value);
      return send_cmd(cmd);
    }
    case 0x3024: {
      float value = 0.0f;
      CLINT32 rc = clp_read_float32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set voltage vref " + clp_format_float(value);
      return send_cmd(cmd);
    }
    case 0x3100: {
      std::string value = clp_buffer_to_string(pBuffer, BufferSize);
      std::string cmd = "set ip address " + value;
      return send_cmd(cmd);
    }
    case 0x3110: {
      std::string value = clp_buffer_to_string(pBuffer, BufferSize);
      std::string cmd = "set ip netmask " + value;
      return send_cmd(cmd);
    }
    case 0x3120: {
      std::string value = clp_buffer_to_string(pBuffer, BufferSize);
      std::string cmd = "set ip gateway " + value;
      return send_cmd(cmd);
    }
    case 0x3130: {
      std::string value = clp_buffer_to_string(pBuffer, BufferSize);
      std::string cmd = "set ip dns " + value;
      return send_cmd(cmd);
    }
    case 0x3140: {
      std::string value = clp_buffer_to_string(pBuffer, BufferSize);
      std::string cmd = "set ip alternate-dns " + value;
      return send_cmd(cmd);
    }
    case 0x3150: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string mode = (value == 0) ? "manual" : "automatic";
      std::string cmd = "set ip mode " + mode;
      return send_cmd(cmd);
    }
    case 0x3160: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = std::string("set telnet ") + (value ? "enable" : "disable");
      return send_cmd(cmd);
    }
    case 0x3164: {
      CLINT32 value = 0;
      CLINT32 rc = clp_read_int32(pBuffer, BufferSize, &value);
      if (rc != CL_ERR_NO_ERR) return rc;
      std::string cmd = "set remotemaintenance " + clp_bool_to_cli(value);
      return send_cmd(cmd);
    }
    case 0x3170: {
      std::string value = clp_buffer_to_string(pBuffer, BufferSize);
      std::string cmd = "set password " + value;
      return send_cmd(cmd);
    }
    default:
      g_last_error = "unknown register address";
      return CL_ERR_INVALID_REFERENCE;
  }
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpContinueWriteRegister(ISerial *pSerial,
                         const CLUINT32 Cookie,
                         const BOOL8 ContinueWaiting,
                         const CLUINT32 TimeOut) {
  (void)pSerial;
  (void)ContinueWaiting;
  (void)TimeOut;

  CLINT32 rc = clp_require_cookie(Cookie);
  if (rc != CL_ERR_NO_ERR) {
    return rc;
  }

  g_last_error = "continue write not implemented";
  return CL_ERR_INVALID_REFERENCE;
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpGetErrorText(CLINT32 errorCode, CLINT8 *errorText, CLUINT32 *errorTextSize, const CLUINT32 Cookie) {
  (void)Cookie;

  const char *text = g_last_error.c_str();
  if (errorCode == CL_ERR_GET_LAST_ERROR) {
    text = g_last_error.c_str();
  }

  if (!errorTextSize) {
    return CL_ERR_INVALID_PTR;
  }

  const CLUINT32 needed = (CLUINT32)strlen(text) + 1;
  if (!errorText || *errorTextSize < needed) {
    *errorTextSize = needed;
    return CL_ERR_BUFFER_TOO_SMALL;
  }

  memcpy(errorText, text, needed);
  *errorTextSize = needed;
  return CL_ERR_NO_ERR;
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpDisconnect(const CLUINT32 Cookie) {
  for (size_t idx = 0; idx < g_connections.size(); ++idx) {
    if (g_connections[idx].cookie == Cookie) {
      g_connections.erase(g_connections.begin() + idx);
      return CL_ERR_NO_ERR;
    }
  }
  g_last_error = "invalid cookie";
  return CL_ERR_INVALID_COOKIE;
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpGetCLProtocolVersion(CLUINT32 *pVersionMajor, CLUINT32 *pVersionMinor) {
  if (pVersionMajor) {
    *pVersionMajor = 1;
  }
  if (pVersionMinor) {
    *pVersionMinor = 2;
  }
  return CL_ERR_NO_ERR;
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpGetParam(ISerial *pSerial,
            CLP_PARAMS param,
            const CLUINT32 Cookie,
            CLINT8 *pBuffer,
            const CLINT64 BufferSize,
            const CLUINT32 TimeOut) {
  (void)TimeOut;

  if (!pBuffer || BufferSize <= 0) {
    return CL_ERR_PARAM_DATA_SIZE;
  }

  switch (param) {
    case CLP_LOG_LEVEL: {
      if (BufferSize < (CLINT64)sizeof(CLUINT32)) {
        return CL_ERR_PARAM_DATA_SIZE;
      }
      CLUINT32 value = (CLUINT32)g_log_level;
      memcpy(pBuffer, &value, sizeof(value));
      return CL_ERR_NO_ERR;
    }
    case CLP_LOG_CALLBACK: {
      const uintptr_t value = (uintptr_t)g_logger;
      if (BufferSize < (CLINT64)sizeof(value)) {
        return CL_ERR_PARAM_DATA_SIZE;
      }
      memcpy(pBuffer, &value, sizeof(value));
      return CL_ERR_NO_ERR;
    }
    case CLP_STOP_PROBE_DEVICE: {
      if (BufferSize < (CLINT64)sizeof(CLUINT32)) {
        return CL_ERR_PARAM_DATA_SIZE;
      }
      CLUINT32 value = g_stop_probe_requested ? 1U : 0U;
      memcpy(pBuffer, &value, sizeof(value));
      return CL_ERR_NO_ERR;
    }
    case CLP_DEVICE_BAUDERATE: {
      ConnectionState *connection = NULL;
      CLINT32 rc = clp_require_cookie(Cookie, &connection);
      if (rc != CL_ERR_NO_ERR) {
        return rc;
      }
      if (BufferSize < (CLINT64)sizeof(CLUINT32)) {
        return CL_ERR_PARAM_DATA_SIZE;
      }
      memcpy(pBuffer, &connection->device_baudrate, sizeof(connection->device_baudrate));
      return CL_ERR_NO_ERR;
    }
    case CLP_DEVICE_SUPPORTED_BAUDERATES: {
      ConnectionState *connection = NULL;
      CLINT32 rc = clp_require_cookie(Cookie, &connection);
      if (rc != CL_ERR_NO_ERR) {
        return rc;
      }
      if (BufferSize < (CLINT64)sizeof(CLUINT32)) {
        return CL_ERR_PARAM_DATA_SIZE;
      }
      if (pSerial) {
        CLUINT32 supported = 0;
        const CLINT32 supported_rc = pSerial->clGetSupportedBaudRates(&supported);
        if (supported_rc == CL_ERR_NO_ERR && supported != 0) {
          connection->supported_baudrates = supported;
        }
      }
      memcpy(pBuffer, &connection->supported_baudrates, sizeof(connection->supported_baudrates));
      return CL_ERR_NO_ERR;
    }
    default:
      return CL_ERR_PARAM_NOT_SUPPORTED;
  }
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpSetParam(ISerial *pSerial,
            CLP_PARAMS param,
            const CLUINT32 Cookie,
            const CLINT8 *pBuffer,
            const CLINT64 BufferSize,
            const CLUINT32 TimeOut) {
  (void)TimeOut;

  if (!pBuffer || BufferSize <= 0) {
    return CL_ERR_PARAM_DATA_SIZE;
  }

  switch (param) {
    case CLP_LOG_LEVEL: {
      if (BufferSize < (CLINT64)sizeof(CLUINT32)) {
        return CL_ERR_PARAM_DATA_SIZE;
      }
      CLUINT32 value = 0;
      memcpy(&value, pBuffer, sizeof(value));
      g_log_level = (CLP_LOG_LEVEL_VALUE)value;
      return CL_ERR_NO_ERR;
    }
    case CLP_LOG_CALLBACK: {
      uintptr_t value = 0;
      if (BufferSize < (CLINT64)sizeof(value)) {
        return CL_ERR_PARAM_DATA_SIZE;
      }
      memcpy(&value, pBuffer, sizeof(value));
      g_logger = (clp_logger_t)value;
      return CL_ERR_NO_ERR;
    }
    case CLP_STOP_PROBE_DEVICE: {
      if (BufferSize < (CLINT64)sizeof(CLUINT32)) {
        return CL_ERR_PARAM_DATA_SIZE;
      }
      CLUINT32 value = 0;
      memcpy(&value, pBuffer, sizeof(value));
      g_stop_probe_requested = (value != 0);
      return CL_ERR_NO_ERR;
    }
    case CLP_DEVICE_BAUDERATE: {
      ConnectionState *connection = NULL;
      CLINT32 rc = clp_require_cookie(Cookie, &connection);
      if (rc != CL_ERR_NO_ERR) {
        return rc;
      }
      if (!pSerial) {
        return CL_ERR_INVALID_PTR;
      }
      if (BufferSize < (CLINT64)sizeof(CLUINT32)) {
        return CL_ERR_PARAM_DATA_SIZE;
      }
      CLUINT32 baudrate = 0;
      memcpy(&baudrate, pBuffer, sizeof(baudrate));
      if (!clp_is_valid_baudrate_value(baudrate)) {
        return CL_ERR_PARAM_DATA_VALUE;
      }
      if (connection->supported_baudrates == 0) {
        connection->supported_baudrates = clp_default_supported_baudrates();
      }
      CLUINT32 supported = 0;
      const CLINT32 supported_rc = pSerial->clGetSupportedBaudRates(&supported);
      if (supported_rc == CL_ERR_NO_ERR && supported != 0) {
        connection->supported_baudrates = supported;
      }
      if ((connection->supported_baudrates & baudrate) == 0) {
        return CL_ERR_BAUD_RATE_NOT_SUPPORTED;
      }
      rc = pSerial->clSetBaudRate(baudrate);
      if (rc != CL_ERR_NO_ERR) {
        return rc;
      }
      connection->device_baudrate = baudrate;
      return CL_ERR_NO_ERR;
    }
    case CLP_DEVICE_SUPPORTED_BAUDERATES:
      return CL_ERR_PARAM_READ_ONLY;
    default:
      return CL_ERR_PARAM_NOT_SUPPORTED;
  }
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpIsParamSupported(CLP_PARAMS param) {
  switch (param) {
    case CLP_LOG_LEVEL:
    case CLP_LOG_CALLBACK:
    case CLP_STOP_PROBE_DEVICE:
    case CLP_DEVICE_BAUDERATE:
    case CLP_DEVICE_SUPPORTED_BAUDERATES:
      return CL_ERR_NO_ERR;
    default:
      return CL_ERR_PARAM_NOT_SUPPORTED;
  }
}

CLPROTOCOLEXPORT CLINT32 CLPROTOCOL
clpGetEventData(const CLUINT32 Cookie, CLINT8 *pBuffer, CLUINT32 *pBufferSize) {
  (void)pBuffer;
  CLINT32 rc = clp_require_cookie(Cookie);
  if (rc != CL_ERR_NO_ERR) {
    return rc;
  }
  if (pBufferSize) {
    *pBufferSize = 0;
  }
  return CL_ERR_TIMEOUT;
}
