#!/usr/bin/env bash

# Install pysh
echo "(1/4) Installing pysh..."
cmake -S . -B build
cd build && make && sudo make install

# Transpile pysh to Python
echo "(2/4) Transpiling pysh to Python..."
cd ../tests || exit
pysh tests.pysh
mv out.py test_all.py

# Run tests
echo "(3/4) Running tests..."
python3 -m pip install pytest
pytest

# Run CTest
echo "(4/4) Running CTest..."
cd ../build
ctest -R test --output-on-failure
