CXX ?= g++
CXXFLAGS ?= -fPIC -std=c++11 -DCLPROTOCOL_EXPORTS -I./include -I./include/genicam_include
LDFLAGS ?= -shared
LIB_SUFFIX ?= cred2
TARGET = libCLProtocol_$(LIB_SUFFIX).so
CLPROTOCOL_PLATFORM_SUBDIR ?= Linux64_x64
EMBED_SCRIPT = ./scripts/gen_embed_xml.sh
SRC = src/clprotocol_cred2.cpp
TEST_SRC = src/clprotocol_cred2_test.cpp
TEST_BIN = clprotocol_cred2_test

all: $(TARGET)

embed:
	$(EMBED_SCRIPT)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) $(TEST_BIN)

install: $(TARGET)
	@if [ -z "$(DESTDIR)" ]; then \
		echo "Set DESTDIR to the install directory, e.g. make install DESTDIR=/opt/clprotocol"; \
		exit 1; \
	fi
	@mkdir -p "$(DESTDIR)/$(CLPROTOCOL_PLATFORM_SUBDIR)" "$(DESTDIR)/bin"
	@cp -f "$(TARGET)" "$(DESTDIR)/$(CLPROTOCOL_PLATFORM_SUBDIR)/"
	@cp -f "share/C-RED2_GenApi.xml" "$(DESTDIR)/"
	@cp -f "share/C-RED2_SFNC_MAPPING.md" "$(DESTDIR)/"
	@cp -f "include/clprotocol_cred2_xml.h" "$(DESTDIR)/"
	@cp -f "scripts/gen_embed_xml.sh" "$(DESTDIR)/bin/"
	@echo "Installed $(TARGET) to $(DESTDIR)/$(CLPROTOCOL_PLATFORM_SUBDIR)"

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(SRC) $(TEST_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^
