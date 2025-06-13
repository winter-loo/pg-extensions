# API exercise for expression evaluation

```sql
select eval('1 + 1');
select eval('least(2, 3, 4');
select eval($$'a' || 'b'$$);
```

## core API

* ExecInitExpr
* ExecEvalExpr
