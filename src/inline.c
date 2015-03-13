/*
 * cycle until the current processor cycles exceeds the given one
 */

static inline void
brute_wait_until(cycles_t * t)
{
    while (get_cycles() < *t);
}

