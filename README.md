
<!-- README.md is generated from README.Rmd. Please edit that file -->

# ALTREP rep.int

The goal of this package is to learn about ALTREP and to prototype an
ALTREP version of `rep.int` for scalar values. Not meant to be used in
real situations.

Base’s `rep.int(42, 10)` allocates an integer vector of 10 times 42.
Using ALTREP we can construct a version of `rep.int` (called `rep_int`
:)) that is lazy and does not allocate a full vector, yet still offers
the same functionality. In case a fully expanded version of the vector
is needed, it materializes it on the fly.

For example: the ALTREP system has a method to ask a data structure for
a value at a given index. Well, in this particular case the value is
always the same, so we can answer this question in constant time (see
below).

This proof of concept is based on the examples of [Luke
Tierney](https://github.com/ALTREP-examples/Rpkg-mutable/blob/master/src/mutable.c)
and the blog post of [Romain
François](https://purrple.cat/blog/2018/10/21/lazy-abs-altrep-cplusplus/).

## Installation

``` r
remotes::install_github("dirkschumacher/repint")
```

## Some examples

``` r
library(repint)
stopifnot(all.equal(rep.int(10, 1e6), rep_int(10, 1e6)))
bench::mark(
  rep.int(10, 1e6),
  rep_int(10, 1e6)
)
#> # A tibble: 2 x 10
#>   expression     min    mean  median      max `itr/sec` mem_alloc  n_gc
#>   <chr>      <bch:t> <bch:t> <bch:t> <bch:tm>     <dbl> <bch:byt> <dbl>
#> 1 rep.int(1…  1.92ms  2.72ms  2.15ms   8.02ms      367.    7.63MB    25
#> 2 rep_int(1… 35.63µs 42.51µs 40.48µs 342.84µs    23526.        0B    43
#> # ... with 2 more variables: n_itr <int>, total_time <bch:tm>
```

`head` and `sort` both work with ALTREP, so they can take advantage of
the `rep_int`
implementation.

``` r
stopifnot(all.equal(head(sort(rep.int(10, 1e6))), head(sort(rep_int(10, 1e6)))))
bench::mark(
  head(sort(rep.int(10, 1e6))),
  head(sort(rep_int(10, 1e6)))
)
#> # A tibble: 2 x 10
#>   expression    min   mean median     max `itr/sec` mem_alloc  n_gc n_itr
#>   <chr>      <bch:> <bch:> <bch:> <bch:t>     <dbl> <bch:byt> <dbl> <int>
#> 1 head(sort… 10.1ms 10.8ms 10.7ms  12.5ms      92.2    19.1MB    22     8
#> 2 head(sort…   58µs 69.7µs 63.5µs 788.6µs   14348.         0B    10  6808
#> # ... with 1 more variable: total_time <bch:tm>
```

We also know that the sequence is sorted:

``` r
is.unsorted(rep_int(42, 10))
#> [1] FALSE
```

It also supports materialization:

``` r
a <- rep_int(42, 10)
.Internal(inspect(a))
#> @7faca02e18d8 13 INTSXP g0c0 [NAM(3)]  rep.int integer lazy
#>   @7fac9d02c0e0 00 NILSXP g1c0 [MARK,NAM(3)]
```

The second line is the description of the materialized content.
Currently Nil. If you want to learn more about that approach, take a
look at Romain’s [blog
post](https://purrple.cat/blog/2018/10/21/lazy-abs-altrep-cplusplus/).

Still lazy:

``` r
b <- sort(a)
.Internal(inspect(a))
#> @7faca02e18d8 13 INTSXP g0c0 [NAM(3)]  rep.int integer lazy
#>   @7fac9d02c0e0 00 NILSXP g1c0 [MARK,NAM(3)]
```

Still lazy:

``` r
a[5]
#> [1] 42
.Internal(inspect(a))
#> @7faca02e18d8 13 INTSXP g0c0 [NAM(3)]  rep.int integer lazy
#>   @7fac9d02c0e0 00 NILSXP g1c0 [MARK,NAM(3)]
```

Still lazy:

``` r
a[4:6]
#> [1] 42 42 42
.Internal(inspect(a))
#> @7faca02e18d8 13 INTSXP g0c0 [NAM(3)]  rep.int integer lazy
#>   @7fac9d02c0e0 00 NILSXP g1c0 [MARK,NAM(3)]
```

Materialized as soon as it is printed:

``` r
b
#>  [1] 42 42 42 42 42 42 42 42 42 42
.Internal(inspect(a))
#> @7faca02e18d8 13 INTSXP g0c0 [NAM(3)]  rep.int integer materialized
#>   @7fac9ee5d6b8 13 INTSXP g0c4 [] (len=10, tl=0) 42,42,42,42,42,...
```
