For successful execution, you should apply the patch ./0001-remove-condition-for-parallel-insert.patch
on postgres source repo on tag REL_13_8.

```shell
git checkout https://git.postgresql.org/git/postgresql.git
git checkout REL_13_8
git apply ./0001-remove-condition-for-parallel-insert.patch
```
