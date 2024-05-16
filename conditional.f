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

: begin immediate
  here
;

: until immediate
  write-0branch
;

: '(' 40 ;
: ')' 41 ;

: ( immediate
  0
  begin
    read
    dup '(' =
    if
      swap 1 + swap
    then
    ')' =
    if
      1 -
    then
    dup
  until
;

( Now it should be possible to leave comments, I think? )

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

dump
5 mod -10 mod + print
5 silly -10 silly + print
