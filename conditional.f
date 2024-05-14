: if immediate
  here
  0 write-0branch
;

: then immediate
  here
  modify-code
;

: mod
  dup
  0 <=
  if
    0 swap -
  then
;

5 mod -10 mod + print
