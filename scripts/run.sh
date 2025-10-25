#!/bin/bash

if [ "$1" = "release" ]
then
    ./build/release/tessellation
else
    ./build/debug/tessellation
fi