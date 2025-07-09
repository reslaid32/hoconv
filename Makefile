CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude

SRC      := hoconv.cc
TARGET   := hoconv

PREFIX   ?= /usr
BINDIR   ?= $(PREFIX)/bin
DESTDIR  ?=

all: $(TARGET)

$(TARGET): $(SRC)
	@echo "  CXX $^ -> $@"
	@$(CXX) $(CXXFLAGS) -o $@ $^

install: $(TARGET)
	@echo "  INSTALL $(TARGET) -> $(DESTDIR)$(BINDIR)/"
	@install -Dm755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

uninstall:
	@echo "  UNINSTALL $(DESTDIR)$(BINDIR)/$(TARGET)"
	@rm -f $(DESTDIR)$(BINDIR)/$(TARGET)

clean:
	@echo "  RM  $(TARGET)"
	@rm -f $(TARGET)

.PHONY: all clean install uninstall
