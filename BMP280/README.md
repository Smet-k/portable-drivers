# User Manual
    ## TBD
---
## How to run tests
```sh
cmake -S . -B build-host -DBMP280_BUILD_TESTS=ON
cmake --build build-host
./build-host/test_bmp280
```