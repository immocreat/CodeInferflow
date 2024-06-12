#!/bin/bash
cmake -B build -DUSE_CUDA=1 -DCMAKE_BUILD_TYPE=Release
cmake --build build --target install -j 8
