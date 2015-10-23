#!/bin/sh
for t in $*; do
  echo ===== $t =====
  ./$t;
done
