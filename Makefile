CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude

SRC      := hoconv.cc
TARGET   := hoconv

all: $(TARGET)

$(TARGET): $(SRC)
	@echo "  CXX $^ -> $@"
	@$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	@echo "  RM  $(TARGET)"
	@rm -f $(TARGET)

.PHONY: all clean
