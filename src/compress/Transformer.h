/***************************************************************************
                           Transformer.h  -  description
                             -------------------
 * Lapped orthogonal transforms.
 * See the accompanying file LOT_ZouAndPearlman.pdf for more information on LOT.

    copyright            : (C) 2012 Fraunhofer ITWM

    This file is part of jseisIO.

    jseisIO is free software: you can redistribute it and/or modify
    it under the terms of the Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    jseisIO is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    Lesser General Public License for more details.

    You should have received a copy of the Lesser General Public License
    along with jseisIO.  If not, see <http://www.gnu.org/licenses/>.

 ***************************************************************************/

#ifndef  TRANSFORMER_H
#define  TRANSFORMER_H

#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../jsDefs.h"

#if __cplusplus < 201103L
#define constexpr const
#endif

namespace jsIO {
class  Transformer {
public:
  ~ Transformer();
  /** No descriptions */
  Transformer();

  void getFilter8(float *filter) const;
  void getFilter16(float *filter) const;
  int lotFwd(float *x, int index, int blockSize, int transLength, int nblocks, float *scratch);
  int lotRev(float *x, int index, int blockSize, int transLength, int nblocks, float *scratch);
public:
  static bool c_integrityTest;
  // private atributes
private:
  void lotFwd8(float *x, int index, int nblocks, float *scratch);
  void lotRev8(float *x, int index, int nblocks, float *scratch);
  void lotFwd16(float *x, int index, int nblocks, float *scratch);
  void lotRev16(float *x, int index, int nblocks, float *scratch);
  void multiplex8(float *x, int xBaseIndex, int nblocks, float *scratch);
  void deMultiplex8(float *x, int xBaseIndex, int nblocks, float *scratch);
  void multiplex16(float *x, int xBaseIndex, int nblocks, float *scratch);
  void deMultiplex16(float *x, int xBaseIndex, int nblocks, float *scratch);

private:
  float *tmp;
  //     This filter is for the length-8 case.  It was selected after tests with
  //     many different filters from the LOT family.
  static constexpr float FILT0 = -0.04739361256361008F;
  static constexpr float FILT1 = -0.04662604257464409F;
  static constexpr float FILT2 = -0.00119693996384740F;
  static constexpr float FILT3 = -0.03382463753223419F;
  static constexpr float FILT4 =  0.02838252671062946F;
  static constexpr float FILT5 = -0.01656247489154339F;
  static constexpr float FILT6 =  0.07613456249237061F;
  static constexpr float FILT7 =  0.07250666618347168F;
  static constexpr float FILT8 = -0.03595214337110519F;
  static constexpr float FILT9 = -0.08879133313894272F;
  static constexpr float FILT10 =  0.13751053810119629F;
  static constexpr float FILT11 =  0.07318570464849472F;
  static constexpr float FILT12 =  0.07364673167467117F;
  static constexpr float FILT13 =  0.14130237698554993F;
  static constexpr float FILT14 = -0.09677020460367203F;
  static constexpr float FILT15 = -0.04223278537392616F;
  static constexpr float FILT16 =  0.01202779170125723F;
  static constexpr float FILT17 = -0.01751151494681835F;
  static constexpr float FILT18 =  0.12299209833145142F;
  static constexpr float FILT19 =  0.22184988856315613F;
  static constexpr float FILT20 = -0.24440830945968628F;
  static constexpr float FILT21 = -0.15647235512733459F;
  static constexpr float FILT22 =  0.01386095304042101F;
  static constexpr float FILT23 = -0.03452030941843987F;
  static constexpr float FILT24 =  0.12838034331798553F;
  static constexpr float FILT25 =  0.15070076286792755F;
  static constexpr float FILT26 = -0.08759644627571106F;
  static constexpr float FILT27 = -0.03334903717041016F;
  static constexpr float FILT28 = -0.02273095957934856F;
  static constexpr float FILT29 = -0.08336708694696426F;
  static constexpr float FILT30 =  0.15335069596767426F;
  static constexpr float FILT31 =  0.13196527957916260F;
  static constexpr float FILT32 =  0.22580096125602722F;
  static constexpr float FILT33 =  0.32441934943199158F;
  static constexpr float FILT34 = -0.36810591816902161F;
  static constexpr float FILT35 = -0.39882853627204895F;
  static constexpr float FILT36 =  0.38496315479278564F;
  static constexpr float FILT37 =  0.35616609454154968F;
  static constexpr float FILT38 = -0.34223899245262146F;
  static constexpr float FILT39 = -0.24626369774341583F;
  static constexpr float FILT40 =  0.33917742967605591F;
  static constexpr float FILT41 =  0.43858313560485840F;
  static constexpr float FILT42 = -0.32764276862144470F;
  static constexpr float FILT43 = -0.13819301128387451F;
  static constexpr float FILT44 = -0.12365391105413437F;
  static constexpr float FILT45 = -0.31876283884048462F;
  static constexpr float FILT46 =  0.43257290124893188F;
  static constexpr float FILT47 =  0.33427309989929199F;
  static constexpr float FILT48 =  0.39115539193153381F;
  static constexpr float FILT49 =  0.37391233444213867F;
  static constexpr float FILT50 =  0.06256388127803802F;
  static constexpr float FILT51 =  0.39160564541816711F;
  static constexpr float FILT52 = -0.41305914521217346F;
  static constexpr float FILT53 = -0.08888602256774902F;
  static constexpr float FILT54 = -0.37077668309211731F;
  static constexpr float FILT55 = -0.40489980578422546F;
  static constexpr float FILT56 =  0.40100517868995667F;
  static constexpr float FILT57 =  0.17178836464881897F;
  static constexpr float FILT58 =  0.45991656184196472F;
  static constexpr float FILT59 =  0.33414626121520996F;
  static constexpr float FILT60 =  0.31529939174652100F;
  static constexpr float FILT61 =  0.46039211750030518F;
  static constexpr float FILT62 =  0.13931363821029663F;
  static constexpr float FILT63 =  0.37151649594306946F;

  static float globalFilt8[64];
  static float globalFilt16[256];
  //       float dp[16];
  //       float dm[16];
};

}



#endif



