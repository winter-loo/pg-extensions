# workerpool

Before starting postgres, set followings in postgres.conf:

```
shared_preload_libraries = 'workerpool'
```

In background, two tables are created: counted_1, counted_2. There are two background workers
continuously inserting data into two tables concurrently.
