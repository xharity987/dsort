CXX = g++
CXXFLAGS = -std=c++20 -O2 -pthread -Wall -Wextra
TARGET = dsort
SOURCES = main.cpp config.cpp mover.cpp signals.cpp cli.cpp watcher.cpp rules.cpp
HEADERS = config.hpp mover.hpp signals.hpp cli.hpp watcher.hpp rules.hpp

# Пути для установки (локально для пользователя)
PREFIX = $(HOME)/.local
BINDIR = $(PREFIX)/bin
CONFDIR = $(HOME)/.config/dsort

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)

install: $(TARGET)
	@echo "Installing binary to $(BINDIR)..."
	mkdir -p $(BINDIR)
	cp $(TARGET) $(BINDIR)/$(TARGET)
	chmod +x $(BINDIR)/$(TARGET)
	
	@echo "Creating config directory at $(CONFDIR)..."
	mkdir -p $(CONFDIR)
	# Копируем конфиг только если его еще нет, чтобы не затереть твой текущий
	@if [ ! -f $(CONFDIR)/config.toml ]; then \
		cp config.toml $(CONFDIR)/config.toml; \
		echo "Default config installed."; \
	else \
		echo "Config already exists, skipping..."; \
	fi

uninstall:
	rm -f $(BINDIR)/$(TARGET)
	@echo "Binary removed. Config in $(CONFDIR) was left intact."

.PHONY: all clean install uninstall
