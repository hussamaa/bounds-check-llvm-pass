CXXFLAGS = $(shell llvm-config --cxxflags) -g -fno-rtti -Wall
LOADABLEOPTS=-Wl,-flat_namespace -Wl,-undefined,suppress

#
# Main pass
#

build/libLLVMBoundsCheck.so: pass/BoundsCheck/BoundsCheck.cpp
	mkdir -p build/
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -shared $(LOADABLEOPTS) -fPIC $^ -o $@

#
# Other
#
.PHONY: regression
regression:
	make && ./run-regression.sh


.PHONY: clean
clean:
	$(RM) -rf build/
