#!/usr/bin/env bash

set -e

./build.sh
cat $@ | ./target/fourward
