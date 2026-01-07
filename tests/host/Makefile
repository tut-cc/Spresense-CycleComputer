CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread -I. -I../../src -Imocks
LDFLAGS = -pthread

# Auto-download GoogleTest
GTEST_DIR = googletest
GTEST_URL = https://github.com/google/googletest.git

# GoogleTest setup
GTEST_ROOT = $(GTEST_DIR)/googletest
GTEST_INCLUDE = -I$(GTEST_ROOT)/include -I$(GTEST_ROOT)

# Source files to test
SRC_DIR = ../../src/system
SRCS = $(SRC_DIR)/TripComputer.cpp \
       TripComputerTest.cpp

OBJS = $(SRCS:.cpp=.o)

# Helper object for mock globals
MOCKS_OBJ = mocks/MockGlobals.o

# GTest Objects
GTEST_OBJ = gtest-all.o
GTEST_MAIN_OBJ = gtest_main.o

TARGET = run_tests

all: $(GTEST_DIR) $(TARGET)

$(GTEST_DIR):
	git clone $(GTEST_URL) $(GTEST_DIR)

# Compile GoogleTest from source
$(GTEST_OBJ): $(GTEST_DIR)
	$(CXX) $(CXXFLAGS) -I$(GTEST_ROOT)/include -I$(GTEST_ROOT) -c $(GTEST_ROOT)/src/gtest-all.cc -o $@

$(GTEST_MAIN_OBJ): $(GTEST_DIR)
	$(CXX) $(CXXFLAGS) -I$(GTEST_ROOT)/include -I$(GTEST_ROOT) -c $(GTEST_ROOT)/src/gtest_main.cc -o $@

# Compile Mock Globals
$(MOCKS_OBJ): mocks/MockGlobals.cpp
	$(CXX) $(CXXFLAGS) $(GTEST_INCLUDE) -c $< -o $@

# Compile Objects
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(GTEST_INCLUDE) -c $< -o $@

# Link
$(TARGET): $(OBJS) $(MOCKS_OBJ) $(GTEST_OBJ) $(GTEST_MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $(OBJS) $(MOCKS_OBJ) $(GTEST_OBJ) $(GTEST_MAIN_OBJ) -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJS) $(MOCKS_OBJ) $(GTEST_OBJ) $(GTEST_MAIN_OBJ) $(TARGET)
	rm -rf $(GTEST_DIR)

.PHONY: all clean
