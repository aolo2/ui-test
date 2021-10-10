#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
echo "Building at: $SCRIPT_DIR"
pushd $SCRIPT_DIR >/dev/null

# echo "Entering directory: src"
pushd src >/dev/null
mkdir -p ../build

# echo -n "Compiling main.c..."
# gcc main.c -g -O0 -o ../build/ui -lX11 -lXext -Wall -Wextra -fsanitize=address
# echo " Done. Built ../build/ui"

# echo -n "Compiling widgets.c..."
gcc widgets.c -g -O0 -o ../build/widgets -lX11 -lXext -Wall -lm -Wextra -Wno-unused-function -Wno-unused-variable -fsanitize=address
# echo " Done. Built ../build/widgets"

# echo "Leaving directory: src"
popd >/dev/null

# echo "Leaving project directory"
popd >/dev/null
