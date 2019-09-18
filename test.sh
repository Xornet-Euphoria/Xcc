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

try 3 "foo = 3; return foo;"
try 4 "foo = 2; return foo * 2;"
try 5 "return 5; return 1;"
try 1 "foo = 3; if (foo == 3) return 1;"
try 0 "foo = 2; if (foo % 2 == 1) return 1; return 0;"
try 1 "foo = 2; if (foo == 2) return 1; else return 0;"
try 1 "foo = 2; if (foo % 2 == 1) return 0; else return 1;"
try 1 "foo = 2; if (foo == 1) return 0; else if (foo == 2) return 1;"
try 1 "foo = 2; if (foo == 2) if (foo % 2 == 0) return 1;"
try 0 "foo = 2; if (foo == 3) return 1; else if (foo == 4) return 2; else return 0;"
try 10 "i = 0; while (i < 10) i = i + 1; return i;"
try 55 "sum = 0; for (i = 1; i <= 10; i = i + 1) sum = sum + i; return sum;"
try 3 "foo = 2; if (foo == 2) {foo = 3;} return foo;"
try 0 "foo = 2; while (foo > 0) {foo = foo - 1;} return foo;"

echo OK
