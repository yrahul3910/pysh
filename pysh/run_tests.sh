#!/usr/bin/env bash

# Install pysh
echo "(1/3) Installing pysh..."
cmake -S . -B build
cd build && make && sudo make install

# Transpile pysh to Python
echo "(2/3) Transpiling pysh to Python..."
cd ../tests || exit
pysh tests.pysh
mv out.py test_all.py

# Run tests
echo "(3/3) Running tests..."
python3 -m pip install pytest
pytest