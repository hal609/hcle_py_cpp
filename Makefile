# Target to configure and build the project. This is the default.
all:
	cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release --target hcle_test

# Target to clean the build directory
clean:
    rm -rf build

# A target to do a full clean rebuild
rebuild: clean all