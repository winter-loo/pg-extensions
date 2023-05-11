# utils for PL/pgSQL

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
