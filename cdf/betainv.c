/* cdf/betainv.c
 *
 * Copyright (C) 2004 Free Software Foundation, Inc.
 * Copyright (C) 2006 Brian Gough
 * Written by Jason H. Stover.
 * Modified for GSL by Brian Gough
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 */

/*
 * Invert the Beta distribution. 
 * 
 * References:
 *
 * Roger W. Abernathy and Robert P. Smith. "Applying Series Expansion
 * to the Inverse Beta Distribution to Find Percentiles of the
 * F-Distribution," ACM Transactions on Mathematical Software, volume
 * 19, number 4, December 1993, pages 474-480.
 *
 * G.W. Hill and A.W. Davis. "Generalized asymptotic expansions of a
 * Cornish-Fisher type," Annals of Mathematical Statistics, volume 39,
 * number 8, August 1968, pages 1264-1273.
 */

#include <config.h>
#include <math.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_randist.h>

#include "error.h"

double
gsl_cdf_beta_Pinv (const double P, const double a, const double b)
{
  double x, mean;

  if (P < 0.0 || P > 1.0)
    {
      CDF_ERROR ("P must be in range 0 < P < 1", GSL_EDOM);
    }

  if (a < 0.0)
    {
      CDF_ERROR ("a < 0", GSL_EDOM);
    }

  if (b < 0.0)
    {
      CDF_ERROR ("b < 0", GSL_EDOM);
    }

  if (P == 0.0)
    {
      return 0.0;
    }

  if (P == 1.0)
    {
      return 1.0;
    }

  if (P > 0.5)
    {
      return gsl_cdf_beta_Qinv (1 - P, a, b);
    }

  mean = a / (a + b);

  if (P < 0.1)
    {
      /* small x */

      double lg_ab = gsl_sf_lngamma (a + b);
      double lg_a = gsl_sf_lngamma (a);
      double lg_b = gsl_sf_lngamma (b);

      double lx = (log (a) + lg_a + lg_b - lg_ab + log (P)) / a;
      x = exp (lx);             /* first approximation */
      x *= pow (1 - x, -(b - 1) / a);   /* second approximation */

      if (x > mean)
        x = mean;
    }
  else
    {
      /* Use expected value as first guess */
      x = mean;
    }

  {
    double lambda, dP, phi;
    unsigned int n = 0;

  start:
    dP = P - gsl_cdf_beta_P (x, a, b);
    phi = gsl_ran_beta_pdf (x, a, b);

    if (dP == 0.0 || n++ > 64)
      goto end;

    lambda = dP / GSL_MAX (2 * fabs (dP / x), phi);

    {
      double step0 = lambda;
      double step1 = -((a - 1) / x - (b - 1) / (1 - x)) * lambda * lambda / 2;

      double step = step0;

      if (fabs (step1) < fabs (step0))
        {
          step += step1;
        }
      else
        {
          /* scale back step to a reasonable size when too large */
          step *= 2 * fabs (step0 / step1);
        };

      if (x + step > 0 && x + step < 1)
        {
          x += step;
        }
      else
        {
          x = sqrt (x) * sqrt (mean);   /* try a new starting point */
        }

      if (fabs (step0) > 1e-10 * x)
        goto start;
    }

  end:
    return x;

  }
}

double
gsl_cdf_beta_Qinv (double Q, double a, double b)
{

  if (Q < 0.0 || Q > 1.0)
    {
      CDF_ERROR ("Q must be inside range 0 < Q < 1", GSL_EDOM);
    }

  if (a < 0.0)
    {
      CDF_ERROR ("a < 0", GSL_EDOM);
    }

  if (b < 0.0)
    {
      CDF_ERROR ("b < 0", GSL_EDOM);
    }

  if (Q == 0.0)
    {
      return 1.0;
    }

  if (Q == 1.0)
    {
      return 0.0;
    }

  if (Q > 0.5)
    {
      return gsl_cdf_beta_Pinv (1 - Q, a, b);
    }
  else
    {
      return 1 - gsl_cdf_beta_Pinv (Q, b, a);
    };
}