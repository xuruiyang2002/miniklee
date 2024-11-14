#!/bin/bash

make clean && make

# Set SRC and OUT from the command-line arguments or default values if not provided
SRC=${1:-./test/multi_branch}

# Run make with the specified SRC and OUT values
make run SRC="$SRC".c OUT="$SRC".ll

