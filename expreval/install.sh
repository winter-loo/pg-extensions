#!/bin/sh -e

bear=$(command -v bear || echo "")

if [ -z "$bear" ]; then
  echo "HINT: bear is not installed. Please install bear to generate the compile_commands.json file."
  echo "You can install bear by running 'sudo apt-get install bear' or build it from source at https://github.com/rizsotto/Bear"
  echo "After installing bear, run 'make clean' and then './install.sh' to generate the compile_commands.json file."
  make
else
  test -f compile_commands.json || bear -- make
fi
make install

psql -c "drop extension if exists expreval;create extension expreval;"
