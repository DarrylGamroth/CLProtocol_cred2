#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
XML_SRC="${ROOT_DIR}/share/C-RED2_GenApi.xml"
OUT_HDR="${ROOT_DIR}/include/clprotocol_cred2_xml.h"

if [[ ! -f "${XML_SRC}" ]]; then
  echo "Missing XML source: ${XML_SRC}" >&2
  exit 1
fi

{
  echo "#ifndef CLPROTOCOL_CRED2_XML_H"
  echo "#define CLPROTOCOL_CRED2_XML_H"
  echo ""
  echo "static const char k_embedded_cred2_xml[] = R\"CLPXML("
  cat "${XML_SRC}"
  echo ")CLPXML\";"
  echo ""
  echo "#endif"
} > "${OUT_HDR}"

echo "Wrote ${OUT_HDR}"
