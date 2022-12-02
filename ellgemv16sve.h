#pragma once

#ifdef __ARM_FEATURE_SVE
#include <arm_sve.h>
#endif /* __ARM_FEATURE_SVE */

#if !(defined SW_PREFETCH_DIST_L1)
#define SW_PREFETCH_DIST_L1 5
#endif /* SW_PREFETCH_DIST_L1 */

#ifdef __ARM_FEATURE_SVE
static int ellgemv16sve(int num_rows, 
                        double *__restrict y,
                        int num_columns,
                        const double *__restrict x, 
                        int64_t ellsize,
                        int rowsize, 
                        const int *__restrict colidx,
                        const double *__restrict a,
                        const double *__restrict ad) {
#ifdef A64FXCPU
#pragma procedure scache_isolate_way L2 = L2WAYS
#pragma procedure scache_isolate_assign a, colidx
#endif /* A64FXCPU */

  if (rowsize != 16) return EINVAL;
  svbool_t pg = svptrue_b64();

#pragma omp for
  for (int i = 0; i < num_rows; i++) {

    if (i < num_rows - SW_PREFETCH_DIST_L1) {
      // load 16 colidx values
      svint32_t cidxv_pf = svld1(svptrue_b32(), &colidx[(i + SW_PREFETCH_DIST_L1) * 16]);
      // extend low half into index vectors and prefetch (col0-col7)
      svprfd_gather_index(pg, x, svunpklo(cidxv_pf), SV_PLDL1KEEP);
      // extend high half into index vector and prefetch (col8-col15)
      svprfd_gather_index(pg, x, svunpkhi(cidxv_pf), SV_PLDL1KEEP);
    }

    // load 16 colidx values
    svint32_t cidxv = svld1(svptrue_b32(), &colidx[i * 16]);
    // extend low half into index vector (col0-col7)
    svint64_t cidxvlo = svunpklo(cidxv);
    // extend high half into index vector (col8-col15)
    svint64_t cidxvhi = svunpkhi(cidxv);

    // load x[col0,...,col7]
    svfloat64_t xv0 = svld1_gather_index(pg, x, cidxvlo);
    // load x[col8,...,cold15]
    svfloat64_t xv1 = svld1_gather_index(pg, x, cidxvhi);

    // load a[0-7]
    svfloat64_t av0 = svld1(pg, &a[i * 16 + 0]);
    // load a[8-15]
    svfloat64_t av1 = svld1(pg, &a[i * 16 + 8]);

    // a <- a[0-7] * x[col0,...,col7]
    av0 = svmul_z(pg, av0, xv0);
    // a <- a + a[8-15] * x[col8,...,col15]
    av1 = svmla_z(pg, av0, av1, xv1);

    // reduce a*x into y
    float64_t y0 = svaddv(pg, av1);

    y[i] += ad[i] * x[i];
    y[i] += y0;
  }
  return 0;
}
#endif /* __ARM_FEATURE_SVE */


