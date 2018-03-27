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
typedef const constexpr
#endif

namespace jsIO
{
  class  Transformer{
    public:
      ~ Transformer();
      /** No descriptions */
      Transformer();

      void getFilter8(float *filter) const;
      void getFilter16(float *filter) const;
      int lotFwd(float* x, int index, int blockSize, int transLength, int nblocks, float* scratch);
      int lotRev(float* x, int index, int blockSize, int transLength, int nblocks, float* scratch);
    public:
      static bool c_integrityTest;
// private atributes
    private:
      void lotFwd8( float* x, int index, int nblocks, float* scratch );
      void lotRev8( float* x, int index, int nblocks, float* scratch );
      void lotFwd16( float* x, int index, int nblocks, float* scratch );
      void lotRev16( float* x, int index, int nblocks, float* scratch );
      void multiplex8(float* x, int xBaseIndex, int nblocks, float* scratch);
      void deMultiplex8(float* x, int xBaseIndex, int nblocks, float* scratch);
      void multiplex16(float* x, int xBaseIndex, int nblocks, float* scratch);
      void deMultiplex16(float* x, int xBaseIndex, int nblocks, float* scratch);

    private:
      float* tmp;
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

    // Filter array for the lot8 transforms.  Length of 64 elements.
      static constexpr float globalFilt8[64] = {
        FILT0,   FILT1,   FILT2,   FILT3,   FILT4,   FILT5,   FILT6,   FILT7,
        FILT8,   FILT9,   FILT10,  FILT11,  FILT12,  FILT13,  FILT14,  FILT15,
        FILT16,  FILT17,  FILT18,  FILT19,  FILT20,  FILT21,  FILT22,  FILT23,
        FILT24,  FILT25,  FILT26,  FILT27,  FILT28,  FILT29,  FILT30,  FILT31,
        FILT32,  FILT33,  FILT34,  FILT35,  FILT36,  FILT37,  FILT38,  FILT39,
        FILT40,  FILT41,  FILT42,  FILT43,  FILT44,  FILT45,  FILT46,  FILT47,
        FILT48,  FILT49,  FILT50,  FILT51,  FILT52,  FILT53,  FILT54,  FILT55,
        FILT56,  FILT57,  FILT58,  FILT59,  FILT60,  FILT61,  FILT62,  FILT63
      };

  /*
      * This filter is for the length-16 case.  It was selected after tests with
      * many different filters from the LOT family.  It has length of 256 elements.
  */
      static constexpr float globalFilt16[256] = {
        -0.05092546716332436F, -0.04800906777381897F,  0.00414894195273519F, -0.02083428017795086F,
        0.00727677764371037F, -0.01368454564362764F,  0.01039289310574532F, -0.00904755853116512F,
        0.01269984804093838F, -0.00400549918413162F,  0.01494983769953251F,  0.00126677460502833F,
        0.01628162339329720F,  0.00898398645222187F,  0.01730429194867611F,  0.02478851750493050F,
        -0.04416475072503090F, -0.06306689232587814F,  0.03449244424700737F, -0.02380717545747757F,
        0.05057477205991745F,  0.00585399661213160F,  0.04923382028937340F,  0.03318041190505028F,
        0.03116884268820286F,  0.04958778619766235F,  0.00236834678798914F,  0.04767716303467751F,
        -0.02650943025946617F,  0.02465381100773811F, -0.04639538377523422F, -0.03057979419827461F,
        -0.03090312704443932F, -0.06785343587398529F,  0.08187537640333176F,  0.01948466524481773F,
        0.06891985237598419F,  0.08599705994129181F, -0.00495519628748298F,  0.08212102204561234F,
        -0.07446990907192230F,  0.01403521653264761F, -0.07664451748132706F, -0.05682519450783730F,
        -0.01186644099652767F, -0.06315757334232330F,  0.06287240236997604F,  0.03753550350666046F,
        -0.01165023352950811F, -0.06340452283620834F,  0.11659996211528778F,  0.09895730763673782F,
        0.00534545863047242F,  0.10264379531145096F, -0.11356112360954285F, -0.04641539230942726F,
        -0.05042800307273865F, -0.11235136538743973F,  0.09689835458993912F,  0.00650629680603743F,
        0.08959857374429703F,  0.10656486451625824F, -0.06378589570522308F, -0.01267870981246233F,
        0.01285405177623034F, -0.03134901076555252F,  0.12193045765161514F,  0.15737636387348175F,
        -0.11007650941610336F, -0.01514582801610231F, -0.07915185391902924F, -0.13471862673759460F,
        0.14168362319469452F,  0.08557172119617462F,  0.02428064867854118F,  0.12253648042678833F,
        -0.15165045857429504F, -0.10914323478937149F,  0.03618234023451805F,  0.00253067375160754F,
        0.04166804254055023F,  0.01321700401604176F,  0.07948959618806839F,  0.15558218955993652F,
        -0.18215398490428925F, -0.16363291442394257F,  0.12292949110269547F,  0.04734316468238831F,
        0.04377902299165726F,  0.12589003145694733F, -0.16998200118541718F, -0.17978045344352722F,
        0.14265312254428864F,  0.06966695189476013F,  0.01126913074404001F,  0.03483581170439720F,
        0.07368443161249161F,  0.07279250770807266F, -0.01334957964718342F,  0.05988258868455887F,
        -0.10735398530960083F, -0.15115077793598175F,  0.19063347578048706F,  0.21396283805370331F,
        -0.20784924924373627F, -0.18499191105365753F,  0.15553742647171021F,  0.11333797872066498F,
        -0.05390042439103127F,  0.02340606972575188F, -0.07172303646802902F, -0.06155961006879807F,
        0.10767285525798798F,  0.14833539724349976F, -0.12189307808876038F, -0.09884083271026611F,
        0.08136536926031113F,  0.06104232743382454F, -0.03727753087878227F, -0.00742463627830148F,
        -0.01261481456458569F, -0.03411408141255379F,  0.05694158002734184F,  0.07956540584564209F,
        -0.10047722607851028F, -0.12376008927822113F,  0.14114053547382355F,  0.09753937274217606F,
        0.14232714474201202F,  0.20832858979701996F, -0.22185355424880981F, -0.22708790004253387F,
        0.24318979680538177F,  0.26028376817703247F, -0.25543534755706787F, -0.26008918881416321F,
        0.26438581943511963F,  0.25928497314453125F, -0.25799125432968140F, -0.24207605421543121F,
        0.24217848479747772F,  0.23651280999183655F, -0.20771762728691101F, -0.14138355851173401F,
        0.17631556093692780F,  0.27445021271705627F, -0.28413963317871094F, -0.27951043844223022F,
        0.24745246767997742F,  0.19036246836185455F, -0.12664590775966644F, -0.03730802610516548F,
        -0.04332009702920914F, -0.12344301491975784F,  0.18856252729892731F,  0.24215136468410492F,
        -0.27186822891235352F, -0.28049305081367493F,  0.26579040288925171F,  0.16568998992443085F,
        0.20833195745944977F,  0.29243454337120056F, -0.27813091874122620F, -0.17133343219757080F,
        0.04240495711565018F, -0.10057521611452103F,  0.22874785959720612F,  0.29450646042823792F,
        -0.29088282585144043F, -0.22571973502635956F,  0.09691336750984192F, -0.04596826806664467F,
        0.17975607514381409F,  0.28661811351776123F, -0.30193713307380676F, -0.21314570307731628F,
        0.23714594542980194F,  0.31508100032806396F, -0.18560475111007690F,  0.01107664406299591F,
        -0.21623404324054718F, -0.31488713622093201F,  0.26970902085304260F,  0.10787588357925415F,
        0.10492639243602753F,  0.27358931303024292F, -0.31658893823623657F, -0.21826802194118500F,
        0.00987942516803741F, -0.18919040262699127F,  0.31637305021286011F,  0.23052150011062622F,
        0.26165023446083069F,  0.27759197354316711F, -0.05150613188743591F,  0.22331990301609039F,
        -0.32924216985702515F, -0.20078578591346741F, -0.07991197705268860F, -0.30131199955940247F,
        0.30354225635528564F,  0.07861359417438507F,  0.19761487841606140F,  0.33342185616493225F,
        -0.22616995871067047F,  0.05068635195493698F, -0.28473275899887085F, -0.26713839173316956F,
        0.28090313076972961F,  0.22914972901344299F,  0.11566746234893799F,  0.34049138426780701F,
        -0.20711791515350342F,  0.14810700714588165F, -0.34126159548759460F, -0.17647065222263336F,
        -0.17796021699905396F, -0.34309706091880798F,  0.15066449344158173F, -0.20486447215080261F,
        0.33997192978858948F,  0.10668116062879562F,  0.22463223338127136F,  0.28189635276794434F,
        0.29416474699974060F,  0.14253759384155273F,  0.26168128848075867F,  0.29512420296669006F,
        0.08735719323158264F,  0.34579148888587952F, -0.11966320872306824F,  0.27994415163993835F,
        -0.28042265772819519F,  0.11689667403697968F, -0.34702816605567932F, -0.08213546127080917F,
        -0.29749101400375366F, -0.26349446177482605F, -0.15335811674594879F, -0.30471205711364746F,
        0.30092546343803406F,  0.05685322359204292F,  0.34059208631515503F,  0.12008785456418991F,
        0.31829196214675903F,  0.18218725919723511F,  0.28621721267700195F,  0.23660972714424133F,
        0.23576197028160095F,  0.28340902924537659F,  0.18350340425968170F,  0.31790497899055481F,
        0.11961393058300018F,  0.33970499038696289F,  0.05408555269241333F,  0.30441030859947205F
      };
//       float dp[16];
//       float dm[16];
  };

}



#endif



