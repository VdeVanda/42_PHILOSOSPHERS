*This project has been created as part of the 42 curriculum by vabatist.*

---
```text
██████╗ ██╗  ██╗██╗██╗      ██████╗ ███████╗ ██████╗ ██████╗ ██╗  ██╗███████╗██████╗ ███████╗
██╔══██╗██║  ██║██║██║     ██╔═══██╗██╔════╝██╔═══██╗██╔══██╗██║  ██║██╔════╝██╔══██╗██╔════╝
██████╔╝███████║██║██║     ██║   ██║███████╗██║   ██║██████╔╝███████║█████╗  ██████╔╝███████╗
██╔═══╝ ██╔══██║██║██║     ██║   ██║╚════██║██║   ██║██╔═══╝ ██╔══██║██╔══╝  ██╔══██╗╚════██║
██║     ██║  ██║██║███████╗╚██████╔╝███████║╚██████╔╝██║     ██║  ██║███████╗██║  ██║███████║
╚═╝     ╚═╝  ╚═╝╚═╝╚══════╝ ╚═════╝ ╚══════╝ ╚═════╝ ╚═╝     ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚══════╝
```

An implementation of the Dining Philosophers problem in C using POSIX threads (`pthread`) and mutexes. The program simulates philosophers who cycle through thinking, eating, and sleeping while safely sharing forks to avoid deadlocks and race conditions. A monitor thread supervises the simulation to detect philosopher death or completion when an optional "must eat" count is provided.

## Build

```bash
make          # build the philo executable
make clean    # remove object files
make fclean   # remove objects and the executable
make re       # rebuild from scratch
```

Build details:
- Compiler: `cc`
- Flags: `-Wall -Wextra -Werror -g -I.`
- Output: `./philo`

## Usage

```bash
./philo <n_philos> <time_die> <time_eat> <time_sleep> [must_eat]
```

Arguments (milliseconds unless stated otherwise):
- `n_philos`: number of philosophers (≥ 1)
- `time_die`: time a philosopher can go without eating before dying
- `time_eat`: time it takes a philosopher to eat
- `time_sleep`: time a philosopher sleeps after eating
- `must_eat` (optional): if provided (> 0), the simulation ends when every philosopher has eaten at least this many times

Examples:
```bash
# 5 philosophers, die after 800ms without food, eat for 200ms, sleep for 200ms
./philo 5 800 200 200

# Same as above, and stop when each philosopher has eaten 5 times
./philo 5 800 200 200 5
```

## Output Format

Each event prints a timestamp (ms since simulation start), the philosopher ID, and a message:
- Green lines: normal events (`has taken a fork`, `is eating`, `is sleeping`, `is thinking`)
- Red line: a philosopher died (`died`)

Example (colors shown in a compatible terminal):
```
<timestamp> <id> has taken a fork
<timestamp> <id> is eating
<timestamp> <id> is sleeping
<timestamp> <id> is thinking
<timestamp> <id> died
```

Notes:
- Printing is synchronized with a `print_m` mutex to avoid interleaved messages.
- When the simulation stops (death or all philosophers are full), further messages are suppressed.

## Design Notes

- **Threads:**
  - One thread per philosopher handles its routine (taking forks, eating, sleeping, thinking).
  - A separate monitor thread checks for death (`time since last meal > time_die`) and whether all philosophers reached `must_eat`.
- **Mutexes:**
  - `forks[]`: one mutex per fork.
  - `print_m`: protects console output.
  - `last_meal_m`: protects updates/reads of `last_meal`.
  - `eat_count_m`: tracks `eat_count` and counts philosophers who reached `must_eat`.
  - `sim-run_m`: guards the `sim_run` flag.
- **Termination conditions:**
  - A philosopher dies: monitor prints death in red and stops the simulation.
  - Optional `must_eat` satisfied for all philosophers: monitor stops the simulation.
- **Single philosopher case:**
  - With only one philosopher, they cannot acquire two forks; the routine waits until `time_die` and the monitor ends the simulation.
- **Odd number of philosophers (anti-starvation):**
  - With an odd number of philosophers, asymmetric fork access can cause starvation (some philosophers monopolize forks while others never eat).
  - Solution: ALL philosophers execute extended thinking time when `num_philos` is odd:
    ```
    thinking_time = (time_eat × 2) - time_sleep
    ```
  - This ensures that during one philosopher's pause, at least 2 others can complete eating cycles, preventing monopolization and ensuring fair fork distribution.
  - Even philosopher counts naturally alternate and don't need this adjustment.
- **Initial Desynchronization Delay:**
  - To prevent all philosophers from simultaneously competing for forks at startup, even-numbered philosophers sleep briefly before entering their main routine loop.
    ```
    if (philo->id % 2 == 0)
    sleep_philo(philo, philo->table->time_die / 2);
    ```
    Optimized Alternative
    For optimal desynchronization across all input ranges while maintaining safety:
    ```
    void *routine(void *ph)
    {
    t_philo *philo;
    long delay;

    philo = (t_philo *)ph;

    // Desynchronization with adaptive delay
    if (philo->id % 2 == 0)
    {
        delay = philo->table->time_eat / 2;
        if (delay > philo->table->time_die / 2)
            delay = philo->table->time_die / 2;

        if (sleep_philo(philo, delay) == false)
            return (NULL);  // Exit immediately if simulation stopped
    }

    while (1)
    {
        if (pick_forks(philo) == false)
            return (NULL);
        if (eating(philo) == false)
            return (NULL);
        if (philo_msg(philo, "is sleeping\n", SLEEP) == false)
            return (NULL);
        if (philo_msg(philo, "is thinking\n", THINK) == false)
            return (NULL);
    }
    return (NULL);
    }
    ```
- **Timing:**
  - Timestamps are generated from system time in milliseconds; the program maintains a start time and prints time deltas.

## Project Layout

- `Makefile`: build targets and compiler flags
- `philo.h`: shared types and function declarations
- `srcs/creating.c`: parse and validate input, allocate and initialize table/philos/forks
- `srcs/monitor.c`: monitor thread to detect death and completion
- `srcs/philo_utils.c`: helper functions (printing, fork management, mutex init)
- `srcs/utils.c`: general utilities (time, `ft_calloc`, `ft_atol`, cleanup)
- `srcs/philo.c`: philosopher routine and core logic (entry point)

## Troubleshooting

- **Input validation:**
  - All times and counts must be positive and fit in `INT_MAX`.
  - If an invalid argument is provided, the program prints an error and exits.
- **Environment:**
  - Tested on Linux with POSIX threads. Ensure your system provides `pthread`.
- **Performance:**
  - Large philosopher counts or very small times may stress scheduling; adjust arguments if outputs look erratic.

## How it works



## Testing

Verify the absence of data races:

- **Helgrind**
```bash
valgrind -s --tool=helgrind timeout 5s ./philo 5 800 200 200 2
```

- **DRD**
```bash
valgrind -s --tool=drd ./philo 5 800 200 200 2
```

**Memory leaks:**
```bash
valgrind -s --leak-check=full --show-leak-kinds=all ./philo 5 800 200 200 2
```
```bash
valgrind --leak-check=full timeout 5s ./philo 5 800 200 200
```

**How many times each philosopher has eaten:**
- Per-philosopher counts (echo live lines and summarize at end)
```bash
./philo 5 800 200 200 7 | tee /dev/tty | awk '/is eating/ {count[$2]++} END {for (p in count) print p, count[p]}' | sort -n
```
- Pretty summary with labels per philosopher
```bash
./philo 5 800 200 200 7 | awk '/is eating/ {id=$2; cnt[id]++} END {for (id in cnt) printf("philo %s ate %d times\n", id, cnt[id])}'
```
- "<id> is eating <count>" by philosopher (sorted)
```bash
./philo 5 800 200 200 7 | awk '/is eating/ {cnt[$2]++} END {for (id in cnt) printf("%d is eating %d\n", id, cnt[id])}' | sort -n
```
- "<id> <count>" pairs (sorted)
```bash
./philo 5 800 200 200 7 | awk '/is eating/ {cnt[$2]++} END {for (id in cnt) print id, cnt[id]}' | sort -n
```
- Using grep + uniq to get "<id> <count>"
```bash
./philo 5 800 200 200 7 | grep "is eating" | awk '{print $2}' | sort -n | uniq -c | awk '{print $2, $1}'
```
- Only the counts (one per philosopher that ate at least once)
```bash
./philo 5 800 200 200 7 | grep "is eating" | awk '{print $2}' | sort -n | uniq -c | awk '{print $1}'
```
