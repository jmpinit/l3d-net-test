LIB_NAME = l3d-cube

SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
BUILD_DIR = $(SELF_DIR)bin/

FIRMWARE_DIR = ../spark/firmware/
SRC_DIR = firmware/
TEST_DIR = $(SRC_DIR)tests/

TESTS = $(wildcard $(TEST_DIR)*.cpp)
TEST_BINS = $(addprefix $(BUILD_DIR),$(notdir $(patsubst %.cpp,%.bin,$(TESTS))))

SOURCES = firmware/l3d-cube.cpp \
	  firmware/neopixel.cpp \
	  firmware/SparkWebSocketServer.cpp \
	  firmware/Base64.cpp

export INCLUDE_DIRS = $$(LIB_CORE_LIBRARIES_PATH)$(LIB_NAME) $$(LIB_CORE_LIBRARIES_PATH)$(LIB_NAME)/$(LIB_NAME)

$(TEST_BINS): $(BUILD_DIR)%.bin : $(TEST_DIR)%.cpp $(SOURCES) | $(BUILD_DIR)
	$(eval TEST_NAME=$(notdir $(basename $@)))
	-rm -r $(FIRMWARE_DIR)applications/$(TEST_NAME)
	mkdir -p $(FIRMWARE_DIR)applications/$(TEST_NAME)
	cp -r $(TEST_DIR)$(TEST_NAME).cpp $(FIRMWARE_DIR)applications/$(TEST_NAME)
	cp $(SRC_DIR)*.cpp $(FIRMWARE_DIR)applications/$(TEST_NAME)
	-rm -r $(FIRMWARE_DIR)libraries/$(LIB_NAME)
	mkdir -p $(FIRMWARE_DIR)libraries/$(LIB_NAME)
	cp -r $(SRC_DIR) $(FIRMWARE_DIR)libraries/$(LIB_NAME)/$(LIB_NAME)
	cd $(FIRMWARE_DIR)build && $(MAKE) APP=$(TEST_NAME)
	cp $(FIRMWARE_DIR)build/applications/$(TEST_NAME)/$(TEST_NAME).bin $@

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

doc:
	doxygen Doxyfile

clean:
	-rm $(TEST_BINS)
	rmdir $(BUILD_DIR)

.PHONY: doc
