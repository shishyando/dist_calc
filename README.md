### Distributed math tasks evaluation

A master discovers workers via UDP broadcast and assigns tasks for evaluation:

```c++
struct Task {
    double left;
    double right;
    double Evaluate() {
        return right - left;  // Placeholder
    }
}
```

Results are aggregated by the master, properly treating workers' failures and recoveries.
