SRC_FILES=main.cpp \
	../../sdk/NexSecureWare/Src/NexSecureWare.cpp \
	../NexSecureUpdater.c \
	../NexBase64.cpp
CFLAGS=-I../ -I../../sdk/NexSecureWare/Inc -Wall

TARGET_BIN_DIR=../bin
TARGET_BIN=$(TARGET_BIN_DIR)/securetool

all: $(TARGET_BIN)

$(TARGET_BIN): $(SRC_FILES)
	g++ $(SRC_FILES) -o $@ $(CFLAGS)

$(TARGET_BIN_DIR):
	@mkdir -p $@

clean:
	@rm -f $(TARGET)
	@rm -f *.o
