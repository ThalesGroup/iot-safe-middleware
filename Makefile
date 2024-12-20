
# Cross compiler for ARM target
#CXX = arm-linux-gnueabihf-g++
#CC = arm-linux-gnueabihf-gcc
#AR = arm-linux-gnueabihf-ar

CPPFLAGS += -I /usr/local/include -DAT_DEBUG
LD_LIBRARIES = -L/usr/local/lib -lCppUTest -lCppUTestExt

VPATH = iotsafelib/common/src iotsafelib/platform/modem/src tests/unit/src examples/simpledemo/src

IOTSAFELIB_OBJECTS =  Applet.o ROT.o SEInterface.o ATInterface.o GenericModem.o LSerial.o Serial.o 
TEST_OBJECTS =  rot_tests_helper.o rot_tests_unit_applet_tests.o rot_tests_unit_runner.o
APP_OBJECTS = simpledemo.o util.o

CPPFLAGS += -I iotsafelib/common/inc -I iotsafelib/platform/modem/inc -I tests/unit/inc -I examples/simpledemo/inc

TEST_TARGET = CppUTestIoTSafe
IOTSAFELIB = iotsafelib.a
APP_TARGET = simpledemo

all: $(IOTSAFELIB) $(APP_TARGET) $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS) iotsafelib.a
	$(CXX) -o $@ $^ $(LD_LIBRARIES) $(LDFLAGS)
#	./$(TEST_TARGET)

$(IOTSAFELIB): $(IOTSAFELIB_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(APP_TARGET): $(APP_OBJECTS) iotsafelib.a
	$(CXX) -o $@ $^ $(LD_LIBRARIES) $(LDFLAGS)

clean:
	rm -f -rf *.o
	rm -f $(TEST_TARGET)
	rm -f $(IOTSAFELIB)
	rm -f $(APP_TARGET)
	rm -f *.a

