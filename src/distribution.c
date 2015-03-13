/*
    $Id: distribution.c,v 1.5 2008-01-12 16:10:21 awgn Exp $

    Copyright (c) 2003 Nicola Bonelli <bonelli@netserv.iet.unipi.it>
                       Raffaello Secchi <secchi@netserv.iet.unipi.it>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <limits.h>
#include <math.h>
#include <global.h>
#include <prototype.h>
#include <typedef.h>

/*
 * f(x)= 1/(max-min)* rect ( (x-(max+min)/2)/(max-min) )
 */

static inline double 
uniform(double min, double max)
{
    return (min+(genrand_real3())*(max-min));
}


/*
 *  f(x)= lambda* exp(-lambda*x)
 */

static inline double 
exponential(double lambda)
{
    return(-1. / lambda * log(genrand_real3()));
}


/*
 * f(x)= alpha/theta * ( theta/(x+theta) )^(alpha+1)
 */

static inline double 
pareto(double alpha, double theta)
{
    return(theta*( pow(genrand_real3(),-1./alpha)-1.));
}


/* 
 * from probablib ... 
 */

static inline double 
normal(double esp,double var)
{
    double u1 = genrand_real3();
    double u2 = genrand_real3();
    double z  = sqrt(-2.0*log(u1))*cos(2*M_PI*u2); 
    return esp+var*z;
}


