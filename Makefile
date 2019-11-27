CXXFLAGS = $(shell llvm-config --cxxflags) -g -fno-rtti -Wall

#
# Main pass
#

build/libLLVMBoundsCheck.so: pass/BoundsCheck/Pass.cpp
	mkdir -p build/
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -shared -fPIC $^ -o $@

#
# Other
#

.PHONY: clean
clean:
	$(RM) -rf build/
