# Copyright 2022 Henry Schreiner
# SPDX-License-Identifier: Apache-2.0

set -ex

echo "Running clang-format against branch $GH_BRANCH, with hash $BASE_COMMIT"
COMMIT_FILES=$(git diff --name-only $BASE_COMMIT)
RESULT_OUTPUT="$(git-clang-format --commit $BASE_COMMIT --diff --binary `which clang-format` $COMMIT_FILES)"

if [ "$RESULT_OUTPUT" == "no modified files to format" ] \
  || [ "$RESULT_OUTPUT" == "clang-format did not modify any files" ] ; then

  echo "clang-format passed."
  exit 0
else
  echo "clang-format check failed."

  exit 1
fi
