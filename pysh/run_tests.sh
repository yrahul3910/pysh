#!/usr/bin/env bash

# Install pysh
echo "(1/5) Installing pysh..."
cmake -S . -B build
cd build && make && sudo make install

# Transpile pysh to Python
echo "(2/5) Transpiling pysh to Python..."
cd ../tests || exit
pysh tests.pysh
mv out.py test_all.py

# Run tests
echo "(3/5) Running tests..."
python3 -m pip install pytest
pytest

# Run CTest
echo "(4/5) Running CTest..."
cd ../build
make test

echo "(5/5) Uploading coverage report to Codecov..."
curl --retry 5 -s https://codecov.io/bash > codecov.bash && bash codecov.bash -Z -X gcov
