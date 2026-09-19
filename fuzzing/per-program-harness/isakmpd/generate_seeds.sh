#!/bin/sh
set -eu

# Metadata is 73 bytes: compartment, instance, type index, payload size,
# FD controls, and 64 bytes of sequence metadata.  Both structured seeds use
# one payload slice and place a real generated FD on slice zero.
mkdir -p seeds

printf '\000\000\003\024\000\001\002\000\000\000' > seeds/bind
dd if=/dev/zero bs=1 count=63 >> seeds/bind 2>/dev/null
printf '\020\000\000\000' >> seeds/bind
printf '\020\002\000\000\000\000\000\000\000\000\000\000\000\000\000\000' >> seeds/bind

printf '\000\000\002\020\000\001\002\000\000\000' > seeds/setsockopt
dd if=/dev/zero bs=1 count=63 >> seeds/setsockopt 2>/dev/null
printf '\001\000\000\000\002\000\000\000\004\000\000\000' >> seeds/setsockopt
printf '\001\000\000\000' >> seeds/setsockopt
