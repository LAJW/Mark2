cmake -H. -Bbuild '-DCMAKE_CXX_COMPILER=/usr/local/Cellar/llvm/6.0.0/bin/clang++' '-DCMAKE_C_COMPILER=/usr/local/Cellar/llvm/6.0.0/bin/clang' '-DCMAKE_CXX_FLAGS=-stdlib=libc++ -nostdinc++ -nobuiltininc -isystem /usr/local/Cellar/llvm/6.0.0/include/c++/v1 -isystem /usr/local/Cellar/llvm/6.0.0/lib/clang/6.0.0/include -isystem /usr/include' -DCMAKE_EXPORT_COMPILE_COMMANDS=on -DCMAKE_BUILD_TYPE=Debug
