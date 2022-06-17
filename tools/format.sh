#!/bin/bash
cd "$(dirname "$0")"
cd ..

# Add the license file.
files=$(find ./src -name "*.hpp" -o -name "*.cpp")

# Use clang-format.
for file in $files
do
  clang-format -i $file
done
