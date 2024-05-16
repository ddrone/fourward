: if immediate
  here
  0 write-0branch
;

: then immediate
  here
  modify-code
;

: else immediate
  here
  0 write-branch
  swap here modify-code
;

: mod
  dup
  0 <=
  if
    0 swap -
  then
;

: silly
  dup
  0 <=
  if
    0 swap -
  else
    2 *
  then
;

5 mod -10 mod + print
5 silly -10 silly + print
