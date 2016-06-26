#!/bin/bash

echo "printing ls"
ls

echo "printing AMCT env vars"
echo "AMCT_TEMP_DIR $AMCT_TEMP_DIR"
echo "AMCT_TEST_NUM $AMCT_TEST_NUM"
echo "AMCT_TEST_NAME $AMCT_TEST_NAME"

echo "printing env vars"
env | grep AMCT