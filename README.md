
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
stopifnot(all.equal(rep.int(10L, 1e6L), rep_int(10L, 1e6L)))
bench::mark(
  rep.int(10L, 1e6L),
  rep_int(10L, 1e6L)
)
#> # A tibble: 2 x 10
#>   expression    min    mean  median      max `itr/sec` mem_alloc  n_gc
#>   <chr>      <bch:> <bch:t> <bch:t> <bch:tm>     <dbl> <bch:byt> <dbl>
#> 1 rep.int(1…    1ms  1.83ms  1.36ms   3.89ms      547.    3.81MB    18
#> 2 rep_int(1… 35.7µs 44.85µs 41.21µs 638.52µs    22297.        0B    40
#> # ... with 2 more variables: n_itr <int>, total_time <bch:tm>
```

`head` and `sort` both work with ALTREP, so they can take advantage of
the `rep_int`
implementation.

``` r
stopifnot(all.equal(head(sort(rep.int(10L, 1e6L))), head(sort(rep_int(10L, 1e6L)))))
bench::mark(
  head(sort(rep.int(10L, 1e6L))),
  head(sort(rep_int(10L, 1e6L)))
)
#> # A tibble: 2 x 10
#>   expression     min    mean median    max `itr/sec` mem_alloc  n_gc n_itr
#>   <chr>      <bch:t> <bch:t> <bch:> <bch:>     <dbl> <bch:byt> <dbl> <int>
#> 1 head(sort…  5.86ms  6.95ms  6.9ms 9.46ms      144.    11.4MB    23    34
#> 2 head(sort… 58.63µs 81.49µs 63.5µs 1.66ms    12271.        0B     8  5854
#> # ... with 1 more variable: total_time <bch:tm>
```

We also know that the sequence is sorted:

``` r
is.unsorted(rep_int(42L, 10L))
#> [1] FALSE
```

It also supports materialization:

``` r
a <- rep_int(42L, 10L)
.Internal(inspect(a))
#> @7ffb9e12aba8 13 INTSXP g0c0 [NAM(3)]  rep.int integer lazy
#>   @7ffb9c801ee0 00 NILSXP g1c0 [MARK,NAM(3)]
```

The second line is the description of the materialized content.
Currently Nil. If you want to learn more about that approach, take a
look at Romain’s [blog
post](https://purrple.cat/blog/2018/10/21/lazy-abs-altrep-cplusplus/).

Still lazy:

``` r
b <- sort(a)
.Internal(inspect(a))
#> @7ffb9e12aba8 13 INTSXP g0c0 [NAM(3)]  rep.int integer lazy
#>   @7ffb9c801ee0 00 NILSXP g1c0 [MARK,NAM(3)]
```

Still lazy:

``` r
a[5]
#> [1] 42
.Internal(inspect(a))
#> @7ffb9e12aba8 13 INTSXP g0c0 [NAM(3)]  rep.int integer lazy
#>   @7ffb9c801ee0 00 NILSXP g1c0 [MARK,NAM(3)]
```

Still lazy:

``` r
a[4:6]
#> [1] 42 42 42
.Internal(inspect(a))
#> @7ffb9e12aba8 13 INTSXP g0c0 [NAM(3)]  rep.int integer lazy
#>   @7ffb9c801ee0 00 NILSXP g1c0 [MARK,NAM(3)]
```

Materialized as soon as it is printed:

``` r
b
#>  [1] 42 42 42 42 42 42 42 42 42 42
.Internal(inspect(a))
#> @7ffb9e12aba8 13 INTSXP g0c0 [NAM(3)]  rep.int integer materialized
#>   @7ffb9c471e58 13 INTSXP g0c4 [] (len=10, tl=0) 42,42,42,42,42,...
```
