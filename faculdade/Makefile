CC = gcc
CXX = g++
TARGET = bin/main.exe
CAMERA_TARGET = bin/camera_capture.exe
SOURCES = src/main.c src/janela.c src/arquivo.c src/lista.c src/formulario.c
CAMERA_SOURCES = src/camera_capture.cpp
PKG_CONFIG = pkg-config

CFLAGS += -Wall -Wextra -Iinclude $(shell $(PKG_CONFIG) --cflags gtk4)
LDLIBS += $(shell $(PKG_CONFIG) --libs gtk4)
CXXFLAGS += -Wall -Wextra -std=c++17 $(shell $(PKG_CONFIG) --cflags opencv4)
CAMERA_LDLIBS += -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_core -lgdi32

.PHONY: all clean run

all: $(TARGET) $(CAMERA_TARGET)

bin:
	cmd /C if not exist bin mkdir bin

$(TARGET): $(SOURCES) | bin
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDLIBS)

$(CAMERA_TARGET): $(CAMERA_SOURCES) | bin
	$(CXX) $(CXXFLAGS) $(CAMERA_SOURCES) -o $(CAMERA_TARGET) -static-libgcc -static-libstdc++ $(CAMERA_LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	-powershell -NoProfile -Command "Remove-Item -LiteralPath 'bin/main.exe','bin/camera_capture.exe' -Force -ErrorAction SilentlyContinue"
