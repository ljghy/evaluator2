# Expression evaluator in C++17

## Build and run

```
mkdir build
cd build
```

#### Windows MSVC

```
cmake ..
cmake --build . --config [Debug|Release]
..\bin\eval.exe
```

#### Windows MinGW

```
cmake -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=[Debug|Release] ..
mingw32-make
..\bin\eval.exe
```

#### Linux GCC

```
cmake -DCMAKE_BUILD_TYPE=[Debug|Release] ..
make 
../bin/eval
```
