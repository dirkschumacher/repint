#' ALTREP rep.int
#'
#' It only works with integers or values that can be converted to an integer
#'
#' @param x the value
#' @param times how many times
#'
#' @export
#' @useDynLib repint make_repint
rep_int <- function(x, times) {
  x <- as.integer(x)
  times <- as.integer(times)
  stopifnot(length(x) == 1)
  stopifnot(length(times) == 1, times >= 0)
  .Call(make_repint, list(x, times))
}