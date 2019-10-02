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

try 4 "fn() {
  return 3;
}

main(){
  foo = fn();
  return foo + 1;
}"
try 3 "main() {
  a = 51;
  b = 12;
  while (b != 0) {
    tmp = b;
    b = a % b;
    a = tmp;
  }
  return a;
}"

try 30 "fn(a,b) {
  a = a + 1;
  b = b + 1;
  return a * b;
}

main() {
  a = 4;
  b = 5;
  return fn(a, b);
}"

try 6 "f(n) {
  if (n == 1) {
    return 1;
  }
  return f(n - 1) * n;
}

main() {
  return f(3);
}"

try 5 "fb(n) {
  if (n == 1) {
    return 1;
  }
  if (n == 2) {
    return 1;
  }
  return fb(n - 2) + fb(n - 1);
}

main(){
  return fb(5);
}"

try 3 "main(){
  x = 3;
  y = &x;
  return *y;
}"

try 3 "main() {
  x = 3;
  y = 4;
  z = &y + 8;
  return *z;
}"

echo OK