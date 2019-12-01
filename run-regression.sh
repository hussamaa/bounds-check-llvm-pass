#!/bin/sh

TESTS_FOLDER=./regression/static
TEST_FILES="$(ls $TESTS_FOLDER)"

BYTECODE_PLAIN_FOLDER=./build/bytecode/plain
BYTECODE_TRANSFORMED_FOLDER=./bytecode/transformed
BYTECODE_TRANSFORMED_OPTIMIZED_FOLDER=./bytecode/transformed+optimized
mkdir -p $BYTECODE_PLAIN_FOLDER $BYTECODE_TRANSFORMED_FOLDER $BYTECODE_TRANSFORMED_OPTIMIZED_FOLDER

BYTECODE_GENERATION_COMMAND="clang -S -emit-llvm "
OPT_PASS_COMMAND="opt -load build/libLLVMBoundsCheck.so -bounds-check "

expect_runtime=0;
echo "****** regression tests ******"
echo ""

for tc in $TEST_FILES; do
  echo -n "$tc"
  is_fail=$(echo $tc | grep "FAIL" | wc -l)
  $($BYTECODE_GENERATION_COMMAND $TESTS_FOLDER/$tc -o $BYTECODE_PLAIN_FOLDER/$tc.ll 2> /dev/null)
  $($OPT_PASS_COMMAND $BYTECODE_PLAIN_FOLDER/$tc.ll > /dev/null 2> /dev/null)
  pass_error_level=$?
  if [ $is_fail -eq   $pass_error_level ]; then
    echo " [OK]"
  else
    echo " [FAIL]"
  fi  
  #echo "Generating bytecode transformed (with bounds checks)..."
  #$($BYTECODE_GENERATION_COMMAND $TESTS_FOLDER/$tc -o $BYTECODE_TRANSFORMED_FOLDER/$tc.ll 2> /dev/null)
  #echo "Generating bytecode transformed and optimized..."
  #$($BYTECODE_GENERATION_COMMAND $TESTS_FOLDER/$tc -o $BYTECODE_TRANSFORMED_OPTIMIZED_FOLDER/$tc.ll 2> /dev/null)
  #echo "[OK]"
  #echo ""
done
echo " "
echo "Date: \n$(date)"
echo "******************************"