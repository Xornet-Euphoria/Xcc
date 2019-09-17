#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./xcc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try 1 "a=1; a;"
try 2 "a = 1; b = 1; a + b;"
try 7 "z = (4 + 3) * 7; z / 7;"
try 4 "a = b = 2; a+b;"
try 1 "a = 1; b = 2; a<b;"
try 7 "foo = 3; bar = 4; foo + bar;"
try 0 "foo = 3; bar = 4; foo > bar;"
try 1 "foo = 3; bar = 4; foo < bar;"

echo OK
