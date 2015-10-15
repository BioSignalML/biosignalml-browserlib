/*****************************************************************************
 *                                                                           *
 *  BioSignalML Browser in C++                                               *
 *                                                                           *
 *  Copyright (c) 2014-2015  David Brooks                                    *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *****************************************************************************/

#include "nrange.h"

#include <cmath>

using namespace browser ;


NumericRange::NumericRange(double start, double end)
/*------------------------------------------------*/
{
  double width = end - start ;
  if (width < 0.0) {
    width = -width ;
    start = end ;
    end = start + width ;
    }
//  else if (width == 0.0)
//    throw ValueError("Grid cannot have zero width") ;
  double l = log10(width) ;
  double f = floor(l) ;
  double x = l - f ;             // Normalised between 0.0 and 1.0
  double scale = pow(10.0, f) ;
  if (x < 0.15) {
    m_major = 1*scale/10 ;       // The '/10' appears to
    m_minor = 0.02*scale ;       // minimise rounding errors
    }
  else if (x < 0.50) {
    m_major = 2*scale/10 ;
    m_minor = 0.05*scale ;
    }
  else if (x < 0.85) {
    m_major = 5*scale/10 ;
    m_minor = 0.10*scale ;
    }
  else {
    m_major = 10*scale/10 ;
    m_minor = 0.20*scale ;
    }
  m_quanta = m_major/POINTS_PER_MAJOR ;
  m_start = m_major*floor(start/m_major) ;
  m_end = m_major*ceil(end/m_major) ;
  m_major_size = (int)floor((m_end - m_start)/m_major + 0.5) ;
  }


double NumericRange::map(const double a, const int extra) const
/*-----------------------------------------------------------*/
{
  double q = m_quanta/float(pow(10, extra)) ;
  return q*floor((a + q/2.0)/q) ;
  }


#ifdef TESTING

#include <iostream>
#include <iomanip>


void test(const NumericRange &r, const double a)
/*--------------------------------------------*/
{
  std::cout << std::setprecision(15) << a << " ==> " << r.map(a) << std::endl ;
  }


int main(void)
/*----------*/
{
  NumericRange r1(3.035687, 30.47) ;
  std::cout << std::setprecision(15)
            << r1.start() << " " << r1.end()
     << " " << r1.major() << " " << r1.minor()
     << " " << r1.quanta() << std::endl ;
  test(r1, 30.035667565) ;
  test(r1, 30.035671565) ;

  NumericRange r2(0, 1806.6) ;
  test(r2, 30.035671565) ;
  test(r2, 1806.6) ;
  }

#endif
