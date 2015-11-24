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

#ifndef BROWSER_NRANGE_H
#define BROWSER_NRANGE_H

namespace browser {

  static const int POINTS_PER_MAJOR = 1000 ;

 /**
  * Calculate spacing of major and minor grid points.
  * 
  * Major spacing is selected to be either 1, 2, or 5, multipled by
  * a power of ten; minor spacing is respectively 0.2, 0.5 or 1.0.
  *
  * Spacing is chosen so that around 10 major grid points span the
  * interval.
  **/
  class NumericRange
  /*---------------*/
  {
   public:
    NumericRange() ;
    NumericRange(const double start, const double end) ;

    double map(const double a, const int extra=0) const ;

    inline double major(void) const { return m_major ; }
    inline double minor(void) const { return m_minor ; }
    inline double quanta(void) const { return m_quanta ; }
    inline double start(void) const { return m_start ; }
    inline double end(void) const { return m_end ; }
    inline int major_size(void) const { return m_major_size ; }

   private:
    double m_major ;
    double m_minor ;
    double m_quanta ;
    double m_start ;
    double m_end ;
    int m_major_size ;
    } ;

  } ;
#endif
