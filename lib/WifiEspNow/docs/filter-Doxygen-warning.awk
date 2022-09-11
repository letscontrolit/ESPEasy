#!/usr/bin/gawk -f
/warning: The following parameters? of .* not documented:/ {
  skip = 1
  next
}
$0 ~ /^  parameter/ {
  if (!skip) {
    print
  }
}
$0 !~ /^  / {
  skip = 0
  print
}