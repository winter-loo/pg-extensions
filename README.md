# snippets for pgsql extension

# files

## control file

- in `key = value` form

## SQL script file

functions, procedures, types, operators are in this file.

# upgrading workflow

- create new file `utils--<current_version>--<next_version>.sql`
- add the new filename to Makefile `DATA`
- change `default_version` inside utils.control
- write code in the new file

Then, shell commands:

```shell
make install
```

Then, pgsql commands:

```sql
-- import `utils` extension for the first time
create extension utils;

-- upgrade only
alter extension utils update;
```

# simple way

```shell
./make_c_extension.sh foo
cd foo
./install.sh
```