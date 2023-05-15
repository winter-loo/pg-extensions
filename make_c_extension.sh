#!/bin/bash -e

# creat a new extension from the `hello_world` project

if [ $# -lt 1 ]; then
  echo "usage: $0 <extension_name> [extension description]"
  exit 1
fi

ext_name="$1"
ext_desc="$1"
[ $# -eq 2 ] && ext_desc="$2"

if [ -e $ext_name ]; then
  echo "'$ext_name' already exists"
  exit 2
fi

mkdir $ext_name

cp hello_world/Makefile $ext_name/
cp hello_world/install.sh $ext_name/
cp hello_world/hello_world--1.0.sql $ext_name/$ext_name--1.0.sql
cp hello_world/hello_world.c $ext_name/$ext_name.c
cp hello_world/hello_world.control $ext_name/$ext_name.control

find $ext_name -type f -exec sed -i "s/hello_world/$ext_name/g" \{\} \;
sed -i "s/comment = 'hello world'/comment = \'$ext_desc\'/g" $ext_name/$ext_name.control