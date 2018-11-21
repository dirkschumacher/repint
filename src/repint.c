// inspired by Luke Tierney and the R Core Team
// https://github.com/ALTREP-examples/Rpkg-mutable/blob/master/src/mutable.c
// and Romain François https://purrple.cat/blog/2018/10/21/lazy-abs-altrep-cplusplus/

#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>
#include <R_ext/Altrep.h>

// modelled after the mutable.c example

// a class for integer
static R_altrep_class_t repint_integer_class;

// a function to construct a new lazy altrep version of rep.int
static SEXP make_repint(SEXP spec) {
  R_altrep_class_t class_t;
  int n_data = LENGTH(spec);
  int type_spec = TYPEOF(spec);
  if (type_spec != VECSXP || n_data != 2) {
    error("sorry, spec needs to be a tuple");
  }
  SEXP data = VECTOR_ELT(spec, 0);
  int type_data = TYPEOF(data);
  SEXP times = VECTOR_ELT(spec, 1);
  SEXP vec = PROTECT(allocVector(VECSXP, 2));
  SET_VECTOR_ELT(vec, 0, data);
  SET_VECTOR_ELT(vec, 1, times);

  switch(type_data) {
    case INTSXP:
      class_t = repint_integer_class;
      break;
    default:
      error("not supported", type2char(type_data));
  }
  SEXP val = R_new_altrep(class_t, vec, R_NilValue);
  UNPROTECT(1);
  return val;
}

// below are all the different methods defined

static SEXP repint_Duplicate(SEXP x, Rboolean deep) {
  return make_repint(R_altrep_data1(x));
}

Rboolean repint_Inspect(SEXP x, int pre, int deep, int pvec,
                         void (*inspect_subtree)(SEXP, int, int, int)) {
  int is_lazy = R_altrep_data2(x) == R_NilValue;
  Rprintf(" rep.int %s %s\n", type2char(TYPEOF(x)), is_lazy ? "lazy" : "materialized");
  SEXP data2 = R_altrep_data2(x);
  if (data2) {
    inspect_subtree(data2, pre, deep, pvec);
  }
  return TRUE;
}


#define TIMES(x) INTEGER(VECTOR_ELT(R_altrep_data1(x), 1))[0]
#define VAL(x) INTEGER(VECTOR_ELT(R_altrep_data1(x), 0))[0]

static R_xlen_t repint_Length(SEXP x) {
  return TIMES(x);
}

static void* repint_Dataptr(SEXP x, Rboolean writeable) {
  SEXP data2 = R_altrep_data2(x);
  R_xlen_t times = repint_Length(x);
  int val = VAL(x);
  if (data2 == R_NilValue) {
    data2 = PROTECT(allocVector(INTSXP, times));
    for (R_xlen_t i = 0; i < times; i++) {
      INTEGER(data2)[i] = val;
    }
    R_set_altrep_data2(x, data2);
    UNPROTECT(1);
  }
  return DATAPTR(data2);
}

static const void* repint_Dataptr_or_null(SEXP x) {
  SEXP data2 = R_altrep_data2(x);
  if (data2 == R_NilValue) return NULL;
  return DATAPTR_OR_NULL(data2);
}

// ALTINTEGER_METHODS

static int repint_integer_Elt(SEXP x, R_xlen_t i) {
  return VAL(x);
}

static
R_xlen_t repint_integer_Get_region(SEXP x, R_xlen_t i, R_xlen_t n, int *buf) {
  R_xlen_t ii;
  R_xlen_t times = INTEGER(VECTOR_ELT(R_altrep_data1(x), 1))[0];
  int val = INTEGER(VECTOR_ELT(R_altrep_data1(x), 0))[0];
  R_xlen_t max_n = times < n ? times : n;
  for (ii = i; ii < max_n; ii++) {
    buf[ii] = val;
  }
  return max_n;
}

SEXP repint_integer_Sum(SEXP x, Rboolean narm)
{
  int val = VAL(x);
  int times = TIMES(x);
  if (val == NA_INTEGER) {
    return ScalarInteger(narm ? 0 : val);
  }
  return ScalarInteger(val * times);
}

SEXP repint_integer_Max(SEXP x, Rboolean narm)
{
  int val = VAL(x);
  int times = TIMES(x);
  if (val == NA_INTEGER) {
    return ScalarInteger(narm ? INFINITY : val);
  }
  return ScalarInteger(val);
}

SEXP repint_integer_Min(SEXP x, Rboolean narm)
{
  int val = VAL(x);
  int times = TIMES(x);
  if (val == NA_INTEGER) {
    return ScalarInteger(narm ? -INFINITY : val);
  }
  return ScalarInteger(val);
}

int repint_integer_Sorted(SEXP x)
{
  return 1;
}

int repint_integer_No_NA(SEXP x)
{
  return VAL(x) != NA_INTEGER;
}

SEXP do_make_repint(SEXP spec)
{
  if (MAYBE_REFERENCED(spec)) {
    spec = duplicate(spec);
  }
  PROTECT(spec);
  SEXP val = make_repint(spec);
  UNPROTECT(1);
  return val;
}

static const R_CallMethodDef CallEntries[] = {
  {"make_repint", (DL_FUNC) &do_make_repint, 1},
  {NULL, NULL, 0}
};

void R_init_repint(DllInfo *dll)
{

  R_altrep_class_t cls =
    R_make_altinteger_class("repint_integer", "repint", dll);
  repint_integer_class = cls;

  /* override ALTREP methods */
  R_set_altrep_Duplicate_method(cls, repint_Duplicate);
  R_set_altrep_Inspect_method(cls, repint_Inspect);
  R_set_altrep_Length_method(cls, repint_Length);

  /* override ALTVEC methods */
  R_set_altvec_Dataptr_method(cls, repint_Dataptr);
  R_set_altvec_Dataptr_or_null_method(cls, repint_Dataptr_or_null);

  /* override ALTINTEGER methods */
  R_set_altinteger_Elt_method(cls, repint_integer_Elt);
  R_set_altinteger_Get_region_method(cls, repint_integer_Get_region);
  R_set_altinteger_Sum_method(cls, repint_integer_Sum);
  R_set_altinteger_Max_method(cls, repint_integer_Max);
  R_set_altinteger_Min_method(cls, repint_integer_Min);
  R_set_altinteger_No_NA_method(cls, repint_integer_No_NA);
  R_set_altinteger_Is_sorted_method(cls, repint_integer_Sorted);

  R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
}
