#!/usr/bin/env bash
set -e

# Perform updates in this directory.
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
cd "$DIR"

git submodule update --init

# Need to init the submodule in SPIRV-Tools
cd Compile/SPIRV-Tools
git submodule update --init
