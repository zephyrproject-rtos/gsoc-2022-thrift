#!/bin/sh
# Copyright 2022 Meta
# SPDX-License-Identifier: Apache-2.0

set -e

FILES="$(git diff --cached --name-only)"

for F in $FILES; do
  BN="$(basename $F)"

  if [ "$(basename $BN .sh)" != "$BN" ]; then
    # clang-format does not seem to process .sh files properly
    continue
  fi

  clang-format -i --style=file $F
  git add $F
done
