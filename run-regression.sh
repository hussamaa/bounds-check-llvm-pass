#!/bin/sh

TESTS_FOLDER=./regression/static
TEST_FILES="$(ls $TESTS_FOLDER)"

BYTECODE_FOLDER=./build/bytecode/
rm -rf $BYTECODE_FOLDER
mkdir -p $BYTECODE_FOLDER

BYTECODE_GENERATION_COMMAND="clang -S -emit-llvm "
OPT_PASS_COMMAND="opt -load build/libLLVMBoundsCheck.so -bounds-check "

echo "****** regression tests ******"
echo ""

for tc in $TEST_FILES; do
  echo -n "$tc"
  is_fail=$(echo $tc | grep "FAIL" | wc -l)
  $($BYTECODE_GENERATION_COMMAND $TESTS_FOLDER/$tc -o $BYTECODE_FOLDER/$tc.bc 2> /dev/null)
  $($OPT_PASS_COMMAND $BYTECODE_FOLDER/$tc.bc > /dev/null 2> /dev/null)
  pass_error_level=$?
  if [ $is_fail -eq $pass_error_level ]; then
    echo " [OK]"
  else
    echo " [FAIL]"
  fi  
  echo -n "$tc (with runtime check)"
  $($OPT_PASS_COMMAND $BYTECODE_FOLDER/$tc.bc --force-runtime-checks -o $BYTECODE_FOLDER/$tc.ll > /dev/null 2> /dev/null)
  $(llc -march=x86-64 -relocation-model=pic $BYTECODE_FOLDER/$tc.ll -o $BYTECODE_FOLDER/$tc.s)  
  $(gcc $BYTECODE_FOLDER/$tc.s -o $BYTECODE_FOLDER/$tc.exe)
  initial_time=$(date +%s)
  ./$BYTECODE_FOLDER/$tc.exe > /dev/null 2> /dev/null  
  runtime_error_level=$?
  final_time=$(date +%s)
  execution_time=$((final_time - initial_time))
  if [ $is_fail -eq 0 ] && [ $runtime_error_level -eq 0 ]; then
    echo " [OK] in $execution_time seconds"
  elif [ $is_fail -eq 1 ] && [ $runtime_error_level -eq 134 ]; then
    echo " [OK] in $execution_time seconds"
  else
    echo " [FAIL]"
  fi
done

echo " "
echo "Date: \n$(date)"
echo "******************************"