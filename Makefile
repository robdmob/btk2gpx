TARGET = btk2gpx
OBJS = btk2gpx.o

ifeq ($(OS), Windows_NT)
	TARGET := $(TARGET).exe
	RM = del
	CFLAGS = -I.
	LIBS = 
else
	RM = rm -f
	CFLAGS = -I.
	LIBS =
endif

CC=gcc
STRIP=strip

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o "$@" $(OBJS) $(LIBS)
	$(STRIP) $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

distclean:
	$(RM) $(TARGET)

ifneq ($(OS), Windows_NT)
install:
	cp $(TARGET) /usr/local/bin/
endif
