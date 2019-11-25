#!/bin/sh

TESTS_FOLDER=./test-cases/
TEST_FILES="$(ls $TESTS_FOLDER)"

BYTECODE_PLAIN_FOLDER=./bytecode/plain
BYTECODE_TRANSFORMED_FOLDER=./bytecode/transformed
BYTECODE_TRANSFORMED_OPTIMIZED_FOLDER=./bytecode/transformed+optimized
mkdir -p $BYTECODE_PLAIN_FOLDER $BYTECODE_TRANSFORMED_FOLDER $BYTECODE_TRANSFORMED_OPTIMIZED_FOLDER

BYTECODE_GENERATION_COMMAND="clang -S -emit-llvm "

for tc in $TEST_FILES; do
  echo "File -> $tc"
  echo "Generating bytecode plain..."
  $($BYTECODE_GENERATION_COMMAND $TESTS_FOLDER/$tc -o $BYTECODE_PLAIN_FOLDER/$tc.ll 2> /dev/null)
  echo "Generating bytecode transformed (with bounds checks)..."
  $($BYTECODE_GENERATION_COMMAND $TESTS_FOLDER/$tc -o $BYTECODE_TRANSFORMED_FOLDER/$tc.ll 2> /dev/null)
  echo "Generating bytecode transformed and optimized..."
  $($BYTECODE_GENERATION_COMMAND $TESTS_FOLDER/$tc -o $BYTECODE_TRANSFORMED_OPTIMIZED_FOLDER/$tc.ll 2> /dev/null)
  echo "[OK]"
  echo ""
done

