#!/bin/bash -e

pg_cmd=$(command -v postgres)
if [ -z "$pg_cmd" ]; then
  echo -en "\033[31;1mmanually install a postgresql first\n\033[0;m"
  exit 0
fi

pg_home_dir=$(dirname $(dirname $pg_cmd))

id=$(openssl rand -hex 32 | cut -c 1-7)
venv_dir=pg-$id
if [ ! -d "$venv_dir" ]; then
  mkdir $venv_dir

  cp -r $pg_home_dir/bin $venv_dir
  cp -r $pg_home_dir/lib $venv_dir
  cp -r $pg_home_dir/share $venv_dir
  cp activate $venv_dir/.activate
  sed -i "s@VIRTUAL_ENV=<TO_BE_SET>@VIRTUAL_ENV=$(pwd)/$venv_dir@g" $venv_dir/.activate

  unset PGHOME
  unset PGDATA
  unset PGPORT
  export PGHOME=$venv_dir
  export PGDATA=$PGHOME/data
  export PATH=$PGHOME/bin:$PATH
  export LD_LIBRARY_PATH=$PGHOME/lib:$LD_LIBRARY_PATH

  mkdir $PGDATA $PGHOME/log
  port=$((12000 + RANDOM % (65535 - 12000)))
  initdb
  sed -i "s%^.*port *= *[0-9]\+%port = $port%g" $PGDATA/postgresql.conf
  pg_ctl start -l $PGHOME/log/logfile

  createdb -p $port testdb

  echo -en "Now execute the following commands:\n\033[32;1m \
    source $PGHOME/.activate\n \
    psql -p $port testdb \
    \033[0m\n"

  echo -en "Restore the original environment by:\n\033[32;1m \
    pg_ctl stop\n \
    deactivate \
    \033[0m\n"
fi
