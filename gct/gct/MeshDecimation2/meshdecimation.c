/* *****************************************************************************
 *
 * Copyright (c) 2012-2017 Alexis Naveros.
 * Portions developed under contract to the SURVICE Engineering Company.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * *****************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>


#include "cc.h"
#include "mm.h"
#include "mmatomic.h"
#include "mmhash.h"

#include "mmbinsort.h"
#include "meshdecimation.h"


////


#if __SSE__ || _M_X64 || _M_IX86_FP >= 1  || CPU_ENABLE_SSE
 #include <xmmintrin.h>
 #define CPU_SSE_SUPPORT (1)
#endif
#if __SSE2__ || _M_X64 || _M_IX86_FP >= 2  || CPU_ENABLE_SSE2
 #include <emmintrin.h>
 #define CPU_SSE2_SUPPORT (1)
#endif
#if __SSE3__ || __AVX__ || CPU_ENABLE_SSE3
 #include <pmmintrin.h>
 #define CPU_SSE3_SUPPORT (1)
#endif
#if __SSSE3__ || __AVX__  || CPU_ENABLE_SSSE3
 #include <tmmintrin.h>
 #define CPU_SSSE3_SUPPORT (1)
#endif
#if __SSE4_1__ || __AVX__  || CPU_ENABLE_SSE4_1
 #include <smmintrin.h>
 #define CPU_SSE4_1_SUPPORT (1)
#endif


#if defined(__GNUC__) || defined(__INTEL_COMPILER)
 #define CPU_ALIGN16 __attribute__((aligned(16)))
 #define CPU_ALIGN32 __attribute__((aligned(32)))
 #define CPU_ALIGN64 __attribute__((aligned(64)))
#elif defined(_MSC_VER)
 #define CPU_ALIGN16 __declspec(align(16))
 #define CPU_ALIGN64 __declspec(align(64))
#else
 #define CPU_ALIGN16
 #define CPU_ALIGN32
 #define CPU_ALIGN64
 #warning "SSE/AVX Disabled: Unsupported Compiler."
 #undef CPU_SSE_SUPPORT
 #undef CPU_SSE2_SUPPORT
 #undef CPU_SSE3_SUPPORT
 #undef CPU_SSSE3_SUPPORT
 #undef CPU_SSE4_1_SUPPORT
#endif


////


/* Define to use double floating point precision */
#define MD_CONF_DOUBLE_PRECISION (1)

/* Define to use double precision just quadric maths. Very strongly recommended. */
#define MD_CONF_QUADRICS_DOUBLE_PRECISION (1)

/* Enable progress report callback */
#define MD_CONF_ENABLE_PROGRESS (1)

/* Align all ops on 64 bytes to reduce cache line fetches */
#define MD_CONF_OP_ALIGNMENT (0x40)

/* Required with multithreading! */
#define MD_CONFIG_DELAYED_OP_REDIRECT (1)

/* Quick hash function instead of cc* */
#define MD_CONFIG_FAST_HASH (1)

/* Try to use some crazy __float128 precision to track the d^2 accumulated error, if available */
#define MD_CONFIG_HIGH_QUADRICS (0)


#if 1
 #define MD_COLLAPSE_COST_COMPACT_TARGET (0.25)
 #define MD_COLLAPSE_COST_COMPACT_FACTOR (0.00125)
#else
 #define MD_COLLAPSE_COST_COMPACT_TARGET (0.4)
 #define MD_COLLAPSE_COST_COMPACT_FACTOR (0.025)
#endif

#define MD_BOUNDARY_WEIGHT (5.0)

#define MD_SYNC_STEP_COUNT (32)

#define MD_QUADRIC_DETERMINANT_MIN (0.0000000001)

#define MD_GLOBAL_LOCK_THRESHOLD (16)


#define MD_THREAD_COUNT_DEFAULT (4)

#define MD_THREAD_COUNT_MAX (64)


////


#if 1
 #define DEBUG_VERBOSE (0)
 #define DEBUG_VERBOSE_QUADRIC (0)
 #define DEBUG_VERBOSE_BOUNDARY (0)
 #define DEBUG_VERBOSE_COLLISION (0)
 #define DEBUG_VERBOSE_COST (0)
 #define DEBUG_VERBOSE_COLLAPSE (0)
 #define DEBUG_VERBOSE_EVALUATE (0)
 #define DEBUG_VERBOSE_MEMORY (0)
#else
 #define DEBUG_VERBOSE (0)
 #define DEBUG_VERBOSE_QUADRIC (1)
 #define DEBUG_VERBOSE_BOUNDARY (1)
 #define DEBUG_VERBOSE_COLLISION (1)
 #define DEBUG_VERBOSE_COST (1)
 #define DEBUG_VERBOSE_COLLAPSE (1)
 #define DEBUG_VERBOSE_EVALUATE (1)
 #define DEBUG_VERBOSE_MEMORY (0)
#endif

/*
#define DEBUG_LIMIT (1)
*/

#define DEBUG_DEBUG_CHECK_SOMETHING (0)


#if DEBUG_VERBOSE || DEBUG_VERBOSE_COLLISION || DEBUG_VERBOSE_COLLAPSE
 #if 0
  #define MD_ERROR(s,f,...) ({fprintf(stderr,s,__VA_ARGS__);if(f) exit(1);})
 #else
  #define MD_ERROR(s,f,...) ({fprintf(stdout,s,__VA_ARGS__);fflush(stdout);})
 #endif
#else
 #define MD_ERROR(s,f,...) ({fprintf(stderr,s,__VA_ARGS__);})
#endif


////


#if MM_ATOMIC_SUPPORT
 #define MD_CONFIG_ATOMIC_SUPPORT (1)
#endif


////


#if MD_CONF_DOUBLE_PRECISION
typedef double mdf;
 #define mdfmin(x,y) fmin((x),(y))
 #define mdfmax(x,y) fmax((x),(y))
 #define mdffloor(x) floor(x)
 #define mdfceil(x) ceil(x)
 #define mdfround(x) round(x)
 #define mdfsqrt(x) sqrt(x)
 #define mdfcbrt(x) cbrt(x)
 #define mdfabs(x) fabs(x)
 #define mdflog2(x) log2(x)
 #define mdfacos(x) acos(x)
#else
typedef float mdf;
 #define mdfmin(x,y) fminf((x),(y))
 #define mdfmax(x,y) fmaxf((x),(y))
 #define mdffloor(x) floorf(x)
 #define mdfceil(x) ceilf(x)
 #define mdfround(x) roundf(x)
 #define mdfsqrt(x) sqrtf(x)
 #define mdfcbrt(x) cbrtf(x)
 #define mdfabs(x) fabsf(x)
 #define mdflog2(x) log2f(x)
 #define mdfacos(x) acosf(x)
#endif

#if MD_CONF_DOUBLE_PRECISION
 #if !MD_CONF_QUADRICS_DOUBLE_PRECISION
  #define MD_CONF_QUADRICS_DOUBLE_PRECISION (1)
 #endif
#endif

#if MD_CONF_QUADRICS_DOUBLE_PRECISION
typedef double mdqf;
#else
typedef float mdqf;
#endif

#if MD_CONFIG_HIGH_QUADRICS
 #if ( __GNUC__ > 4 || ( __GNUC__ == 4 && __GNUC_MINOR__ >= 6 ) ) && !defined(__INTEL_COMPILER) && ( defined(__i386__) || defined(__x86_64__) || defined(__ia64__) )
typedef __float128 mdqfhigh;
 #else
  #undef MD_CONFIG_HIGH_QUADRICS
typedef double mdqfhigh;
 #endif
#elif MD_CONF_QUADRICS_DOUBLE_PRECISION
typedef double mdqfhigh;
#else
typedef float mdqfhigh;
#endif

#define MD_SIZEOF_MDI (4)
typedef int32_t mdi;


////


#ifndef M_PI
 #define M_PI (3.14159265358979323846264338327)
#endif

#define MD_VectorSubStore(x,y,z) (x)[0]=(y)[0]-(z)[0];(x)[1]=(y)[1]-(z)[1];(x)[2]=(y)[2]-(z)[2]
#define MD_VectorCrossProduct(x,y,z) ({(x)[0]=((y)[1]*(z)[2])-((y)[2]*(z)[1]);(x)[1]=((y)[2]*(z)[0])-((y)[0]*(z)[2]);(x)[2]=((y)[0]*(z)[1])-((y)[1]*(z)[0]);})
#define MD_VectorDotProduct(x,y) ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define MD_VectorCopy(x,y) ({(x)[0]=(y)[0];(x)[1]=(y)[1];(x)[2]=(y)[2];})
#define MD_VectorMulScalar(x,y) (x)[0]*=(y);(x)[1]*=(y);(x)[2]*=(y)
#define MD_VectorNormalize(x) ({mdf _f;_f=1.0/sqrt((x)[0]*(x)[0]+(x)[1]*(x)[1]+(x)[2]*(x)[2]);(x)[0]*=_f;(x)[1]*=_f;(x)[2]*=_f;})
#define MD_VectorZero(x) ({(x)[0]=0.0;(x)[1]=0.0;(x)[2]=0.0;})
#define MD_VectorMagnitude(x) (sqrt((x)[0]*(x)[0]+(x)[1]*(x)[1]+(x)[2]*(x)[2]))
#define MD_VectorAddMulScalar(x,y,z) (x)[0]+=(y)[0]*(z);(x)[1]+=(y)[1]*(z);(x)[2]+=(y)[2]*(z)


////


typedef struct
{
  mdqf area;
  mdqf a2, ab, ac, ad;
  mdqf b2, bc, bd;
  mdqf c2, cd;
  mdqfhigh d2;
} mathQuadric;

static inline void mathQuadricInit( mathQuadric *q, mdqf a, mdqf b, mdqf c, mdqf d, mdqf area )
{
  q->area = area;
  q->a2 = a * a;
  q->ab = a * b;
  q->ac = a * c;
  q->ad = a * d;
  q->b2 = b * b;
  q->bc = b * c;
  q->bd = b * d;
  q->c2 = c * c;
  q->cd = c * d;
  q->d2 = (mdqfhigh)d * (mdqfhigh)d;
#if DEBUG_VERBOSE_QUADRIC
  printf( "    Q Init %f ; %f %f %f %f %f %f %f %f %f %f\n", (double)q->area, (double)q->a2, (double)q->ab, (double)q->ac, (double)q->ad, (double)q->b2, (double)q->bc, (double)q->bd, (double)q->c2, (double)q->cd, (double)q->d2 );
#endif
  return;
}


////


static inline mdqf mathMatrix3x3Determinant( mdqf *m )
{
  mdqf det;
  det  = m[0*3+0] * ( ( m[2*3+2] * m[1*3+1] ) - ( m[2*3+1] * m[1*3+2] ) );
  det -= m[1*3+0] * ( ( m[2*3+2] * m[0*3+1] ) - ( m[2*3+1] * m[0*3+2] ) );
  det += m[2*3+0] * ( ( m[1*3+2] * m[0*3+1] ) - ( m[1*3+1] * m[0*3+2] ) );
  return det;
}

static inline void mathMatrix3x3MulVector( mdqf *vdst, mdqf *m, mdqf *v )
{
  vdst[0] = ( v[0] * m[0*3+0] ) + ( v[1] * m[1*3+0] ) + ( v[2] * m[2*3+0] );
  vdst[1] = ( v[0] * m[0*3+1] ) + ( v[1] * m[1*3+1] ) + ( v[2] * m[2*3+1] );
  vdst[2] = ( v[0] * m[0*3+2] ) + ( v[1] * m[1*3+2] ) + ( v[2] * m[2*3+2] );
  return;
}

static inline void mathQuadricToMatrix3x3( mdqf *m, mathQuadric *q )
{
  m[0*3+0] = q->a2;
  m[0*3+1] = q->ab;
  m[0*3+2] = q->ac;
  m[1*3+0] = q->ab;
  m[1*3+1] = q->b2;
  m[1*3+2] = q->bc;
  m[2*3+0] = q->ac;
  m[2*3+1] = q->bc;
  m[2*3+2] = q->c2;
  return;
}

static inline void mathMatrix3x3Invert( mdqf *mdst, mdqf *m, mdqf det )
{
  mdf detinv;
  detinv = 1.0 / det;
  mdst[0*3+0] =  ( ( m[2*3+2] * m[1*3+1] ) - ( m[2*3+1] * m[1*3+2] ) ) * detinv;
  mdst[0*3+1] = -( ( m[2*3+2] * m[0*3+1] ) - ( m[2*3+1] * m[0*3+2] ) ) * detinv;
  mdst[0*3+2] =  ( ( m[1*3+2] * m[0*3+1] ) - ( m[1*3+1] * m[0*3+2] ) ) * detinv;
  mdst[1*3+0] = -( ( m[2*3+2] * m[1*3+0] ) - ( m[2*3+0] * m[1*3+2] ) ) * detinv;
  mdst[1*3+1] =  ( ( m[2*3+2] * m[0*3+0] ) - ( m[2*3+0] * m[0*3+2] ) ) * detinv;
  mdst[1*3+2] = -( ( m[1*3+2] * m[0*3+0] ) - ( m[1*3+0] * m[0*3+2] ) ) * detinv;
  mdst[2*3+0] =  ( ( m[2*3+1] * m[1*3+0] ) - ( m[2*3+0] * m[1*3+1] ) ) * detinv;
  mdst[2*3+1] = -( ( m[2*3+1] * m[0*3+0] ) - ( m[2*3+0] * m[0*3+1] ) ) * detinv;
  mdst[2*3+2] =  ( ( m[1*3+1] * m[0*3+0] ) - ( m[1*3+0] * m[0*3+1] ) ) * detinv;
  return;
}


////


static inline int mathQuadricSolve( mathQuadric *q, mdf *v )
{
  mdqf det, m[9], minv[9], vector[3], vres[3];
  mdqf areascale;
  mathQuadricToMatrix3x3( m, q );
  det = mathMatrix3x3Determinant( m );
/*
Det scale diagnotic
Scale 10x
  QAdd Sr0 1.120477 ; 0.022271 0.030416 -0.065258 -1.217629 0.042473 -0.090558 -1.704271 0.196298 3.621082 68.438599
  QAdd Sr0 112.048033 ; 222.705246 304.155192 -652.582478 -121762.889543 424.729952 -905.579057 -170427.095051 1962.994524 362109.207338 68438554.421304
  Det 0.000000440467
  Det 440472.329126436263
If scale is 10x, then:
  q0->area : 100x (10^2)
  q0->a2 : 10000x (10^4)
  q0->ab : 10000x (10^4)
  q0->ac : 10000x (10^4)
  q0->ad : 100000x (10^5)
  q0->b2 : 10000x (10^4)
  q0->bc : 10000x (10^4)
  q0->bd : 100000x (10^5)
  q0->c2 : 10000x (10^4)
  q0->cd : 100000x (10^5)
  q0->d2 : 1000000x (10^6)
  det : 1000000000000x (10^12)
*/
  areascale = q->area * q->area;
  areascale = areascale * areascale * areascale;
  if( mdfabs( det ) <= ( MD_QUADRIC_DETERMINANT_MIN * areascale ) )
  {
#if DEBUG_VERBOSE_QUADRIC
  printf( "        Det : %.12f ; Fail\n", (double)det );
#endif
    return 0;
  }
#if DEBUG_VERBOSE_QUADRIC
  printf( "        Det : %.12f ; Pass\n", (double)det );
#endif
  mathMatrix3x3Invert( minv, m, det );
  vector[0] = -q->ad;
  vector[1] = -q->bd;
  vector[2] = -q->cd;
  mathMatrix3x3MulVector( vres, minv, vector );
  v[0] = (mdf)vres[0];
  v[1] = (mdf)vres[1];
  v[2] = (mdf)vres[2];
#if DEBUG_VERBOSE_QUADRIC
  printf( "    Vector ; %f %f %f ; %f %f %f ( Det %.12f )\n", (double)vector[0], (double)vector[1], (double)vector[2], (double)v[0], (double)v[1], (double)v[2], det );
#endif
  return 1;
}

static inline void mathQuadricZero( mathQuadric *q )
{
  q->area = 0.0;
  q->a2 = 0.0;
  q->ab = 0.0;
  q->ac = 0.0;
  q->ad = 0.0;
  q->b2 = 0.0;
  q->bc = 0.0;
  q->bd = 0.0;
  q->c2 = 0.0;
  q->cd = 0.0;
  q->d2 = 0.0;
  return;
}

static inline void mathQuadricAddStoreQuadric( mathQuadric *qdst, mathQuadric *q0, mathQuadric *q1 )
{
#if DEBUG_VERBOSE_QUADRIC
  printf( "    QAdd Sr0 %f ; %f %f %f %f %f %f %f %f %f %f\n", (double)q0->area, (double)q0->a2, (double)q0->ab, (double)q0->ac, (double)q0->ad, (double)q0->b2, (double)q0->bc, (double)q0->bd, (double)q0->c2, (double)q0->cd, (double)q0->d2 );
  printf( "    QAdd Sr1 %f ; %f %f %f %f %f %f %f %f %f %f\n", (double)q1->area, (double)q1->a2, (double)q1->ab, (double)q1->ac, (double)q1->ad, (double)q1->b2, (double)q1->bc, (double)q1->bd, (double)q1->c2, (double)q1->cd, (double)q1->d2 );
#endif
  qdst->area = q0->area + q1->area;
  qdst->a2 = q0->a2 + q1->a2;
  qdst->ab = q0->ab + q1->ab;
  qdst->ac = q0->ac + q1->ac;
  qdst->ad = q0->ad + q1->ad;
  qdst->b2 = q0->b2 + q1->b2;
  qdst->bc = q0->bc + q1->bc;
  qdst->bd = q0->bd + q1->bd;
  qdst->c2 = q0->c2 + q1->c2;
  qdst->cd = q0->cd + q1->cd;
  qdst->d2 = q0->d2 + q1->d2;
#if DEBUG_VERBOSE_QUADRIC
  printf( "      QSum   %f ; %f %f %f %f %f %f %f %f %f %f\n", (double)qdst->area, (double)qdst->a2, (double)qdst->ab, (double)qdst->ac, (double)qdst->ad, (double)qdst->b2, (double)qdst->bc, (double)qdst->bd, (double)qdst->c2, (double)qdst->cd, (double)qdst->d2 );
#endif
  return;
}

static inline void mathQuadricAddQuadric( mathQuadric *qdst, mathQuadric *q )
{
#if DEBUG_VERBOSE_QUADRIC
  printf( "    QAdd Src %f ; %f %f %f %f %f %f %f %f %f %f\n", (double)q->area, (double)q->a2, (double)q->ab, (double)q->ac, (double)q->ad, (double)q->b2, (double)q->bc, (double)q->bd, (double)q->c2, (double)q->cd, (double)q->d2 );
  printf( "    QAdd Dst %f ; %f %f %f %f %f %f %f %f %f %f\n", (double)qdst->area, (double)qdst->a2, (double)qdst->ab, (double)qdst->ac, (double)qdst->ad, (double)qdst->b2, (double)qdst->bc, (double)qdst->bd, (double)qdst->c2, (double)qdst->cd, (double)qdst->d2 );
#endif
  qdst->area += q->area;
  qdst->a2 += q->a2;
  qdst->ab += q->ab;
  qdst->ac += q->ac;
  qdst->ad += q->ad;
  qdst->b2 += q->b2;
  qdst->bc += q->bc;
  qdst->bd += q->bd;
  qdst->c2 += q->c2;
  qdst->cd += q->cd;
  qdst->d2 += q->d2;
#if DEBUG_VERBOSE_QUADRIC
  printf( "      QSum   %f ; %f %f %f %f %f %f %f %f %f %f\n", (double)qdst->area, (double)qdst->a2, (double)qdst->ab, (double)qdst->ac, (double)qdst->ad, (double)qdst->b2, (double)qdst->bc, (double)qdst->bd, (double)qdst->c2, (double)qdst->cd, (double)qdst->d2 );
#endif
  return;
}

/* A volatile variable is used to force the compiler to do the math strictly in the order specified. */
static mdf mathQuadricEvaluate( mathQuadric *q, mdf *v )
{
  volatile mdqfhigh d;
  mdqfhigh vh[3];
  vh[0] = v[0];
  vh[1] = v[1];
  vh[2] = v[2];
  d = ( vh[0] * vh[0] * (mdqfhigh)q->a2 ) + ( vh[1] * vh[1] * (mdqfhigh)q->b2 ) + ( vh[2] * vh[2] * (mdqfhigh)q->c2 );
  d += 2.0 * ( ( vh[0] * vh[1] * (mdqfhigh)q->ab ) + ( vh[0] * vh[2] * (mdqfhigh)q->ac ) + ( vh[1] * vh[2] * (mdqfhigh)q->bc ) );
  d += 2.0 * ( ( vh[0] * (mdqfhigh)q->ad ) + ( vh[1] * (mdqfhigh)q->bd ) + ( vh[2] * (mdqfhigh)q->cd ) );
  d += q->d2;
#if DEBUG_VERBOSE_QUADRIC
  printf( "        Q Eval %f ; %f %f %f %f %f %f %f %f %f %f : %.12f\n", (double)q->area, (double)q->a2, (double)q->ab, (double)q->ac, (double)q->ad, (double)q->b2, (double)q->bc, (double)q->bd, (double)q->c2, (double)q->cd, (double)q->d2, (double)d );
#endif
  return (mdf)d;
}

static inline void mathQuadricMul( mathQuadric *qdst, mdf f )
{
  qdst->a2 *= f;
  qdst->ab *= f;
  qdst->ac *= f;
  qdst->ad *= f;
  qdst->b2 *= f;
  qdst->bc *= f;
  qdst->bd *= f;
  qdst->c2 *= f;
  qdst->cd *= f;
  qdst->d2 *= f;
  return;
}


//////


typedef struct
{
  mtMutex mutex;
  mtSignal signal;
  int resetcount;
  volatile int index;
  volatile int count[2];
} mdBarrier;

static void mdBarrierInit( mdBarrier *barrier, int count )
{
  mtMutexInit( &barrier->mutex );
  mtSignalInit( &barrier->signal );
  barrier->resetcount = count;
  barrier->index = 0;
  barrier->count[0] = count;
  barrier->count[1] = count;
  return;
}

static void mdBarrierDestroy( mdBarrier *barrier )
{
  mtMutexDestroy( &barrier->mutex );
  mtSignalDestroy( &barrier->signal );
  return;
}

static int mdBarrierSync( mdBarrier *barrier )
{
  int index, ret;
  mtMutexLock( &barrier->mutex );
  index = barrier->index;
  ret = 0;
  if( !( --barrier->count[index] ) )
  {
    ret = 1;
    mtSignalBroadcast( &barrier->signal );
    index ^= 1;
    barrier->index = index;
    barrier->count[index] = barrier->resetcount;
  }
  else
  {
    for( ; barrier->count[index] ; )
      mtSignalWait( &barrier->signal, &barrier->mutex );
  }
  mtMutexUnlock( &barrier->mutex );
  return ret;
}

static int mdBarrierSyncTimeout( mdBarrier *barrier, long milliseconds )
{
  int index, ret;
  mtMutexLock( &barrier->mutex );
  index = barrier->index;
  ret = 0;
  if( !( --barrier->count[index] ) )
  {
    ret = 1;
    mtSignalBroadcast( &barrier->signal );
    index ^= 1;
    barrier->index = index;
    barrier->count[index] = barrier->resetcount;
  }
  else
  {
    mtSignalWaitTimeout( &barrier->signal, &barrier->mutex, milliseconds );
    if( !( barrier->count[index] ) )
      ret = 1;
    else
      barrier->count[index]++;
  }
  mtMutexUnlock( &barrier->mutex );
  return ret;
}


//////


/* 16 bytes ; OK ~ Custom tridata packed after it, kept aligned on 8 bytes */
typedef struct
{
  mdi v[3];
  union
  {
    int edgeflags;
    mdi redirectindex;
  } u;
} mdTriangle;

typedef struct
{
  mdi v[2];
  mdi triindex;
  void *op;
} mdEdge;

/* 76 bytes, try packing to 64 bytes? */
typedef struct CPU_ALIGN16
{
#if CPU_SSE_SUPPORT
  mdf CPU_ALIGN16 point[4];
#else
  mdf point[3];
#endif
#if MD_CONFIG_ATOMIC_SUPPORT
  mmAtomic32 atomicowner;
#else
  int owner;
  mtSpin ownerspinlock;
#endif
  mdi trirefbase;
  mdi trirefcount;
  mdi redirectindex;
  mathQuadric quadric;
} mdVertex;


typedef struct
{
  int threadcount;
  uint32_t operationflags;
  int updatestatusflag;

  /* User supplied raw data */
  void *point;
  size_t pointstride;
  void *indices;
  size_t indicesstride;
  void *tridata;
  size_t tridatasize;
  void (*indicesUserToNative)( mdi *dst, void *src );
  void (*indicesNativeToUser)( void *dst, mdi *src );
  void (*vertexUserToNative)( mdf *dst, void *src );
  void (*vertexNativeToUser)( void *dst, mdf *src );
  double (*edgeweight)( void *tridata0, void *tridata1 );
  void (*vertexmerge)( void *mergecontext, int dstindex, int srcindex, double dstfactor, double srcfactor );
  void *mergecontext;
  void (*vertexcopy)( void *copycontext, int dstindex, int srcindex );
  void *copycontext;
  void (*writenormal)( void *dst, mdf *src );

  /* Per-vertex triangle references */
  mdi *trireflist;
  mdi trirefcount;
  long trirefalloc;
  char paddingA[64];
#if MD_CONFIG_ATOMIC_SUPPORT
  mmAtomic32 trireflock;
#else
  mtSpin trirefspinlock;
#endif
  char paddingB[64];

  /* Synchronization stuff */
  mdBarrier workbarrier;
  mdBarrier globalbarrier;
  int updatebuffercount;
  int updatebuffershift;

  /* List of triangles */
  void *trilist;
  long tricount;
  long tripackcount;
  size_t trisize;

  /* List of vertices */
  mdVertex *vertexlist;
  long vertexcount;
  long vertexalloc;
  long vertexpackcount;

  /* Hash table to locate edges from their vertex indices */
  void *edgehashtable;

  /* Collapse penalty function */
  mdf (*collapsepenalty)( mdf *newpoint, mdf *oldpoint, mdf *leftpoint, mdf *rightpoint, int *denyflag, mdf compactnesstarget );

  /* To compute vertex normals */
  void *normalbase;
  int normalformat;
  size_t normalstride;

  /* Decimation strength, max cost */
  mdf maxcollapsecost;

  char paddingC[64];
#if MD_CONFIG_ATOMIC_SUPPORT
  mmAtomic32 globalvertexlock;
#else
  mtSpin globalvertexspinlock;
#endif
  char paddingD[64];

  /* Advanced configuration options */
  mdf compactnesstarget;
  mdf compactnesspenalty;
  mdf boundaryweight;
  int syncstepcount;
  mdf normalsearchangle;

  /* Normal recomputation buffers */
  int clonesearchindex;
  void *vertexnormal;
  void *trinormal;

} mdMesh;


////


static void mdIndicesCharToNative( mdi *dst, void *src )
{
  unsigned char *s;
  s = src;
  dst[0] = s[0];
  dst[1] = s[1];
  dst[2] = s[2];
  return;
}

static void mdIndicesShortToNative( mdi *dst, void *src )
{
  unsigned short *s;
  s = src;
  dst[0] = s[0];
  dst[1] = s[1];
  dst[2] = s[2];
  return;
}

static void mdIndicesIntToNative( mdi *dst, void *src )
{
  unsigned int *s;
  s = src;
  dst[0] = s[0];
  dst[1] = s[1];
  dst[2] = s[2];
  return;
}

static void mdIndicesInt8ToNative( mdi *dst, void *src )
{
  uint8_t *s;
  s = src;
  dst[0] = s[0];
  dst[1] = s[1];
  dst[2] = s[2];
  return;
}

static void mdIndicesInt16ToNative( mdi *dst, void *src )
{
  uint16_t *s;
  s = src;
  dst[0] = s[0];
  dst[1] = s[1];
  dst[2] = s[2];
  return;
}

static void mdIndicesInt32ToNative( mdi *dst, void *src )
{
  uint32_t *s;
  s = src;
  dst[0] = s[0];
  dst[1] = s[1];
  dst[2] = s[2];
  return;
}

static void mdIndicesInt64ToNative( mdi *dst, void *src )
{
  uint64_t *s;
  s = src;
  dst[0] = s[0];
  dst[1] = s[1];
  dst[2] = s[2];
  return;
}


static void mdIndicesNativeToChar( void *dst, mdi *src )
{
  unsigned char *d;
  d = dst;
  d[0] = src[0];
  d[1] = src[1];
  d[2] = src[2];
  return;
}

static void mdIndicesNativeToShort( void *dst, mdi *src )
{
  unsigned short *d;
  d = dst;
  d[0] = src[0];
  d[1] = src[1];
  d[2] = src[2];
  return;
}

static void mdIndicesNativeToInt( void *dst, mdi *src )
{
  unsigned int *d;
  d = dst;
  d[0] = src[0];
  d[1] = src[1];
  d[2] = src[2];
  return;
}

static void mdIndicesNativeToInt8( void *dst, mdi *src )
{
  uint8_t *d;
  d = dst;
  d[0] = src[0];
  d[1] = src[1];
  d[2] = src[2];
  return;
}

static void mdIndicesNativeToInt16( void *dst, mdi *src )
{
  uint16_t *d;
  d = dst;
  d[0] = src[0];
  d[1] = src[1];
  d[2] = src[2];
  return;
}

static void mdIndicesNativeToInt32( void *dst, mdi *src )
{
  uint32_t *d;
  d = dst;
  d[0] = src[0];
  d[1] = src[1];
  d[2] = src[2];
  return;
}

static void mdIndicesNativeToInt64( void *dst, mdi *src )
{
  uint64_t *d;
  d = dst;
  d[0] = src[0];
  d[1] = src[1];
  d[2] = src[2];
  return;
}


static void mdVertexFloatToNative( mdf *dst, void *src )
{
  float *s;
  s = src;
  dst[0] = s[0];
  dst[1] = s[1];
  dst[2] = s[2];
  return;
}

static void mdVertexDoubleToNative( mdf *dst, void *src )
{
  double *s;
  s = src;
  dst[0] = s[0];
  dst[1] = s[1];
  dst[2] = s[2];
  return;
}

static void mdVertexShortToNative( mdf *dst, void *src )
{
  short *s;
  s = src;
  dst[0] = s[0];
  dst[1] = s[1];
  dst[2] = s[2];
  return;
}

static void mdVertexIntToNative( mdf *dst, void *src )
{
  int *s;
  s = src;
  dst[0] = s[0];
  dst[1] = s[1];
  dst[2] = s[2];
  return;
}

static void mdVertexInt16ToNative( mdf *dst, void *src )
{
  int16_t *s;
  s = src;
  dst[0] = s[0];
  dst[1] = s[1];
  dst[2] = s[2];
  return;
}

static void mdVertexInt32ToNative( mdf *dst, void *src )
{
  int32_t *s;
  s = src;
  dst[0] = s[0];
  dst[1] = s[1];
  dst[2] = s[2];
  return;
}


static void mdVertexNativeToFloat( void *dst, mdf *src )
{
  float *d;
  d = dst;
  d[0] = src[0];
  d[1] = src[1];
  d[2] = src[2];
  return;
}

static void mdVertexNativeToDouble( void *dst, mdf *src )
{
  double *d;
  d = dst;
  d[0] = src[0];
  d[1] = src[1];
  d[2] = src[2];
  return;
}

static void mdVertexNativeToShort( void *dst, mdf *src )
{
  short *d;
  d = dst;
  d[0] = src[0];
  d[1] = src[1];
  d[2] = src[2];
  return;
}

static void mdVertexNativeToInt( void *dst, mdf *src )
{
  int *d;
  d = dst;
  d[0] = src[0];
  d[1] = src[1];
  d[2] = src[2];
  return;
}

static void mdVertexNativeToInt16( void *dst, mdf *src )
{
  int16_t *d;
  d = dst;
  d[0] = src[0];
  d[1] = src[1];
  d[2] = src[2];
  return;
}

static void mdVertexNativeToInt32( void *dst, mdf *src )
{
  int32_t *d;
  d = dst;
  d[0] = src[0];
  d[1] = src[1];
  d[2] = src[2];
  return;
}


static void mdNormalNativeToChar( void *dst, mdf *src )
{
  mdf v;
  char *d;
  d = dst;
  v = mdfmin( 1.0, mdfmax( -1.0, src[0] ) );
  if( v > 0.0 )
    d[0] = (char)( ( v * (mdf)((1<<CHAR_BIT)-1) ) + 0.5 );
  else
    d[0] = (char)( ( v * (mdf)(1<<CHAR_BIT) ) - 0.5 );
  v = mdfmin( 1.0, mdfmax( -1.0, src[1] ) );
  if( v > 0.0 )
    d[1] = (char)( ( v * (mdf)((1<<CHAR_BIT)-1) ) + 0.5 );
  else
    d[1] = (char)( ( v * (mdf)(1<<CHAR_BIT) ) - 0.5 );
  v = mdfmin( 1.0, mdfmax( -1.0, src[2] ) );
  if( v > 0.0 )
    d[2] = (char)( ( v * (mdf)((1<<CHAR_BIT)-1) ) + 0.5 );
  else
    d[2] = (char)( ( v * (mdf)(1<<CHAR_BIT) ) - 0.5 );
  return;
}

static void mdNormalNativeToShort( void *dst, mdf *src )
{
  mdf v;
  short *d;
  d = dst;
  v = mdfmin( 1.0, mdfmax( -1.0, src[0] ) );
  if( v > 0.0 )
    d[0] = (short)( ( v * (mdf)(((1<<CHAR_BIT)*sizeof(short))-1) ) + 0.5 );
  else
    d[0] = (short)( ( v * (mdf)((1<<CHAR_BIT)*sizeof(short)) ) - 0.5 );
  v = mdfmin( 1.0, mdfmax( -1.0, src[1] ) );
  if( v > 0.0 )
    d[1] = (short)( ( v * (mdf)(((1<<CHAR_BIT)*sizeof(short))-1) ) + 0.5 );
  else
    d[1] = (short)( ( v * (mdf)((1<<CHAR_BIT)*sizeof(short)) ) - 0.5 );
  v = mdfmin( 1.0, mdfmax( -1.0, src[2] ) );
  if( v > 0.0 )
    d[2] = (short)( ( v * (mdf)(((1<<CHAR_BIT)*sizeof(short))-1) ) + 0.5 );
  else
    d[2] = (short)( ( v * (mdf)((1<<CHAR_BIT)*sizeof(short)) ) - 0.5 );
  return;
}

static void mdNormalNativeToInt8( void *dst, mdf *src )
{
  mdf v;
  int8_t *d;
  d = dst;
  v = mdfmin( 1.0, mdfmax( -1.0, src[0] ) );
  if( v > 0.0 )
    d[0] = (int8_t)( ( v * 127.0 ) + 0.5 );
  else
    d[0] = (int8_t)( ( v * 128.0 ) - 0.5 );
  v = mdfmin( 1.0, mdfmax( -1.0, src[1] ) );
  if( v > 0.0 )
    d[1] = (int8_t)( ( v * 127.0 ) + 0.5 );
  else
    d[1] = (int8_t)( ( v * 128.0 ) - 0.5 );
  v = mdfmin( 1.0, mdfmax( -1.0, src[2] ) );
  if( v > 0.0 )
    d[2] = (int8_t)( ( v * 127.0 ) + 0.5 );
  else
    d[2] = (int8_t)( ( v * 128.0 ) - 0.5 );
  return;
}

static void mdNormalNativeToInt16( void *dst, mdf *src )
{
  mdf v;
  int16_t *d;
  d = dst;
  v = mdfmin( 1.0, mdfmax( -1.0, src[0] ) );
  if( v > 0.0 )
    d[0] = (int8_t)( ( v * 32767.0 ) + 0.5 );
  else
    d[0] = (int8_t)( ( v * 32768.0 ) - 0.5 );
  v = mdfmin( 1.0, mdfmax( -1.0, src[1] ) );
  if( v > 0.0 )
    d[1] = (int8_t)( ( v * 32767.0 ) + 0.5 );
  else
    d[1] = (int8_t)( ( v * 32768.0 ) - 0.5 );
  v = mdfmin( 1.0, mdfmax( -1.0, src[2] ) );
  if( v > 0.0 )
    d[2] = (int8_t)( ( v * 32767.0 ) + 0.5 );
  else
    d[2] = (int8_t)( ( v * 32768.0 ) - 0.5 );
  return;
}

static void mdNormalNativeTo10_10_10_2( void *dst,  mdf *src )
{
  int part;
  uint32_t sum;
  mdf v;
  sum = 0;
  v = mdfmin( 1.0, mdfmax( -1.0, src[0] ) );
  if( v > 0.0 )
    part = (int)( ( v * 511.0 ) + 0.5 );
  else
    part = (int)( ( v * 512.0 ) - 0.5 );
  sum |= ( (uint32_t)part & 1023 ) << 0;
  v = mdfmin( 1.0, mdfmax( -1.0, src[1] ) );
  if( v > 0.0 )
    part = (int)( ( v * 511.0 ) + 0.5 );
  else
    part = (int)( ( v * 512.0 ) - 0.5 );
  sum |= ( (uint32_t)part & 1023 ) << 10;
  v = mdfmin( 1.0, mdfmax( -1.0, src[2] ) );
  if( v > 0.0 )
    part = (int)( ( v * 511.0 ) + 0.5 );
  else
    part = (int)( ( v * 512.0 ) - 0.5 );
  sum |= ( (uint32_t)part & 1023 ) << 20;
#if 0
  v = mdfmin( 1.0, mdfmax( -1.0, src[3] ) );
  if( v > 0.0 )
    part = (int)( ( v * 1.0 ) + 0.5 );
  else
    part = (int)( ( v * 2.0 ) - 0.5 );
  sum |= ( (uint32_t)part & 3 ) << 30;
#endif
  *(uint32_t *)dst = sum;
  return;
}


////


static void mdTriangleComputeQuadric( mdMesh *mesh, mdTriangle *tri, mathQuadric *q )
{
  mdf area, norm, vecta[3], vectb[3], plane[4];
  mdVertex *vertex0, *vertex1, *vertex2;

  vertex0 = &mesh->vertexlist[ tri->v[0] ];
  vertex1 = &mesh->vertexlist[ tri->v[1] ];
  vertex2 = &mesh->vertexlist[ tri->v[2] ];
  MD_VectorSubStore( vecta, vertex1->point, vertex0->point );
  MD_VectorSubStore( vectb, vertex2->point, vertex0->point );
  MD_VectorCrossProduct( plane, vectb, vecta );
  norm = mdfsqrt( MD_VectorDotProduct( plane, plane ) );
  if( norm )
  {
    area = 0.5 * norm;
    plane[0] *= 0.5;
    plane[1] *= 0.5;
    plane[2] *= 0.5;
    plane[3] = -MD_VectorDotProduct( plane, vertex0->point );
  }
  else
  {
    area = 0.0;
    plane[0] = 0.0;
    plane[1] = 0.0;
    plane[2] = 0.0;
    plane[3] = 0.0;
  }
#if DEBUG_VERBOSE_QUADRIC
  printf( "  Plane %f %f %f %f : Area %f\n", plane[0], plane[1], plane[2], plane[3], area );
#endif
  mathQuadricInit( q, plane[0], plane[1], plane[2], plane[3], area );
  return;
}


static mdf mdEdgeSolvePoint( mdVertex *vertex0, mdVertex *vertex1, mdf *point )
{
  mdf cost, bestcost;
  mdf midpoint[3];
  mdf *bestpoint;
  mathQuadric q;

  mathQuadricAddStoreQuadric( &q, &vertex0->quadric, &vertex1->quadric );
  midpoint[0] = 0.5 * ( vertex0->point[0] + vertex1->point[0] );
  midpoint[1] = 0.5 * ( vertex0->point[1] + vertex1->point[1] );
  midpoint[2] = 0.5 * ( vertex0->point[2] + vertex1->point[2] );
  bestcost = mathQuadricEvaluate( &q, midpoint );
#if DEBUG_VERBOSE_QUADRIC
  printf( "        MidCost %f %f %f : %f (%g)\n", midpoint[0], midpoint[1], midpoint[2], bestcost, bestcost );
#endif

  if( mathQuadricSolve( &q, point ) )
  {
    /* In _theory_, solving the quadric should always provide the optimal cost solution */
    /* In practice, rare floating point cases can screw things up, so we compare against the midpoint for safety */
    cost = mathQuadricEvaluate( &q, point );
#if DEBUG_VERBOSE_QUADRIC >= 2
    printf( "        Quadric Eval Check ; Quadric %.12f ; Mid %.12f ; Ratio %f\n", cost, bestcost, cost / bestcost );
#endif
    if( cost < bestcost )
      bestcost = cost;
    else
      MD_VectorCopy( point, midpoint );
  }
  else
  {
    bestpoint = midpoint;
    cost = mathQuadricEvaluate( &q, vertex0->point );
#if DEBUG_VERBOSE_QUADRIC
    printf( "        Vx0Cost %f %f %f : %f (%g)\n", vertex0->point[0], vertex0->point[1], vertex0->point[2], cost, cost );
#endif
    if( cost < bestcost )
    {
      bestcost = cost;
      bestpoint = vertex0->point;
    }
    cost = mathQuadricEvaluate( &q, vertex1->point );
#if DEBUG_VERBOSE_QUADRIC
    printf( "        Vx1Cost %f %f %f : %f (%g)\n", vertex1->point[0], vertex1->point[1], vertex1->point[2], cost, cost );
#endif
    if( cost < bestcost )
    {
      bestcost = cost;
      bestpoint = vertex1->point;
    }
#if DEBUG_VERBOSE_QUADRIC
    printf( "    Vector : %f %f %f\n", (double)point[0], (double)point[1], (double)point[2] );
#endif
    MD_VectorCopy( point, bestpoint );
  }

  return bestcost;
}


////


static void mdMeshAccumulateBoundary( mdVertex *vertex0, mdVertex *vertex1, mdVertex *vertex2, mdf boundaryweight )
{
  mdf normal[3], sideplane[4], vecta[3], vectb[3], norm, norminv, area;
  mathQuadric q;

  MD_VectorSubStore( vecta, vertex1->point, vertex0->point );
  MD_VectorSubStore( vectb, vertex2->point, vertex0->point );
  MD_VectorCrossProduct( normal, vectb, vecta );
  norm = mdfsqrt( normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2] );
  area = 0.5 * norm;
  norminv = 1.0 / norm;
  MD_VectorMulScalar( normal, norminv );
#if DEBUG_VERBOSE_BOUNDARY
  printf( "  Normal %f %f %f\n", normal[0], normal[1], normal[2] );
#endif
  MD_VectorCrossProduct( sideplane, vecta, normal );
  MD_VectorNormalize( sideplane );
  MD_VectorMulScalar( sideplane, area );
  sideplane[3] = -MD_VectorDotProduct( sideplane, vertex0->point );
#if DEBUG_VERBOSE_BOUNDARY
  printf( "  Boundary plane %f %f %f %f : Area %f\n", sideplane[0], sideplane[1], sideplane[2], sideplane[3], area );
#endif
  mathQuadricInit( &q, sideplane[0], sideplane[1], sideplane[2], sideplane[3], area );
  mathQuadricMul( &q, boundaryweight );
#if MD_CONFIG_ATOMIC_SUPPORT
  mmAtomicSpin32( &vertex0->atomicowner, -1, 0xffff );
  mathQuadricAddQuadric( &vertex0->quadric, &q );
  mmAtomicWrite32( &vertex0->atomicowner, -1 );
#else
  mtSpinLock( &vertex0->ownerspinlock );
  mathQuadricAddQuadric( &vertex0->quadric, &q );
  mtSpinUnlock( &vertex0->ownerspinlock );
#endif
#if MD_CONFIG_ATOMIC_SUPPORT
  mmAtomicSpin32( &vertex1->atomicowner, -1, 0xffff );
  mathQuadricAddQuadric( &vertex1->quadric, &q );
  mmAtomicWrite32( &vertex1->atomicowner, -1 );
#else
  mtSpinLock( &vertex1->ownerspinlock );
  mathQuadricAddQuadric( &vertex1->quadric, &q );
  mtSpinUnlock( &vertex1->ownerspinlock );
#endif
  return;
}


////


static void mdEdgeHashClearEntry( void *entry )
{
  mdEdge *edge;
  edge = entry;
  edge->v[0] = -1;
  return;
}

static int mdEdgeHashEntryValid( void *entry )
{
  mdEdge *edge;
  edge = entry;
  return ( edge->v[0] >= 0 ? 1 : 0 );
}

static uint32_t mdEdgeHashEntryKey( void *entry )
{
  uint32_t hashkey;
  mdEdge *edge;
  edge = entry;

#if MD_CONFIG_FAST_HASH
  hashkey  = edge->v[0];
  hashkey += hashkey << 10;
  hashkey ^= hashkey >> 6;
  hashkey += edge->v[1];
  hashkey += hashkey << 10;
  hashkey ^= hashkey >> 6;
  hashkey += hashkey << 3;
  hashkey ^= hashkey >> 11;
  hashkey += hashkey << 15;
#else
 #if MD_SIZEOF_MDI == 4
  #if CPUCONF_WORD_SIZE == 64
  {
    uint64_t *v = (uint64_t *)edge->v;
    hashkey = ccHash32Int64Inline( *v );
  }
  #else
  hashkey = ccHash32Data4Inline( (void *)edge->v );
  #endif
 #elif MD_SIZEOF_MDI == 8
  #if CPUCONF_WORD_SIZE == 64
  hashkey = ccHash32Array64( (uint64_t *)edge->v, 2 );
  #else
  hashkey = ccHash32Array32( (uint32_t *)edge->v, 4 );
  #endif
 #else
  hashkey = ccHash32Data( edge->v, 2*sizeof(mdi) );
 #endif
#endif

  return hashkey;
}

static int mdEdgeHashEntryCmp( void *entry, void *entryref )
{
  mdEdge *edge, *edgeref;
  edge = entry;
  edgeref = entryref;
  if( edge->v[0] == -1 )
    return MM_HASH_ENTRYCMP_INVALID;
  if( ( edge->v[0] == edgeref->v[0] ) && ( edge->v[1] == edgeref->v[1] ) )
    return MM_HASH_ENTRYCMP_FOUND;
  return MM_HASH_ENTRYCMP_SKIP;
}

static mmHashAccess mdEdgeHashEdge =
{
  .clearentry = mdEdgeHashClearEntry,
  .entryvalid = mdEdgeHashEntryValid,
  .entrykey = mdEdgeHashEntryKey,
  .entrycmp = mdEdgeHashEntryCmp
};

static int mdMeshHashInit( mdMesh *mesh, size_t trianglecount, mdf hashextrabits, uint32_t lockpageshift, size_t maxmemorysize )
{
  size_t edgecount, hashmemsize, meshmemsize, totalmemorysize;
  uint32_t hashbits, hashbitsmin, hashbitsmax;

  edgecount = trianglecount * 3;

  /* Hard minimum count of hash table bits */
  hashbitsmin = ccLog2Int64( edgecount ) + 1;
  hashbitsmax = hashbitsmin + 4;

  hashbits = (uint32_t)mdfround( mdflog2( (mdf)edgecount ) + hashextrabits );
  if( hashbits < hashbitsmin )
    hashbits = hashbitsmin;
  else if( hashbits > hashbitsmax )
    hashbits = hashbitsmax;

  if( hashbits < 12 )
    hashbits = 12;
  /* lockpageshift = 7; works great, 128 hash entries per lock page */
  if( lockpageshift < 3 )
    lockpageshift = 3;
  else if( lockpageshift > 16 )
    lockpageshift = 16;

  meshmemsize = ( mesh->tricount * mesh->trisize ) + ( mesh->vertexcount * sizeof(mdVertex) );
  for( ; ; hashbits-- )
  {
    if( hashbits < hashbitsmin )
      return 0;
    hashmemsize = mmHashRequiredSize( sizeof(mdEdge), hashbits, lockpageshift );
    totalmemorysize = hashmemsize + meshmemsize;

    /* Increase estimate of memory consumption by 25% to account for extra stuff not counted here */
    totalmemorysize += totalmemorysize >> 2;

#if DEBUG_VERBOSE_MEMORY
    printf( "  Hash bits : %d (%d)\n", hashbits, hashbitsmin );
    printf( "    Estimated Memory Requirements : %lld bytes (%lld MB)\n", (long long)totalmemorysize, (long long)totalmemorysize >> 20 );
    printf( "    Memory Hard Limit : %lld bytes (%lld MB)\n", (long long)maxmemorysize, (long long)maxmemorysize >> 20 );
#endif

    if( ( maxmemorysize ) && ( totalmemorysize > maxmemorysize ) )
      continue;
    mesh->edgehashtable = malloc( hashmemsize );
    if( mesh->edgehashtable )
      break;
  }

  mmHashInit( mesh->edgehashtable, &mdEdgeHashEdge, sizeof(mdEdge), hashbits, lockpageshift, MM_HASH_FLAGS_NO_COUNT );

  return 1;
}

static void mdMeshHashEnd( mdMesh *mesh )
{
  free( mesh->edgehashtable );
  return;
}


////


typedef struct CPU_ALIGN64
{
  void **opbuffer;
  int opcount;
  int opalloc;
#if MD_CONFIG_ATOMIC_SUPPORT
  mmAtomic32 atomlock;
#else
  mtSpin spinlock;
#endif
} mdUpdateBuffer;


typedef struct
{
#if MD_CONFIG_ATOMIC_SUPPORT
  mmAtomic32 flags;
#else
  int flags;
  mtSpin spinlock;
#endif
  mdUpdateBuffer *updatebuffer;
  mdi v0, v1;
#if CPU_SSE_SUPPORT
  mdf CPU_ALIGN16 collapsepoint[4];
#else
  mdf collapsepoint[3];
#endif
  mdf collapsecost;
  mdf value;
  mdf penalty;
  mmListNode list;
} mdOp;

#define MD_OP_FLAGS_DETACHED (0x1)
#define MD_OP_FLAGS_DELETION_PENDING (0x2)
#define MD_OP_FLAGS_UPDATE_QUEUED (0x4)
#define MD_OP_FLAGS_UPDATE_NEEDED (0x8)
#define MD_OP_FLAGS_DELETED (0x10)


/* If threadcount exceeds this number, updatebuffers will be shared by nearby cores */
#define MD_THREAD_UPDATE_BUFFER_COUNTMAX (8)

typedef struct CPU_ALIGN64
{
  int threadid;

  /* Memory block for ops */
  mmBlockHead opblock;

  /* Hierarchical bucket sort of ops */
  void *binsort;

  /* List of ops flagged by other threads in need of update */
  mdUpdateBuffer updatebuffer[MD_THREAD_UPDATE_BUFFER_COUNTMAX];

  /* Per-thread status trackers */
  volatile long statusbuildtricount;
  volatile long statusbuildrefcount;
  volatile long statuspopulatecount;
  volatile long statusdeletioncount;
  volatile long statuscollisioncount;

} mdThreadData;


static void mdUpdateBufferInit( mdUpdateBuffer *updatebuffer, int opalloc )
{
  updatebuffer->opbuffer = malloc( opalloc * sizeof(mdOp *) );
  updatebuffer->opcount = 0;
  updatebuffer->opalloc = opalloc;
#if MD_CONFIG_ATOMIC_SUPPORT
  mmAtomicWrite32( &updatebuffer->atomlock, 0x0 );
#else
  mtSpinInit( &updatebuffer->spinlock );
#endif
  return;
}

static void mdUpdateBufferEnd( mdUpdateBuffer *updatebuffer )
{
#ifndef MD_CONFIG_ATOMIC_SUPPORT
  mtSpinDestroy( &updatebuffer->spinlock );
#endif
  free( updatebuffer->opbuffer );
  return;
}

static void mdUpdateBufferAdd( mdUpdateBuffer *updatebuffer, mdOp *op, int orflags )
{
  int32_t flags;
#if MD_CONFIG_ATOMIC_SUPPORT
  for( ; ; )
  {
    flags = mmAtomicRead32( &op->flags );
    if( flags & MD_OP_FLAGS_UPDATE_QUEUED )
    {
      if( !( flags & MD_OP_FLAGS_UPDATE_NEEDED ) || ( orflags ) )
        mmAtomicOr32( &op->flags, orflags | MD_OP_FLAGS_UPDATE_NEEDED );
      return;
    }
    if( mmAtomicCmpReplace32( &op->flags, flags, flags | orflags | MD_OP_FLAGS_UPDATE_QUEUED | MD_OP_FLAGS_UPDATE_NEEDED ) )
      break;
  }
  /* TODO: Avoid spin lock, use atomic increment for write offset? Careful with this realloc, pointer could become invalid */
  mmAtomicSpin32( &updatebuffer->atomlock, 0x0, 0x1 );
  if( updatebuffer->opcount >= updatebuffer->opalloc )
  {
    updatebuffer->opalloc <<= 1;
    updatebuffer->opbuffer = realloc( updatebuffer->opbuffer, updatebuffer->opalloc * sizeof(mdOp *) );
  }
  updatebuffer->opbuffer[ updatebuffer->opcount++ ] = op;
  mmAtomicWrite32( &updatebuffer->atomlock, 0x0 );
#else
  mtSpinLock( &op->spinlock );
  op->flags |= orflags;
  flags = op->flags;
  if( flags & MD_OP_FLAGS_UPDATE_QUEUED )
  {
    if( !( flags & MD_OP_FLAGS_UPDATE_NEEDED ) )
      op->flags |= MD_OP_FLAGS_UPDATE_NEEDED;
    mtSpinUnlock( &op->spinlock );
    return;
  }
  op->flags |= MD_OP_FLAGS_UPDATE_QUEUED | MD_OP_FLAGS_UPDATE_NEEDED;
  mtSpinUnlock( &op->spinlock );
  mtSpinLock( &updatebuffer->spinlock );
  if( updatebuffer->opcount >= updatebuffer->opalloc )
  {
    updatebuffer->opalloc <<= 1;
    updatebuffer->opbuffer = realloc( updatebuffer->opbuffer, updatebuffer->opalloc * sizeof(mdOp *) );
  }
  updatebuffer->opbuffer[ updatebuffer->opcount++ ] = op;
  mtSpinUnlock( &updatebuffer->spinlock );
#endif
  return;
}



////



#define MD_COMPACTNESS_NORMALIZATION_FACTOR (0.5*4.0*1.732050808)

static mdf mdEdgeCollapsePenaltyTriangle( mdf *newpoint, mdf *oldpoint, mdf *leftpoint, mdf *rightpoint, int *denyflag, mdf compactnesstarget )
{
  mdf penalty, compactness, oldcompactness, newcompactness, vecta2, norm;
  mdf vecta[3], oldvectb[3], oldvectc[3], newvectb[3], newvectc[3], oldnormal[3], newnormal[3];
  /* Normal of old triangle */
  MD_VectorSubStore( vecta, rightpoint, leftpoint );
  MD_VectorSubStore( oldvectb, oldpoint, leftpoint );
  MD_VectorCrossProduct( oldnormal, vecta, oldvectb );
  /* Normal of new triangle */
  MD_VectorSubStore( newvectb, newpoint, leftpoint );
  MD_VectorCrossProduct( newnormal, vecta, newvectb );
  /* Detect normal inversion */
  if( MD_VectorDotProduct( oldnormal, newnormal ) < 0.0 )
  {
    *denyflag = 1;
    return 0.0;
  }
  /* Penalize long thin triangles */
  penalty = 0.0;
  vecta2 = MD_VectorDotProduct( vecta, vecta );
  MD_VectorSubStore( newvectc, newpoint, rightpoint );
  newcompactness = MD_COMPACTNESS_NORMALIZATION_FACTOR * mdfsqrt( MD_VectorDotProduct( newnormal, newnormal ) );
  norm = vecta2 + MD_VectorDotProduct( newvectb, newvectb ) + MD_VectorDotProduct( newvectc, newvectc );
  if( newcompactness < ( compactnesstarget * norm ) )
  {
    newcompactness /= norm;
    MD_VectorSubStore( oldvectc, oldpoint, rightpoint );
    oldcompactness = ( MD_COMPACTNESS_NORMALIZATION_FACTOR * mdfsqrt( MD_VectorDotProduct( oldnormal, oldnormal ) ) ) / ( vecta2 + MD_VectorDotProduct( oldvectb, oldvectb ) + MD_VectorDotProduct( oldvectc, oldvectc ) );
    compactness = fmin( compactnesstarget, oldcompactness ) - newcompactness;
    if( compactness > 0.0 )
      penalty = compactness;
  }
  return penalty;
}

#if CPU_SSE4_1_SUPPORT

 #if !MD_CONF_DOUBLE_PRECISION

static float mdEdgeCollapsePenaltyTriangleSSE4p1f( float *newpoint, float *oldpoint, float *leftpoint, float *rightpoint, int *denyflag, float compactnesstarget )
{
  float penalty, compactness, oldcompactness, newcompactness;
  static float zero = 0.0;
  __m128 left, vecta, oldvectb, oldvectc, newvectb, newvectc, oldnormal, newnormal;
  /* Normal of old triangle */
  left = _mm_load_ps( leftpoint );
  vecta = _mm_sub_ps( _mm_load_ps( rightpoint ), left );
  oldvectb = _mm_sub_ps( _mm_load_ps( oldpoint ), left );
  oldnormal = _mm_sub_ps(
    _mm_mul_ps( _mm_shuffle_ps( vecta, vecta, _MM_SHUFFLE(3,0,2,1) ), _mm_shuffle_ps( oldvectb, oldvectb, _MM_SHUFFLE(3,1,0,2) ) ),
    _mm_mul_ps( _mm_shuffle_ps( vecta, vecta, _MM_SHUFFLE(3,1,0,2) ), _mm_shuffle_ps( oldvectb, oldvectb, _MM_SHUFFLE(3,0,2,1) ) )
  );
  /* Normal of new triangle */
  newvectb = _mm_sub_ps( _mm_load_ps( newpoint ), left );
  newnormal = _mm_sub_ps(
    _mm_mul_ps( _mm_shuffle_ps( vecta, vecta, _MM_SHUFFLE(3,0,2,1) ), _mm_shuffle_ps( newvectb, newvectb, _MM_SHUFFLE(3,1,0,2) ) ),
    _mm_mul_ps( _mm_shuffle_ps( vecta, vecta, _MM_SHUFFLE(3,1,0,2) ), _mm_shuffle_ps( newvectb, newvectb, _MM_SHUFFLE(3,0,2,1) ) )
  );
  /* Detect normal inversion */
  if( _mm_comilt_ss( _mm_dp_ps( oldnormal, newnormal, 0x1 | 0x70 ), _mm_load_ss( &zero ) ) )
  {
    *denyflag = 1;
    return 0.0;
  }
  /* Penalize long thin triangles */
  penalty = 0.0;
  vecta = _mm_dp_ps( vecta, vecta, 0x1 | 0x70 );
  newvectc = _mm_sub_ps( _mm_load_ps( newpoint ), _mm_load_ps( rightpoint ) );
  newnormal = _mm_dp_ps( newnormal, newnormal, 0x1 | 0x70 );
  newvectb = _mm_dp_ps( newvectb, newvectb, 0x1 | 0x70 );
  newvectc = _mm_dp_ps( newvectc, newvectc, 0x1 | 0x70 );
#ifdef MD_CONFIG_SSE_APPROX
  newcompactness = _mm_cvtss_f32( _mm_mul_ss( _mm_set_ss( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_rcp_ss( _mm_mul_ss( _mm_rsqrt_ss( newnormal ), _mm_add_ss( _mm_add_ss( vecta, newvectb ), newvectc ) ) ) ) );
#else
  newcompactness = _mm_cvtss_f32( _mm_div_ss( _mm_mul_ss( _mm_set_ss( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_sqrt_ss( newnormal ) ), _mm_add_ss( _mm_add_ss( vecta, newvectb ), newvectc ) ) );
#endif
  if( newcompactness < compactnesstarget )
  {
    oldvectc = _mm_sub_ps( _mm_load_ps( oldpoint ), _mm_load_ps( rightpoint ) );
    oldnormal = _mm_dp_ps( oldnormal, oldnormal, 0x1 | 0x70 );
    oldvectb = _mm_dp_ps( oldvectb, oldvectb, 0x1 | 0x70 );
    oldvectc = _mm_dp_ps( oldvectc, oldvectc, 0x1 | 0x70 );
#ifdef MD_CONFIG_SSE_APPROX
    oldcompactness = _mm_cvtss_f32( _mm_mul_ss( _mm_set_ss( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_rcp_ss( _mm_mul_ss( _mm_rsqrt_ss( oldnormal ), _mm_add_ss( _mm_add_ss( vecta, oldvectb ), oldvectc ) ) ) ) );
#else
    oldcompactness = _mm_cvtss_f32( _mm_div_ss( _mm_mul_ss( _mm_set_ss( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_sqrt_ss( oldnormal ) ), _mm_add_ss( _mm_add_ss( vecta, oldvectb ), oldvectc ) ) );
#endif
    compactness = fmin( compactnesstarget, oldcompactness ) - newcompactness;
    if( compactness > 0.0 )
      penalty = compactness;
  }
  return penalty;
}

 #else

static double mdEdgeCollapsePenaltyTriangleSSE4p1d( double *newpoint, double *oldpoint, double *leftpoint, double *rightpoint, int *denyflag, double compactnesstarget )
{
  __m128d vecta0, vecta1, oldvectb0, oldvectb1, oldvectc0, oldvectc1, newvectb0, newvectb1, newvectc0, newvectc1;
  __m128d oldnormal0, oldnormal1, newnormal0, newnormal1;
  __m128d dotprsum;
  __m128d left0, left1;
  double newcompactness, oldcompactness, compactness, penalty, norm;
  static double zero = 0.0;
  /* Normal of old triangle */
  left0 = _mm_load_pd( leftpoint+0 );
  left1 = _mm_load_sd( leftpoint+2 );
  vecta0 = _mm_sub_pd( _mm_load_pd( rightpoint+0 ), left0 );
  vecta1 = _mm_sub_pd( _mm_load_pd( rightpoint+2 ), left1 );
  oldvectb0 = _mm_sub_pd( _mm_load_pd( oldpoint+0 ), left0 );
  oldvectb1 = _mm_sub_pd( _mm_load_pd( oldpoint+2 ), left1 );
  oldnormal0 = _mm_sub_pd(
    _mm_mul_pd( _mm_shuffle_pd( vecta0, vecta1, _MM_SHUFFLE2(0,1) ), _mm_unpacklo_pd( oldvectb1, oldvectb0 ) ),
    _mm_mul_pd( _mm_unpacklo_pd( vecta1, vecta0 ), _mm_shuffle_pd( oldvectb0, oldvectb1, _MM_SHUFFLE2(0,1) ) )
  );
  oldnormal1 = _mm_sub_sd(
    _mm_mul_sd( vecta0, _mm_unpackhi_pd( oldvectb0, oldvectb0 ) ),
    _mm_mul_sd( _mm_unpackhi_pd( vecta0, vecta0 ), oldvectb0 )
  );
  /* Normal of new triangle */
  newvectb0 = _mm_sub_pd( _mm_load_pd( newpoint+0 ), left0 );
  newvectb1 = _mm_sub_pd( _mm_load_pd( newpoint+2 ), left1 );
  newnormal0 = _mm_sub_pd(
    _mm_mul_pd( _mm_shuffle_pd( vecta0, vecta1, _MM_SHUFFLE2(0,1) ), _mm_unpacklo_pd( newvectb1, newvectb0 ) ),
    _mm_mul_pd( _mm_unpacklo_pd( vecta1, vecta0 ), _mm_shuffle_pd( newvectb0, newvectb1, _MM_SHUFFLE2(0,1) ) )
  );
  newnormal1 = _mm_sub_sd(
    _mm_mul_sd( vecta0, _mm_unpackhi_pd( newvectb0, newvectb0 ) ),
    _mm_mul_sd( _mm_unpackhi_pd( vecta0, vecta0 ), newvectb0 )
  );
  /* Detect normal inversion */
  dotprsum = _mm_add_sd( _mm_dp_pd( oldnormal0, oldnormal0, 0x1 | 0x30 ), _mm_mul_sd( oldnormal1, newnormal1 ) );
  if( _mm_comilt_sd( dotprsum, _mm_load_sd( &zero ) ) )
  {
    *denyflag = 1;
    return 0.0;
  }
  /* Penalize long thin triangles */
  penalty = 0.0;
  vecta0 = _mm_add_sd( _mm_dp_pd( vecta0, vecta0, 0x1 | 0x30 ), _mm_mul_sd( vecta1, vecta1 ) );
  newvectc0 = _mm_sub_pd( _mm_load_pd( newpoint+0 ), _mm_load_pd( rightpoint+0 ) );
  newvectc1 = _mm_sub_sd( _mm_load_sd( newpoint+2 ), _mm_load_sd( rightpoint+2 ) );
  newnormal0 = _mm_add_sd( _mm_dp_pd( newnormal0, newnormal0, 0x1 | 0x30 ), _mm_mul_sd( newnormal1, newnormal1 ) );
  newvectb0 = _mm_add_sd( _mm_dp_pd( newvectb0, newvectb0, 0x1 | 0x30 ), _mm_mul_sd( newvectb1, newvectb1 ) );
  newvectc0 = _mm_add_sd( _mm_dp_pd( newvectc0, newvectc0, 0x1 | 0x30 ), _mm_mul_sd( newvectc1, newvectc1 ) );
  norm = _mm_cvtsd_f64( _mm_add_sd( vecta0, _mm_add_sd( newvectb0, newvectc0 ) ) );
  newcompactness = _mm_cvtsd_f64( _mm_mul_sd( _mm_set_sd( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_sqrt_sd( newnormal0, newnormal0 ) ) );
  if( newcompactness < ( compactnesstarget * norm ) )
  {
    newcompactness /= norm;
    oldvectc0 = _mm_sub_pd( _mm_load_pd( oldpoint+0 ), _mm_load_pd( rightpoint+0 ) );
    oldvectc1 = _mm_sub_sd( _mm_load_sd( oldpoint+2 ), _mm_load_sd( rightpoint+2 ) );
    oldnormal0 = _mm_add_sd( _mm_dp_pd( oldnormal0, oldnormal0, 0x1 | 0x30 ), _mm_mul_sd( oldnormal1, oldnormal1 ) );
    oldvectb0 = _mm_add_sd( _mm_dp_pd( oldvectb0, oldvectb0, 0x1 | 0x30 ), _mm_mul_sd( oldvectb1, oldvectb1 ) );
    oldvectc0 = _mm_add_sd( _mm_dp_pd( oldvectc0, oldvectc0, 0x1 | 0x30 ), _mm_mul_sd( oldvectc1, oldvectc1 ) );
    oldcompactness = _mm_cvtsd_f64( _mm_div_sd( _mm_mul_sd( _mm_set_sd( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_sqrt_sd( oldnormal0, oldnormal0 ) ), _mm_add_sd( vecta0, _mm_add_sd( oldvectb0, oldvectc0 ) ) ) );
    compactness = fmin( compactnesstarget, oldcompactness ) - newcompactness;
    if( compactness > 0.0 )
      penalty = compactness;
  }
  return penalty;
}

 #endif

#elif CPU_SSE3_SUPPORT

 #if !MD_CONF_DOUBLE_PRECISION

static float mdEdgeCollapsePenaltyTriangleSSE3f( float *newpoint, float *oldpoint, float *leftpoint, float *rightpoint, int *denyflag, float compactnesstarget )
{
  float penalty, compactness, oldcompactness, newcompactness;
  static float zero = 0.0;
  __m128 left, vecta, oldvectb, oldvectc, newvectb, newvectc, oldnormal, newnormal, dotproduct;
  /* Normal of old triangle */
  left = _mm_load_ps( leftpoint );
  vecta = _mm_sub_ps( _mm_load_ps( rightpoint ), left );
  oldvectb = _mm_sub_ps( _mm_load_ps( oldpoint ), left );
  oldnormal = _mm_sub_ps(
    _mm_mul_ps( _mm_shuffle_ps( vecta, vecta, _MM_SHUFFLE(3,0,2,1) ), _mm_shuffle_ps( oldvectb, oldvectb, _MM_SHUFFLE(3,1,0,2) ) ),
    _mm_mul_ps( _mm_shuffle_ps( vecta, vecta, _MM_SHUFFLE(3,1,0,2) ), _mm_shuffle_ps( oldvectb, oldvectb, _MM_SHUFFLE(3,0,2,1) ) )
  );
  /* Normal of new triangle */
  newvectb = _mm_sub_ps( _mm_load_ps( newpoint ), left );
  newnormal = _mm_sub_ps(
    _mm_mul_ps( _mm_shuffle_ps( vecta, vecta, _MM_SHUFFLE(3,0,2,1) ), _mm_shuffle_ps( newvectb, newvectb, _MM_SHUFFLE(3,1,0,2) ) ),
    _mm_mul_ps( _mm_shuffle_ps( vecta, vecta, _MM_SHUFFLE(3,1,0,2) ), _mm_shuffle_ps( newvectb, newvectb, _MM_SHUFFLE(3,0,2,1) ) )
  );
  /* Detect normal inversion */
  dotproduct = _mm_mul_ps( oldnormal, newnormal );
  dotproduct = _mm_hadd_ps( dotproduct, dotproduct );
  dotproduct = _mm_hadd_ps( dotproduct, dotproduct );
  if( _mm_comilt_ss( dotproduct, _mm_load_ss( &zero ) ) )
  {
    *denyflag = 1;
    return 0.0;
  }
  /* Penalize long thin triangles */
  penalty = 0.0;
  vecta = _mm_mul_ps( vecta, vecta );
  vecta = _mm_hadd_ps( vecta, vecta );
  vecta = _mm_hadd_ps( vecta, vecta );
  newvectc = _mm_sub_ps( _mm_load_ps( newpoint ), _mm_load_ps( rightpoint ) );
  newnormal = _mm_mul_ps( newnormal, newnormal );
  newnormal = _mm_hadd_ps( newnormal, newnormal );
  newnormal = _mm_hadd_ps( newnormal, newnormal );
  newvectb = _mm_mul_ps( newvectb, newvectb );
  newvectb = _mm_hadd_ps( newvectb, newvectb );
  newvectb = _mm_hadd_ps( newvectb, newvectb );
  newvectc = _mm_mul_ps( newvectc, newvectc );
  newvectc = _mm_hadd_ps( newvectc, newvectc );
  newvectc = _mm_hadd_ps( newvectc, newvectc );
#ifdef MD_CONFIG_SSE_APPROX
  newcompactness = _mm_cvtss_f32( _mm_mul_ss( _mm_set_ss( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_rcp_ss( _mm_mul_ss( _mm_rsqrt_ss( newnormal ), _mm_add_ss( _mm_add_ss( vecta, newvectb ), newvectc ) ) ) ) );
#else
  newcompactness = _mm_cvtss_f32( _mm_div_ss( _mm_mul_ss( _mm_set_ss( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_sqrt_ss( newnormal ) ), _mm_add_ss( _mm_add_ss( vecta, newvectb ), newvectc ) ) );
#endif
  if( newcompactness < compactnesstarget )
  {
    oldvectc = _mm_sub_ps( _mm_load_ps( oldpoint ), _mm_load_ps( rightpoint ) );
    oldnormal = _mm_mul_ps( oldnormal, oldnormal );
    oldnormal = _mm_hadd_ps( oldnormal, oldnormal );
    oldnormal = _mm_hadd_ps( oldnormal, oldnormal );
    oldvectb = _mm_mul_ps( oldvectb, oldvectb );
    oldvectb = _mm_hadd_ps( oldvectb, oldvectb );
    oldvectb = _mm_hadd_ps( oldvectb, oldvectb );
    oldvectc = _mm_mul_ps( oldvectc, oldvectc );
    oldvectc = _mm_hadd_ps( oldvectc, oldvectc );
    oldvectc = _mm_hadd_ps( oldvectc, oldvectc );
#ifdef MD_CONFIG_SSE_APPROX
    oldcompactness = _mm_cvtss_f32( _mm_mul_ss( _mm_set_ss( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_rcp_ss( _mm_mul_ss( _mm_rsqrt_ss( oldnormal ), _mm_add_ss( _mm_add_ss( vecta, oldvectb ), oldvectc ) ) ) ) );
#else
    oldcompactness = _mm_cvtss_f32( _mm_div_ss( _mm_mul_ss( _mm_set_ss( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_sqrt_ss( oldnormal ) ), _mm_add_ss( _mm_add_ss( vecta, oldvectb ), oldvectc ) ) );
#endif
    compactness = fmin( compactnesstarget, oldcompactness ) - newcompactness;
    if( compactness > 0.0 )
      penalty = compactness;
  }
  return penalty;
}

 #else

static double mdEdgeCollapsePenaltyTriangleSSE3d( double *newpoint, double *oldpoint, double *leftpoint, double *rightpoint, int *denyflag, double compactnesstarget )
{
  __m128d vecta0, vecta1, oldvectb0, oldvectb1, oldvectc0, oldvectc1, newvectb0, newvectb1, newvectc0, newvectc1;
  __m128d oldnormal0, oldnormal1, newnormal0, newnormal1;
  __m128d dotprsum;
  __m128d left0, left1;
  double newcompactness, oldcompactness, compactness, penalty, norm;
  static double zero = 0.0;
  /* Normal of old triangle */
  left0 = _mm_load_pd( leftpoint+0 );
  left1 = _mm_load_sd( leftpoint+2 );
  vecta0 = _mm_sub_pd( _mm_load_pd( rightpoint+0 ), left0 );
  vecta1 = _mm_sub_pd( _mm_load_pd( rightpoint+2 ), left1 );
  oldvectb0 = _mm_sub_pd( _mm_load_pd( oldpoint+0 ), left0 );
  oldvectb1 = _mm_sub_pd( _mm_load_pd( oldpoint+2 ), left1 );
  oldnormal0 = _mm_sub_pd(
    _mm_mul_pd( _mm_shuffle_pd( vecta0, vecta1, _MM_SHUFFLE2(0,1) ), _mm_unpacklo_pd( oldvectb1, oldvectb0 ) ),
    _mm_mul_pd( _mm_unpacklo_pd( vecta1, vecta0 ), _mm_shuffle_pd( oldvectb0, oldvectb1, _MM_SHUFFLE2(0,1) ) )
  );
  oldnormal1 = _mm_sub_sd(
    _mm_mul_sd( vecta0, _mm_unpackhi_pd( oldvectb0, oldvectb0 ) ),
    _mm_mul_sd( _mm_unpackhi_pd( vecta0, vecta0 ), oldvectb0 )
  );
  /* Normal of new triangle */
  newvectb0 = _mm_sub_pd( _mm_load_pd( newpoint+0 ), left0 );
  newvectb1 = _mm_sub_pd( _mm_load_pd( newpoint+2 ), left1 );
  newnormal0 = _mm_sub_pd(
    _mm_mul_pd( _mm_shuffle_pd( vecta0, vecta1, _MM_SHUFFLE2(0,1) ), _mm_unpacklo_pd( newvectb1, newvectb0 ) ),
    _mm_mul_pd( _mm_unpacklo_pd( vecta1, vecta0 ), _mm_shuffle_pd( newvectb0, newvectb1, _MM_SHUFFLE2(0,1) ) )
  );
  newnormal1 = _mm_sub_sd(
    _mm_mul_sd( vecta0, _mm_unpackhi_pd( newvectb0, newvectb0 ) ),
    _mm_mul_sd( _mm_unpackhi_pd( vecta0, vecta0 ), newvectb0 )
  );
  /* Detect normal inversion */
  dotprsum = _mm_mul_pd( oldnormal0, newnormal0 );
  dotprsum = _mm_add_sd( _mm_hadd_pd( dotprsum, dotprsum ), _mm_mul_sd( oldnormal1, newnormal1 ) );
  if( _mm_comilt_sd( dotprsum, _mm_load_sd( &zero ) ) )
  {
    *denyflag = 1;
    return 0.0;
  }
  /* Penalize long thin triangles */
  penalty = 0.0;
  vecta0 = _mm_mul_pd( vecta0, vecta0 );
  vecta0 = _mm_add_sd( _mm_hadd_pd( vecta0, vecta0 ), _mm_mul_sd( vecta1, vecta1 ) );
  newvectc0 = _mm_sub_pd( _mm_load_pd( newpoint+0 ), _mm_load_pd( rightpoint+0 ) );
  newvectc1 = _mm_sub_sd( _mm_load_sd( newpoint+2 ), _mm_load_sd( rightpoint+2 ) );
  newnormal0 = _mm_mul_pd( newnormal0, newnormal0 );
  newnormal0 = _mm_add_sd( _mm_hadd_pd( newnormal0, newnormal0 ), _mm_mul_sd( newnormal1, newnormal1 ) );
  newvectb0 = _mm_mul_pd( newvectb0, newvectb0 );
  newvectb0 = _mm_add_sd( _mm_hadd_pd( newvectb0, newvectb0 ), _mm_mul_sd( newvectb1, newvectb1 ) );
  newvectc0 = _mm_mul_pd( newvectc0, newvectc0 );
  newvectc0 = _mm_add_sd( _mm_hadd_pd( newvectc0, newvectc0 ), _mm_mul_sd( newvectc1, newvectc1 ) );
  norm = _mm_cvtsd_f64( _mm_add_sd( vecta0, _mm_add_sd( newvectb0, newvectc0 ) ) );
  newcompactness = _mm_cvtsd_f64( _mm_mul_sd( _mm_set_sd( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_sqrt_sd( newnormal0, newnormal0 ) ) );
  if( newcompactness < ( compactnesstarget * norm ) )
  {
    newcompactness /= norm;
    oldvectc0 = _mm_sub_pd( _mm_load_pd( oldpoint+0 ), _mm_load_pd( rightpoint+0 ) );
    oldvectc1 = _mm_sub_sd( _mm_load_sd( oldpoint+2 ), _mm_load_sd( rightpoint+2 ) );
    oldnormal0 = _mm_mul_pd( oldnormal0, oldnormal0 );
    oldnormal0 = _mm_add_sd( _mm_hadd_pd( oldnormal0, oldnormal0 ), _mm_mul_sd( oldnormal1, oldnormal1 ) );
    oldvectb0 = _mm_mul_pd( oldvectb0, oldvectb0 );
    oldvectb0 = _mm_add_sd( _mm_hadd_pd( oldvectb0, oldvectb0 ), _mm_mul_sd( oldvectb1, oldvectb1 ) );
    oldvectc0 = _mm_mul_pd( oldvectc0, oldvectc0 );
    oldvectc0 = _mm_add_sd( _mm_hadd_pd( oldvectc0, oldvectc0 ), _mm_mul_sd( oldvectc1, oldvectc1 ) );
    oldcompactness = _mm_cvtsd_f64( _mm_div_sd( _mm_mul_sd( _mm_set_sd( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_sqrt_sd( oldnormal0, oldnormal0 ) ), _mm_add_sd( vecta0, _mm_add_sd( oldvectb0, oldvectc0 ) ) ) );
    compactness = fmin( compactnesstarget, oldcompactness ) - newcompactness;
    if( compactness > 0.0 )
      penalty = compactness;
  }

  return penalty;
}

 #endif

#elif CPU_SSE2_SUPPORT

 #if !MD_CONF_DOUBLE_PRECISION

static float mdEdgeCollapsePenaltyTriangleSSE2f( float *newpoint, float *oldpoint, float *leftpoint, float *rightpoint, int *denyflag, float compactnesstarget )
{
  float penalty, compactness, oldcompactness, newcompactness, vectadp, norm;
  __m128 left;
  m128f vecta, oldvectb, oldvectc, newvectb, newvectc, oldnormal, newnormal;
  /* Normal of old triangle */
  left = _mm_load_ps( leftpoint );
  vecta.v = _mm_sub_ps( _mm_load_ps( rightpoint ), left );
  oldvectb.v = _mm_sub_ps( _mm_load_ps( oldpoint ), left );
  oldnormal.v = _mm_sub_ps(
    _mm_mul_ps( _mm_shuffle_ps( vecta.v, vecta.v, _MM_SHUFFLE(3,0,2,1) ), _mm_shuffle_ps( oldvectb.v, oldvectb.v, _MM_SHUFFLE(3,1,0,2) ) ),
    _mm_mul_ps( _mm_shuffle_ps( vecta.v, vecta.v, _MM_SHUFFLE(3,1,0,2) ), _mm_shuffle_ps( oldvectb.v, oldvectb.v, _MM_SHUFFLE(3,0,2,1) ) )
  );
  /* Normal of new triangle */
  newvectb.v = _mm_sub_ps( _mm_load_ps( newpoint ), left );
  newnormal.v = _mm_sub_ps(
    _mm_mul_ps( _mm_shuffle_ps( vecta.v, vecta.v, _MM_SHUFFLE(3,0,2,1) ), _mm_shuffle_ps( newvectb.v, newvectb.v, _MM_SHUFFLE(3,1,0,2) ) ),
    _mm_mul_ps( _mm_shuffle_ps( vecta.v, vecta.v, _MM_SHUFFLE(3,1,0,2) ), _mm_shuffle_ps( newvectb.v, newvectb.v, _MM_SHUFFLE(3,0,2,1) ) )
  );
  /* Detect normal inversion */
  if( MD_VectorDotProduct( oldnormal.f, newnormal.f ) < 0.0 )
  {
    *denyflag = 1;
    return 0.0;
  }
  /* Penalize long thin triangles */
  penalty = 0.0;
  vectadp = MD_VectorDotProduct( vecta.f, vecta.f );
  newvectc.v = _mm_sub_ps( _mm_load_ps( newpoint ), _mm_load_ps( rightpoint ) );
#ifdef MD_CONFIG_SSE_APPROX
  norm = vectadp + MD_VectorDotProduct( newvectb.f, newvectb.f ) + MD_VectorDotProduct( newvectc.f, newvectc.f );
  newcompactness = _mm_cvtss_f32( _mm_mul_ss( _mm_set_ss( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_rcp_ss( _mm_mul_ss( _mm_rsqrt_ss( _mm_set_ss( MD_VectorDotProduct( newnormal.f, newnormal.f ) ) ), _mm_set_ss( norm ) ) ) ) );
  if( newcompactness < compactnesstarget )
  {
#else
  newcompactness = MD_COMPACTNESS_NORMALIZATION_FACTOR * sqrtf( MD_VectorDotProduct( newnormal.f, newnormal.f ) );
  norm = vectadp + MD_VectorDotProduct( newvectb.f, newvectb.f ) + MD_VectorDotProduct( newvectc.f, newvectc.f );
  if( newcompactness < ( compactnesstarget * norm ) )
  {
    newcompactness /= norm;
#endif
    oldvectc.v = _mm_sub_ps( _mm_load_ps( oldpoint ), _mm_load_ps( rightpoint ) );
#ifdef MD_CONFIG_SSE_APPROX
    norm = vectadp + MD_VectorDotProduct( oldvectb.f, oldvectb.f ) + MD_VectorDotProduct( oldvectc.f, oldvectc.f );
    oldcompactness = _mm_cvtss_f32( _mm_mul_ss( _mm_set_ss( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_rcp_ss( _mm_mul_ss( _mm_rsqrt_ss( _mm_set_ss( MD_VectorDotProduct( oldnormal.f, oldnormal.f ) ) ), _mm_set_ss( norm ) ) ) ) );
#else
    oldcompactness = ( MD_COMPACTNESS_NORMALIZATION_FACTOR * sqrtf( MD_VectorDotProduct( oldnormal.f, oldnormal.f ) ) ) / ( vectadp + MD_VectorDotProduct( oldvectb.f, oldvectb.f ) + MD_VectorDotProduct( oldvectc.f, oldvectc.f ) );
#endif
    compactness = fmin( compactnesstarget, oldcompactness ) - newcompactness;
    if( compactness > 0.0 )
      penalty = compactness;
  }
  return penalty;
}

 #else

static double mdEdgeCollapsePenaltyTriangleSSE2d( double *newpoint, double *oldpoint, double *leftpoint, double *rightpoint, int *denyflag, double compactnesstarget )
{
  __m128d vecta0, vecta1, oldvectb0, oldvectb1, oldvectc0, oldvectc1, newvectb0, newvectb1, newvectc0, newvectc1;
  __m128d oldnormal0, oldnormal1, newnormal0, newnormal1;
  __m128d dotprsum;
  __m128d left0, left1;
  double newcompactness, oldcompactness, compactness, penalty, norm;
  static double zero = 0.0;
  /* Normal of old triangle */
  left0 = _mm_load_pd( leftpoint+0 );
  left1 = _mm_load_sd( leftpoint+2 );
  vecta0 = _mm_sub_pd( _mm_load_pd( rightpoint+0 ), left0 );
  vecta1 = _mm_sub_pd( _mm_load_pd( rightpoint+2 ), left1 );
  oldvectb0 = _mm_sub_pd( _mm_load_pd( oldpoint+0 ), left0 );
  oldvectb1 = _mm_sub_pd( _mm_load_pd( oldpoint+2 ), left1 );
  oldnormal0 = _mm_sub_pd(
    _mm_mul_pd( _mm_shuffle_pd( vecta0, vecta1, _MM_SHUFFLE2(0,1) ), _mm_unpacklo_pd( oldvectb1, oldvectb0 ) ),
    _mm_mul_pd( _mm_unpacklo_pd( vecta1, vecta0 ), _mm_shuffle_pd( oldvectb0, oldvectb1, _MM_SHUFFLE2(0,1) ) )
  );
  oldnormal1 = _mm_sub_sd(
    _mm_mul_sd( vecta0, _mm_unpackhi_pd( oldvectb0, oldvectb0 ) ),
    _mm_mul_sd( _mm_unpackhi_pd( vecta0, vecta0 ), oldvectb0 )
  );
  /* Normal of new triangle */
  newvectb0 = _mm_sub_pd( _mm_load_pd( newpoint+0 ), left0 );
  newvectb1 = _mm_sub_pd( _mm_load_pd( newpoint+2 ), left1 );
  newnormal0 = _mm_sub_pd(
    _mm_mul_pd( _mm_shuffle_pd( vecta0, vecta1, _MM_SHUFFLE2(0,1) ), _mm_unpacklo_pd( newvectb1, newvectb0 ) ),
    _mm_mul_pd( _mm_unpacklo_pd( vecta1, vecta0 ), _mm_shuffle_pd( newvectb0, newvectb1, _MM_SHUFFLE2(0,1) ) )
  );
  newnormal1 = _mm_sub_sd(
    _mm_mul_sd( vecta0, _mm_unpackhi_pd( newvectb0, newvectb0 ) ),
    _mm_mul_sd( _mm_unpackhi_pd( vecta0, vecta0 ), newvectb0 )
  );
  /* Detect normal inversion */
  dotprsum = _mm_mul_pd( oldnormal0, newnormal0 );
  dotprsum = _mm_add_sd( _mm_add_sd( dotprsum, _mm_unpackhi_pd( dotprsum, dotprsum ) ), _mm_mul_sd( oldnormal1, newnormal1 ) );
  if( _mm_comilt_sd( dotprsum, _mm_load_sd( &zero ) ) )
  {
    *denyflag = 1;
    return 0.0;
  }
  /* Penalize long thin triangles */
  penalty = 0.0;
  vecta0 = _mm_mul_pd( vecta0, vecta0 );
  vecta0 = _mm_add_sd( _mm_add_sd( vecta0, _mm_unpackhi_pd( vecta0, vecta0 ) ), _mm_mul_sd( vecta1, vecta1 ) );
  newvectc0 = _mm_sub_pd( _mm_load_pd( newpoint+0 ), _mm_load_pd( rightpoint+0 ) );
  newvectc1 = _mm_sub_sd( _mm_load_sd( newpoint+2 ), _mm_load_sd( rightpoint+2 ) );
  newnormal0 = _mm_mul_pd( newnormal0, newnormal0 );
  newnormal0 = _mm_add_sd( _mm_add_sd( newnormal0, _mm_unpackhi_pd( newnormal0, newnormal0 ) ), _mm_mul_sd( newnormal1, newnormal1 ) );
  newvectb0 = _mm_mul_pd( newvectb0, newvectb0 );
  newvectb0 = _mm_add_sd( _mm_add_sd( newvectb0, _mm_unpackhi_pd( newvectb0, newvectb0 ) ), _mm_mul_sd( newvectb1, newvectb1 ) );
  newvectc0 = _mm_mul_pd( newvectc0, newvectc0 );
  newvectc0 = _mm_add_sd( _mm_add_sd( newvectc0, _mm_unpackhi_pd( newvectc0, newvectc0 ) ), _mm_mul_sd( newvectc1, newvectc1 ) );
  norm = _mm_cvtsd_f64( _mm_add_sd( vecta0, _mm_add_sd( newvectb0, newvectc0 ) ) );
  newcompactness = _mm_cvtsd_f64( _mm_mul_sd( _mm_set_sd( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_sqrt_sd( newnormal0, newnormal0 ) ) );
  if( newcompactness < ( compactnesstarget * norm ) )
  {
    newcompactness /= norm;
    oldvectc0 = _mm_sub_pd( _mm_load_pd( oldpoint+0 ), _mm_load_pd( rightpoint+0 ) );
    oldvectc1 = _mm_sub_sd( _mm_load_sd( oldpoint+2 ), _mm_load_sd( rightpoint+2 ) );
    oldnormal0 = _mm_mul_pd( oldnormal0, oldnormal0 );
    oldnormal0 = _mm_add_sd( _mm_add_sd( oldnormal0, _mm_unpackhi_pd( oldnormal0, oldnormal0 ) ), _mm_mul_sd( oldnormal1, oldnormal1 ) );
    oldvectb0 = _mm_mul_pd( oldvectb0, oldvectb0 );
    oldvectb0 = _mm_add_sd( _mm_add_sd( oldvectb0, _mm_unpackhi_pd( oldvectb0, oldvectb0 ) ), _mm_mul_sd( oldvectb1, oldvectb1 ) );
    oldvectc0 = _mm_mul_pd( oldvectc0, oldvectc0 );
    oldvectc0 = _mm_add_sd( _mm_add_sd( oldvectc0, _mm_unpackhi_pd( oldvectc0, oldvectc0 ) ), _mm_mul_sd( oldvectc1, oldvectc1 ) );
    oldcompactness = _mm_cvtsd_f64( _mm_div_sd( _mm_mul_sd( _mm_set_sd( MD_COMPACTNESS_NORMALIZATION_FACTOR ), _mm_sqrt_sd( oldnormal0, oldnormal0 ) ), _mm_add_sd( vecta0, _mm_add_sd( oldvectb0, oldvectc0 ) ) ) );
    compactness = fmin( compactnesstarget, oldcompactness ) - newcompactness;
    if( compactness > 0.0 )
      penalty = compactness;
  }
  return penalty;
}

 #endif

#endif


////


static mdf mdEdgeCollapsePenaltyAll( mdMesh *mesh, mdThreadData *tdata, mdi *trireflist, mdi trirefcount, mdi pivotindex, mdi skipindex, mdf *collapsepoint, int *denyflag )
{
  int index;
  mdf penalty;
  mdi triindex;
  mdTriangle *tri;
  mdf (*collapsepenalty)( mdf *newpoint, mdf *oldpoint, mdf *leftpoint, mdf *rightpoint, int *denyflag, mdf compactnesstarget );

  collapsepenalty = mesh->collapsepenalty;
  penalty = 0.0;
  for( index = 0 ; index < trirefcount ; index++ )
  {
    if( *denyflag )
      break;
    triindex = trireflist[ index ];
    tri = ADDRESS( mesh->trilist, triindex * mesh->trisize );
    if( tri->v[0] == -1 )
      continue;

#if DEBUG_VERBOSE_COST >= 2
    printf( "    Penalty Tri %d,%d,%d ( Pivot %d ; Skip %d )\n", tri->v[0], tri->v[1], tri->v[2], pivotindex, skipindex );
#endif

#if DEBUG_DEBUG_CHECK_SOMETHING
    mdi triv[3];
    triv[0] = tri->v[0];
    triv[1] = tri->v[1];
    triv[2] = tri->v[2];
#endif

    if( tri->v[0] == pivotindex )
    {
      if( ( tri->v[1] == skipindex ) || ( tri->v[2] == skipindex ) )
        continue;
      penalty += collapsepenalty( collapsepoint, mesh->vertexlist[ tri->v[0] ].point, mesh->vertexlist[ tri->v[2] ].point, mesh->vertexlist[ tri->v[1] ].point, denyflag, mesh->compactnesstarget );
    }
    else if( tri->v[1] == pivotindex )
    {
      if( ( tri->v[2] == skipindex ) || ( tri->v[0] == skipindex ) )
        continue;
      penalty += collapsepenalty( collapsepoint, mesh->vertexlist[ tri->v[1] ].point, mesh->vertexlist[ tri->v[0] ].point, mesh->vertexlist[ tri->v[2] ].point, denyflag, mesh->compactnesstarget );
    }
    else if( tri->v[2] == pivotindex )
    {
      if( ( tri->v[0] == skipindex ) || ( tri->v[1] == skipindex ) )
        continue;
      penalty += collapsepenalty( collapsepoint, mesh->vertexlist[ tri->v[2] ].point, mesh->vertexlist[ tri->v[1] ].point, mesh->vertexlist[ tri->v[0] ].point, denyflag, mesh->compactnesstarget );
    }
    else
    {
#if DEBUG_DEBUG_CHECK_SOMETHING
      MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
      printf( "CopyV : %d %d %d (%d)\n", triv[0], triv[1], triv[2], pivotindex );
      printf( "TriV : %d %d %d (%d)\n", tri->v[0], tri->v[1], tri->v[2], pivotindex );
      sleep( 1 );
      printf( "CopyV : %d %d %d (%d)\n", triv[0], triv[1], triv[2], pivotindex );
      printf( "TriV : %d %d %d (%d)\n", tri->v[0], tri->v[1], tri->v[2], pivotindex );
#endif

    }
  }

  penalty *= mesh->compactnesspenalty;

#if DEBUG_VERBOSE_COST
  printf( "    Penalty Sum : %.12f\n", penalty );
#endif

  return penalty;
}


static mdf mdEdgeCollapsePenalty( mdMesh *mesh, mdThreadData *tdata, mdi v0, mdi v1, mdf *collapsepoint, int *denyflag )
{
  mdf penalty, collapsearea, penaltyfactor;
  mdVertex *vertex0, *vertex1;

  vertex0 = &mesh->vertexlist[ v0 ];
  vertex1 = &mesh->vertexlist[ v1 ];

  collapsearea = vertex0->quadric.area + vertex1->quadric.area;
  penaltyfactor = collapsearea * collapsearea / ( vertex0->trirefcount + vertex1->trirefcount );

#if DEBUG_VERBOSE_COST
  printf( "  Compute Penalty Edge %d,%d\n", v0, v1 );
#endif

  *denyflag = 0;
  penalty  = mdEdgeCollapsePenaltyAll( mesh, tdata, &mesh->trireflist[ vertex0->trirefbase ], vertex0->trirefcount, v0, v1, collapsepoint, denyflag );
  penalty += mdEdgeCollapsePenaltyAll( mesh, tdata, &mesh->trireflist[ vertex1->trirefbase ], vertex1->trirefcount, v1, v0, collapsepoint, denyflag );

#if DEBUG_VERBOSE_COST
  printf( "    Penalty Total : %.12f * %.12f -> %.12f\n", penalty, penaltyfactor, penalty * penaltyfactor );
#endif

  penalty *= penaltyfactor;

  return penalty;
}



////



static void mdMeshEdgeOpCallback( void *opaque, void *entry, int newflag )
{
  mdEdge *edge;
  edge = entry;
  edge->op = opaque;
  return;
}


static double mdMeshOpValueCallback( void *item )
{
  mdOp *op;
  op = item;
  return (double)op->collapsecost;
}

static void mdMeshAddOp( mdMesh *mesh, mdThreadData *tdata, mdi v0, mdi v1 )
{
  int denyflag, opflags;
  mdOp *op;
  mdEdge edge;

  op = mmBlockAlloc( &tdata->opblock );
  opflags = 0x0;
  op->updatebuffer = tdata->updatebuffer;
  op->v0 = v0;
  op->v1 = v1;
  op->value = mdEdgeSolvePoint( &mesh->vertexlist[v0], &mesh->vertexlist[v1], op->collapsepoint );
#if CPU_SSE_SUPPORT
  op->collapsepoint[3] = 0.0;
#endif
  op->penalty = mdEdgeCollapsePenalty( mesh, tdata, v0, v1, op->collapsepoint, &denyflag );
  op->collapsecost = op->value + op->penalty;
  if( ( denyflag ) || ( op->collapsecost > mesh->maxcollapsecost ) )
    opflags |= MD_OP_FLAGS_DETACHED;
  else
    mmBinSortAdd( tdata->binsort, op, op->collapsecost );
#if MD_CONFIG_ATOMIC_SUPPORT
  mmAtomicWrite32( &op->flags, opflags );
#else
  op->flags = opflags;
  mtSpinInit( &op->spinlock );
#endif

  edge.v[0] = v0;
  edge.v[1] = v1;
  if( mmHashLockCallEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge, mdMeshEdgeOpCallback, op, 0 ) != MM_HASH_SUCCESS )
    MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );

#if DEBUG_VERBOSE_COLLAPSE
  printf( "  Added Edge Op %d,%d ; Point %f %f %f ; Value %f ; Penalty %f ; Cost %f (%g, %f)\n", (int)op->v0, (int)op->v1, op->collapsepoint[0], op->collapsepoint[1], op->collapsepoint[2], op->value, op->penalty, op->collapsecost, op->collapsecost, op->collapsecost / mesh->maxcollapsecost );
#endif

  return;
}

static void mdMeshPopulateOpList( mdMesh *mesh, mdThreadData *tdata, mdi tribase, mdi tricount )
{
  mdTriangle *tri, *tristart, *triend;
  long populatecount;

  tristart = ADDRESS( mesh->trilist, tribase * mesh->trisize );
  triend = ADDRESS( tristart, tricount * mesh->trisize );

  populatecount = 0;
  for( tri = tristart ; tri < triend ; tri = ADDRESS( tri, mesh->trisize ) )
  {
#if DEBUG_VERBOSE_COLLAPSE
  printf( "Triangle Edges %d,%d,%d\n", tri->v[0], tri->v[1], tri->v[2] );
#endif
    if( ( ( tri->v[0] < tri->v[1] ) || ( tri->u.edgeflags & 0x1 ) ) && !( tri->u.edgeflags & 0x10 ) )
      mdMeshAddOp( mesh, tdata, tri->v[0], tri->v[1] );
    if( ( ( tri->v[1] < tri->v[2] ) || ( tri->u.edgeflags & 0x2 ) ) && !( tri->u.edgeflags & 0x20 ) )
      mdMeshAddOp( mesh, tdata, tri->v[1], tri->v[2] );
    if( ( ( tri->v[2] < tri->v[0] ) || ( tri->u.edgeflags & 0x4 ) ) && !( tri->u.edgeflags & 0x40 ) )
      mdMeshAddOp( mesh, tdata, tri->v[2], tri->v[0] );
    populatecount++;
    tdata->statuspopulatecount = populatecount;
  }

  return;
}



////



/* Merge vertex attributes of v0 and v1, write to v0 */
static inline void mdEdgeCollapseMergeVertexAttribs( mdMesh *mesh, mdThreadData *tdata, mdi v0, mdi v1, mdf *collapsepoint )
{
  mdVertex *vertex0, *vertex1;
  mdf dist[3], dist0, dist1, weightsum, weightsuminv;
  mdf weight0, weight1;

  vertex0 = &mesh->vertexlist[ v0 ];
  MD_VectorSubStore( dist, collapsepoint, vertex0->point );
  dist0 = MD_VectorMagnitude( dist );
  vertex1 = &mesh->vertexlist[ v1 ];
  MD_VectorSubStore( dist, collapsepoint, vertex1->point );
  dist1 = MD_VectorMagnitude( dist );
  weight0 = dist1 * vertex0->quadric.area;
  weight1 = dist0 * vertex1->quadric.area;
  weightsum = weight0 + weight1;
  if( weightsum )
  {
    weightsuminv = 1.0 / weightsum;
    weight0 *= weightsuminv;
    weight1 *= weightsuminv;
  }
  else
  {
    weight0 = 0.5;
    weight1 = 0.5;
  }
  mesh->vertexmerge( mesh->mergecontext, v0, v1, weight0, weight1 );
  return;
}


/* Delete triangle and return outer vertex */
static mdi mdEdgeCollapseDeleteTriangle( mdMesh *mesh, mdThreadData *tdata, mdi v0, mdi v1, int *retdelflags )
{
  int delflags;
  mdi outer;
  mdEdge edge;
  mdTriangle *tri;
  mdOp *op;

  *retdelflags = 0x0;

  edge.v[0] = v0;
  edge.v[1] = v1;
  if( mmHashLockDeleteEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge, 1 ) != MM_HASH_SUCCESS )
    return -1;
  op = edge.op;
  if( op )
    mdUpdateBufferAdd( &op->updatebuffer[ tdata->threadid >> mesh->updatebuffershift ], op, MD_OP_FLAGS_DELETION_PENDING );

  tri = ADDRESS( mesh->trilist, edge.triindex * mesh->trisize );

#if DEBUG_VERBOSE_COLLAPSE
  printf( "  Delete Triangle %d,%d,%d\n", tri->v[0], tri->v[1], tri->v[2] );
#endif

  if( tri->v[0] != v0 )
  {
    edge.v[0] = tri->v[0];
    edge.v[1] = tri->v[1];
    if( mmHashLockDeleteEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge, 1 ) == MM_HASH_SUCCESS )
    {
      op = edge.op;
      if( op )
        mdUpdateBufferAdd( &op->updatebuffer[ tdata->threadid >> mesh->updatebuffershift ], op, MD_OP_FLAGS_DELETION_PENDING );
    }
    else
    {
#if 0
      /* Shouldn't happen with a proper watertight mesh, but it can happen if edges are reused... */
      MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
#endif
    }
  }
  if( tri->v[1] != v0 )
  {
    edge.v[0] = tri->v[1];
    edge.v[1] = tri->v[2];
    if( mmHashLockDeleteEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge, 1 ) == MM_HASH_SUCCESS )
    {
      op = edge.op;
      if( op )
        mdUpdateBufferAdd( &op->updatebuffer[ tdata->threadid >> mesh->updatebuffershift ], op, MD_OP_FLAGS_DELETION_PENDING );
    }
    else
    {
#if 0
      /* Shouldn't happen with a proper watertight mesh, but it can happen if edges are reused... */
      MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
#endif
    }
  }
  if( tri->v[2] != v0 )
  {
    edge.v[0] = tri->v[2];
    edge.v[1] = tri->v[0];
    if( mmHashLockDeleteEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge, 1 ) == MM_HASH_SUCCESS )
    {
      op = edge.op;
      if( op )
        mdUpdateBufferAdd( &op->updatebuffer[ tdata->threadid >> mesh->updatebuffershift ], op, MD_OP_FLAGS_DELETION_PENDING );
    }
    else
    {
#if 0
      /* Shouldn't happen with a proper watertight mesh, but it can happen if edges are reused... */
      MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
#endif
    }
  }

  /* Determine outer vertex */
  /* TODO: Replace branches with bitwise arithmetics */
  delflags = 0x0;
  if( tri->v[0] == v0 )
  {
    outer = tri->v[2];
    if( tri->u.edgeflags & 0x2 )
      delflags |= 0x1;
    if( tri->u.edgeflags & 0x4 )
      delflags |= 0x2;
  }
  else if( tri->v[1] == v0 )
  {
    outer = tri->v[0];
    if( tri->u.edgeflags & 0x4 )
      delflags |= 0x1;
    if( tri->u.edgeflags & 0x1 )
      delflags |= 0x2;
  }
  else if( tri->v[2] == v0 )
  {
    outer = tri->v[1];
    if( tri->u.edgeflags & 0x1 )
      delflags |= 0x1;
    if( tri->u.edgeflags & 0x2 )
      delflags |= 0x2;
  }
  else
  {
    MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
    outer = tri->v[0];
  }

  *retdelflags = delflags;

  /* Invalidate triangle */
  tri->v[0] = -1;

  return outer;
}


static void mdEdgeCollapseUpdateTriangle( mdMesh *mesh, mdThreadData *tdata, mdTriangle *tri, mdi newv, int pivot, int left, int right )
{
  mdEdge edge;
  mdOp *op;

#if DEBUG_VERBOSE_COLLAPSE
  printf( "  Collapse Update %d : Tri %d,%d,%d\n", newv, tri->v[pivot], tri->v[right], tri->v[left] );
#endif

  edge.v[0] = tri->v[pivot];
  edge.v[1] = tri->v[right];
  edge.op = 0;
  if( edge.v[0] == newv )
  {
    if( mmHashLockReadEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge ) != MM_HASH_SUCCESS )
    {
#if 0
      /* Shouldn't happen with a proper watertight mesh, but it can happen if edges are reused... */
      MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
#endif
    }
  }
  else
  {
    if( mmHashLockDeleteEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge, 1 ) == MM_HASH_SUCCESS )
    {
      edge.v[0] = newv;
      if( mmHashLockAddEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge, 1 ) != MM_HASH_SUCCESS )
        MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
    }
    else
    {
#if 0
      /* Shouldn't happen with a proper watertight mesh, but it can happen if edges are reused... */
      MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
#endif
    }
  }
  op = edge.op;
  if( op )
  {
#if DEBUG_VERBOSE_COLLAPSE
    printf( "    Update Edge %d,%d Before ; Point %f %f %f ; Cost %f\n", op->v0, op->v1, op->collapsepoint[0], op->collapsepoint[1], op->collapsepoint[2], op->collapsecost );
#endif
#if MD_CONFIG_DELAYED_OP_REDIRECT
    op->value = mdEdgeSolvePoint( &mesh->vertexlist[edge.v[0]], &mesh->vertexlist[edge.v[1]], op->collapsepoint );
#else
    op->v0 = newv;
    op->value = mdEdgeSolvePoint( &mesh->vertexlist[op->v0], &mesh->vertexlist[op->v1], op->collapsepoint );
#endif
#if CPU_SSE_SUPPORT
    op->collapsepoint[3] = 0.0;
#endif
    mdUpdateBufferAdd( &op->updatebuffer[ tdata->threadid >> mesh->updatebuffershift ], op, 0x0 );
#if DEBUG_VERBOSE_COLLAPSE
    printf( "    Update Edge %d,%d After  ; Point %f %f %f ; Cost %f\n", op->v0, op->v1, op->collapsepoint[0], op->collapsepoint[1], op->collapsepoint[2], op->collapsecost );
    printf( "    Edge %d,%d ; Value %f ; Penalty %f ; Cost %f\n", op->v0, op->v1, op->value, op->penalty, op->collapsecost );
#endif
  }

  edge.v[0] = tri->v[left];
  edge.v[1] = tri->v[pivot];
  edge.op = 0;
  if( edge.v[1] == newv )
  {
    if( mmHashLockReadEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge ) != MM_HASH_SUCCESS )
    {
#if 0
      /* Shouldn't happen with a proper watertight mesh, but it can happen if edges are reused... */
      MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
#endif
    }
  }
  else
  {
    if( mmHashLockDeleteEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge, 1 ) == MM_HASH_SUCCESS )
    {
      edge.v[1] = newv;
      if( mmHashLockAddEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge, 1 ) != MM_HASH_SUCCESS )
        MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
    }
    else
    {
#if 0
      /* Shouldn't happen with a proper watertight mesh, but it can happen if edges are reused... */
      MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
#endif
    }
  }
  op = edge.op;
  if( op )
  {
#if DEBUG_VERBOSE_COLLAPSE
    printf( "    Update Edge %d,%d Before ; Point %f %f %f ; Cost %f\n", op->v0, op->v1, op->collapsepoint[0], op->collapsepoint[1], op->collapsepoint[2], op->collapsecost );
#endif
#if MD_CONFIG_DELAYED_OP_REDIRECT
    op->value = mdEdgeSolvePoint( &mesh->vertexlist[edge.v[0]], &mesh->vertexlist[edge.v[1]], op->collapsepoint );
#else
    op->v1 = newv;
    op->value = mdEdgeSolvePoint( &mesh->vertexlist[op->v0], &mesh->vertexlist[op->v1], op->collapsepoint );
#endif
#if CPU_SSE_SUPPORT
    op->collapsepoint[3] = 0.0;
#endif
    mdUpdateBufferAdd( &op->updatebuffer[ tdata->threadid >> mesh->updatebuffershift ], op, 0x0 );
#if DEBUG_VERBOSE_COLLAPSE
    printf( "    Update Edge %d,%d After  ; Point %f %f %f ; Cost %f\n", op->v0, op->v1, op->collapsepoint[0], op->collapsepoint[1], op->collapsepoint[2], op->collapsecost );
    printf( "    Edge %d,%d ; Value %f ; Penalty %f ; Cost %f\n", op->v0, op->v1, op->value, op->penalty, op->collapsecost );
#endif
  }

  tri->v[pivot] = newv;

  return;
}



/*
Walk through the list of all triangles attached to a vertex
- Update cost of collapse for other edges of triangles
- Build up the updated list of triangle references for new vertex
*/
static mdi *mdEdgeCollapseUpdateAll( mdMesh *mesh, mdThreadData *tdata, mdi *trireflist, mdi trirefcount, mdi oldv, mdi newv, mdi *trirefstore )
{
  int index;
  mdi triindex;
  mdTriangle *tri;

  for( index = 0 ; index < trirefcount ; index++ )
  {
    triindex = trireflist[ index ];
    tri = ADDRESS( mesh->trilist, triindex * mesh->trisize );
    if( tri->v[0] == -1 )
      continue;

#if DEBUG_VERBOSE_COLLAPSE
    printf( "  Tri %d : %d %d %d\n", index, tri->v[0], tri->v[1], tri->v[2] );
#endif

    *trirefstore = triindex;
    trirefstore++;
    if( tri->v[0] == oldv )
      mdEdgeCollapseUpdateTriangle( mesh, tdata, tri, newv, 0, 2, 1 );
    else if( tri->v[1] == oldv )
      mdEdgeCollapseUpdateTriangle( mesh, tdata, tri, newv, 1, 0, 2 );
    else if( tri->v[2] == oldv )
      mdEdgeCollapseUpdateTriangle( mesh, tdata, tri, newv, 2, 1, 0 );
    else
      MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
  }

  return trirefstore;
}


static void mdVertexInvalidateTri( mdMesh *mesh, mdThreadData *tdata, mdi v0, mdi v1 )
{
  mdEdge edge;
  mdOp *op;

  edge.v[0] = v0;
  edge.v[1] = v1;
  if( mmHashLockReadEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge ) == MM_HASH_SUCCESS )
  {
    op = edge.op;
    if( op )
      mdUpdateBufferAdd( &op->updatebuffer[ tdata->threadid >> mesh->updatebuffershift ], op, 0x0 );
  }
#if 0
  /* Shouldn't happen with a proper watertight mesh, but it can happen if edges are reused... */
  else
    MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
#endif

  return;
}

static void mdVertexInvalidate( mdMesh *mesh, mdThreadData *tdata, mdi vertexindex )
{
  mdi index, triindex, trirefcount;
  mdi *trireflist;
  mdTriangle *tri;
  mdVertex *vertex;

  /* Vertices of the collapsed edge */
  vertex = &mesh->vertexlist[ vertexindex ];
  trireflist = &mesh->trireflist[ vertex->trirefbase ];
  trirefcount = vertex->trirefcount;

  for( index = 0 ; index < trirefcount ; index++ )
  {
    triindex = trireflist[ index ];
    tri = ADDRESS( mesh->trilist, triindex * mesh->trisize );
    if( tri->v[0] == -1 )
      continue;
    if( tri->v[0] == vertexindex )
    {
      mdVertexInvalidateTri( mesh, tdata, tri->v[0], tri->v[1] );
      mdVertexInvalidateTri( mesh, tdata, tri->v[2], tri->v[0] );
    }
    else if( tri->v[1] == vertexindex )
    {
      mdVertexInvalidateTri( mesh, tdata, tri->v[1], tri->v[2] );
      mdVertexInvalidateTri( mesh, tdata, tri->v[0], tri->v[1] );
    }
    else if( tri->v[2] == vertexindex )
    {
      mdVertexInvalidateTri( mesh, tdata, tri->v[2], tri->v[0] );
      mdVertexInvalidateTri( mesh, tdata, tri->v[1], tri->v[2] );
    }
    else
      MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
  }

  return;
}

static void mdVertexInvalidateAll( mdMesh *mesh, mdThreadData *tdata, mdi *trireflist, mdi trirefcount, mdi pivotindex )
{
  int index;
  mdi triindex;
  mdTriangle *tri;

  for( index = 0 ; index < trirefcount ; index++ )
  {
    triindex = trireflist[ index ];
    tri = ADDRESS( mesh->trilist, triindex * mesh->trisize );
    if( tri->v[0] == -1 )
      continue;
    if( tri->v[0] == pivotindex )
    {
      mdVertexInvalidate( mesh, tdata, tri->v[1] );
      mdVertexInvalidate( mesh, tdata, tri->v[2] );
    }
    else if( tri->v[1] == pivotindex )
    {
      mdVertexInvalidate( mesh, tdata, tri->v[2] );
      mdVertexInvalidate( mesh, tdata, tri->v[0] );
    }
    else if( tri->v[2] == pivotindex )
    {
      mdVertexInvalidate( mesh, tdata, tri->v[0] );
      mdVertexInvalidate( mesh, tdata, tri->v[1] );
    }
    else
      MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
  }

  return;
}


static void mdEdgeCollapseLinkOuter( mdMesh *mesh, mdThreadData *tdata, mdi newv, mdi outer )
{
  int sideflags;
  mdEdge edge;
  mdOp *op;

  if( outer == -1 )
    return;
  sideflags = 0x0;

  edge.v[0] = newv;
  edge.v[1] = outer;
  if( mmHashLockReadEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge ) == MM_HASH_SUCCESS )
  {
    sideflags |= 0x1;
    op = edge.op;
    if( op )
      return;
  }
  edge.v[0] = outer;
  edge.v[1] = newv;
  if( mmHashLockReadEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge ) == MM_HASH_SUCCESS )
  {
    sideflags |= 0x2;
    op = edge.op;
    if( op )
      return;
  }

  if( ( newv < outer ) && ( sideflags & 0x1 ) )
    mdMeshAddOp( mesh, tdata, newv, outer );
  else if( sideflags & 0x2 )
    mdMeshAddOp( mesh, tdata, outer, newv );

  return;
}



static void mdEdgeCollapsePropagateBoundary( mdMesh *mesh, mdi v0, mdi v1 )
{
  mdEdge edge;
  mdTriangle *tri;

  edge.v[0] = v1;
  edge.v[1] = v0;
  if( mmHashLockReadEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge ) == MM_HASH_SUCCESS )
    return;
  edge.v[0] = v0;
  edge.v[1] = v1;
  if( mmHashLockReadEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge ) != MM_HASH_SUCCESS )
    return;

  tri = ADDRESS( mesh->trilist, edge.triindex * mesh->trisize );
  if( tri->v[0] == v0 )
    tri->u.edgeflags |= 0x1;
  else if( tri->v[1] == v0 )
    tri->u.edgeflags |= 0x2;
  else if( tri->v[2] == v0 )
    tri->u.edgeflags |= 0x4;
  else
    MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );

  /* TODO: Grow the boundary for affected vertex quadrics */

  return;
}




#define MD_LOCK_BUFFER_STATIC (512)

typedef struct
{
  mdi vertexstatic[MD_LOCK_BUFFER_STATIC];
  mdi *vertexlist;
  int vertexcount;
  int vertexalloc;
} mdLockBuffer;

static inline void mdLockBufferInit( mdLockBuffer *buffer, int maxvertexcount )
{
  buffer->vertexlist = buffer->vertexstatic;
  buffer->vertexalloc = MD_LOCK_BUFFER_STATIC;
  if( maxvertexcount > MD_LOCK_BUFFER_STATIC )
  {
    buffer->vertexlist = malloc( maxvertexcount * sizeof(mdi) );
    buffer->vertexalloc = maxvertexcount;
  }
  buffer->vertexcount = 0;
  return;
}

static inline void mdLockBufferResize( mdLockBuffer *buffer, int maxvertexcount )
{
  int index;
  mdi *vertexlist;
  if( maxvertexcount > buffer->vertexalloc )
  {
    vertexlist = malloc( maxvertexcount * sizeof(mdi) );
    for( index = 0 ; index < buffer->vertexcount ; index++ )
      vertexlist[index] = buffer->vertexlist[index];
    if( buffer->vertexlist != buffer->vertexstatic )
      free( buffer->vertexlist );
    buffer->vertexlist = vertexlist;
    buffer->vertexalloc = maxvertexcount;
  }
  return;
}

static inline void mdLockBufferEnd( mdLockBuffer *buffer )
{
  if( buffer->vertexlist != buffer->vertexstatic )
    free( buffer->vertexlist );
  buffer->vertexlist = buffer->vertexstatic;
  buffer->vertexalloc = MD_LOCK_BUFFER_STATIC;
  buffer->vertexcount = 0;
  return;
}

void mdLockBufferUnlockAll( mdMesh *mesh, mdThreadData *tdata, mdLockBuffer *buffer )
{
  int index;
  mdVertex *vertex;
  for( index = 0 ; index < buffer->vertexcount ; index++ )
  {
    vertex = &mesh->vertexlist[ buffer->vertexlist[index] ];
#if MD_CONFIG_ATOMIC_SUPPORT
    if( mmAtomicRead32( &vertex->atomicowner ) == tdata->threadid )
      mmAtomicCmpXchg32( &vertex->atomicowner, tdata->threadid, -1 );
#else
    mtSpinLock( &vertex->ownerspinlock );
    if( vertex->owner == tdata->threadid )
      vertex->owner = -1;
    mtSpinUnlock( &vertex->ownerspinlock );
#endif
  }
  buffer->vertexcount = 0;
  return;
}

/* If it fails, release all locks */
int mdLockBufferTryLock( mdMesh *mesh, mdThreadData *tdata, mdLockBuffer *buffer, mdi vertexindex )
{
  int32_t owner;
  mdVertex *vertex;
  vertex = &mesh->vertexlist[ vertexindex ];

#if MD_CONFIG_ATOMIC_SUPPORT
  owner = mmAtomicRead32( &vertex->atomicowner );
  if( owner == tdata->threadid )
    return 1;
  if( ( owner != -1 ) || !( mmAtomicCmpReplace32( &vertex->atomicowner, -1, tdata->threadid ) ) )
  {
    mdLockBufferUnlockAll( mesh, tdata, buffer );
    return 0;
  }
#else
  mtSpinLock( &vertex->ownerspinlock );
  owner = vertex->owner;
  if( owner == tdata->threadid )
  {
    mtSpinUnlock( &vertex->ownerspinlock );
    return 1;
  }
  if( owner != -1 )
  {
    mtSpinUnlock( &vertex->ownerspinlock );
    mdLockBufferUnlockAll( mesh, tdata, buffer );
    return 0;
  }
  vertex->owner = tdata->threadid;
  mtSpinUnlock( &vertex->ownerspinlock );
#endif

  buffer->vertexlist[buffer->vertexcount++] = vertexindex;
  return 1;
}

/* If it fails, release all locks then wait for the desired lock to become available */
int mdLockBufferLock( mdMesh *mesh, mdThreadData *tdata, mdLockBuffer *buffer, mdi vertexindex )
{
  int32_t owner;
  mdVertex *vertex;
  vertex = &mesh->vertexlist[ vertexindex ];

#if MD_CONFIG_ATOMIC_SUPPORT
  owner = mmAtomicRead32( &vertex->atomicowner );
  if( owner == tdata->threadid )
    return 1;
  if( ( owner == -1 ) && mmAtomicCmpReplace32( &vertex->atomicowner, -1, tdata->threadid ) )
  {
    buffer->vertexlist[buffer->vertexcount++] = vertexindex;
    return 1;
  }
  /* Lock failed, release all locks and wait until we get the lock we got stuck on */
  mdLockBufferUnlockAll( mesh, tdata, buffer );
  mmAtomicSpinWaitEq32( &vertex->atomicowner, -1 );
#else
  mtSpinLock( &vertex->ownerspinlock );
  owner = vertex->owner;
  if( owner == tdata->threadid )
  {
    mtSpinUnlock( &vertex->ownerspinlock );
    return 1;
  }
  if( owner == -1 )
  {
    vertex->owner = tdata->threadid;
    mtSpinUnlock( &vertex->ownerspinlock );
    buffer->vertexlist[buffer->vertexcount++] = vertexindex;
    return 1;
  }
  mtSpinUnlock( &vertex->ownerspinlock );
  /* Lock failed, release all locks */
  mdLockBufferUnlockAll( mesh, tdata, buffer );
#endif
  return 0;
}


static int mdPivotLockRefs( mdMesh *mesh, mdThreadData *tdata, mdLockBuffer *buffer, mdi vertexindex )
{
  int index;
  mdi triindex, iav, ibv, trirefcount;
  mdi *trireflist;
  mdTriangle *tri;
  mdVertex *vertex;

  vertex = &mesh->vertexlist[ vertexindex ];
  trireflist = &mesh->trireflist[ vertex->trirefbase ];
  trirefcount = vertex->trirefcount;

  for( index = 0 ; index < trirefcount ; index++ )
  {
    triindex = trireflist[ index ];
    tri = ADDRESS( mesh->trilist, triindex * mesh->trisize );
    if( tri->v[0] == -1 )
      continue;
    if( tri->v[0] == vertexindex )
    {
      iav = tri->v[1];
      ibv = tri->v[2];
    }
    else if( tri->v[1] == vertexindex )
    {
      iav = tri->v[2];
      ibv = tri->v[0];
    }
    else if( tri->v[2] == vertexindex )
    {
      iav = tri->v[0];
      ibv = tri->v[1];
    }
    else
    {
      MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
      continue;
    }
    if( !( mdLockBufferLock( mesh, tdata, buffer, iav ) ) )
      return 0;
    if( !( mdLockBufferLock( mesh, tdata, buffer, ibv ) ) )
      return 0;
  }

  return 1;
}


static void mdOpResolveLockEdge( mdMesh *mesh, mdThreadData *tdata, mdLockBuffer *lockbuffer, mdOp *op )
{
  int failcount, globalflag;
  mdVertex *vertex0, *vertex1;

  failcount = 0;
  globalflag = 0;
  for( ; ; )
  {
    if( ( failcount > MD_GLOBAL_LOCK_THRESHOLD ) && !( globalflag ) )
    {
      mdLockBufferUnlockAll( mesh, tdata, lockbuffer );
#if MD_CONFIG_ATOMIC_SUPPORT
      mmAtomicSpin32( &mesh->globalvertexlock, 0x0, 0x1 );
#else
      mtSpinLock( &mesh->globalvertexspinlock );
#endif
      globalflag = 1;
    }
    if( !( mdLockBufferLock( mesh, tdata, lockbuffer, op->v0 ) ) || !( mdLockBufferLock( mesh, tdata, lockbuffer, op->v1 ) ) )
    {
      failcount++;
      continue;
    }
    vertex0 = &mesh->vertexlist[ op->v0 ];
    vertex1 = &mesh->vertexlist[ op->v1 ];
#if MD_CONFIG_DELAYED_OP_REDIRECT
    if( vertex0->redirectindex != -1 )
      op->v0 = vertex0->redirectindex;
    else if( vertex1->redirectindex != -1 )
      op->v1 = vertex1->redirectindex;
    else
      break;
#else
    break;
#endif
  }

#if MD_CONFIG_ATOMIC_SUPPORT
  if( globalflag )
    mmAtomicWrite32( &mesh->globalvertexlock, 0x0 );
#else
  if( globalflag )
    mtSpinUnlock( &mesh->globalvertexspinlock );
#endif

  return;
}

static int mdOpResolveLockEdgeTry( mdMesh *mesh, mdThreadData *tdata, mdLockBuffer *lockbuffer, mdOp *op )
{
  mdVertex *vertex0, *vertex1;

  for( ; ; )
  {
    if( !( mdLockBufferTryLock( mesh, tdata, lockbuffer, op->v0 ) ) || !( mdLockBufferTryLock( mesh, tdata, lockbuffer, op->v1 ) ) )
      return 0;
    vertex0 = &mesh->vertexlist[ op->v0 ];
    vertex1 = &mesh->vertexlist[ op->v1 ];
#if MD_CONFIG_DELAYED_OP_REDIRECT
    if( vertex0->redirectindex != -1 )
      op->v0 = vertex0->redirectindex;
    else if( vertex1->redirectindex != -1 )
      op->v1 = vertex1->redirectindex;
    else
      break;
#else
    break;
#endif
  }

  return 1;
}

static void mdOpResolveLockFull( mdMesh *mesh, mdThreadData *tdata, mdLockBuffer *lockbuffer, mdOp *op )
{
  int failcount, globalflag;
  mdVertex *vertex0, *vertex1;

  failcount = 0;
  globalflag = 0;
  for( ; ; )
  {
    if( ( failcount > MD_GLOBAL_LOCK_THRESHOLD ) && !( globalflag ) )
    {
      mdLockBufferUnlockAll( mesh, tdata, lockbuffer );
#if MD_CONFIG_ATOMIC_SUPPORT
      mmAtomicSpin32( &mesh->globalvertexlock, 0x0, 0x1 );
#else
      mtSpinLock( &mesh->globalvertexspinlock );
#endif
      globalflag = 1;
    }
    if( !( mdLockBufferLock( mesh, tdata, lockbuffer, op->v0 ) ) || !( mdLockBufferLock( mesh, tdata, lockbuffer, op->v1 ) ) )
    {
      failcount++;
      continue;
    }
    vertex0 = &mesh->vertexlist[ op->v0 ];
    vertex1 = &mesh->vertexlist[ op->v1 ];
#if MD_CONFIG_DELAYED_OP_REDIRECT
    if( vertex0->redirectindex != -1 )
      op->v0 = vertex0->redirectindex;
    else if( vertex1->redirectindex != -1 )
      op->v1 = vertex1->redirectindex;
    else
    {
      mdLockBufferResize( lockbuffer, 2 + ( ( vertex0->trirefcount + vertex1->trirefcount ) << 1 ) );
      if( !( mdPivotLockRefs( mesh, tdata, lockbuffer, op->v0 ) ) || !( mdPivotLockRefs( mesh, tdata, lockbuffer, op->v1 ) ) )
      {
        failcount++;
        continue;
      }
      break;
    }
#else
    mdLockBufferResize( lockbuffer, 2 + ( ( vertex0->trirefcount + vertex1->trirefcount ) << 1 ) );
    if( !( mdPivotLockRefs( mesh, tdata, lockbuffer, op->v0 ) ) || !( mdPivotLockRefs( mesh, tdata, lockbuffer, op->v1 ) ) )
    {
      failcount++;
      continue;
    }
    break;
#endif
  }

#if MD_CONFIG_ATOMIC_SUPPORT
  if( globalflag )
    mmAtomicWrite32( &mesh->globalvertexlock, 0x0 );
#else
  if( globalflag )
    mtSpinUnlock( &mesh->globalvertexspinlock );
#endif

  return;
}


#define MD_EDGE_COLLAPSE_TRIREF_STATIC (512)

static void mdEdgeCollapse( mdMesh *mesh, mdThreadData *tdata, mdi v0, mdi v1, mdf *collapsepoint )
{
  int index, delflags0, delflags1;
  long deletioncount;
  mdi newv, trirefcount, trirefmax, outer0, outer1;
  mdi *trireflist, *trirefstore;
  mdi trirefstatic[MD_EDGE_COLLAPSE_TRIREF_STATIC];
  mdVertex *vertex0, *vertex1;

  /* Vertices of the collapsed edge */
  vertex0 = &mesh->vertexlist[ v0 ];
  vertex1 = &mesh->vertexlist[ v1 ];

  /* Collapse other custom vertex attributes */
  if( mesh->vertexmerge )
    mdEdgeCollapseMergeVertexAttribs( mesh, tdata, v0, v1, collapsepoint );

#if DEBUG_VERBOSE_COLLAPSE
  printf( "Collapse %d,%d ; Point %f %f %f ( Overwrite %d ; Delete %d )\n", (int)v0, (int)v1, collapsepoint[0], collapsepoint[1], collapsepoint[2], (int)v0, (int)v1 );
#endif

  /* New vertex overwriting v0 */
  newv = v0;

  /* Delete the triangles on both sides of the edge and all associated edges */
  outer0 = mdEdgeCollapseDeleteTriangle( mesh, tdata, v0, v1, &delflags0 );
  outer1 = mdEdgeCollapseDeleteTriangle( mesh, tdata, v1, v0, &delflags1 );

  /* Track count of deletions */
  deletioncount = tdata->statusdeletioncount;
  if( outer0 != -1 )
    deletioncount++;
  if( outer1 != -1 )
    deletioncount++;
  tdata->statusdeletioncount = deletioncount;

#if DEBUG_VERBOSE_COLLAPSE
  printf( "  Redirect %d -> %d ; %d -> %d\n", (int)v0, (int)vertex0->redirectindex, (int)v1, (int)vertex1->redirectindex );
#endif

#if DEBUG_VERBOSE_COLLAPSE
  printf( "    Move Point %f %f %f ( %f %f %f ) -> %f %f %f\n", vertex0->point[0], vertex0->point[1], vertex0->point[2], vertex1->point[0], vertex1->point[1], vertex1->point[2], collapsepoint[0], collapsepoint[1], collapsepoint[2] );
#endif

  /* Set up new vertex over v0 */
  MD_VectorCopy( vertex0->point, collapsepoint );
  mathQuadricAddQuadric( &vertex0->quadric, &vertex1->quadric );

  /* Propagate boundaries from deleted triangles */
  if( delflags0 )
  {
    if( delflags0 & 0x1 )
      mdEdgeCollapsePropagateBoundary( mesh, newv, outer0 );
    if( delflags0 & 0x2 )
      mdEdgeCollapsePropagateBoundary( mesh, outer0, newv );
  }
  if( delflags1 )
  {
    if( delflags1 & 0x1 )
      mdEdgeCollapsePropagateBoundary( mesh, newv, outer1 );
    if( delflags1 & 0x2 )
      mdEdgeCollapsePropagateBoundary( mesh, outer1, newv );
  }

  /* Redirect vertex1 to vertex0 */
  vertex1->redirectindex = newv;

  /* Maximum theoritical count of triangle references for our new vertex, we need a chunk of memory that big */
  trirefmax = vertex0->trirefcount + vertex1->trirefcount;

  /* Buffer to temporarily store our new trirefs */
  trireflist = trirefstatic;
  if( trirefmax > MD_EDGE_COLLAPSE_TRIREF_STATIC )
    trireflist = malloc( trirefmax * sizeof(mdi) );

  /* Update all triangles connected to vertex0 and vertex1 */
  trirefstore = trireflist;
  trirefstore = mdEdgeCollapseUpdateAll( mesh, tdata, &mesh->trireflist[ vertex0->trirefbase ], vertex0->trirefcount, v0, newv, trirefstore );
  trirefstore = mdEdgeCollapseUpdateAll( mesh, tdata, &mesh->trireflist[ vertex1->trirefbase ], vertex1->trirefcount, v1, newv, trirefstore );

  /* Find where to store the trirefs */
  trirefcount = (int)( trirefstore - trireflist );

#if DEBUG_VERBOSE_COLLAPSE
  printf( "TriRefCount %d ; Alloc %d\n", trirefcount, trirefmax );
#endif

  if( trirefcount > vertex0->trirefcount )
  {
    if( trirefcount <= vertex1->trirefcount )
      vertex0->trirefbase = vertex1->trirefbase;
    else
    {
      /* Multithreading, acquire lock */
#if MD_CONFIG_ATOMIC_SUPPORT
      mmAtomicSpin32( &mesh->trireflock, 0x0, 0x1 );
#else
      mtSpinLock( &mesh->trirefspinlock );
#endif
      vertex0->trirefbase = mesh->trirefcount;
      mesh->trirefcount += trirefcount;
      if( mesh->trirefcount >= mesh->trirefalloc )
        MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
#if MD_CONFIG_ATOMIC_SUPPORT
      mmAtomicWrite32( &mesh->trireflock, 0x0 );
#else
      mtSpinUnlock( &mesh->trirefspinlock );
#endif
    }
  }

  /* Mark vertex1 as unused */
  vertex1->trirefcount = 0;

  /* Store trirefs */
  vertex0->trirefcount = trirefcount;
  trirefstore = &mesh->trireflist[ vertex0->trirefbase ];
  for( index = 0 ; index < trirefcount ; index++ )
  {
#if DEBUG_VERBOSE_COLLAPSE
    mdTriangle *tri;
    tri = ADDRESS( mesh->trilist, trireflist[index] * mesh->trisize );
    printf( "    Triref %d : %d,%d,%d\n", index, tri->v[0], tri->v[1], tri->v[2] );
#endif
    trirefstore[index] = trireflist[index];
  }

  /* Invalidate all cost calculations in neighborhood of pivot vertex */
  mdVertexInvalidateAll( mesh, tdata, trireflist, trirefcount, newv );

  /* If buffer wasn't static, free it */
  if( trireflist != trirefstatic )
    free( trireflist );

  /* Verify if we should create new ops between newv and outer vertices of deleted triangles */
  mdEdgeCollapseLinkOuter( mesh, tdata, newv, outer0 );
  mdEdgeCollapseLinkOuter( mesh, tdata, newv, outer1 );

#if DEBUG_VERBOSE_COLLAPSE
  printf( "Collapse End %d,%d\n", (int)v0, (int)v1 );
#endif

  return;
}



////



typedef struct
{
  int collisionflag;
  mdi trileft;
  mdi triright;
} mdEdgeCollisionData;

static void mdEdgeCollisionCallback( void *opaque, void *entry, int newflag )
{
  mdEdge *edge;
  mdEdgeCollisionData *ecd;
  edge = entry;
  ecd = opaque;
  if( ( edge->triindex != ecd->trileft ) && ( edge->triindex != ecd->triright ) )
    ecd->collisionflag = 1;
  return;
}


/*
Prevent 2D collapses

Check all triangles attached to v1 that would have to attach back to v0
If any of the edge is already present in the hash table, deny the collapse
*/
static int mdEdgeCollisionCheck( mdMesh *mesh, mdThreadData *tdata, mdi v0, mdi v1 )
{
  int index, trirefcount, left, right;
  mdi triindex, vsrc, vdst;
  mdi *trireflist;
  mdEdge edge;
  mdTriangle *tri;
  mdVertex *vertex0, *vertex1;
  mdEdgeCollisionData ecd;

  vertex0 = &mesh->vertexlist[ v0 ];
  vertex1 = &mesh->vertexlist[ v1 ];
  if( vertex0->trirefcount < vertex1->trirefcount )
  {
    vsrc = v0;
    vdst = v1;
    trireflist = &mesh->trireflist[ vertex0->trirefbase ];
    trirefcount = vertex0->trirefcount;
  }
  else
  {
    vsrc = v1;
    vdst = v0;
    trireflist = &mesh->trireflist[ vertex1->trirefbase ];
    trirefcount = vertex1->trirefcount;
  }

#if DEBUG_VERBOSE_COLLISION
  printf( "Collision Check %d,%d\n", (int)v0, (int)v1 );
  printf( "  Src %d ; Dst %d\n", vsrc, vdst );
#endif

  /* Find the triangles that would be deleted so that we don't detect false collisions with them */
  ecd.collisionflag = 0;
  ecd.trileft = -1;
  edge.v[0] = v0;
  edge.v[1] = v1;
  if( mmHashLockReadEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge ) == MM_HASH_SUCCESS )
    ecd.trileft = edge.triindex;
  ecd.triright = -1;
  edge.v[0] = v1;
  edge.v[1] = v0;
  if( mmHashLockReadEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge ) == MM_HASH_SUCCESS )
    ecd.triright = edge.triindex;

  /* Check all trirefs for collision */
  for( index = 0 ; index < trirefcount ; index++ )
  {
    triindex = trireflist[ index ];
    if( ( triindex == ecd.trileft ) || ( triindex == ecd.triright ) )
      continue;
    tri = ADDRESS( mesh->trilist, triindex * mesh->trisize );
    if( tri->v[0] == -1 )
      continue;

#if DEBUG_VERBOSE_COLLISION
    printf( "    Tri %d : %d,%d,%d\n", index, tri->v[0], tri->v[1], tri->v[2] );
#endif

    if( tri->v[0] == vsrc )
    {
      left = 2;
      right = 1;
    }
    else if( tri->v[1] == vsrc )
    {
      left = 0;
      right = 2;
    }
    else if( tri->v[2] == vsrc )
    {
      left = 1;
      right = 0;
    }
    else
    {
      MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
      continue;
    }

    edge.v[0] = vdst;
    edge.v[1] = tri->v[right];
    mmHashLockCallEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge, mdEdgeCollisionCallback, &ecd, 0 );
    if( ecd.collisionflag )
      return 0;
    edge.v[0] = tri->v[left];
    edge.v[1] = vdst;
    mmHashLockCallEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge, mdEdgeCollisionCallback, &ecd, 0 );
    if( ecd.collisionflag )
      return 0;
  }

  return 1;
}



////



/* Mesh init step 0, allocate, NOT threaded */
static int mdMeshInit( mdMesh *mesh, size_t maxmemorysize )
{
  int retval;

  /* Allocate vertices, no extra room for vertices, we overwrite existing ones as we decimate */
  mesh->vertexlist = mmAlignAlloc( mesh->vertexalloc * sizeof(mdVertex), 0x40 );

  /* Allocate space for per-vertex lists of face references, including future vertices */
  mesh->trirefcount = 0;
  mesh->trirefalloc = 2 * 6 * mesh->tricount;
  mesh->trireflist = malloc( mesh->trirefalloc * sizeof(mdi) );

  /* Allocate triangles */
  mesh->trisize = ( sizeof(mdTriangle) + mesh->tridatasize + 0x7 ) & ~0x7;
  mesh->trilist = malloc( mesh->tricount * mesh->trisize );

  /* Allocate edge hash table */
  retval = 1;
  if( !( mesh->operationflags & MD_FLAGS_NO_DECIMATION ) )
    retval = mdMeshHashInit( mesh, mesh->tricount, 2.0, 7, maxmemorysize );

#if MD_CONFIG_ATOMIC_SUPPORT
  mmAtomicWrite32( &mesh->trireflock, 0x0 );
  mmAtomicWrite32( &mesh->globalvertexlock, 0x0 );
#else
  mtSpinInit( &mesh->trirefspinlock );
  mtSpinInit( &mesh->globalvertexspinlock );
#endif

  return retval;
}


/* Mesh init step 1, initialize vertices, threaded */
static void mdMeshInitVertices( mdMesh *mesh, mdThreadData *tdata, int threadcount )
{
  int vertexindex, vertexindexmax, vertexperthread;
  mdVertex *vertex;
  void *point;

  vertexperthread = ( mesh->vertexcount / threadcount ) + 1;
  vertexindex = tdata->threadid * vertexperthread;
  vertexindexmax = vertexindex + vertexperthread;
  if( vertexindexmax > mesh->vertexcount )
    vertexindexmax = mesh->vertexcount;

  vertex = &mesh->vertexlist[vertexindex];
  point = ADDRESS( mesh->point, vertexindex * mesh->pointstride );
  for( ; vertexindex < vertexindexmax ; vertexindex++, vertex++ )
  {
#if MD_CONFIG_ATOMIC_SUPPORT
    mmAtomicWrite32( &vertex->atomicowner, -1 );
#else
    vertex->owner = -1;
    mtSpinInit( &vertex->ownerspinlock );
#endif
    mesh->vertexUserToNative( vertex->point, point );
#if CPU_SSE_SUPPORT
    vertex->point[3] = 0.0;
#endif
    vertex->trirefcount = 0;
    vertex->redirectindex = -1;

    mathQuadricZero( &vertex->quadric );
    point = ADDRESS( point, mesh->pointstride );
  }

  return;
}


static inline void mdMeshForbidEdge( mdMesh *mesh, int v0, int v1 )
{
  int edgeflags;
  mdEdge edge;
  mdTriangle *tri;
  edge.v[0] = v0;
  edge.v[1] = v1;
  if( mmHashLockReadEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge ) == MM_HASH_SUCCESS )
  {
    tri = ADDRESS( mesh->trilist, edge.triindex * mesh->trisize );
    edgeflags = 0;
    if( tri->v[0] == v0 )
      edgeflags = ( tri->v[1] == v1 ? 0x10 : 0x40 );
    else if( tri->v[1] == v0 )
      edgeflags = ( tri->v[2] == v1 ? 0x20 : 0x10 );
    else if( tri->v[2] == v0 )
      edgeflags = ( tri->v[0] == v1 ? 0x40 : 0x20 );
    tri->u.edgeflags |= edgeflags;
  }
  return;
}


/* Mesh init step 2, initialize triangles, threaded */
static void mdMeshInitTriangles( mdMesh *mesh, mdThreadData *tdata, int threadcount )
{
  int i, triperthread, triindex, triindexmax;
  long buildtricount;
  void *indices, *tridata;
  mdTriangle *tri;
  mdVertex *vertex;
  mdEdge edge;
  mathQuadric q;

  triperthread = ( mesh->tricount / threadcount ) + 1;
  triindex = tdata->threadid * triperthread;
  triindexmax = triindex + triperthread;
  if( triindexmax > mesh->tricount )
    triindexmax = mesh->tricount;

  /* Initialize triangles */
  buildtricount = 0;
  indices = ADDRESS( mesh->indices, triindex * mesh->indicesstride );
  tridata = ADDRESS( mesh->tridata, triindex * mesh->tridatasize );
  tri = ADDRESS( mesh->trilist, triindex * mesh->trisize );
  edge.op = 0;
  for( ; triindex < triindexmax ; triindex++, indices = ADDRESS( indices, mesh->indicesstride ), tri = ADDRESS( tri, mesh->trisize ), tridata = ADDRESS( tridata, mesh->tridatasize ) )
  {
    mesh->indicesUserToNative( tri->v, indices );
#if DEBUG_VERBOSE_QUADRIC
    printf( "Triangle %d,%d,%d\n", (int)tri->v[0], (int)tri->v[1], (int)tri->v[2] );
#endif
    mdTriangleComputeQuadric( mesh, tri, &q );
    tri->u.edgeflags = 0;

    for( i = 0 ; i < 3 ; i++ )
    {
      vertex = &mesh->vertexlist[ tri->v[i] ];
#if DEBUG_VERBOSE_QUADRIC
      printf( "  Accum quadric to vertex %d\n", (int)tri->v[i] );
#endif
#if MD_CONFIG_ATOMIC_SUPPORT
      mmAtomicSpin32( &vertex->atomicowner, -1, tdata->threadid );
      mathQuadricAddQuadric( &vertex->quadric, &q );
      vertex->trirefcount++;
      mmAtomicWrite32( &vertex->atomicowner, -1 );
#else
      mtSpinLock( &vertex->ownerspinlock );
      mathQuadricAddQuadric( &vertex->quadric, &q );
      vertex->trirefcount++;
      mtSpinUnlock( &vertex->ownerspinlock );
#endif
    }
    if( mesh->tridatasize )
      memcpy( ADDRESS(tri,sizeof(mdTriangle)), tridata, mesh->tridatasize );

    if( !( mesh->operationflags & MD_FLAGS_NO_DECIMATION ) )
    {
      edge.triindex = triindex;
      edge.v[0] = tri->v[0];
      edge.v[1] = tri->v[1];
      if( mmHashLockAddEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge, 1 ) != MM_HASH_SUCCESS )
      {
/*
        MD_ERROR( "ERROR: Bad geometry, duplicate edge %d %d ; %s:%d\n", 1, (int)edge.v[0], (int)edge.v[1], __FILE__, __LINE__ );
*/
        tri->u.edgeflags |= 0x10;
        mdMeshForbidEdge( mesh, edge.v[0], edge.v[1] );
        mdMeshForbidEdge( mesh, edge.v[1], edge.v[0] );
        tdata->statuscollisioncount++;
      }
      edge.v[0] = tri->v[1];
      edge.v[1] = tri->v[2];
      if( mmHashLockAddEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge, 1 ) != MM_HASH_SUCCESS )
      {
/*
        MD_ERROR( "ERROR: Bad geometry, duplicate edge %d %d ; %s:%d\n", 1, (int)edge.v[0], (int)edge.v[1], __FILE__, __LINE__ );
*/
        tri->u.edgeflags |= 0x20;
        mdMeshForbidEdge( mesh, edge.v[0], edge.v[1] );
        mdMeshForbidEdge( mesh, edge.v[1], edge.v[0] );
        tdata->statuscollisioncount++;
      }
      edge.v[0] = tri->v[2];
      edge.v[1] = tri->v[0];
      if( mmHashLockAddEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge, 1 ) != MM_HASH_SUCCESS )
      {
/*
        MD_ERROR( "ERROR: Bad geometry, duplicate edge %d %d ; %s:%d\n", 1, (int)edge.v[0], (int)edge.v[1], __FILE__, __LINE__ );
*/
        tri->u.edgeflags |= 0x40;
        mdMeshForbidEdge( mesh, edge.v[0], edge.v[1] );
        mdMeshForbidEdge( mesh, edge.v[1], edge.v[0] );
        tdata->statuscollisioncount++;
      }
    }

    buildtricount++;
    tdata->statusbuildtricount = buildtricount;
  }

  return;
}


/* Mesh init step 3, initialize vertex trirefbase, NOT threaded */
static void mdMeshInitTrirefs( mdMesh *mesh )
{
  mdi vertexindex, trirefcount;
  mdVertex *vertex;

  /* Compute base of vertex triangle references */
  trirefcount = 0;
  vertex = mesh->vertexlist;
  for( vertexindex = 0 ; vertexindex < mesh->vertexcount ; vertexindex++, vertex++ )
  {
    vertex->trirefbase = trirefcount;
    trirefcount += vertex->trirefcount;
    vertex->trirefcount = 0;
  }
  mesh->trirefcount = trirefcount;

  return;
}


/* Accumulate quadrics from boundaries or weighted edges as returned by user callback */
static inline void mdMeshAccumBoundaryEdges( mdMesh *mesh, mdTriangle *tri, mdVertex **trivertex )
{
  int hashread;
  mdf edgeweight;
  mdEdge edge;
  mdTriangle *trilink;

  edge.v[0] = tri->v[1];
  edge.v[1] = tri->v[0];
  hashread = mmHashLockReadEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge );
  if( !( tri->u.edgeflags & 0x10 ) && ( hashread != MM_HASH_SUCCESS ) )
  {
#if DEBUG_VERBOSE_BOUNDARY
    printf( "Boundary %d,%d (%d)\n", tri->v[1], tri->v[0], tri->v[2] );
#endif
    edgeweight = mesh->boundaryweight;
    tri->u.edgeflags |= 0x1;
  }
  else if( ( mesh->edgeweight ) && ( hashread == MM_HASH_SUCCESS ) )
  {
    trilink = ADDRESS( mesh->trilist, edge.triindex * mesh->trisize );
    edgeweight = mesh->edgeweight( ADDRESS( tri, sizeof(mdTriangle) ), ADDRESS( trilink, sizeof(mdTriangle) ) );
    if( edgeweight <= 0.0 )
      goto skip01;
#if DEBUG_VERBOSE_BOUNDARY
    printf( "Edge weight %d,%d (%d) ; %f\n", tri->v[1], tri->v[0], tri->v[2], edgeweight );
#endif
  }
  else
    goto skip01;
  mdMeshAccumulateBoundary( trivertex[0], trivertex[1], trivertex[2], edgeweight );
  skip01:

  edge.v[0] = tri->v[2];
  edge.v[1] = tri->v[1];
  hashread = mmHashLockReadEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge );
  if( !( tri->u.edgeflags & 0x20 ) && ( hashread != MM_HASH_SUCCESS ) )
  {
#if DEBUG_VERBOSE_BOUNDARY
    printf( "Boundary %d,%d (%d)\n", tri->v[2], tri->v[1], tri->v[0] );
#endif
    edgeweight = mesh->boundaryweight;
    tri->u.edgeflags |= 0x2;
  }
  else if( ( mesh->edgeweight ) && ( hashread == MM_HASH_SUCCESS ) )
  {
    trilink = ADDRESS( mesh->trilist, edge.triindex * mesh->trisize );
    edgeweight = mesh->edgeweight( ADDRESS( tri, sizeof(mdTriangle) ), ADDRESS( trilink, sizeof(mdTriangle) ) );
    if( edgeweight <= 0.0 )
      goto skip12;
#if DEBUG_VERBOSE_BOUNDARY
    printf( "Edge weight %d,%d (%d) ; %f\n", tri->v[2], tri->v[1], tri->v[0], edgeweight );
#endif
  }
  else
    goto skip12;
  mdMeshAccumulateBoundary( trivertex[1], trivertex[2], trivertex[0], mesh->boundaryweight );
  skip12:

  edge.v[0] = tri->v[0];
  edge.v[1] = tri->v[2];
  hashread = mmHashLockReadEntry( mesh->edgehashtable, &mdEdgeHashEdge, &edge );
  if( !( tri->u.edgeflags & 0x40 ) && ( hashread != MM_HASH_SUCCESS ) )
  {
#if DEBUG_VERBOSE_BOUNDARY
    printf( "Boundary %d,%d (%d)\n", tri->v[0], tri->v[2], tri->v[1] );
#endif
    edgeweight = mesh->boundaryweight;
    tri->u.edgeflags |= 0x4;
  }
  else if( ( mesh->edgeweight ) && ( hashread == MM_HASH_SUCCESS ) )
  {
    trilink = ADDRESS( mesh->trilist, edge.triindex * mesh->trisize );
    edgeweight = mesh->edgeweight( ADDRESS( tri, sizeof(mdTriangle) ), ADDRESS( trilink, sizeof(mdTriangle) ) );
    if( edgeweight <= 0.0 )
      goto skip20;
#if DEBUG_VERBOSE_BOUNDARY
    printf( "Edge weight %d,%d (%d) ; %f\n", tri->v[0], tri->v[2], tri->v[1], edgeweight );
#endif
  }
  else
    goto skip20;
  mdMeshAccumulateBoundary( trivertex[2], trivertex[0], trivertex[1], mesh->boundaryweight );
  skip20:

  return;
}


/* Mesh init step 4, store vertex trirefs and accumulate boundary quadrics, threaded */
static void mdMeshBuildTrirefs( mdMesh *mesh, mdThreadData *tdata, int threadcount )
{
  int i, triperthread, triindex, triindexmax;
  long buildrefcount;
  mdTriangle *tri;
  mdVertex *vertex, *trivertex[3];

  triperthread = ( mesh->tricount / threadcount ) + 1;
  triindex = tdata->threadid * triperthread;
  triindexmax = triindex + triperthread;
  if( triindexmax > mesh->tricount )
    triindexmax = mesh->tricount;

  /* Store vertex triangle references and accumulate boundary quadrics */
  buildrefcount = 0;
  tri = ADDRESS( mesh->trilist, triindex * mesh->trisize );
  for( ; triindex < triindexmax ; triindex++, tri = ADDRESS( tri, mesh->trisize ) )
  {
    for( i = 0 ; i < 3 ; i++ )
    {
      vertex = &mesh->vertexlist[ tri->v[i] ];
#if MD_CONFIG_ATOMIC_SUPPORT
      mmAtomicSpin32( &vertex->atomicowner, -1, tdata->threadid );
      mesh->trireflist[ vertex->trirefbase + vertex->trirefcount++ ] = triindex;
      mmAtomicWrite32( &vertex->atomicowner, -1 );
#else
      mtSpinLock( &vertex->ownerspinlock );
      mesh->trireflist[ vertex->trirefbase + vertex->trirefcount++ ] = triindex;
      mtSpinUnlock( &vertex->ownerspinlock );
#endif
      trivertex[i] = vertex;
    }

    if( !( mesh->operationflags & MD_FLAGS_NO_DECIMATION ) )
      mdMeshAccumBoundaryEdges( mesh, tri, trivertex );

    buildrefcount++;
    tdata->statusbuildrefcount = buildrefcount;
  }

  return;
}


/* Mesh clean up */
static void mdMeshEnd( mdMesh *mesh )
{
#ifndef MD_CONFIG_ATOMIC_SUPPORT
  mdi index;
  mdVertex *vertex;
  vertex = mesh->vertexlist;
  for( index = 0 ; index < mesh->vertexcount ; index++, vertex++ )
    mtSpinDestroy( &vertex->ownerspinlock );
  mtSpinDestroy( &mesh->trirefspinlock );
  mtSpinDestroy( &mesh->globalvertexspinlock );
#endif
  mmAlignFree( mesh->vertexlist );
  free( mesh->trireflist );
  free( mesh->trilist );
  return;
}



////



static void mdSortOp( mdMesh *mesh, mdThreadData *tdata, mdOp *op, int denyflag )
{
  mdf collapsecost;
  collapsecost = op->value + op->penalty;
  if( ( denyflag ) || ( collapsecost > mesh->maxcollapsecost ) )
  {
#if MD_CONFIG_ATOMIC_SUPPORT
    if( !( mmAtomicRead32( &op->flags ) & MD_OP_FLAGS_DETACHED ) )
    {
      mmBinSortRemove( tdata->binsort, op, op->collapsecost );
      mmAtomicOr32( &op->flags, MD_OP_FLAGS_DETACHED );
    }
#else
    mtSpinLock( &op->spinlock );
    if( !( op->flags & MD_OP_FLAGS_DETACHED ) )
    {
      mmBinSortRemove( tdata->binsort, op, op->collapsecost );
      op->flags |= MD_OP_FLAGS_DETACHED;
    }
    mtSpinUnlock( &op->spinlock );
#endif
  }
  else
  {
#if MD_CONFIG_ATOMIC_SUPPORT
    if( mmAtomicRead32( &op->flags ) & MD_OP_FLAGS_DETACHED )
    {
      mmBinSortAdd( tdata->binsort, op, collapsecost );
      mmAtomicAnd32( &op->flags, ~MD_OP_FLAGS_DETACHED );
    }
    else if( op->collapsecost != collapsecost )
      mmBinSortUpdate( tdata->binsort, op, op->collapsecost, collapsecost );
#else
    mtSpinLock( &op->spinlock );
    if( op->flags & MD_OP_FLAGS_DETACHED )
    {
      mmBinSortAdd( tdata->binsort, op, collapsecost );
      op->flags &= ~MD_OP_FLAGS_DETACHED;
    }
    else if( op->collapsecost != collapsecost )
      mmBinSortUpdate( tdata->binsort, op, op->collapsecost, collapsecost );
    mtSpinUnlock( &op->spinlock );
#endif
    op->collapsecost = collapsecost;
  }

  return;
}


static void mdUpdateOp( mdMesh *mesh, mdThreadData *tdata, mdOp *op, int32_t opflagsmask )
{
  int denyflag, flags;
#if MD_CONFIG_ATOMIC_SUPPORT
  for( ; ; )
  {
    flags = mmAtomicRead32( &op->flags );
    if( mmAtomicCmpReplace32( &op->flags, flags, flags & opflagsmask ) )
      break;
  }
#else
  mtSpinLock( &op->spinlock );
  flags = op->flags;
  op->flags &= opflagsmask;
  mtSpinUnlock( &op->spinlock );
#endif
  if( !( flags & MD_OP_FLAGS_UPDATE_NEEDED ) )
    return;
  if( flags & MD_OP_FLAGS_DELETED )
    return;
  if( flags & MD_OP_FLAGS_DELETION_PENDING )
  {
    if( !( flags & MD_OP_FLAGS_DETACHED ) )
      mmBinSortRemove( tdata->binsort, op, op->collapsecost );
    /* Race condition, flag the op as deleted but don't free it ~ Free them all at the end with FreeAll(). */
    /*    mmBlockFree( &tdata->opblock, op );  */
#if MD_CONFIG_ATOMIC_SUPPORT
    mmAtomicOr32( &op->flags, MD_OP_FLAGS_DELETED );
#else
    mtSpinLock( &op->spinlock );
    op->flags |= MD_OP_FLAGS_DELETED;
    mtSpinUnlock( &op->spinlock );
#endif
  }
  else
  {
    op->penalty = mdEdgeCollapsePenalty( mesh, tdata, op->v0, op->v1, op->collapsepoint, &denyflag );
    mdSortOp( mesh, tdata, op, denyflag );
  }
  return;
}

static void mdUpdateBufferOps( mdMesh *mesh, mdThreadData *tdata, mdUpdateBuffer *updatebuffer, mdLockBuffer *lockbuffer )
{
  int index;
  mdOp *op;

#if MD_CONFIG_ATOMIC_SUPPORT
  mmAtomicSpin32( &updatebuffer->atomlock, 0x0, 0x1 );
#else
  mtSpinLock( &updatebuffer->spinlock );
#endif
  for( index = 0 ; index < updatebuffer->opcount ; index++ )
  {
    op = updatebuffer->opbuffer[index];
    if( mdOpResolveLockEdgeTry( mesh, tdata, lockbuffer, op ) )
    {
      mdUpdateOp( mesh, tdata, op, ~( MD_OP_FLAGS_UPDATE_QUEUED | MD_OP_FLAGS_UPDATE_NEEDED ) );
      mdLockBufferUnlockAll( mesh, tdata, lockbuffer );
    }
    else
    {
#if MD_CONFIG_ATOMIC_SUPPORT
      mmAtomicWrite32( &updatebuffer->atomlock, 0x0 );
#else
      mtSpinUnlock( &updatebuffer->spinlock );
#endif
      mdOpResolveLockEdge( mesh, tdata, lockbuffer, op );
      mdUpdateOp( mesh, tdata, op, ~( MD_OP_FLAGS_UPDATE_QUEUED | MD_OP_FLAGS_UPDATE_NEEDED ) );
      mdLockBufferUnlockAll( mesh, tdata, lockbuffer );
#if MD_CONFIG_ATOMIC_SUPPORT
      mmAtomicSpin32( &updatebuffer->atomlock, 0x0, 0x1 );
#else
      mtSpinLock( &updatebuffer->spinlock );
#endif
    }
  }
  updatebuffer->opcount = 0;
#if MD_CONFIG_ATOMIC_SUPPORT
  mmAtomicWrite32( &updatebuffer->atomlock, 0x0 );
#else
  mtSpinUnlock( &updatebuffer->spinlock );
#endif

  return;
}


static int mdMeshProcessQueue( mdMesh *mesh, mdThreadData *tdata )
{
  int index, decimationcount, stepindex;
  int32_t opflags;
  mdf maxcost;
  mdOp *op;
  mdLockBuffer lockbuffer;
#if DEBUG_LIMIT
  int limit = DEBUG_LIMIT;
#endif

  mdLockBufferInit( &lockbuffer, 2 );

  stepindex = 0;
  maxcost = 0.0;

  decimationcount = 0;
  for( ; ; )
  {
    /* Update all ops flagged as requiring update */
    if( mesh->operationflags & MD_FLAGS_CONTINUOUS_UPDATE )
    {
      for( index = 0 ; index < mesh->updatebuffercount ; index++ )
        mdUpdateBufferOps( mesh, tdata, &tdata->updatebuffer[index], &lockbuffer );
    }

    /* Acquire first op */
    op = mmBinSortGetFirstItem( tdata->binsort, maxcost );
    if( !( op ) )
    {
      if( ++stepindex >= mesh->syncstepcount )
        break;
      maxcost = mesh->maxcollapsecost * ( (mdf)stepindex / (mdf)mesh->syncstepcount );
      mdBarrierSync( &mesh->workbarrier );
      /* Update all ops flagged as requiring update */
      if( !( mesh->operationflags & MD_FLAGS_CONTINUOUS_UPDATE ) )
      {
        for( index = 0 ; index < mesh->updatebuffercount ; index++ )
          mdUpdateBufferOps( mesh, tdata, &tdata->updatebuffer[index], &lockbuffer );
      }
      continue;
    }

#if DEBUG_VERBOSE || DEBUG_VERBOSE_COST
 #if MD_CONFIG_ATOMIC_SUPPORT
    printf( "Op %p ; Edge %d,%d (0x%x) ; Point %f %f %f ; Value %f ; Penalty %f ; Cost %f\n", op, op->v0, op->v1, mmAtomicRead32( &op->flags ), op->collapsepoint[0], op->collapsepoint[1], op->collapsepoint[2], op->value, op->penalty, op->collapsecost );
 #else
    printf( "Op %p ; Edge %d,%d (0x%x) ; Point %f %f %f ; Value %f ; Penalty %f ; Cost %f\n", op, op->v0, op->v1, op->flags, op->collapsepoint[0], op->collapsepoint[1], op->collapsepoint[2], op->value, op->penalty, op->collapsecost );
 #endif
#endif

    /* Acquire lock for op edge and all trirefs vertices */
    mdOpResolveLockFull( mesh, tdata, &lockbuffer, op );

    /* If our op was flagged for update between mdUpdateBufferOps() and before we acquired lock, no big deal, catch the update */
#if MD_CONFIG_ATOMIC_SUPPORT
    opflags = mmAtomicRead32( &op->flags );
#else
    mtSpinLock( &op->spinlock );
    opflags = op->flags;
    mtSpinUnlock( &op->spinlock );
#endif
    if( opflags & MD_OP_FLAGS_UPDATE_NEEDED )
    {
      mdLockBufferUnlockAll( mesh, tdata, &lockbuffer );
      mdUpdateOp( mesh, tdata, op, ~MD_OP_FLAGS_UPDATE_NEEDED );
      continue;
    }

#if DEBUG_PENALTY_CHECK
    int denyflag;
    mdf penalty;
    penalty = mdEdgeCollapsePenalty( mesh, tdata, op->v0, op->v1, op->collapsepoint, &denyflag );
    if( fabs( penalty - op->penalty ) > 0.001 * fmax( penalty, op->penalty ) )
      printf( "CRAP : %f %f\n", penalty, op->penalty );
#endif

    if( !( mdEdgeCollisionCheck( mesh, tdata, op->v0, op->v1 ) ) )
    {
#if MD_CONFIG_ATOMIC_SUPPORT
      if( mmAtomicRead32( &op->flags ) & MD_OP_FLAGS_DETACHED )
        MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
      mmAtomicOr32( &op->flags, MD_OP_FLAGS_DETACHED );
      mmBinSortRemove( tdata->binsort, op, op->collapsecost );
#else
      mtSpinLock( &op->spinlock );
      if( op->flags & MD_OP_FLAGS_DETACHED )
        MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
      op->flags |= MD_OP_FLAGS_DETACHED;
      mtSpinUnlock( &op->spinlock );
      mmBinSortRemove( tdata->binsort, op, op->collapsecost );
#endif
      goto opdone;
    }

    mdEdgeCollapse( mesh, tdata, op->v0, op->v1, op->collapsepoint );
    decimationcount++;

#if DEBUG_LIMIT
    if( decimationcount >= limit )
      break;
#endif

    opdone:
    /* Release all locks for op */
    mdLockBufferUnlockAll( mesh, tdata, &lockbuffer );
  }

  mdLockBufferEnd( &lockbuffer );

#if DEBUG_VERBOSE
  printf( "Final Count of Collapses : %d\n", decimationcount );
#endif

  return decimationcount;
}



//////



typedef struct
{
  mdf normal[3];
  mdf factor[3];
} mdTriNormal;


static mdi mdMeshPackCountTriangles( mdMesh *mesh )
{
  mdi tricount;
  mdTriangle *tri, *triend;

  tricount = 0;
  tri = mesh->trilist;
  triend = ADDRESS( tri, mesh->tricount * mesh->trisize );
  for( ; tri < triend ; tri = ADDRESS( tri, mesh->trisize ) )
  {
    if( tri->v[0] == -1 )
      continue;
    tri->u.redirectindex = tricount;
    tricount++;
  }

  mesh->tripackcount = tricount;
  return tricount;
}

static mdf mdMeshAngleFactor( mdf dotangle )
{
  mdf factor;
  if( dotangle >= 1.0 )
    factor = 0.0;
  else if( dotangle <= -1.0 )
    factor = 0.5 * M_PI;
  else
  {
    factor = mdfacos( dotangle );
    if( isnan( factor ) )
      factor = 0.0;
  }
  return factor;
}

static void mdMeshBuildTriangleNormals( mdMesh *mesh )
{
  mdTriangle *tri, *triend;
  mdVertex *vertex0, *vertex1, *vertex2;
  mdf vecta[3], vectb[3], vectc[3], normalfactor, magna, magnb, magnc, norm, norminv;
  mdTriNormal *trinormal;

  trinormal = mesh->trinormal;

  normalfactor = 1.0;
  if( mesh->operationflags & MD_FLAGS_TRIANGLE_WINDING_CCW )
    normalfactor = -1.0;

  tri = mesh->trilist;
  triend = ADDRESS( tri, mesh->tricount * mesh->trisize );
  for( ; tri < triend ; tri = ADDRESS( tri, mesh->trisize ) )
  {
    if( tri->v[0] == -1 )
      continue;

    /* Compute triangle normal */
    vertex0 = &mesh->vertexlist[ tri->v[0] ];
    vertex1 = &mesh->vertexlist[ tri->v[1] ];
    vertex2 = &mesh->vertexlist[ tri->v[2] ];
    MD_VectorSubStore( vecta, vertex1->point, vertex0->point );
    MD_VectorSubStore( vectb, vertex2->point, vertex0->point );
    MD_VectorCrossProduct( trinormal->normal, vectb, vecta );

    norm = mdfsqrt( MD_VectorDotProduct( trinormal->normal, trinormal->normal ) );
    if( norm )
    {
      norminv = normalfactor / norm;
      trinormal->normal[0] *= norminv;
      trinormal->normal[1] *= norminv;
      trinormal->normal[2] *= norminv;
    }

    MD_VectorSubStore( vectc, vertex2->point, vertex1->point );
    magna = MD_VectorMagnitude( vecta );
    magnb = MD_VectorMagnitude( vectb );
    magnc = MD_VectorMagnitude( vectc );
    trinormal->factor[0] = norm * mdMeshAngleFactor(  MD_VectorDotProduct( vecta, vectb ) / ( magna * magnb ) );
    trinormal->factor[1] = norm * mdMeshAngleFactor( -MD_VectorDotProduct( vecta, vectc ) / ( magna * magnc ) );
    trinormal->factor[2] = norm * mdMeshAngleFactor(  MD_VectorDotProduct( vectb, vectc ) / ( magnb * magnc ) );

    trinormal++;
  }

  return;
}


static int mdMeshVertexComputeNormal( mdMesh *mesh, mdi vertexindex, mdi *trireflist, int trirefcount, mdf *normal )
{
  int index, pivot, validflag;
  mdi triindex;
  mdf norm, norminv;
  mdTriangle *tri;
  mdTriNormal *trinormal, *tn;

  trinormal = mesh->trinormal;

  /* Loop through all trirefs associated with the vertex */
  validflag = 0;
  MD_VectorZero( normal );
  for( index = 0 ; index < trirefcount ; index++ )
  {
    triindex = trireflist[ index ];
    if( triindex == -1 )
      continue;
    tri = ADDRESS( mesh->trilist, triindex * mesh->trisize );
    if( tri->v[0] == -1 )
      continue;

    if( tri->v[0] == vertexindex )
      pivot = 0;
    else if( tri->v[1] == vertexindex )
      pivot = 1;
    else if( tri->v[2] == vertexindex )
      pivot = 2;
    else
    {
      MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
      continue;
    }

    tn = &trinormal[ tri->u.redirectindex ];
    MD_VectorAddMulScalar( normal, tn->normal, tn->factor[pivot] );
    validflag = 1;
  }

  if( !( validflag ) )
    return 0;

  norm = mdfsqrt( MD_VectorDotProduct( normal, normal ) );
  if( norm )
  {
    norminv = 1.0 / norm;
    normal[0] *= norminv;
    normal[1] *= norminv;
    normal[2] *= norminv;
  }

  return 1;
}


static mdi mdMeshCloneVertex( mdMesh *mesh, mdi cloneindex, mdf *point )
{
  mdi vertexindex, retindex;
  mdVertex *vertex;

  retindex = -1;
  vertex = &mesh->vertexlist[ mesh->clonesearchindex ];
  for( vertexindex = mesh->clonesearchindex ; vertexindex < mesh->vertexalloc ; vertexindex++, vertex++ )
  {
    if( ( vertexindex < mesh->vertexcount ) && ( vertex->trirefcount ) )
      continue;
    vertex->trirefcount = -1;
    vertex->redirectindex = -1;
    /* Copy the point from the cloned vertex */
    MD_VectorCopy( vertex->point, point );
    /* Copy custom vertex attributes, if any */
    if( mesh->vertexcopy )
      mesh->vertexcopy( mesh->copycontext, vertexindex, cloneindex );
    retindex = vertexindex;
    if( vertexindex >= mesh->vertexcount )
      mesh->vertexcount = vertexindex+1;
    break;
  }
  mesh->clonesearchindex = vertexindex;
  return retindex;
}


static void mdMeshVertexRedirectTriRefs( mdMesh *mesh, mdi vertexindex, mdi newvertexindex, mdi *trireflist, int trirefcount )
{
  int index;
  mdi triindex;
  mdTriangle *tri;

  for( index = 0 ; index < trirefcount ; index++ )
  {
    triindex = trireflist[ index ];
    if( triindex == -1 )
      continue;
    tri = ADDRESS( mesh->trilist, triindex * mesh->trisize );
    if( tri->v[0] == -1 )
      continue;
    if( tri->v[0] == vertexindex )
      tri->v[0] = newvertexindex;
    else if( tri->v[1] == vertexindex )
      tri->v[1] = newvertexindex;
    else if( tri->v[2] == vertexindex )
      tri->v[2] = newvertexindex;
    else
      MD_ERROR( "SHOULD NOT HAPPEN %s:%d\n", 1, __FILE__, __LINE__ );
  }

  return;
}


/* Find a target normal */
static int mdMeshVertexFindTarget( mdMesh *mesh, mdi *trireflist, int trirefcount, mdf **targetnormal )
{
  int i0, i1;
  mdi triindex0, triindex1;
  mdf dotangle, bestdotangle;
  mdTriangle *tri0, *tri1;
  mdTriNormal *trinormal, *tn0, *tn1;

  /* Of all triangles, find the most diverging pair, pick one */
  targetnormal[0] = 0;
  targetnormal[1] = 0;
  trinormal = mesh->trinormal;
  bestdotangle = mesh->normalsearchangle;
  for( i0 = 0 ; i0 < trirefcount ; i0++ )
  {
    triindex0 = trireflist[ i0 ];
    if( triindex0 == -1 )
      continue;
    tri0 = ADDRESS( mesh->trilist, triindex0 * mesh->trisize );
    if( tri0->v[0] == -1 )
      continue;
    tn0 = &trinormal[ tri0->u.redirectindex ];
    for( i1 = i0+1 ; i1 < trirefcount ; i1++ )
    {
      triindex1 = trireflist[ i1 ];
      if( triindex1 == -1 )
        continue;
      tri1 = ADDRESS( mesh->trilist, triindex1 * mesh->trisize );
      if( tri1->v[0] == -1 )
        continue;
      tn1 = &trinormal[ tri1->u.redirectindex ];
      dotangle = MD_VectorDotProduct( tn0->normal, tn1->normal );
      if( dotangle < bestdotangle )
      {
        bestdotangle = dotangle;
        targetnormal[0] = tn0->normal;
        targetnormal[1] = tn1->normal;
      }
    }
  }

  return ( targetnormal[0] != 0 );
}



#define MD_MESH_TRIREF_MAX (256)

static int mdMeshVertexBuildNormal( mdMesh *mesh, mdi vertexindex, mdi *trireflist, int trirefcount, mdf *point, mdf *normal )
{
  int index, trirefbuffercount;
  mdi triindex, newvertexindex;
  mdi trirefbuffer[MD_MESH_TRIREF_MAX];
  mdf dotangle0, dotangle1;
  mdf *newnormal, *targetnormal[2];
  mdTriangle *tri;
  mdTriNormal *trinormal, *tn;

  if( trirefcount > MD_MESH_TRIREF_MAX )
    return 1;

  /* Loop to repeat as we retire trirefs from the list */
  trinormal = mesh->trinormal;
  for( ; ; )
  {
    /* Compute normal for vertex */
    if( !( mdMeshVertexComputeNormal( mesh, vertexindex, trireflist, trirefcount, normal ) ) )
      return 0;

    /* If user doesn't allow vertex splitting, take the normal as it is */
    if( !( mesh->operationflags & MD_FLAGS_NORMAL_VERTEX_SPLITTING ) )
      break;

    /* Find a pair of target normals */
    if( !( mdMeshVertexFindTarget( mesh, trireflist, trirefcount, targetnormal ) ) )
      break;

    /* Find all trirefs that agree with targetnormal[1] and store them independently */
    trirefbuffercount = 0;
    for( index = 0 ; index < trirefcount ; index++ )
    {
      triindex = trireflist[ index ];
      if( triindex == -1 )
        continue;
      tri = ADDRESS( mesh->trilist, triindex * mesh->trisize );
      if( tri->v[0] == -1 )
        continue;
      tn = &trinormal[ tri->u.redirectindex ];
      dotangle1 = MD_VectorDotProduct( targetnormal[1], tn->normal );
      if( dotangle1 < mesh->normalsearchangle )
        continue;
      dotangle0 = MD_VectorDotProduct( targetnormal[0], tn->normal );
      if( dotangle0 > dotangle1 )
        continue;
      trirefbuffer[trirefbuffercount++] = triindex;
      trireflist[ index ] = -1;
    }
    if( !( trirefbuffercount ) )
      break;

    /* Find an unused vertex, bail out if none can be found */
    newvertexindex = mdMeshCloneVertex( mesh, vertexindex, point );
    if( newvertexindex == -1 )
      break;

    /* Correct all trirefs to new vertex */
    mdMeshVertexRedirectTriRefs( mesh, vertexindex, newvertexindex, trirefbuffer, trirefbuffercount );

    /* Spawn a new vertex */
    newnormal = ADDRESS( mesh->vertexnormal, newvertexindex * 3 * sizeof(mdf) );
    mdMeshVertexBuildNormal( mesh, newvertexindex, trirefbuffer, trirefbuffercount, point, newnormal );
  }

  return 1;
}


/* In some rare circumstances, a vertex can be unused even with redirectindex == -1 and trirefs leading to deleted triangles */
static int mdMeshVertexCheckUse( mdMesh *mesh, mdi *trireflist, int trirefcount )
{
  int index;
  mdi triindex;
  mdTriangle *tri;
  for( index = 0 ; index < trirefcount ; index++ )
  {
    triindex = trireflist[ index ];
    if( triindex == -1 )
      continue;
    tri = ADDRESS( mesh->trilist, triindex * mesh->trisize );
    if( tri->v[0] == -1 )
      continue;
    return 1;
  }
  return 0;
}


static void mdMeshWriteVertices( mdMesh *mesh )
{
  mdi vertexindex, writeindex;
  mdf *point;
  mdVertex *vertex;

  point = mesh->point;
  writeindex = 0;
  vertex = mesh->vertexlist;
  for( vertexindex = 0 ; vertexindex < mesh->vertexcount ; vertexindex++, vertex++ )
  {
    if( !( mesh->operationflags & MD_FLAGS_NO_VERTEX_PACKING ) )
    {
      if( vertex->redirectindex != -1 )
        continue;
      if( !( vertex->trirefcount ) )
        continue;
      if( ( vertex->trirefcount != -1 ) && !( mdMeshVertexCheckUse( mesh, &mesh->trireflist[ vertex->trirefbase ], vertex->trirefcount ) ) )
        continue;
    }
    vertex->redirectindex = writeindex;
    mesh->vertexNativeToUser( point, vertex->point );
    if( ( mesh->vertexcopy ) && ( writeindex != vertexindex  ) )
      mesh->vertexcopy( mesh->copycontext, writeindex, vertexindex );
    point = ADDRESS( point, mesh->pointstride );
    writeindex++;
  }
  mesh->vertexpackcount = writeindex;
  if( mesh->operationflags & MD_FLAGS_NO_VERTEX_PACKING )
    mesh->vertexpackcount = mesh->vertexcount;
  return;
}


static void mdMeshWriteIndices( mdMesh *mesh )
{
  mdi finaltricount, v[3];
  mdTriangle *tri, *triend;
  mdVertex *vertex0, *vertex1, *vertex2;
  void *indices, *tridata;

  indices = mesh->indices;
  finaltricount = 0;
  tri = mesh->trilist;
  triend = ADDRESS( tri, mesh->tricount * mesh->trisize );
  tridata = mesh->tridata;
  for( ; tri < triend ; tri = ADDRESS( tri, mesh->trisize ), tridata = ADDRESS( tridata, mesh->tridatasize ) )
  {
    if( tri->v[0] == -1 )
      continue;
    vertex0 = &mesh->vertexlist[ tri->v[0] ];
    v[0] = vertex0->redirectindex;
    vertex1 = &mesh->vertexlist[ tri->v[1] ];
    v[1] = vertex1->redirectindex;
    vertex2 = &mesh->vertexlist[ tri->v[2] ];
    v[2] = vertex2->redirectindex;
    mesh->indicesNativeToUser( indices, v );
    if( mesh->tridatasize )
      memcpy( tridata, ADDRESS(tri,sizeof(mdTriangle)), mesh->tridatasize );
    indices = ADDRESS( indices, mesh->indicesstride );
    finaltricount++;
  }

  mesh->tripackcount = finaltricount;
  return;
}


/* Write vertices and indices, recompute normals, store them along with vertices and indices at once */
static void mdMeshWriteVerticesAndNormals( mdMesh *mesh )
{
  mdi vertexindex, writeindex;
  mdf *point, *normal;
  mdVertex *vertex;
  void (*writenormal)( void *dst, mdf *src );
  void *normaldst;

  /* Start search for free vertices to clone at 0 */
  mesh->clonesearchindex = 0;
  mesh->trinormal = malloc( mesh->tripackcount * sizeof(mdTriNormal) );
  mesh->vertexnormal = malloc( mesh->vertexalloc * 3 * sizeof(normal) );

  /* Count triangles and assign redirectindex to each in sequence */
  mdMeshPackCountTriangles( mesh );

  /* Build up mesh->trinormal, store normals, area and vertex angles of each triangle */
  mdMeshBuildTriangleNormals( mesh );

  /* Build each vertex normal */
  vertex = mesh->vertexlist;
  for( vertexindex = 0 ; vertexindex < mesh->vertexcount ; vertexindex++, vertex++ )
  {
    if( !( vertex->trirefcount ) || ( vertex->trirefcount == -1 ) )
      continue;
    normal = ADDRESS( mesh->vertexnormal, vertexindex * 3 * sizeof(mdf) );
    if( !( mdMeshVertexBuildNormal( mesh, vertexindex, &mesh->trireflist[ vertex->trirefbase ], vertex->trirefcount, vertex->point, normal ) ) )
      vertex->trirefcount = 0;
  }

  /* Write vertices along with normals and other attributes */
  writenormal = mesh->writenormal;
  point = mesh->point;
  writeindex = 0;
  vertex = mesh->vertexlist;
  for( vertexindex = 0 ; vertexindex < mesh->vertexcount ; vertexindex++, vertex++ )
  {
    if( !( mesh->operationflags & MD_FLAGS_NO_VERTEX_PACKING ) )
    {
      if( vertex->redirectindex != -1 )
        continue;
      if( !( vertex->trirefcount ) )
        continue;
    }
    vertex->redirectindex = writeindex;
    mesh->vertexNativeToUser( point, vertex->point );
    normal = ADDRESS( mesh->vertexnormal, vertexindex * 3 * sizeof(mdf) );
    normaldst = ADDRESS( mesh->normalbase, writeindex * mesh->normalstride );
    writenormal( normaldst, normal );
    if( ( mesh->vertexcopy ) && ( writeindex != vertexindex  ) )
      mesh->vertexcopy( mesh->copycontext, writeindex, vertexindex );
    point = ADDRESS( point, mesh->pointstride );
    writeindex++;
  }

  mesh->vertexpackcount = writeindex;
  if( mesh->operationflags & MD_FLAGS_NO_VERTEX_PACKING )
    mesh->vertexpackcount = mesh->vertexcount;

  free( mesh->vertexnormal );
  free( mesh->trinormal );

  return;
}



//////



typedef struct
{
  int threadid;
  mdMesh *mesh;
  long deletioncount;
  long collisioncount;
  long decimationcount;
  mdThreadData *tdata;
  int stage;
} mdThreadInit;

#ifndef MD_CONFIG_ATOMIC_SUPPORT
int mdFreeOpCallback( void *chunk, void *userpointer )
{
  mdOp *op;
  op = chunk;
  mtSpinDestroy( &op->spinlock );
  return 0;
}
#endif


static void *mdThreadMain( void *value )
{
  int index, tribase, trimax, triperthread, nodeindex;
  int groupthreshold;
  mdThreadInit *tinit;
  mdThreadData tdata;
  mdMesh *mesh;

  tinit = value;
  mesh = tinit->mesh;
  tinit->tdata = &tdata;

  /* Thread memory initialization */
  tdata.threadid = tinit->threadid;
  tdata.statusbuildtricount = 0;
  tdata.statusbuildrefcount = 0;
  tdata.statuspopulatecount = 0;
  tdata.statusdeletioncount = 0;
  tdata.statuscollisioncount = 0;
  groupthreshold = mesh->tricount >> 9;
  if( groupthreshold < 256 )
    groupthreshold = 256;

  nodeindex = -1;
  if( mmcontext.numaflag )
  {
    mmThreadBindToCpu( tdata.threadid );
    nodeindex = mmCpuGetNode( tdata.threadid );
    mmBlockNodeInit( &tdata.opblock, nodeindex, sizeof(mdOp), 16384, 16384, MD_CONF_OP_ALIGNMENT );
  }
  else
    mmBlockInit( &tdata.opblock, sizeof(mdOp), 16384, 16384, MD_CONF_OP_ALIGNMENT );

  tdata.binsort = mmBinSortInit( offsetof(mdOp,list), 64, 16, -0.2 * mesh->maxcollapsecost, 1.2 * mesh->maxcollapsecost, groupthreshold, mdMeshOpValueCallback, 6, nodeindex );
  for( index = 0 ; index < mesh->updatebuffercount ; index++ )
    mdUpdateBufferInit( &tdata.updatebuffer[index], 4096 );

  /* Wait until all threads have properly initialized */
  if( mesh->updatestatusflag )
    mdBarrierSync( &mesh->globalbarrier );

  /* Build mesh step 1 */
  if( !( tdata.threadid ) )
    tinit->stage = MD_STATUS_STAGE_BUILDVERTICES;
  mdMeshInitVertices( mesh, &tdata, mesh->threadcount );
  mdBarrierSync( &mesh->workbarrier );

  /* Build mesh step 2 */
  if( !( tdata.threadid ) )
    tinit->stage = MD_STATUS_STAGE_BUILDTRIANGLES;
  mdMeshInitTriangles( mesh, &tdata, mesh->threadcount );
  mdBarrierSync( &mesh->globalbarrier );

  /* Build mesh step 3 is not parallel, have the main thread run it */
  if( !( tdata.threadid ) )
    tinit->stage = MD_STATUS_STAGE_BUILDTRIREFS;
  mdBarrierSync( &mesh->globalbarrier );

  /* Build mesh step 4 */
  mdMeshBuildTrirefs( mesh, &tdata, mesh->threadcount );
  mdBarrierSync( &mesh->workbarrier );

  if( !( mesh->operationflags & MD_FLAGS_NO_DECIMATION ) )
  {
    /* Initialize the thread's op queue */
    if( !( tdata.threadid ) )
      tinit->stage = MD_STATUS_STAGE_BUILDQUEUE;
    triperthread = ( mesh->tricount / mesh->threadcount ) + 1;
    tribase = tdata.threadid * triperthread;
    trimax = tribase + triperthread;
    if( trimax > mesh->tricount )
      trimax = mesh->tricount;
    /* Initialize a list of ops for all edges */
    mdMeshPopulateOpList( mesh, &tdata, tribase, trimax - tribase );

    /* Wait for all threads to reach this point */
    mdBarrierSync( &mesh->workbarrier );

    /* Process the thread's op queue */
    if( !( tdata.threadid ) )
      tinit->stage = MD_STATUS_STAGE_DECIMATION;
    tinit->decimationcount = mdMeshProcessQueue( mesh, &tdata );
  }

  /* Wait for all threads to reach this point */
  tinit->deletioncount = tdata.statusdeletioncount;
  tinit->collisioncount = tdata.statuscollisioncount;
  mdBarrierSync( &mesh->globalbarrier );

  /* If we didn't use atomic operations, we have spinlocks to destroy in each op */
#ifndef MD_CONFIG_ATOMIC_SUPPORT
  mmBlockProcessList( &tdata.opblock, 0, mdFreeOpCallback );
#endif

  /* Free thread memory allocations */
  mmBlockFreeAll( &tdata.opblock );
  for( index = 0 ; index < mesh->updatebuffercount ; index++ )
    mdUpdateBufferEnd( &tdata.updatebuffer[index] );
  mmBinSortFree( tdata.binsort );

  return 0;
}



//////////



static const char *mdStatusStageName[] =
{
 [MD_STATUS_STAGE_INIT] = "Initializing",
 [MD_STATUS_STAGE_BUILDVERTICES] = "Building Vertices",
 [MD_STATUS_STAGE_BUILDTRIANGLES] = "Building Triangles",
 [MD_STATUS_STAGE_BUILDTRIREFS] = "Building Trirefs",
 [MD_STATUS_STAGE_BUILDQUEUE] = "Building Queues",
 [MD_STATUS_STAGE_DECIMATION] = "Decimating Mesh",
 [MD_STATUS_STAGE_STORE] = "Storing Geometry",
 [MD_STATUS_STAGE_DONE] = "Done"
};

static double mdStatusStageProgress[] =
{
 [MD_STATUS_STAGE_INIT] = 0.0,
 [MD_STATUS_STAGE_BUILDVERTICES] = 2.0,
 [MD_STATUS_STAGE_BUILDTRIANGLES] = 6.0,
 [MD_STATUS_STAGE_BUILDTRIREFS] = 6.0,
 [MD_STATUS_STAGE_BUILDQUEUE] = 8.0,
 [MD_STATUS_STAGE_DECIMATION] = 75.0,
 [MD_STATUS_STAGE_STORE] = 3.0,
 [MD_STATUS_STAGE_DONE] = 0.0
};

static void mdUpdateStatus( mdMesh *mesh, mdThreadInit *threadinit, int stage, mdStatus *status )
{
  int threadid, stageindex;
  long buildtricount, buildrefcount, populatecount, deletioncount;
  double progress, subprogress;
  mdThreadInit *tinit;
  mdThreadData *tdata;

  subprogress = 0.0;
  if( !( threadinit ) )
    status->stage = stage;
  else
  {
    tinit = &threadinit[0];
    status->stage = tinit->stage;

    tdata = tinit->tdata;
    if( (unsigned)status->stage >= MD_STATUS_STAGE_COUNT )
      return;
    buildtricount = 0;
    buildrefcount = 0;
    populatecount = 0;
    deletioncount = 0;
    for( threadid = 0 ; threadid < mesh->threadcount ; threadid++ )
    {
      tinit = &threadinit[threadid];
      tdata = tinit->tdata;
      buildtricount += tdata->statusbuildtricount;
      buildrefcount += tdata->statusbuildrefcount;
      populatecount += tdata->statuspopulatecount;
      deletioncount += tdata->statusdeletioncount;
    }
    status->trianglecount = mesh->tricount - deletioncount;
    if( status->stage == MD_STATUS_STAGE_DECIMATION )
      subprogress = 1.0 - ( (double)status->trianglecount / (double)mesh->tricount );
    else if( status->stage == MD_STATUS_STAGE_BUILDQUEUE )
      subprogress = (double)populatecount / (double)mesh->tricount;
    else if( status->stage == MD_STATUS_STAGE_BUILDTRIANGLES )
      subprogress = (double)buildtricount / (double)mesh->tricount;
    else if( status->stage == MD_STATUS_STAGE_BUILDTRIREFS )
      subprogress = (double)buildrefcount / (double)mesh->tricount;
    subprogress = fmax( 0.0, fmin( 1.0, subprogress ) );
  }

  progress = 0.0;
  status->stagename = mdStatusStageName[status->stage];
  for( stageindex = 0 ; stageindex < status->stage ; stageindex++ )
    progress += mdStatusStageProgress[stageindex];

  progress += subprogress * mdStatusStageProgress[status->stage];
  if( progress > status->progress )
    status->progress = progress;

  return;
}



//////////



void mdOperationInit( mdOperation *op )
{
  /* Input */
  memset( op, 0, sizeof(mdOperation) );

  /* Advanced settings, default values */
  op->compactnesstarget = MD_COLLAPSE_COST_COMPACT_TARGET;
  op->compactnesspenalty = MD_COLLAPSE_COST_COMPACT_FACTOR;
  op->boundaryweight = MD_BOUNDARY_WEIGHT;
  op->syncstepcount = MD_SYNC_STEP_COUNT;
  op->normalsearchangle = 45.0;
  mmInit();
  if( mmcontext.sysmemory )
  {
    op->maxmemorysize = ( mmcontext.sysmemory >> 1 ) + ( mmcontext.sysmemory >> 2 ); /* By default, allow to allocate up to 75% of system memory */
    if( op->maxmemorysize < 1024*1024*1024 )
      op->maxmemorysize = 1024*1024*1024;
  }

  return;
}

void mdOperationData( mdOperation *op, size_t vertexcount, void *vertex, int vertexformat, size_t vertexstride, size_t tricount, void *indices, int indicesformat, size_t indicesstride )
{
  op->vertexcount = vertexcount;
  op->vertex = vertex;
  op->vertexformat = vertexformat;
  op->vertexstride = vertexstride;
  op->indices = indices;
  op->indicesformat = indicesformat;
  op->indicesstride = indicesstride;
  op->tricount = tricount;
  return;
}

void mdOperationStrength( mdOperation *op, double decimationstrength )
{
  /* pow( decimationstrength/4.0, 6.0 ) */
  decimationstrength *= 0.25;
  decimationstrength *= decimationstrength;
  op->decimationstrength = decimationstrength * decimationstrength * decimationstrength;
  return;
}

void mdOperationTriData( mdOperation *op, void *tridata, size_t tridatasize, double (*edgeweight)( void *tridata0, void *tridata1 ) )
{
  op->tridata = tridata;
  op->tridatasize = tridatasize;
  op->edgeweight = edgeweight;
  return;
}

void mdOperationVertexCopy( mdOperation *op, void (*vertexcopy)( void *copycontext, int dstindex, int srcindex ), void *copycontext )
{
  op->vertexcopy = vertexcopy;
  op->copycontext = copycontext;
  return;
}

void mdOperationVertexMerge( mdOperation *op, void (*vertexmerge)( void *mergecontext, int dstindex, int srcindex, double dstfactor, double srcfactor ), void *mergecontext )
{
  op->vertexmerge = vertexmerge;
  op->mergecontext = mergecontext;
  return;
}

void mdOperationComputeNormals( mdOperation *op, void *base, int format, size_t stride )
{
  op->normalbase = base;
  op->normalformat = format;
  op->normalstride = stride;
  return;
}

void mdOperationStatusCallback( mdOperation *op, void (*statuscallback)( void *statuscontext, const mdStatus *status ), void *statuscontext, long milliseconds )
{
  op->statusmilliseconds = milliseconds;
  op->statuscontext = statuscontext;
  op->statuscallback = statuscallback;
  return;
}



//////



int mdMeshDecimation( mdOperation *operation, int threadcount, int flags )
{
  int threadid, maxthreadcount, retval;
  long statuswait;
  mdMesh mesh;
  mtThread thread[MD_THREAD_COUNT_MAX];
  mdThreadInit threadinit[MD_THREAD_COUNT_MAX];
  mdThreadInit *tinit;
  mdStatus status;
#if MD_CONF_ENABLE_PROGRESS
  long deletioncount;
#endif

  retval = 1;
  operation->decimationcount = 0;
  operation->msecs = 0;

  maxthreadcount = operation->tricount / 1024;
  if( maxthreadcount > MD_THREAD_COUNT_MAX )
    maxthreadcount = MD_THREAD_COUNT_MAX;
  if( maxthreadcount == 0 )
    maxthreadcount = 1;
  if( threadcount <= 0 )
  {
    threadcount = mmcontext.cpucount;
    if( threadcount <= 0 )
      threadcount = MD_THREAD_COUNT_DEFAULT;
  }
  if( threadcount > maxthreadcount )
    threadcount = maxthreadcount;

  /* Get operation general settings */
  mesh.point = operation->vertex;
  mesh.pointstride = operation->vertexstride;
  mesh.vertexcount = operation->vertexcount;
  mesh.indices = operation->indices;
  mesh.indicesstride = operation->indicesstride;
  mesh.tridata = operation->tridata;
  mesh.tridatasize = operation->tridatasize;
  switch( operation->indicesformat )
  {
    case MD_FORMAT_BYTE:
    case MD_FORMAT_UBYTE:
      mesh.indicesUserToNative = mdIndicesCharToNative;
      mesh.indicesNativeToUser = mdIndicesNativeToChar;
      break;
    case MD_FORMAT_SHORT:
    case MD_FORMAT_USHORT:
      mesh.indicesUserToNative = mdIndicesShortToNative;
      mesh.indicesNativeToUser = mdIndicesNativeToShort;
      break;
    case MD_FORMAT_INT:
    case MD_FORMAT_UINT:
      mesh.indicesUserToNative = mdIndicesIntToNative;
      mesh.indicesNativeToUser = mdIndicesNativeToInt;
      break;
    case MD_FORMAT_INT8:
    case MD_FORMAT_UINT8:
      mesh.indicesUserToNative = mdIndicesInt8ToNative;
      mesh.indicesNativeToUser = mdIndicesNativeToInt8;
      break;
    case MD_FORMAT_INT16:
    case MD_FORMAT_UINT16:
      mesh.indicesUserToNative = mdIndicesInt16ToNative;
      mesh.indicesNativeToUser = mdIndicesNativeToInt16;
      break;
    case MD_FORMAT_INT32:
    case MD_FORMAT_UINT32:
      mesh.indicesUserToNative = mdIndicesInt32ToNative;
      mesh.indicesNativeToUser = mdIndicesNativeToInt32;
      break;
    case MD_FORMAT_INT64:
    case MD_FORMAT_UINT64:
      mesh.indicesUserToNative = mdIndicesInt64ToNative;
      mesh.indicesNativeToUser = mdIndicesNativeToInt64;
      break;
    default:
      return 0;
  }
  switch( operation->vertexformat )
  {
    case MD_FORMAT_FLOAT:
      mesh.vertexUserToNative = mdVertexFloatToNative;
      mesh.vertexNativeToUser = mdVertexNativeToFloat;
      break;
    case MD_FORMAT_DOUBLE:
      mesh.vertexUserToNative = mdVertexDoubleToNative;
      mesh.vertexNativeToUser = mdVertexNativeToDouble;
      break;
    case MD_FORMAT_SHORT:
      mesh.vertexUserToNative = mdVertexShortToNative;
      mesh.vertexNativeToUser = mdVertexNativeToShort;
      break;
    case MD_FORMAT_INT:
      mesh.vertexUserToNative = mdVertexIntToNative;
      mesh.vertexNativeToUser = mdVertexNativeToInt;
      break;
    case MD_FORMAT_INT16:
      mesh.vertexUserToNative = mdVertexInt16ToNative;
      mesh.vertexNativeToUser = mdVertexNativeToInt16;
      break;
    case MD_FORMAT_INT32:
      mesh.vertexUserToNative = mdVertexInt32ToNative;
      mesh.vertexNativeToUser = mdVertexNativeToInt32;
      break;
    default:
      return 0;
  }
  mesh.edgeweight = operation->edgeweight;
  mesh.vertexmerge = operation->vertexmerge;
  mesh.mergecontext = operation->mergecontext;
  mesh.vertexcopy = operation->vertexcopy;
  mesh.copycontext = operation->copycontext;
  mesh.tricount = operation->tricount;
  if( mesh.tricount < 2 )
    return 0;
  mesh.maxcollapsecost = operation->decimationstrength;

  /* Record start time */
  operation->msecs = mmGetMillisecondsTime();

  mesh.threadcount = threadcount;
  mesh.operationflags = flags;

  /* To compute vertex normals */
  mesh.normalbase = operation->normalbase;
  mesh.normalformat = operation->normalformat;
  mesh.normalstride = operation->normalstride;
  if( mesh.normalbase )
  {
    switch( mesh.normalformat )
    {
      case MD_FORMAT_FLOAT:
        mesh.writenormal = mdVertexNativeToFloat;
        break;
      case MD_FORMAT_DOUBLE:
        mesh.writenormal = mdVertexNativeToDouble;
        break;
      case MD_FORMAT_BYTE:
        mesh.writenormal = mdNormalNativeToChar;
        break;
      case MD_FORMAT_SHORT:
        mesh.writenormal = mdNormalNativeToShort;
        break;
      case MD_FORMAT_INT8:
        mesh.writenormal = mdNormalNativeToInt8;
        break;
      case MD_FORMAT_INT16:
        mesh.writenormal = mdNormalNativeToInt16;
        break;
      case MD_FORMAT_INT_2_10_10_10_REV:
        mesh.writenormal = mdNormalNativeTo10_10_10_2;
        break;
      default:
        return 0;
    }
  }

  /* Advanced configuration options */
  mesh.compactnesstarget = operation->compactnesstarget;
  mesh.compactnesspenalty = operation->compactnesspenalty;
  mesh.boundaryweight = operation->boundaryweight;
  mesh.syncstepcount = operation->syncstepcount;
  if( mesh.syncstepcount < 1 )
    mesh.syncstepcount = 1;
  if( mesh.syncstepcount > 1024 )
    mesh.syncstepcount = 1024;
  mesh.normalsearchangle = cos( 1.0 * operation->normalsearchangle * (M_PI/180.0) );
  if( mesh.normalsearchangle > 0.9 )
    mesh.normalsearchangle = 0.9;

  /* Synchronization */
  mdBarrierInit( &mesh.workbarrier, threadcount );
  mdBarrierInit( &mesh.globalbarrier, threadcount+1 );

  /* Determine update buffer shift required, find the count of updatebuffers */
  for( mesh.updatebuffershift = 0 ; ( threadcount >> mesh.updatebuffershift ) > MD_THREAD_UPDATE_BUFFER_COUNTMAX ; mesh.updatebuffershift++ );
  mesh.updatebuffercount = ( ( threadcount - 1 ) >> mesh.updatebuffershift ) + 1;

  /* Runtime picking of collapse penalty computation path */
  mesh.collapsepenalty = mdEdgeCollapsePenaltyTriangle;
#if CPU_SSE_SUPPORT
 #if !MD_CONF_DOUBLE_PRECISION
  #if CPU_SSE4_1_SUPPORT
    mesh.collapsepenalty = mdEdgeCollapsePenaltyTriangleSSE4p1f;
  #elif CPU_SSE3_SUPPORT
    mesh.collapsepenalty = mdEdgeCollapsePenaltyTriangleSSE3f;
  #elif CPU_SSE2_SUPPORT
    mesh.collapsepenalty = mdEdgeCollapsePenaltyTriangleSSE2f;
  #endif
 #else
  #if CPU_SSE4_1_SUPPORT
    mesh.collapsepenalty = mdEdgeCollapsePenaltyTriangleSSE4p1d;
  #elif CPU_SSE3_SUPPORT
    mesh.collapsepenalty = mdEdgeCollapsePenaltyTriangleSSE3d;
  #elif CPU_SSE2_SUPPORT
    mesh.collapsepenalty = mdEdgeCollapsePenaltyTriangleSSE2d;
  #endif
 #endif
#endif

  /* Initialize entire mesh storage */
  mesh.vertexalloc = operation->vertexalloc;
  if( mesh.vertexalloc < mesh.vertexcount )
    mesh.vertexalloc = mesh.vertexcount;
  if( !( mdMeshInit( &mesh, operation->maxmemorysize ) ) )
  {
    retval = 0;
    goto error;
  }

  mesh.updatestatusflag = 0;
  status.progress = 0.0;
  statuswait = ( operation->statusmilliseconds > 10 ? operation->statusmilliseconds : 10 );
  status.trianglecount = 0;
  if( operation->statuscallback )
  {
    mesh.updatestatusflag = 1;
    mdUpdateStatus( &mesh, 0, MD_STATUS_STAGE_INIT, &status );
    operation->statuscallback( operation->statuscontext, &status );
  }

  /* Launch threads! */
  tinit = threadinit;
  for( threadid = 0 ; threadid < threadcount ; threadid++, tinit++ )
  {
    tinit->threadid = threadid;
    tinit->mesh = &mesh;
    tinit->stage = MD_STATUS_STAGE_INIT;
    mtThreadCreate( &thread[threadid], mdThreadMain, tinit, MT_THREAD_FLAGS_JOINABLE, 0, 0 );
  }

  /* Wait until all threads have properly initialized */
  if( mesh.updatestatusflag )
    mdBarrierSync( &mesh.globalbarrier );

  /* Wait for all threads to reach step 3 */
#if MD_CONF_ENABLE_PROGRESS
  if( !( mesh.updatestatusflag ) )
    mdBarrierSync( &mesh.globalbarrier );
  else
  {
    for( ; !( mdBarrierSyncTimeout( &mesh.globalbarrier, statuswait ) ) ; )
    {
      mdUpdateStatus( &mesh, threadinit, 0, &status );
      operation->statuscallback( operation->statuscontext, &status );
    }
  }
#else
  mdBarrierSync( &mesh.globalbarrier );
#endif

  /* Build mesh step 3 is not parallel, have the main thread run it */
  mdMeshInitTrirefs( &mesh );

  /* Wake up all threads */
  mdBarrierSync( &mesh.globalbarrier );

  /* Wait for all threads to complete */
#if MD_CONF_ENABLE_PROGRESS
  if( !( mesh.updatestatusflag ) )
    mdBarrierSync( &mesh.globalbarrier );
  else
  {
    for( ; !( mdBarrierSyncTimeout( &mesh.globalbarrier, statuswait ) ) ; )
    {
      mdUpdateStatus( &mesh, threadinit, 0, &status );
      operation->statuscallback( operation->statuscontext, &status );
    }
  }
  deletioncount = 0;
  for( threadid = 0 ; threadid < threadcount ; threadid++ )
  {
    deletioncount += threadinit[threadid].deletioncount;
    mtThreadJoin( &thread[threadid] );
  }
  status.trianglecount = mesh.tricount - deletioncount;
#else
  mdBarrierSync( &mesh.globalbarrier );
  for( threadid = 0 ; threadid < threadcount ; threadid++ )
  {
    mtThreadJoin( &thread[threadid] );
  }
#endif

  /* Count sums of all threads */
  operation->decimationcount = 0;
  operation->collisioncount = 0;
  tinit = threadinit;
  for( threadid = 0 ; threadid < threadcount ; threadid++, tinit++ )
  {
    operation->decimationcount += tinit->decimationcount;
    operation->collisioncount += tinit->collisioncount;
  }

  if( mesh.updatestatusflag )
  {
    mdUpdateStatus( &mesh, 0, MD_STATUS_STAGE_STORE, &status );
    operation->statuscallback( operation->statuscontext, &status );
  }

  /* Write out the final mesh */
  if( ( mesh.normalbase ) && ( mesh.writenormal ) )
    mdMeshWriteVerticesAndNormals( &mesh );
  else
    mdMeshWriteVertices( &mesh );
  mdMeshWriteIndices( &mesh );
  operation->vertexcount = mesh.vertexpackcount;
  operation->tricount = mesh.tripackcount;

  if( mesh.updatestatusflag )
  {
    mdUpdateStatus( &mesh, 0, MD_STATUS_STAGE_DONE, &status );
    operation->statuscallback( operation->statuscontext, &status );
  }

  /* Requires mmhash.c compiled with MM_HASH_DEBUG_STATISTICS */
#if MM_HASH_DEBUG_STATISTICS
  {
    long accesscount, collisioncount, relocationcount;
    long entrycount, entrycountmax, hashsizemax;

    mmHashStatistics( mesh.edgehashtable, &accesscount, &collisioncount, &relocationcount, &entrycount, &entrycountmax, &hashsizemax );

    printf( "Hash Access     : %ld\n", accesscount );
    printf( "Hash Collision  : %ld\n", collisioncount );
    printf( "Hash Relocation : %ld\n", relocationcount );
    printf( "Entry Count     : %ld\n", entrycount );
    printf( "Entry Count Max : %ld\n", entrycountmax );
    printf( "Hash Size Max   : %ld\n", hashsizemax );
  }
#endif

  /* Free all global data */
  error:
  if( !( mesh.operationflags & MD_FLAGS_NO_DECIMATION ) )
    mdMeshHashEnd( &mesh );
  mdMeshEnd( &mesh );
  mdBarrierDestroy( &mesh.workbarrier );
  mdBarrierDestroy( &mesh.globalbarrier );

  /* Store total processing time */
  operation->msecs = mmGetMillisecondsTime() - operation->msecs;

  return retval;
}


