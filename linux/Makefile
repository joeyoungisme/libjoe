
TESTER=$(patsubst %.c,%,$(wildcard src/test/*.c))

OBJ=$(patsubst %.c,%.o,$(wildcard src/*.c))

INCLUDE+= -I ./include/

LIBS+=-lpthread

BUILD_DIR=build

CFLAG+= -D DEBUG_FLAG

all : $(BUILD_DIR) $(OBJ) $(TESTER)
	ar rcs $(BUILD_DIR)/liblix.a $(shell find src/ -name \*.o)

$(BUILD_DIR) :
	mkdir -p $(BUILD_DIR)

$(TESTER) : $(OBJ) $(TESTER).c
	$(CC) $(INCLUDE) $(CFLAG) $@.c -o $(BUILD_DIR)/$(notdir $@) $(LIBS) $(wildcard $(BUILD_DIR)/*.o)

%.o : %.c
	$(CC) $(INCLUDE) $(CFLAG) -c -o $(BUILD_DIR)/$(notdir $@) $< $(LIBS)

clean:
	rm -rf $(BUILD_DIR)
