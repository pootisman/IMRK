#!/bin/bash

if [ ! -d bin ]
then
  echo "Directory bin does not exist, creating it."
  mkdir bin
else
  echo "Bin exists, continue building."
fi
