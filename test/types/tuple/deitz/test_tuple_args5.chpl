def f((x, (y, z)): (int, (int, int)) = (4, (5, 6))) {
  writeln(x + y + z);
}

var t = tuple(1, (2, 3));
f(t);
f();
