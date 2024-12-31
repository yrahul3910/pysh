#!/usr/bin/env bash

# Install pysh
echo "(1/4) Installing pysh..."
cmake -S . -B build
cd build && make && sudo make install

# Transpile pysh to Python
echo "(2/4) Transpiling pysh to Python..."
cd ../tests || exit
pysh tests.pysh
mv tests.py test_all.py

# Run tests
echo "(3/4) Running tests..."
python3 -m pip install pytest pytest-cov
pytest --cov=./ --cov-report=xml

# Run CTest
echo "(4/4) Running CTest..."
cd ../build
make test
