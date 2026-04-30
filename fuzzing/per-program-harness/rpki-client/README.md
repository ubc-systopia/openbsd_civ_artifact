## Setting Up the `ptr_checker` Library
```sh
cp -r /path/to/ptr_checker ./ptr_checker
cd ptr_checker

make ENABLE_PTR_CHECK=1 ENABLE_MSAN_CHECK=0

export BUFFER_CHECKER_ROOT=$PWD
export AFL_PRELOAD="${BUFFER_CHECKER_ROOT}/libbuffer_check.so"
cd ..
```
## Setup
```sh
git clone https://github.com/rpki-client/rpki-client-portable.git
cd rpki-client-portable/
git checkout 71aed40a0e73d1fa0d7e4591a02bd36b58b7c350

```
