
/***************************************************************************
                           Transformer.cpp  -  description
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

  
#include "Transformer.h"
#include "../PSProLogging.h"

namespace jsIO
{
  DECLARE_LOGGER(TransformerLog);

  bool Transformer::c_integrityTest = false;

  float Transformer::globalFilt8[64] = {
          FILT0,   FILT1,   FILT2,   FILT3,   FILT4,   FILT5,   FILT6,   FILT7,
          FILT8,   FILT9,   FILT10,  FILT11,  FILT12,  FILT13,  FILT14,  FILT15,
          FILT16,  FILT17,  FILT18,  FILT19,  FILT20,  FILT21,  FILT22,  FILT23,
          FILT24,  FILT25,  FILT26,  FILT27,  FILT28,  FILT29,  FILT30,  FILT31,
          FILT32,  FILT33,  FILT34,  FILT35,  FILT36,  FILT37,  FILT38,  FILT39,
          FILT40,  FILT41,  FILT42,  FILT43,  FILT44,  FILT45,  FILT46,  FILT47,
          FILT48,  FILT49,  FILT50,  FILT51,  FILT52,  FILT53,  FILT54,  FILT55,
          FILT56,  FILT57,  FILT58,  FILT59,  FILT60,  FILT61,  FILT62,  FILT63
        };
  float Transformer::globalFilt16[256] = {
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

  Transformer::~Transformer(){
    delete []tmp;
  }

  Transformer::Transformer(){
    tmp = new float[16];
  }


//    Returns a copy of the filter that is used for the 8-sized block transform.
//    This is FYI only, for examination of the filter.
//    float *filter should be preallocated with the size of 64*sizeof(float)
  void Transformer::getFilter8(float *filter) const
  {
    memcpy(filter, globalFilt8, 64*sizeof(float));
  }


  /**
   * Forward lapped orthogonal transform for the length-8 case.
   *
   * @param  x  array to be transformed.
   * @param  index  index of first element in array to transform.
   * @param  nblocks  number of blocks.
   * @param  scratch  work array - must be nsamps+8 in length.
   */
  void Transformer::lotFwd8( float* x, int index, int nblocks, float* scratch ) 
  {
    int i, k, nsamps;
    float d0p15, d1p14, d2p13, d3p12, d4p11, d5p10, d6p9, d7p8;
    float d0m15, d1m14, d2m13, d3m12, d4m11, d5m10, d6m9, d7m8;
		
    if (c_integrityTest) {
      /* Exchange the order of the samples, but don't alter any amplitudes. */
      for ( i=0; i<nblocks*8; i+=2 ) {
        float save = x[i+index];
        x[i+index] = x[i+1+index];
        x[i+1+index] = save;
      }
      return;
    }

    nsamps = nblocks << 3;   /* nsamps = nblocks * 8 */

    /* Mirror at left side. */
    scratch[0] = x[3+index];
    scratch[1] = x[2+index];
    scratch[2] = x[1+index];
    scratch[3] = x[0+index];

    /* Mirror at right side. */
    scratch[4 + nsamps] = x[-1 + nsamps+index];
    scratch[5 + nsamps] = x[-2 + nsamps+index];
    scratch[6 + nsamps] = x[-3 + nsamps+index];
    scratch[7 + nsamps] = x[-4 + nsamps+index];
	  
    for ( i=0; i<nsamps; i++ ) scratch[i+4] = x[i+index];

    k = 0;
    /* One loop for each block of samples. */
    for ( i=0; i<nsamps; i+=8 ) {

      d0p15 = scratch[i+0] + scratch[i+15];
      d1p14 = scratch[i+1] + scratch[i+14];
      d2p13 = scratch[i+2] + scratch[i+13];
      d3p12 = scratch[i+3] + scratch[i+12];
      d4p11 = scratch[i+4] + scratch[i+11];
      d5p10 = scratch[i+5] + scratch[i+10];
      d6p9  = scratch[i+6] + scratch[i+9];
      d7p8  = scratch[i+7] + scratch[i+8];
      d0m15 = scratch[i+0] - scratch[i+15];
      d1m14 = scratch[i+1] - scratch[i+14];
      d2m13 = scratch[i+2] - scratch[i+13];
      d3m12 = scratch[i+3] - scratch[i+12];
      d4m11 = scratch[i+4] - scratch[i+11];
      d5m10 = scratch[i+5] - scratch[i+10];
      d6m9  = scratch[i+6] - scratch[i+9];
      d7m8  = scratch[i+7] - scratch[i+8];

      x[k+index] = FILT0 * d0p15  +  FILT8 * d1p14
          + FILT16 * d2p13  +  FILT24 * d3p12
          + FILT32 * d4p11  +  FILT40 * d5p10
          + FILT48 * d6p9  +  FILT56 * d7p8;
      x[k+1+index] = FILT1 * d0m15  +  FILT9 * d1m14
          + FILT17 * d2m13  +  FILT25 * d3m12
          + FILT33 * d4m11  +  FILT41 * d5m10
          + FILT49 * d6m9  +  FILT57 * d7m8;

      x[k+2+index] = FILT2 * d0p15  +  FILT10 * d1p14
          + FILT18 * d2p13  +  FILT26 * d3p12
          + FILT34 * d4p11  +  FILT42 * d5p10
          + FILT50 * d6p9  +  FILT58 * d7p8;
      x[k+3+index] = FILT3  * d0m15  +  FILT11  * d1m14
          + FILT19 * d2m13  +  FILT27 * d3m12
          + FILT35 * d4m11  +  FILT43 * d5m10
          + FILT51 * d6m9  +  FILT59 * d7m8;

      x[k+4+index] = FILT4 * d0p15  +  FILT12 * d1p14
          + FILT20 * d2p13  +  FILT28 * d3p12
          + FILT36 * d4p11  +  FILT44 * d5p10
          + FILT52 * d6p9  +  FILT60 * d7p8;
      x[k+5+index] = FILT5 * d0m15  +  FILT13 * d1m14
          + FILT21 * d2m13  +  FILT29 * d3m12
          + FILT37 * d4m11  +  FILT45 * d5m10
          + FILT53 * d6m9  +  FILT61 * d7m8;

      x[k+6+index] = FILT6 * d0p15  +  FILT14 * d1p14
          + FILT22 * d2p13  +  FILT30 * d3p12
          + FILT38 * d4p11  +  FILT46 * d5p10
          + FILT54 * d6p9  +  FILT62 * d7p8;
      x[k+7+index] = FILT7 * d0m15  +  FILT15 * d1m14
          + FILT23 * d2m13  +  FILT31 * d3m12
          + FILT39 * d4m11  +  FILT47 * d5m10
          + FILT55 * d6m9  +  FILT63 * d7m8;

      k += 8;
	    
    }

    return;
  }


  /**
   * Reverse lapped orthogonal transform for the length-8 case.
   *
   * @param  x  array to be inverse transformed.
   * @param  index  index of first element in array to transform.
   * @param  nblocks  number of blocks.
   * @param  scratch  work array - must be nsamps+8 in length.
   */
  void Transformer::lotRev8( float* x, int index, int nblocks, float* scratch ) 
  {
    int i, nsamps, nblocksM1;

    if (c_integrityTest) {
      /* Exchange the order of the samples, but don't alter any amplitudes. */
      for ( i=0; i<nblocks*8; i+=2 ) {
        float save = x[i+index];
        x[i+index] = x[i+1+index];
        x[i+1+index] = save;
      }
      return;
    }

    nsamps = nblocks << 3;   /* nsamps = nblocks * 8 */
    nblocksM1 = nblocks - 1;
	  
    int xIndex = index;
    int scratchIndex = 0;

    /* Loop through the blocks. */
    for ( i=0; i<nblocks; i++ ) {
      /* It's faster not to check the DC, since it's almost surely
      * non-zero. */
      tmp[0] = FILT0 * x[xIndex];   tmp[1] = FILT8 * x[xIndex];
      tmp[2] = FILT16 * x[xIndex];  tmp[3] = FILT24 * x[xIndex];
      tmp[4] = FILT32 * x[xIndex];  tmp[5] = FILT40 * x[xIndex];
      tmp[6] = FILT48 * x[xIndex];  tmp[7] = FILT56 * x[xIndex];
	    
      if ( ISNOTZERO(x[2+xIndex]) ) {
        tmp[0] += FILT2 * x[2+xIndex];   tmp[1] += FILT10 * x[2+xIndex];
        tmp[2] += FILT18 * x[2+xIndex];  tmp[3] += FILT26 * x[2+xIndex];
        tmp[4] += FILT34 * x[2+xIndex];  tmp[5] += FILT42 * x[2+xIndex];
        tmp[6] += FILT50 * x[2+xIndex];  tmp[7] += FILT58 * x[2+xIndex];
      }

      if ( ISNOTZERO(x[4+xIndex]) ) {
        tmp[0] += FILT4 * x[4+xIndex];   tmp[1] += FILT12 * x[4+xIndex];
        tmp[2] += FILT20 * x[4+xIndex];  tmp[3] += FILT28 * x[4+xIndex];
        tmp[4] += FILT36 * x[4+xIndex];  tmp[5] += FILT44 * x[4+xIndex];
        tmp[6] += FILT52 * x[4+xIndex];  tmp[7] += FILT60 * x[4+xIndex];
      }
	    
      if ( ISNOTZERO(x[6+xIndex]) ) {
        tmp[0] += FILT6 * x[6+xIndex];   tmp[1] += FILT14 * x[6+xIndex];
        tmp[2] += FILT22 * x[6+xIndex];  tmp[3] += FILT30 * x[6+xIndex];
        tmp[4] += FILT38 * x[6+xIndex];  tmp[5] += FILT46 * x[6+xIndex];
        tmp[6] += FILT54 * x[6+xIndex];  tmp[7] += FILT62 * x[6+xIndex];
      }
	    
	    
      /* Ditto for not checking x[1]. */
      tmp[8] = FILT1 * x[1+xIndex];    tmp[9] = FILT9 * x[1+xIndex];
      tmp[10] = FILT17 * x[1+xIndex];  tmp[11] = FILT25 * x[1+xIndex];
      tmp[12] = FILT33 * x[1+xIndex];  tmp[13] = FILT41 * x[1+xIndex];
      tmp[14] = FILT49 * x[1+xIndex];  tmp[15] = FILT57 * x[1+xIndex];
	    
      if ( ISNOTZERO(x[3+xIndex]) ) {
        tmp[8] += FILT3 * x[3+xIndex];    tmp[9] += FILT11 * x[3+xIndex];
        tmp[10] += FILT19 * x[3+xIndex];  tmp[11] += FILT27 * x[3+xIndex];
        tmp[12] += FILT35 * x[3+xIndex];  tmp[13] += FILT43 * x[3+xIndex];
        tmp[14] += FILT51 * x[3+xIndex];  tmp[15] += FILT59 * x[3+xIndex];
      }
	    
      if ( ISNOTZERO(x[5+xIndex]) ) {
        tmp[8] += FILT5 * x[5+xIndex];    tmp[9] += FILT13 * x[5+xIndex];
        tmp[10] += FILT21 * x[5+xIndex];  tmp[11] += FILT29 * x[5+xIndex];
        tmp[12] += FILT37 * x[5+xIndex];  tmp[13] += FILT45 * x[5+xIndex];
        tmp[14] += FILT53 * x[5+xIndex];  tmp[15] += FILT61 * x[5+xIndex];
      }
	    
      if ( ISNOTZERO(x[7+xIndex]) ) {
        tmp[8] += FILT7 * x[7+xIndex];    tmp[9] += FILT15 * x[7+xIndex];
        tmp[10] += FILT23 * x[7+xIndex];  tmp[11] += FILT31 * x[7+xIndex];
        tmp[12] += FILT39 * x[7+xIndex];  tmp[13] += FILT47 * x[7+xIndex];
        tmp[14] += FILT55 * x[7+xIndex];  tmp[15] += FILT63 * x[7+xIndex];
      }
	    
      scratch[15+scratchIndex] = tmp[0] - tmp[8];   tmp[8] += tmp[0];
      scratch[14+scratchIndex] = tmp[1] - tmp[9];   tmp[9] += tmp[1];
      scratch[13+scratchIndex] = tmp[2] - tmp[10];  tmp[10] += tmp[2];
      scratch[12+scratchIndex] = tmp[3] - tmp[11];  tmp[11] += tmp[3];
      scratch[11+scratchIndex] = tmp[4] - tmp[12];  tmp[12] += tmp[4];
      scratch[10+scratchIndex] = tmp[5] - tmp[13];  tmp[13] += tmp[5];
      scratch[9+scratchIndex]  = tmp[6] - tmp[14];   tmp[14] += tmp[6];
      scratch[8+scratchIndex]  = tmp[7] - tmp[15];   tmp[15] += tmp[7];

      if ( i == 0 ) {
        /* Left edge. */
        scratch[4+scratchIndex] = tmp[11];
        scratch[5+scratchIndex] = tmp[10];
        scratch[6+scratchIndex] = tmp[9];
        scratch[7+scratchIndex] = tmp[8];
      }
	    
      scratch[scratchIndex] += tmp[8];
      scratch[1+scratchIndex] += tmp[9];
      scratch[2+scratchIndex] += tmp[10];
      scratch[3+scratchIndex] += tmp[11];
      scratch[4+scratchIndex] += tmp[12];
      scratch[5+scratchIndex] += tmp[13];
      scratch[6+scratchIndex] += tmp[14];
      scratch[7+scratchIndex] += tmp[15];

      if ( i == nblocksM1 ) {
        /* Right edge (last time thru loop). */
        scratch[8+scratchIndex] += scratch[15+scratchIndex];
        scratch[9+scratchIndex] += scratch[14+scratchIndex];
        scratch[10+scratchIndex] += scratch[13+scratchIndex];
        scratch[11+scratchIndex] += scratch[12+scratchIndex];
      }

      xIndex += 8;
      scratchIndex += 8;
    }
	  
    for ( i=0; i<nsamps; i++ ) x[i+index] = scratch[i+4];

    return;
  }

  


//    * Returns a copy of the filter that is used for the 16-sized block transform.
//    * This is FYI only, for examination of the filter.

  void Transformer::getFilter16(float *filter) const
  {
    memcpy(filter, globalFilt16, 256*sizeof(float));
  }

  /**
   * Forward lapped orthogonal transform for the length-16 case.
   *
   * @param  x  array to be transformed.
   * @param  index  index of first element in array to transform.
   * @param  nblocks  number of blocks.
   * @param  scratch  work array - must be nsamps+32 in length.
   */
  void Transformer::lotFwd16( float* x, int index, int nblocks, float* scratch ) 
  {
    int nsamps, i, k, l;
     
    float d0p31, d1p30, d2p29, d3p28, d4p27, d5p26, d6p25, d7p24;
    float d8p23, d9p22, d10p21, d11p20, d12p19, d13p18, d14p17, d15p16;
    float d0m31, d1m30, d2m29, d3m28, d4m27, d5m26, d6m25, d7m24;
    float d8m23, d9m22, d10m21, d11m20, d12m19, d13m18, d14m17, d15m16;
      
     
    if (c_integrityTest) {
      /* Exchange the order of the samples, but don't alter any amplitudes. */
      for ( i=0; i<nblocks*16; i+=4 ) {
        float save0  = x[i+index];
        float save1  = x[i+1+index];
        x[i+index]   = x[i+3+index];
        x[i+1+index] = x[i+2+index];
        x[i+2+index] = save1;
        x[i+3+index] = save0;
      }
      return;
    }

    nsamps = nblocks << 4;   /* nsamps = nblocks * 16 */

    /* Mirror at left side. */
    scratch[0] = x[7+index];
    scratch[1] = x[6+index];
    scratch[2] = x[5+index];
    scratch[3] = x[4+index];
    scratch[4] = x[3+index];
    scratch[5] = x[2+index];
    scratch[6] = x[1+index];
    scratch[7] = x[index];

    /* Mirror at right side. */
    scratch[8+nsamps]  = x[-1+nsamps+index];
    scratch[9+nsamps]  = x[-2+nsamps+index];
    scratch[10+nsamps] = x[-3+nsamps+index];
    scratch[11+nsamps] = x[-4+nsamps+index];
    scratch[12+nsamps] = x[-5+nsamps+index];
    scratch[13+nsamps] = x[-6+nsamps+index];
    scratch[14+nsamps] = x[-7+nsamps+index];
    scratch[15+nsamps] = x[-8+nsamps+index];

    for ( i=0; i<nsamps; i++ ) scratch[i+8] = x[i+index];

    k = 0;
    for ( i=0; i<nsamps; i+=16 ) {
       /*
      for(int m=0;m<16;m++)
      {
        dp[m] =  scratch[m+i] + scratch[31-m+i];
        dm[m] =  scratch[m+i] - scratch[31-m+i];
      }
        */
      d0p31  = scratch[i] + scratch[31+i];
      d1p30  = scratch[1+i] + scratch[30+i];
      d2p29  = scratch[2+i] + scratch[29+i];
      d3p28  = scratch[3+i] + scratch[28+i];
      d4p27  = scratch[4+i] + scratch[27+i];
      d5p26  = scratch[5+i] + scratch[26+i];
      d6p25  = scratch[6+i] + scratch[25+i];
      d7p24  = scratch[7+i] + scratch[24+i];
      d8p23  = scratch[8+i] + scratch[23+i];
      d9p22  = scratch[9+i] + scratch[22+i];
      d10p21 = scratch[10+i] + scratch[21+i];
      d11p20 = scratch[11+i] + scratch[20+i];
      d12p19 = scratch[12+i] + scratch[19+i];
      d13p18 = scratch[13+i] + scratch[18+i];
      d14p17 = scratch[14+i] + scratch[17+i];
      d15p16 = scratch[15+i] + scratch[16+i];

      d0m31  = scratch[i] - scratch[31+i];
      d1m30  = scratch[1+i] - scratch[30+i];
      d2m29  = scratch[2+i] - scratch[29+i];
      d3m28  = scratch[3+i] - scratch[28+i];
      d4m27  = scratch[4+i] - scratch[27+i];
      d5m26  = scratch[5+i] - scratch[26+i];
      d6m25  = scratch[6+i] - scratch[25+i];
      d7m24  = scratch[7+i] - scratch[24+i];
      d8m23  = scratch[8+i] - scratch[23+i];
      d9m22  = scratch[9+i] - scratch[22+i];
      d10m21 = scratch[10+i] - scratch[21+i];
      d11m20 = scratch[11+i] - scratch[20+i];
      d12m19 = scratch[12+i] - scratch[19+i];
      d13m18 = scratch[13+i] - scratch[18+i];
      d14m17 = scratch[14+i] - scratch[17+i];
      d15m16 = scratch[15+i] - scratch[16+i];
        
      for ( l=0; l<16; l+=2 ) {
        
        x[k+index] =  globalFilt16[l] * d0p31 + globalFilt16[16+l] * d1p30 + globalFilt16[32+l] * d2p29 + globalFilt16[48+l] * d3p28
            + globalFilt16[64+l] * d4p27 + globalFilt16[80+l] * d5p26 + globalFilt16[96+l] * d6p25 + globalFilt16[112+l] * d7p24
            + globalFilt16[128+l] * d8p23 + globalFilt16[144+l] * d9p22 + globalFilt16[160+l] * d10p21 + globalFilt16[176+l] * d11p20
            + globalFilt16[192+l] * d12p19 + globalFilt16[208+l] * d13p18 + globalFilt16[224+l] * d14p17 + globalFilt16[240+l] * d15p16;

        x[k+1+index] =  globalFilt16[1+l] * d0m31 + globalFilt16[17+l] * d1m30 + globalFilt16[33+l] * d2m29 + globalFilt16[49+l] * d3m28
            + globalFilt16[65+l] * d4m27 + globalFilt16[81+l] * d5m26 + globalFilt16[97+l] * d6m25 + globalFilt16[113+l] * d7m24
            + globalFilt16[129+l] * d8m23 + globalFilt16[145+l] * d9m22 + globalFilt16[161+l] * d10m21 + globalFilt16[177+l] * d11m20
            + globalFilt16[193+l] * d12m19 + globalFilt16[209+l] * d13m18 + globalFilt16[225+l] * d14m17 + globalFilt16[241+l] * d15m16;
        /*
        for(int m=0;m<16;m++)
        {
          x[k+index] =  globalFilt16[l+16*m] * dp[m];
          x[k+1+index] = globalFilt16[1+l+16*m] * dm[m];
        }
         */
        k += 2;	
		    
      }
    }

    return;
  }


  /**
   * Reverse lapped orthogonal transform for the length-16 case.
   *
   * @param  x  array to be inverse transformed.
   * @param  index  index of first element in array to transform.
   * @param  nblocks  number of blocks.
   * @param  scratch  work array - must be nsamps+32 in length.
   */
  void Transformer::lotRev16( float* x, int index, int nblocks, float* scratch ) 
  {
    int nsamps, i, nblocksM1;//,allZerosInBlock
    float tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
    float tmp8, tmp9, tmp10, tmp11, tmp12, tmp13, tmp14, tmp15;
    float tmp16, tmp17, tmp18, tmp19, tmp20, tmp21, tmp22, tmp23;
    float tmp24, tmp25, tmp26, tmp27, tmp28, tmp29, tmp30, tmp31;

    if (c_integrityTest) {
      /* Exchange the order of the samples, but don't alter any amplitudes. */
      for ( i=0; i<nblocks*16; i+=4 ) {
        float save0  = x[i+index];
        float save1  = x[i+1+index];
        x[i+index]   = x[i+3+index];
        x[i+1+index] = x[i+2+index];
        x[i+2+index] = save1;
        x[i+3+index] = save0;
      }
      return;
    }

    nsamps = nblocks << 4;   /* nsamps = nblocks * 16 */
    nblocksM1 = nblocks - 1;

    /* Check for all zeros. */ 
    bool allZeros = true;
    for ( i=0; i<nsamps; i++ ) {
      if ( ISNOTZERO(x[i+index]) ) { 
        allZeros = false;
        break;		  
      }	  
    }
	 
    if (allZeros) return;

    int xIndex = index;
    int scratchIndex = 0;

    /* Loop through the blocks. */
    for ( i=0; i<nblocks; i++ ) {
		  
      if (ISNOTZERO(x[xIndex])) {	 
        tmp0 = globalFilt16[0] * x[xIndex];     tmp2 = globalFilt16[16] * x[xIndex];
        tmp4 = globalFilt16[32] * x[xIndex];    tmp6 = globalFilt16[48] * x[xIndex];
        tmp8 = globalFilt16[64] * x[xIndex];    tmp10 = globalFilt16[80] * x[xIndex];
        tmp12 = globalFilt16[96] * x[xIndex];   tmp14 = globalFilt16[112] * x[xIndex];
        tmp16 = globalFilt16[128] * x[xIndex];  tmp18 = globalFilt16[144] * x[xIndex];
        tmp20 = globalFilt16[160] * x[xIndex];  tmp22 = globalFilt16[176] * x[xIndex];
        tmp24 = globalFilt16[192] * x[xIndex];  tmp26 = globalFilt16[208] * x[xIndex];
        tmp28 = globalFilt16[224] * x[xIndex];  tmp30 = globalFilt16[240] * x[xIndex];
      } else {
        tmp0 = 0.0F;   tmp2 = 0.0F;   tmp4 = 0.0F;   tmp6 = 0.0F;
        tmp8 = 0.0F;   tmp10 = 0.0F;  tmp12 = 0.0F;  tmp14 = 0.0F;
        tmp16 = 0.0F;  tmp18 = 0.0F;  tmp20 = 0.0F;  tmp22 = 0.0F;
        tmp24 = 0.0F;  tmp26 = 0.0F;  tmp28 = 0.0F;  tmp30 = 0.0F;
      }
      
      if ( ISNOTZERO(x[1+xIndex]) ) {    
        tmp1 = globalFilt16[1] * x[1+xIndex];     tmp3 = globalFilt16[17] * x[1+xIndex];
        tmp5 = globalFilt16[33] * x[1+xIndex];    tmp7 = globalFilt16[49] * x[1+xIndex];
        tmp9 = globalFilt16[65] * x[1+xIndex];    tmp11 = globalFilt16[81] * x[1+xIndex];
        tmp13 = globalFilt16[97] * x[1+xIndex];   tmp15 = globalFilt16[113] * x[1+xIndex];
        tmp17 = globalFilt16[129] * x[1+xIndex];  tmp19 = globalFilt16[145] * x[1+xIndex];
        tmp21 = globalFilt16[161] * x[1+xIndex];  tmp23 = globalFilt16[177] * x[1+xIndex];
        tmp25 = globalFilt16[193] * x[1+xIndex];  tmp27 = globalFilt16[209] * x[1+xIndex];
        tmp29 = globalFilt16[225] * x[1+xIndex];  tmp31 = globalFilt16[241] * x[1+xIndex];
      } else {
        tmp1 = 0.0F;   tmp3 = 0.0F;   tmp5 = 0.0F;   tmp7 = 0.0F;
        tmp9 = 0.0F;  tmp11 = 0.0F;  tmp13 = 0.0F;   tmp15 = 0.0F;
        tmp17 = 0.0F;  tmp19 = 0.0F;  tmp21 = 0.0F;  tmp23 = 0.0F;
        tmp25 = 0.0F;  tmp27 = 0.0F;  tmp29 = 0.0F;  tmp31 = 0.0F;
      }
	      
      if ( ISNOTZERO(x[2+xIndex])) { 
        tmp0 += globalFilt16[2] * x[2+xIndex];     tmp2 += globalFilt16[18] * x[2+xIndex];
        tmp4 += globalFilt16[34] * x[2+xIndex];    tmp6 += globalFilt16[50] * x[2+xIndex];
        tmp8 += globalFilt16[66] * x[2+xIndex];    tmp10 += globalFilt16[82] * x[2+xIndex];
        tmp12 += globalFilt16[98] * x[2+xIndex];   tmp14 += globalFilt16[114] * x[2+xIndex];
        tmp16 += globalFilt16[130] * x[2+xIndex];  tmp18 += globalFilt16[146] * x[2+xIndex];
        tmp20 += globalFilt16[162] * x[2+xIndex];  tmp22 += globalFilt16[178] * x[2+xIndex];
        tmp24 += globalFilt16[194] * x[2+xIndex];  tmp26 += globalFilt16[210] * x[2+xIndex];
        tmp28 += globalFilt16[226] * x[2+xIndex];  tmp30 += globalFilt16[242] * x[2+xIndex];
      }
      
      if ( ISNOTZERO(x[3+xIndex]) ) {
        tmp1 += globalFilt16[3] * x[3+xIndex];     tmp3 += globalFilt16[19] * x[3+xIndex];
        tmp5 += globalFilt16[35] * x[3+xIndex];    tmp7 += globalFilt16[51] * x[3+xIndex];
        tmp9 += globalFilt16[67] * x[3+xIndex];    tmp11 += globalFilt16[83] * x[3+xIndex];
        tmp13 += globalFilt16[99] * x[3+xIndex];   tmp15 += globalFilt16[115] * x[3+xIndex];
        tmp17 += globalFilt16[131] * x[3+xIndex];  tmp19 += globalFilt16[147] * x[3+xIndex];
        tmp21 += globalFilt16[163] * x[3+xIndex];  tmp23 += globalFilt16[179] * x[3+xIndex];
        tmp25 += globalFilt16[195] * x[3+xIndex];  tmp27 += globalFilt16[211] * x[3+xIndex];
        tmp29 += globalFilt16[227] * x[3+xIndex];  tmp31 += globalFilt16[243] * x[3+xIndex];
      }

      if ( ISNOTZERO(x[4+xIndex]) ) {
        tmp0 += globalFilt16[4] * x[4+xIndex];     tmp2 += globalFilt16[20] * x[4+xIndex];
        tmp4 += globalFilt16[36] * x[4+xIndex];    tmp6 += globalFilt16[52] * x[4+xIndex];
        tmp8 += globalFilt16[68] * x[4+xIndex];    tmp10 += globalFilt16[84] * x[4+xIndex];
        tmp12 += globalFilt16[100] * x[4+xIndex];  tmp14 += globalFilt16[116] * x[4+xIndex];
        tmp16 += globalFilt16[132] * x[4+xIndex];  tmp18 += globalFilt16[148] * x[4+xIndex];
        tmp20 += globalFilt16[164] * x[4+xIndex];  tmp22 += globalFilt16[180] * x[4+xIndex];
        tmp24 += globalFilt16[196] * x[4+xIndex];  tmp26 += globalFilt16[212] * x[4+xIndex];
        tmp28 += globalFilt16[228] * x[4+xIndex];  tmp30 += globalFilt16[244] * x[4+xIndex];
      }
      
      if ( ISNOTZERO(x[5+xIndex]) ) { 
        tmp1 += globalFilt16[5] * x[5+xIndex];     tmp3 += globalFilt16[21] * x[5+xIndex];
        tmp5 += globalFilt16[37] * x[5+xIndex];    tmp7 += globalFilt16[53] * x[5+xIndex];
        tmp9 += globalFilt16[69] * x[5+xIndex];    tmp11 += globalFilt16[85] * x[5+xIndex];
        tmp13 += globalFilt16[101] * x[5+xIndex];  tmp15 += globalFilt16[117] * x[5+xIndex];
        tmp17 += globalFilt16[133] * x[5+xIndex];  tmp19 += globalFilt16[149] * x[5+xIndex];
        tmp21 += globalFilt16[165] * x[5+xIndex];  tmp23 += globalFilt16[181] * x[5+xIndex];
        tmp25 += globalFilt16[197] * x[5+xIndex];  tmp27 += globalFilt16[213] * x[5+xIndex];
        tmp29 += globalFilt16[229] * x[5+xIndex];  tmp31 += globalFilt16[245] * x[5+xIndex];
      }
       
      if ( ISNOTZERO(x[6+xIndex]) ) {
        tmp0 += globalFilt16[6] * x[6+xIndex];     tmp2 += globalFilt16[22] * x[6+xIndex];
        tmp4 += globalFilt16[38] * x[6+xIndex];    tmp6 += globalFilt16[54] * x[6+xIndex];
        tmp8 += globalFilt16[70] * x[6+xIndex];    tmp10 += globalFilt16[86] * x[6+xIndex];
        tmp12 += globalFilt16[102] * x[6+xIndex];  tmp14 += globalFilt16[118] * x[6+xIndex];
        tmp16 += globalFilt16[134] * x[6+xIndex];  tmp18 += globalFilt16[150] * x[6+xIndex];
        tmp20 += globalFilt16[166] * x[6+xIndex];  tmp22 += globalFilt16[182] * x[6+xIndex];
        tmp24 += globalFilt16[198] * x[6+xIndex];  tmp26 += globalFilt16[214] * x[6+xIndex];
        tmp28 += globalFilt16[230] * x[6+xIndex];  tmp30 += globalFilt16[246] * x[6+xIndex];
      }
      
      if ( ISNOTZERO(x[7+xIndex]) ) {
        tmp1 += globalFilt16[7] * x[7+xIndex];     tmp3 += globalFilt16[23] * x[7+xIndex];
        tmp5 += globalFilt16[39] * x[7+xIndex];    tmp7 += globalFilt16[55] * x[7+xIndex];
        tmp9 += globalFilt16[71] * x[7+xIndex];    tmp11 += globalFilt16[87] * x[7+xIndex];
        tmp13 += globalFilt16[103] * x[7+xIndex];  tmp15 += globalFilt16[119] * x[7+xIndex];
        tmp17 += globalFilt16[135] * x[7+xIndex];  tmp19 += globalFilt16[151] * x[7+xIndex];
        tmp21 += globalFilt16[167] * x[7+xIndex];  tmp23 += globalFilt16[183] * x[7+xIndex];
        tmp25 += globalFilt16[199] * x[7+xIndex];  tmp27 += globalFilt16[215] * x[7+xIndex];
        tmp29 += globalFilt16[231] * x[7+xIndex];  tmp31 += globalFilt16[247] * x[7+xIndex];
      }
      
      if ( ISNOTZERO(x[8+xIndex]) ) {
        tmp0 += globalFilt16[8] * x[8+xIndex];     tmp2 += globalFilt16[24] * x[8+xIndex];
        tmp4 += globalFilt16[40] * x[8+xIndex];    tmp6 += globalFilt16[56] * x[8+xIndex];
        tmp8 += globalFilt16[72] * x[8+xIndex];    tmp10 += globalFilt16[88] * x[8+xIndex];
        tmp12 += globalFilt16[104] * x[8+xIndex];  tmp14 += globalFilt16[120] * x[8+xIndex];
        tmp16 += globalFilt16[136] * x[8+xIndex];  tmp18 += globalFilt16[152] * x[8+xIndex];
        tmp20 += globalFilt16[168] * x[8+xIndex];  tmp22 += globalFilt16[184] * x[8+xIndex];
        tmp24 += globalFilt16[200] * x[8+xIndex];  tmp26 += globalFilt16[216] * x[8+xIndex];
        tmp28 += globalFilt16[232] * x[8+xIndex];  tmp30 += globalFilt16[248] * x[8+xIndex];
      }
      
      if ( ISNOTZERO(x[9+xIndex]) ) {
        tmp1 += globalFilt16[9] * x[9+xIndex];     tmp3 += globalFilt16[25] * x[9+xIndex];
        tmp5 += globalFilt16[41] * x[9+xIndex];    tmp7 += globalFilt16[57] * x[9+xIndex];
        tmp9 += globalFilt16[73] * x[9+xIndex];    tmp11 += globalFilt16[89] * x[9+xIndex];
        tmp13 += globalFilt16[105] * x[9+xIndex];  tmp15 += globalFilt16[121] * x[9+xIndex];
        tmp17 += globalFilt16[137] * x[9+xIndex];  tmp19 += globalFilt16[153] * x[9+xIndex];
        tmp21 += globalFilt16[169] * x[9+xIndex];  tmp23 += globalFilt16[185] * x[9+xIndex];
        tmp25 += globalFilt16[201] * x[9+xIndex];  tmp27 += globalFilt16[217] * x[9+xIndex];
        tmp29 += globalFilt16[233] * x[9+xIndex];  tmp31 += globalFilt16[249] * x[9+xIndex];
      }

      if ( ISNOTZERO(x[10+xIndex]) ) {
        tmp0 += globalFilt16[10] * x[10+xIndex];    tmp2 += globalFilt16[26] * x[10+xIndex];
        tmp4 += globalFilt16[42] * x[10+xIndex];    tmp6 += globalFilt16[58] * x[10+xIndex];
        tmp8 += globalFilt16[74] * x[10+xIndex];    tmp10 += globalFilt16[90] * x[10+xIndex];
        tmp12 += globalFilt16[106] * x[10+xIndex];  tmp14 += globalFilt16[122] * x[10+xIndex];
        tmp16 += globalFilt16[138] * x[10+xIndex];  tmp18 += globalFilt16[154] * x[10+xIndex];
        tmp20 += globalFilt16[170] * x[10+xIndex];  tmp22 += globalFilt16[186] * x[10+xIndex];
        tmp24 += globalFilt16[202] * x[10+xIndex];  tmp26 += globalFilt16[218] * x[10+xIndex];
        tmp28 += globalFilt16[234] * x[10+xIndex];  tmp30 += globalFilt16[250] * x[10+xIndex];
      }
      
      if ( ISNOTZERO(x[11+xIndex]) ) {
        tmp1 += globalFilt16[11] * x[11+xIndex];    tmp3 += globalFilt16[27] * x[11+xIndex];
        tmp5 += globalFilt16[43] * x[11+xIndex];    tmp7 += globalFilt16[59] * x[11+xIndex];
        tmp9 += globalFilt16[75] * x[11+xIndex];    tmp11 += globalFilt16[91] * x[11+xIndex];
        tmp13 += globalFilt16[107] * x[11+xIndex];  tmp15 += globalFilt16[123] * x[11+xIndex];
        tmp17 += globalFilt16[139] * x[11+xIndex];  tmp19 += globalFilt16[155] * x[11+xIndex];
        tmp21 += globalFilt16[171] * x[11+xIndex];  tmp23 += globalFilt16[187] * x[11+xIndex];
        tmp25 += globalFilt16[203] * x[11+xIndex];  tmp27 += globalFilt16[219] * x[11+xIndex];
        tmp29 += globalFilt16[235] * x[11+xIndex];  tmp31 += globalFilt16[251] * x[11+xIndex];
      }
      
      if ( ISNOTZERO(x[12+xIndex]) ) {
        tmp0 += globalFilt16[12] * x[12+xIndex];    tmp2 += globalFilt16[28] * x[12+xIndex];
        tmp4 += globalFilt16[44] * x[12+xIndex];    tmp6 += globalFilt16[60] * x[12+xIndex];
        tmp8 += globalFilt16[76] * x[12+xIndex];    tmp10 += globalFilt16[92] * x[12+xIndex];
        tmp12 += globalFilt16[108] * x[12+xIndex];  tmp14 += globalFilt16[124] * x[12+xIndex];
        tmp16 += globalFilt16[140] * x[12+xIndex];  tmp18 += globalFilt16[156] * x[12+xIndex];
        tmp20 += globalFilt16[172] * x[12+xIndex];  tmp22 += globalFilt16[188] * x[12+xIndex];
        tmp24 += globalFilt16[204] * x[12+xIndex];  tmp26 += globalFilt16[220] * x[12+xIndex];
        tmp28 += globalFilt16[236] * x[12+xIndex];  tmp30 += globalFilt16[252] * x[12+xIndex];
      }
      
      if ( ISNOTZERO(x[13+xIndex]) ) {
        tmp1 += globalFilt16[13] * x[13+xIndex];    tmp3 += globalFilt16[29] * x[13+xIndex];
        tmp5 += globalFilt16[45] * x[13+xIndex];    tmp7 += globalFilt16[61] * x[13+xIndex];
        tmp9 += globalFilt16[77] * x[13+xIndex];    tmp11 += globalFilt16[93] * x[13+xIndex];
        tmp13 += globalFilt16[109] * x[13+xIndex];  tmp15 += globalFilt16[125] * x[13+xIndex];
        tmp17 += globalFilt16[141] * x[13+xIndex];  tmp19 += globalFilt16[157] * x[13+xIndex];
        tmp21 += globalFilt16[173] * x[13+xIndex];  tmp23 += globalFilt16[189] * x[13+xIndex];
        tmp25 += globalFilt16[205] * x[13+xIndex];  tmp27 += globalFilt16[221] * x[13+xIndex];
        tmp29 += globalFilt16[237] * x[13+xIndex];  tmp31 += globalFilt16[253] * x[13+xIndex];
      }
      
      if ( ISNOTZERO(x[14+xIndex]) ) {
        tmp0 += globalFilt16[14] * x[14+xIndex];    tmp2 += globalFilt16[30] * x[14+xIndex];
        tmp4 += globalFilt16[46] * x[14+xIndex];    tmp6 += globalFilt16[62] * x[14+xIndex];
        tmp8 += globalFilt16[78] * x[14+xIndex];    tmp10 += globalFilt16[94] * x[14+xIndex];
        tmp12 += globalFilt16[110] * x[14+xIndex];  tmp14 += globalFilt16[126] * x[14+xIndex];
        tmp16 += globalFilt16[142] * x[14+xIndex];  tmp18 += globalFilt16[158] * x[14+xIndex];
        tmp20 += globalFilt16[174] * x[14+xIndex];  tmp22 += globalFilt16[190] * x[14+xIndex];
        tmp24 += globalFilt16[206] * x[14+xIndex];  tmp26 += globalFilt16[222] * x[14+xIndex];
        tmp28 += globalFilt16[238] * x[14+xIndex];  tmp30 += globalFilt16[254] * x[14+xIndex];
      }
      
      if ( ISNOTZERO(x[15+xIndex]) ) {
        tmp1 += globalFilt16[15] * x[15+xIndex];    tmp3 += globalFilt16[31] * x[15+xIndex];
        tmp5 += globalFilt16[47] * x[15+xIndex];    tmp7 += globalFilt16[63] * x[15+xIndex];
        tmp9 += globalFilt16[79] * x[15+xIndex];    tmp11 += globalFilt16[95] * x[15+xIndex];
        tmp13 += globalFilt16[111] * x[15+xIndex];  tmp15 += globalFilt16[127] * x[15+xIndex];
        tmp17 += globalFilt16[143] * x[15+xIndex];  tmp19 += globalFilt16[159] * x[15+xIndex];
        tmp21 += globalFilt16[175] * x[15+xIndex];  tmp23 += globalFilt16[191] * x[15+xIndex];
        tmp25 += globalFilt16[207] * x[15+xIndex];  tmp27 += globalFilt16[223] * x[15+xIndex];
        tmp29 += globalFilt16[239] * x[15+xIndex];  tmp31 += globalFilt16[255] * x[15+xIndex];
      }

      if ( i == 0 ) {
        /* Left edge. */
        scratch[8+scratchIndex] = tmp14 + tmp15;
        scratch[9+scratchIndex] = tmp12 + tmp13;
        scratch[10+scratchIndex] = tmp10 + tmp11;
        scratch[11+scratchIndex] = tmp8 + tmp9;
        scratch[12+scratchIndex] = tmp6 + tmp7;
        scratch[13+scratchIndex] = tmp4 + tmp5;
        scratch[14+scratchIndex] = tmp2 + tmp3;
        scratch[15+scratchIndex] = tmp0 + tmp1;
      }
	      
      scratch[scratchIndex] += tmp0 + tmp1;     scratch[31+scratchIndex] = tmp0 - tmp1;
      scratch[1+scratchIndex] += tmp2 + tmp3;     scratch[30+scratchIndex] = tmp2 - tmp3;
      scratch[2+scratchIndex] += tmp4 + tmp5;     scratch[29+scratchIndex] = tmp4 - tmp5;
      scratch[3+scratchIndex] += tmp6 + tmp7;     scratch[28+scratchIndex] = tmp6 - tmp7;
      scratch[4+scratchIndex] += tmp8 + tmp9;     scratch[27+scratchIndex] = tmp8 - tmp9;
      scratch[5+scratchIndex] += tmp10 + tmp11;   scratch[26+scratchIndex] = tmp10 - tmp11;
      scratch[6+scratchIndex] += tmp12 + tmp13;   scratch[25+scratchIndex] = tmp12 - tmp13;
      scratch[7+scratchIndex] += tmp14 + tmp15;   scratch[24+scratchIndex] = tmp14 - tmp15;
      scratch[8+scratchIndex] += tmp16 + tmp17;   scratch[23+scratchIndex] = tmp16 - tmp17;
      scratch[9+scratchIndex] += tmp18 + tmp19;   scratch[22+scratchIndex] = tmp18 - tmp19;
      scratch[10+scratchIndex] += tmp20 + tmp21;  scratch[21+scratchIndex] = tmp20 - tmp21;
      scratch[11+scratchIndex] += tmp22 + tmp23;  scratch[20+scratchIndex] = tmp22 - tmp23;
      scratch[12+scratchIndex] += tmp24 + tmp25;  scratch[19+scratchIndex] = tmp24 - tmp25;
      scratch[13+scratchIndex] += tmp26 + tmp27;  scratch[18+scratchIndex] = tmp26 - tmp27;
      scratch[14+scratchIndex] += tmp28 + tmp29;  scratch[17+scratchIndex] = tmp28 - tmp29;
      scratch[15+scratchIndex] += tmp30 + tmp31;  scratch[16+scratchIndex] = tmp30 - tmp31;

      if ( i == nblocksM1 ) {
        /* Right edge (last time thru loop). */
        scratchIndex += 16;
        scratch[scratchIndex] += tmp0 - tmp1;
        scratch[1+scratchIndex] += tmp2 - tmp3;
        scratch[2+scratchIndex] += tmp4 - tmp5;
        scratch[3+scratchIndex] += tmp6 - tmp7;
        scratch[4+scratchIndex] += tmp8 - tmp9;
        scratch[5+scratchIndex] += tmp10 - tmp11;
        scratch[6+scratchIndex] += tmp12 - tmp13;
        scratch[7+scratchIndex] += tmp14 - tmp15;      
      }
      xIndex += 16;
      scratchIndex += 16;
    }

    for ( i=0; i<nsamps; i++ ) x[i+index] = scratch[i+8];

    return;
  }


  /**
   * Forward lapped orthogonal transform for the length-8 or length-16 case.
   *
   * @param  x  array to be transformed.
   * @param  index  index of first element in array to transform.
   * @param  blockSize  the block size.
   * @param  transLength  the transform length - must be 8 or 16.
   * @param  nblocks  number of blocks.
   * @param  scratch  work array - must be nsamps+32 in length.
   */
  int Transformer::lotFwd(float* x, int index, int blockSize, int transLength, int nblocks, float* scratch)
  {
    int i, nsubBlocks;
    if (transLength == 8) {
      nsubBlocks = (int)(blockSize / 8);
      if(nsubBlocks*8 != blockSize){
//         throw "Error in lotFwd nsubBlocks*8 != blockSize";
        ERROR_PRINTF(TransformerLog,"blockSize must be multiple of 8");
        return JS_USERERROR;
      }
      lotFwd8(x, index, nsubBlocks*nblocks, scratch);
      if (blockSize > 8){
        for (i=0; i<nblocks; i++){
          multiplex8(x, i*blockSize+index, nsubBlocks, scratch);
        } 
      } 
    } else if (transLength == 16) {
      nsubBlocks = blockSize / 16;
      if(nsubBlocks*16 != blockSize){
//         throw "Error in lotFwd nsubBlocks*16 != blockSize";
        ERROR_PRINTF(TransformerLog,"blockSize must be multiple of 16");
        return JS_USERERROR;
      }

      lotFwd16(x, index, nsubBlocks*nblocks, scratch);
      if (blockSize > 16){
        for (i=0; i<nblocks; i++){
          multiplex16(x, i*blockSize+index, nsubBlocks, scratch);
        }  
      }  
    } else {
//        throw "Error in lotFwd transLength must be equal to 8 or 16.";
      ERROR_PRINTF(TransformerLog,"transLength must be equal to 8 or 16");
      return JS_USERERROR;
    }
    return JS_OK;
  }


  /**
   * Reverse lapped orthogonal transform for the length-8 or length-16 case.
   *
   * @param  x  array to be transformed.
   * @param  index  index of first element in array to transform.
   * @param  blockSize  the block size.
   * @param  transLength  the transform length - must be 8 or 16.
   * @param  nblocks  number of blocks.
   * @param  scratch  work array - must be nsamps+32 in length.
   */
  int Transformer::lotRev(float* x, int index, int blockSize, int transLength, int nblocks, float* scratch)
  {
    int i, nsubBlocks;
    if (transLength == 8) {
      nsubBlocks = blockSize / 8;
      if(nsubBlocks*8 != blockSize){
//         throw "Error in lotFwd nsubBlocks*8 != blockSize";
        ERROR_PRINTF(TransformerLog,"blockSize must be multiple of 8");
        return JS_USERERROR;
      }
      if (blockSize > 8)
        for (i=0; i<nblocks; i++) deMultiplex8(x, i*blockSize+index, nsubBlocks, scratch);
      lotRev8(x, index, nsubBlocks*nblocks, scratch);
    } else if (transLength == 16) {
      nsubBlocks = blockSize / 16;
      if(nsubBlocks*16 != blockSize){
//         throw "Error in lotFwd nsubBlocks*16 != blockSize";
        ERROR_PRINTF(TransformerLog,"blockSize must be multiple of 16");
        return JS_USERERROR;
      }
      if (blockSize > 16)
        for (i=0; i<nblocks; i++) deMultiplex16(x, i*blockSize+index, nsubBlocks, scratch);
      lotRev16(x, index, nsubBlocks*nblocks, scratch);
    } else {
//        throw "Error in lotFwd transLength must be equal to 8 or 16.";
      ERROR_PRINTF(TransformerLog,"transLength must be equal to 8 or 16");
      return JS_USERERROR;
    }
    return JS_OK;
  }


  /**
   * Multiplexes values to minimum cache misses when transforming in the
   * second dimension.  For the length-8 case.
   *
   * @param  x  array to multiplex.
   * @param  xBaseIndex  offset in array.
   * @param  nblocks  the number of blocks.
   * @param  scratch  a work array.
   */
  void Transformer::multiplex8(float* x, int xBaseIndex, int nblocks, float* scratch) 
  {
    int indexScratch0 = 0;
    int indexScratch1 = nblocks;  
    int indexScratch2 = nblocks*2;
    int indexScratch3 = nblocks*3;  
    int indexScratch4 = nblocks*4;  
    int indexScratch5 = nblocks*5;  
    int indexScratch6 = nblocks*6;  
    int indexScratch7 = nblocks*7;  

    int i;

    int xIndex = xBaseIndex;

    for (i=0; i<nblocks; i++) {
      scratch[indexScratch0+i] = x[xIndex+0];
      scratch[indexScratch1+i] = x[xIndex+1];
      scratch[indexScratch2+i] = x[xIndex+2];
      scratch[indexScratch3+i] = x[xIndex+3];
      scratch[indexScratch4+i] = x[xIndex+4];
      scratch[indexScratch5+i] = x[xIndex+5];
      scratch[indexScratch6+i] = x[xIndex+6];
      scratch[indexScratch7+i] = x[xIndex+7];
      xIndex += 8;  
    }
    // Surprisingly this didn't help.
    //~ ArrayUtil.arraycopy(scratch, 0, x, xBaseIndex, nblocks*8);
    for (i=0; i<nblocks*8; i++) x[i+xBaseIndex] = scratch[i];

    return;
  }


  /**
   * Demultiplexes values to minimum cache misses when transforming in the
   * second dimension.  For the length-8 case.
   *
   * @param  x  array to demultiplex.
   * @param  xBaseIndex  offset in array.
   * @param  nblocks  the number of blocks.
   * @param  scratch  a work array.
   */
  void Transformer::deMultiplex8(float* x, int xBaseIndex, int nblocks, float* scratch) 
  {
    int x0Index = 0;
    int x1Index = nblocks;
    int x2Index = nblocks*2;
    int x3Index = nblocks*3;
    int x4Index = nblocks*4;
    int x5Index = nblocks*5;
    int x6Index = nblocks*6;
    int x7Index = nblocks*7;

    int i;
		
    int scratchIndex = 0;
		
    for (i=0; i<nblocks; i++) {
		  
      scratch[scratchIndex+0] = x[xBaseIndex+x0Index+i];
      scratch[scratchIndex+1] = x[xBaseIndex+x1Index+i];
      scratch[scratchIndex+2] = x[xBaseIndex+x2Index+i];
      scratch[scratchIndex+3] = x[xBaseIndex+x3Index+i];
      scratch[scratchIndex+4] = x[xBaseIndex+x4Index+i];
      scratch[scratchIndex+5] = x[xBaseIndex+x5Index+i];
      scratch[scratchIndex+6] = x[xBaseIndex+x6Index+i];
      scratch[scratchIndex+7] = x[xBaseIndex+x7Index+i];
	    
      scratchIndex += 8;
    }

    for (i=0; i<nblocks*8; i++) x[i+xBaseIndex] = scratch[i];

    return;
  }


  /**
   * Multiplexes values to minimum cache misses when transforming in the
   * second dimension.  For the length-16 case.
   *
   * @param  x  array to multiplex.
   * @param  xBaseIndex  offset in array.
   * @param  nblocks  the number of blocks.
   * @param  scratch  a work array.
   */
  void Transformer::multiplex16(float* x, int xBaseIndex, int nblocks, float* scratch)
  {
    int scratchIndex0 = 0;
    int scratchIndex1 = nblocks;
    int scratchIndex2 = nblocks*2;
    int scratchIndex3 = nblocks*3;
    int scratchIndex4 = nblocks*4;
    int scratchIndex5 = nblocks*5; 
    int scratchIndex6 = nblocks*6; 
    int scratchIndex7 = nblocks*7; 
    int scratchIndex8 = nblocks*8; 
    int scratchIndex9 = nblocks*9; 
    int scratchIndex10 = nblocks*10;
    int scratchIndex11 = nblocks*11; 
    int scratchIndex12 = nblocks*12; 
    int scratchIndex13 = nblocks*13; 
    int scratchIndex14 = nblocks*14; 
    int scratchIndex15 = nblocks*15;  

    int i;
		
    int xIndex = xBaseIndex;	
		
    for (i=0; i<nblocks; i++) {
		
      scratch[scratchIndex0+i] = x[xIndex+0];
      scratch[scratchIndex1+i] = x[xIndex+1];
      scratch[scratchIndex2+i] = x[xIndex+2];
      scratch[scratchIndex3+i] = x[xIndex+3];
      scratch[scratchIndex4+i] = x[xIndex+4];
      scratch[scratchIndex5+i] = x[xIndex+5];
      scratch[scratchIndex6+i] = x[xIndex+6];
      scratch[scratchIndex7+i] = x[xIndex+7];	     
      scratch[scratchIndex8+i] = x[xIndex+8];
      scratch[scratchIndex9+i] = x[xIndex+9];
      scratch[scratchIndex10+i] = x[xIndex+10];
      scratch[scratchIndex11+i] = x[xIndex+11];
      scratch[scratchIndex12+i] = x[xIndex+12];
      scratch[scratchIndex13+i] = x[xIndex+13];
      scratch[scratchIndex14+i] = x[xIndex+14];
      scratch[scratchIndex15+i] = x[xIndex+15];
	    
      xIndex += 16;
    }

    for (i=0; i<nblocks*16; i++) x[i+xBaseIndex] = scratch[i];

    return;
  }


  /**
   * Demultiplexes values to minimum cache misses when transforming in the
   * second dimension.  For the length-16 case.
   *
   * @param  x  array to demultiplex.
   * @param  xBaseIndex  offset in array.
   * @param  nblocks  the number of blocks.
   * @param  scratch  a work array.
   */
  void Transformer::deMultiplex16(float* x, int xBaseIndex, int nblocks, float* scratch)
  {
    int x0Index = 0;
    int x1Index = nblocks;
    int x2Index = nblocks*2;
    int x3Index = nblocks*3;
    int x4Index = nblocks*4;
    int x5Index = nblocks*5;
    int x6Index = nblocks*6;
    int x7Index = nblocks*7;
    int x8Index = nblocks*8;
    int x9Index = nblocks*9;
    int x10Index = nblocks*10;
    int x11Index = nblocks*11;
    int x12Index = nblocks*12;
    int x13Index = nblocks*13;
    int x14Index = nblocks*14;
    int x15Index = nblocks*15;
		
    int i;
		
    int scratchIndex = 0;
		
    for (i=0; i<nblocks; i++) {
	    
      scratch[scratchIndex+0] = x[xBaseIndex+x0Index+i];
      scratch[scratchIndex+1] = x[xBaseIndex+x1Index+i];
      scratch[scratchIndex+2] = x[xBaseIndex+x2Index+i];
      scratch[scratchIndex+3] = x[xBaseIndex+x3Index+i];
      scratch[scratchIndex+4] = x[xBaseIndex+x4Index+i];
      scratch[scratchIndex+5] = x[xBaseIndex+x5Index+i];
      scratch[scratchIndex+6] = x[xBaseIndex+x6Index+i];
      scratch[scratchIndex+7] = x[xBaseIndex+x7Index+i];
      scratch[scratchIndex+8] = x[xBaseIndex+x8Index+i];
      scratch[scratchIndex+9] = x[xBaseIndex+x9Index+i];
      scratch[scratchIndex+10] = x[xBaseIndex+x10Index+i];
      scratch[scratchIndex+11] = x[xBaseIndex+x11Index+i];
      scratch[scratchIndex+12] = x[xBaseIndex+x12Index+i];
      scratch[scratchIndex+13] = x[xBaseIndex+x13Index+i];
      scratch[scratchIndex+14] = x[xBaseIndex+x14Index+i];
      scratch[scratchIndex+15] = x[xBaseIndex+x15Index+i];

      scratchIndex += 16;
	    
    }
	  
    for (i=0; i<nblocks*16; i++) x[i+xBaseIndex] = scratch[i];

    return;
  }
}



