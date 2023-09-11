/* -----------------------------------------------------------------------------
 *
 * Copyright (c) 2009-2016 Alexis Naveros.
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
 * -----------------------------------------------------------------------------
 */

/*
 * Notes to future self
 * 
 * Constants such as 0x4924924924924924 are to simplify a ... | ( addbits ^ 0x4 )
 * Preferable to do it all at once at the end than once per bit loop
 * 
 * Other magic numbers properly set next rotate value based on rotation and addbits.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <float.h>
#include <math.h>


#include "cc.h"
#include "ccspacecurve.h"


#define CC_SPACECURVE_BITS_PER_INT (32)


////


unsigned int ccSpaceCurveMap2D( int mapbits, unsigned int *point )
{
  int bitshift;
  unsigned int x, y;
  unsigned int rx, ry, s, d, t;
  x = point[0];
  y = point[1];
  s = 1 << mapbits;
  d = 0;
  for( bitshift = mapbits-1 ; bitshift >= 0 ; bitshift-- )
  {
    rx = ( x >> bitshift ) & 0x1;
    ry = ( y >> bitshift ) & 0x1;
    d += ( ( ( 3 * rx ) ^ ry ) << bitshift ) << bitshift;
    if( ry == 0 )
    {
      if( rx == 1 )
      {
        s = ( ( (unsigned int)1 ) << bitshift ) - 1;
        t = x;
        x = s - y;
        y = s - t;
      }
      else
      {
        t = x;
        x = y;
        y = t;
      }
    }
  }

  return d;
}

void ccSpaceCurveUnmap2D( int mapbits, unsigned int curveindex, unsigned int *point )
{
  int bitshift;
  unsigned int rx, ry, s, x, y, t;
  x = 0;
  y = 0;
  for( bitshift = 0 ; bitshift < mapbits ; bitshift++ )
  {
    rx = ( curveindex >> 1 ) & 0x1;
    ry = ( curveindex ^ rx ) & 0x1;
    if( ry == 0 )
    {
      if( rx == 1 )
      {
        s = ( ( (unsigned int)1 ) << bitshift ) - 1;
        t = x;
        x = s - y;
        y = s - t;
      }
      else
      {
        t = x;
        x = y;
        y = t;
      }
    }
    x += rx << bitshift;
    y += ry << bitshift;
    curveindex >>= 2;
  }
  point[0] = x;
  point[1] = y;
  return;
}


////



uint64_t ccSpaceCurveMap2D64( int mapbits, uint64_t *point )
{
  int bitshift;
  uint64_t x, y;
  uint64_t rx, ry, s, d, t;
  x = point[0];
  y = point[1];
  s = 1 << mapbits;
  d = 0;
  for( bitshift = mapbits-1 ; bitshift >= 0 ; bitshift-- )
  {
    rx = ( x >> bitshift ) & 0x1;
    ry = ( y >> bitshift ) & 0x1;
    d += ( ( ( 3 * rx ) ^ ry ) << bitshift ) << bitshift;
    if( ry == 0 )
    {
      if( rx == 1 )
      {
        s = ( ( (uint64_t)1 ) << bitshift ) - 1;
        t = x;
        x = s - y;
        y = s - t;
      }
      else
      {
        t = x;
        x = y;
        y = t;
      }
    }
  }

  return d;
}

void ccSpaceCurveUnmap2D64( int mapbits, uint64_t curveindex, uint64_t *point )
{
  int bitshift;
  uint64_t rx, ry, s, x, y, t;
  x = 0;
  y = 0;
  for( bitshift = 0 ; bitshift < mapbits ; bitshift++ )
  {
    rx = ( curveindex >> 1 ) & 0x1;
    ry = ( curveindex ^ rx ) & 0x1;
    if( ry == 0 )
    {
      if( rx == 1 )
      {
        s = ( ( (uint64_t)1 ) << bitshift ) - 1;
        t = x;
        x = s - y;
        y = s - t;
      }
      else
      {
        t = x;
        x = y;
        y = t;
      }
    }
    x += rx << bitshift;
    y += ry << bitshift;
    curveindex >>= 2;
  }
  point[0] = x;
  point[1] = y;
  return;
}


////


#ifdef CC_SPACECURVE_DIMCOUNT
 #undef CC_SPACECURVE_DIMCOUNT
#endif
#ifdef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING1
#endif
#ifdef CC_SPACECURVE_MAPBITS
 #undef CC_SPACECURVE_MAPBITS
#endif
#define CC_SPACECURVE_DIMCOUNT (3)
#define CC_SPACECURVE_DIMSPACING (CC_SPACECURVE_BITS_PER_INT/CC_SPACECURVE_DIMCOUNT)
#define CC_SPACECURVE_DIMSPACING1 (CC_SPACECURVE_DIMSPACING-1)


unsigned int ccSpaceCurveMap3D( int mapbits, unsigned int *point )
{
  int bit, rotation, totalbits;
  unsigned int curveindex, pointindex, indexpart;
  unsigned int addmask, addbits;

  pointindex = point[2] ^ ( point[2] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[1] ^ ( point[1] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[0] ^ ( point[0] >> 1 );

  rotation = 0;
  addmask = 0;
  curveindex = 0;
  bit = mapbits - 1;
  do
  {
    indexpart = pointindex >> bit;
    addmask ^= ( indexpart & 0x1 ) | ( ( indexpart >> (1*CC_SPACECURVE_DIMSPACING1) ) & 0x2 ) | ( ( indexpart >> (2*CC_SPACECURVE_DIMSPACING1) ) & 0x4 );
    addbits = ( ( addmask >> rotation ) | ( addmask << ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    curveindex = ( curveindex << CC_SPACECURVE_DIMCOUNT ) | addbits;
    addmask = 1 << rotation;
    /* Magic number! */
    rotation = ( 0x641289 >> ( ( ( addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ) << 1 ) | ( rotation << 3 ) ) ) & 0x3;
  } while( --bit >= 0 );

  totalbits = CC_SPACECURVE_DIMCOUNT * mapbits;
  curveindex ^= (unsigned int)0x24924924 & ( ( ( (unsigned int)1 ) << ( totalbits - CC_SPACECURVE_DIMCOUNT ) ) - 1 );
  for( bit = 1 ; bit < totalbits ; bit <<= 1 )
    curveindex ^= curveindex >> bit;
  return curveindex;
}


void ccSpaceCurveUnmap3D( int mapbits, unsigned int curveindex, unsigned int *point )
{
  int bit, rotation, totalbits;
  unsigned int pointindex;
  unsigned int addmask, addbits;
  unsigned int p0, p1, p2;

  totalbits = CC_SPACECURVE_DIMCOUNT * mapbits;
  rotation = 0;
  addmask = 0;
  curveindex ^= ( curveindex ^ ( ( (unsigned int)0x49249249 ) & ( ( ( (unsigned int)2 ) << ( totalbits - CC_SPACECURVE_DIMCOUNT ) ) - 1 ) ) ) >> 1;
  pointindex = 0;
  bit = totalbits - CC_SPACECURVE_DIMCOUNT;
  do
  {
    addbits = ( curveindex >> bit ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    pointindex <<= CC_SPACECURVE_DIMCOUNT;
    pointindex |= addmask ^ ( ( ( addbits << rotation ) | ( addbits >> ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 ) );
    addmask = 1 << rotation;
    /* Magic number! */
    rotation = ( 0x641289 >> ( ( ( addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ) << 1 ) | ( rotation << 3 ) ) ) & 0x3;
  } while( ( bit -= CC_SPACECURVE_DIMCOUNT ) >= 0 );
  for( bit = CC_SPACECURVE_DIMCOUNT ; bit < totalbits ; bit <<= 1 )
    pointindex ^= pointindex >> bit;

  p0 = pointindex & 0x1;
  p1 = pointindex & 0x2;
  p2 = pointindex & 0x4;
  for( bit = 1 ; bit < mapbits ; bit++ )
  {
    pointindex >>= CC_SPACECURVE_DIMCOUNT;
    p0 |= ( pointindex & 0x1 ) << bit;
    p1 |= ( pointindex & 0x2 ) << bit;
    p2 |= ( pointindex & 0x4 ) << bit;
  }
  point[0] = p0 >> 0;
  point[1] = p1 >> 1;
  point[2] = p2 >> 2;

  return;
}


////


#ifdef CC_SPACECURVE_DIMCOUNT
 #undef CC_SPACECURVE_DIMCOUNT
#endif
#ifdef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING1
#endif
#ifdef CC_SPACECURVE_MAPBITS
 #undef CC_SPACECURVE_MAPBITS
#endif
#define CC_SPACECURVE_DIMCOUNT (3)
#define CC_SPACECURVE_DIMSPACING (64/CC_SPACECURVE_DIMCOUNT)
#define CC_SPACECURVE_DIMSPACING1 (CC_SPACECURVE_DIMSPACING-1)


uint64_t ccSpaceCurveMap3D64( int mapbits, uint64_t *point )
{
  int bit, rotation, totalbits;
  uint64_t curveindex, pointindex, indexpart;
  uint64_t addmask, addbits;

  pointindex = point[2] ^ ( point[2] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[1] ^ ( point[1] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[0] ^ ( point[0] >> 1 );

  rotation = 0;
  addmask = 0;
  curveindex = 0;
  bit = mapbits - 1;
  do
  {
    indexpart = pointindex >> bit;
    addmask ^= ( indexpart & 0x1 ) | ( ( indexpart >> (1*CC_SPACECURVE_DIMSPACING1) ) & 0x2 ) | ( ( indexpart >> (2*CC_SPACECURVE_DIMSPACING1) ) & 0x4 );
    addbits = ( ( addmask >> rotation ) | ( addmask << ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    curveindex = ( curveindex << CC_SPACECURVE_DIMCOUNT ) | addbits;
    addmask = 1 << rotation;
    /* Magic number! */
    rotation = ( 0x641289 >> ( ( ( addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ) << 1 ) | ( rotation << 3 ) ) ) & 0x3;
  } while( --bit >= 0 );

  totalbits = CC_SPACECURVE_DIMCOUNT * mapbits;
  curveindex ^= (uint64_t)0x4924924924924924 & ( ( ( (uint64_t)1 ) << ( totalbits - CC_SPACECURVE_DIMCOUNT ) ) - 1 );
  for( bit = 1 ; bit < totalbits ; bit <<= 1 )
    curveindex ^= curveindex >> bit;

  return curveindex;
}


void ccSpaceCurveUnmap3D64( int mapbits, uint64_t curveindex, uint64_t *point )
{
  int bit, rotation, totalbits;
  uint64_t pointindex;
  uint64_t addmask, addbits;
  uint64_t p0, p1, p2;

  totalbits = CC_SPACECURVE_DIMCOUNT * mapbits;
  rotation = 0;
  addmask = 0;
  curveindex ^= ( curveindex ^ ( ( (uint64_t)0x9249249249249249 ) & ( ( ( (uint64_t)2 ) << ( totalbits - CC_SPACECURVE_DIMCOUNT ) ) - 1 ) ) ) >> 1;
  pointindex = 0;
  bit = totalbits - CC_SPACECURVE_DIMCOUNT;
  do
  {
    addbits = ( curveindex >> bit ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    pointindex <<= CC_SPACECURVE_DIMCOUNT;
    pointindex |= addmask ^ ( ( ( addbits << rotation ) | ( addbits >> ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 ) );
    addmask = 1 << rotation;
    /* Magic number! */
    rotation = ( 0x641289 >> ( ( ( addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ) << 1 ) | ( rotation << 3 ) ) ) & 0x3;
  } while( ( bit -= CC_SPACECURVE_DIMCOUNT ) >= 0 );
  for( bit = CC_SPACECURVE_DIMCOUNT ; bit < totalbits ; bit <<= 1 )
    pointindex ^= pointindex >> bit;

  p0 = pointindex & 0x1;
  p1 = pointindex & 0x2;
  p2 = pointindex & 0x4;
  for( bit = 1 ; bit < mapbits ; bit++ )
  {
    pointindex >>= CC_SPACECURVE_DIMCOUNT;
    p0 |= ( pointindex & 0x1 ) << bit;
    p1 |= ( pointindex & 0x2 ) << bit;
    p2 |= ( pointindex & 0x4 ) << bit;
  }
  point[0] = p0 >> 0;
  point[1] = p1 >> 1;
  point[2] = p2 >> 2;

  return;
}


////


#ifdef CC_SPACECURVE_DIMCOUNT
 #undef CC_SPACECURVE_DIMCOUNT
#endif
#ifdef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING1
#endif
#ifdef CC_SPACECURVE_MAPBITS
 #undef CC_SPACECURVE_MAPBITS
#endif
#define CC_SPACECURVE_DIMCOUNT (3)
#define CC_SPACECURVE_DIMSPACING (CC_SPACECURVE_BITS_PER_INT/CC_SPACECURVE_DIMCOUNT)
#define CC_SPACECURVE_DIMSPACING1 (CC_SPACECURVE_DIMSPACING-1)
#define CC_SPACECURVE_MAPBITS (10)


unsigned int ccSpaceCurveMap3D10Bits( unsigned int *point )
{
  int bit, rotation;
  unsigned int curveindex, pointindex, indexpart;
  unsigned int addmask, addbits;

  pointindex = point[2] ^ ( point[2] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[1] ^ ( point[1] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[0] ^ ( point[0] >> 1 );

  rotation = 0;
  addmask = 0;
  curveindex = 0;
  bit = CC_SPACECURVE_MAPBITS-1;
  do
  {
    indexpart = pointindex >> bit;
    addmask ^= ( indexpart & 0x1 ) | ( ( indexpart >> (1*CC_SPACECURVE_DIMSPACING1) ) & 0x2 ) | ( ( indexpart >> (2*CC_SPACECURVE_DIMSPACING1) ) & 0x4 );
    addbits = ( ( addmask >> rotation ) | ( addmask << ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    curveindex = ( curveindex << CC_SPACECURVE_DIMCOUNT ) | addbits;
    addmask = 1 << rotation;
    /* Magic number! */
    rotation = ( 0x641289 >> ( ( ( addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ) << 1 ) | ( rotation << 3 ) ) ) & 0x3;
  } while( --bit >= 0 );

  curveindex ^= (unsigned int)0x24924924 & ( ( ( (unsigned int)1 ) << ((CC_SPACECURVE_MAPBITS-1)*CC_SPACECURVE_DIMCOUNT) ) - 1 );
  for( bit = 1 ; bit < ( CC_SPACECURVE_DIMCOUNT * CC_SPACECURVE_MAPBITS ) ; bit <<= 1 )
    curveindex ^= curveindex >> bit;
  return curveindex;
}


void ccSpaceCurveUnmap3D10Bits( unsigned int curveindex, unsigned int *point )
{
  int bit, rotation;
  unsigned int pointindex;
  unsigned int addmask, addbits;
  unsigned int p0, p1, p2;

  rotation = 0;
  addmask = 0;
  curveindex ^= ( curveindex ^ ( ( (unsigned int)0x49249249 ) & ( ( ( (unsigned int)2 ) << ((CC_SPACECURVE_MAPBITS-1)*CC_SPACECURVE_DIMCOUNT) ) - 1 ) ) ) >> 1;
  pointindex = 0;
  bit = ( CC_SPACECURVE_DIMCOUNT * CC_SPACECURVE_MAPBITS ) - CC_SPACECURVE_DIMCOUNT;
  do
  {
    addbits = ( curveindex >> bit ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    pointindex <<= CC_SPACECURVE_DIMCOUNT;
    pointindex |= addmask ^ ( ( ( addbits << rotation ) | ( addbits >> ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 ) );
    addmask = 1 << rotation;
    /* Magic number! */
    rotation = ( 0x641289 >> ( ( ( addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ) << 1 ) | ( rotation << 3 ) ) ) & 0x3;
  } while( ( bit -= CC_SPACECURVE_DIMCOUNT ) >= 0 );

  p0 = pointindex & 0x1;
  p1 = pointindex & 0x2;
  p2 = pointindex & 0x4;
  for( bit = 1 ; bit < CC_SPACECURVE_MAPBITS ; bit++ )
  {
    pointindex >>= CC_SPACECURVE_DIMCOUNT;
    p0 |= ( pointindex & 0x1 ) << bit;
    p1 |= ( pointindex & 0x2 ) << bit;
    p2 |= ( pointindex & 0x4 ) << bit;
  }
  point[0] = p0 >> 0;
  point[1] = p1 >> 1;
  point[2] = p2 >> 2;

  return;
}


////


#ifdef CC_SPACECURVE_DIMCOUNT
 #undef CC_SPACECURVE_DIMCOUNT
#endif
#ifdef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING1
#endif
#ifdef CC_SPACECURVE_MAPBITS
 #undef CC_SPACECURVE_MAPBITS
#endif
#define CC_SPACECURVE_DIMCOUNT (3)
#define CC_SPACECURVE_DIMSPACING (64/CC_SPACECURVE_DIMCOUNT)
#define CC_SPACECURVE_DIMSPACING1 (CC_SPACECURVE_DIMSPACING-1)
#define CC_SPACECURVE_MAPBITS (21)


uint64_t ccSpaceCurveMap3D21Bits64( uint64_t *point )
{
  int bit, rotation;
  uint64_t curveindex, pointindex, indexpart;
  uint64_t addmask, addbits;

  pointindex = point[2] ^ ( point[2] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[1] ^ ( point[1] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[0] ^ ( point[0] >> 1 );

  rotation = 0;
  addmask = 0;
  curveindex = 0;
  bit = CC_SPACECURVE_MAPBITS-1;
  do
  {
    indexpart = pointindex >> bit;
    addmask ^= ( indexpart & 0x1 ) | ( ( indexpart >> (1*CC_SPACECURVE_DIMSPACING1) ) & 0x2 ) | ( ( indexpart >> (2*CC_SPACECURVE_DIMSPACING1) ) & 0x4 );
    addbits = ( ( addmask >> rotation ) | ( addmask << ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    curveindex = ( curveindex << CC_SPACECURVE_DIMCOUNT ) | addbits;
    addmask = 1 << rotation;
    /* Magic number! */
    rotation = ( 0x641289 >> ( ( ( addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ) << 1 ) | ( rotation << 3 ) ) ) & 0x3;
  } while( --bit >= 0 );

  curveindex ^= (uint64_t)0x4924924924924924 & ( ( ( (uint64_t)1 ) << ((CC_SPACECURVE_MAPBITS-1)*CC_SPACECURVE_DIMCOUNT) ) - 1 );
  for( bit = 1 ; bit < ( CC_SPACECURVE_DIMCOUNT * CC_SPACECURVE_MAPBITS ) ; bit <<= 1 )
    curveindex ^= curveindex >> bit;
  return curveindex;
}


void ccSpaceCurveUnmap3D21Bits64( uint64_t curveindex, uint64_t *point )
{
  int bit, rotation;
  uint64_t pointindex;
  uint64_t addmask, addbits;
  uint64_t p0, p1, p2;

  rotation = 0;
  addmask = 0;
  curveindex ^= ( curveindex ^ ( ( (uint64_t)0x9249249249249249 ) & ( ( ( (uint64_t)2 ) << ((CC_SPACECURVE_MAPBITS-1)*CC_SPACECURVE_DIMCOUNT) ) - 1 ) ) ) >> 1;
  pointindex = 0;
  bit = ( CC_SPACECURVE_DIMCOUNT * CC_SPACECURVE_MAPBITS ) - CC_SPACECURVE_DIMCOUNT;
  do
  {
    addbits = ( curveindex >> bit ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    pointindex <<= CC_SPACECURVE_DIMCOUNT;
    pointindex |= addmask ^ ( ( ( addbits << rotation ) | ( addbits >> ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 ) );
    addmask = 1 << rotation;
    /* Magic number! */
    rotation = ( 0x641289 >> ( ( ( addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ) << 1 ) | ( rotation << 3 ) ) ) & 0x3;
  } while( ( bit -= CC_SPACECURVE_DIMCOUNT ) >= 0 );

  p0 = pointindex & 0x1;
  p1 = pointindex & 0x2;
  p2 = pointindex & 0x4;
  for( bit = 1 ; bit < CC_SPACECURVE_MAPBITS ; bit++ )
  {
    pointindex >>= CC_SPACECURVE_DIMCOUNT;
    p0 |= ( pointindex & 0x1 ) << bit;
    p1 |= ( pointindex & 0x2 ) << bit;
    p2 |= ( pointindex & 0x4 ) << bit;
  }
  point[0] = p0 >> 0;
  point[1] = p1 >> 1;
  point[2] = p2 >> 2;

  return;
}


////


#ifdef CC_SPACECURVE_DIMCOUNT
 #undef CC_SPACECURVE_DIMCOUNT
#endif
#ifdef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING1
#endif
#ifdef CC_SPACECURVE_MAPBITS
 #undef CC_SPACECURVE_MAPBITS
#endif
#define CC_SPACECURVE_DIMCOUNT (4)
#define CC_SPACECURVE_DIMSPACING (CC_SPACECURVE_BITS_PER_INT/CC_SPACECURVE_DIMCOUNT)
#define CC_SPACECURVE_DIMSPACING1 (CC_SPACECURVE_DIMSPACING-1)


unsigned int ccSpaceCurveMap4D( int mapbits, unsigned int *point )
{
  int bit, rotation, totalbits;
  unsigned int curveindex, pointindex, indexpart;
  unsigned int addmask, addbits;

  pointindex = point[3] ^ ( point[3] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[2] ^ ( point[2] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[1] ^ ( point[1] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[0] ^ ( point[0] >> 1 );

  rotation = 0;
  addmask = 0;
  curveindex = 0;
  bit = mapbits - 1;
  do
  {
    indexpart = pointindex >> bit;
    addmask ^= ( indexpart & 0x1 ) | ( ( indexpart >> (1*CC_SPACECURVE_DIMSPACING1) ) & 0x2 ) | ( ( indexpart >> (2*CC_SPACECURVE_DIMSPACING1) ) & 0x4 ) | ( ( indexpart >> (3*CC_SPACECURVE_DIMSPACING1) ) & 0x8 );
    addbits = ( ( addmask >> rotation ) | ( addmask << ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    curveindex = ( curveindex << CC_SPACECURVE_DIMCOUNT ) | addbits;
    addmask = 1 << rotation;
    /* Magic number! */
    rotation = ( rotation + ( ( 0x23242321 >> ( ( addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ) << 2 ) ) & 0x7 ) ) & (CC_SPACECURVE_DIMCOUNT-1);
  } while( --bit >= 0 );

  totalbits = CC_SPACECURVE_DIMCOUNT * mapbits;
  curveindex ^= (unsigned int)0x88888888 & ( ( ( (unsigned int)1 ) << ( totalbits - CC_SPACECURVE_DIMCOUNT ) ) - 1 );
  for( bit = 1 ; bit < totalbits ; bit <<= 1 )
    curveindex ^= curveindex >> bit;
  return curveindex;
}

void ccSpaceCurveUnmap4D( int mapbits, unsigned int curveindex, unsigned int *point )
{
  int bit, rotation, totalbits;
  unsigned int pointindex;
  unsigned int addmask, addbits;
  unsigned int p0, p1, p2, p3;

  totalbits = CC_SPACECURVE_DIMCOUNT * mapbits;
  rotation = 0;
  addmask = 0;
  curveindex ^= ( curveindex ^ ( ( (unsigned int)0x11111111 ) & ( ( ( (unsigned int)2 ) << ( totalbits - CC_SPACECURVE_DIMCOUNT ) ) - 1 ) ) ) >> 1;
  pointindex = 0;
  bit = totalbits - CC_SPACECURVE_DIMCOUNT;
  do
  {
    addbits = ( curveindex >> bit ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    pointindex <<= CC_SPACECURVE_DIMCOUNT;
    pointindex |= addmask ^ ( ( ( addbits << rotation ) | ( addbits >> ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 ) );
    addmask = 1 << rotation;
    /* Magic number! */
    rotation = ( rotation + ( ( 0x23242321 >> ( ( addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ) << 2 ) ) & 0x7 ) ) & (CC_SPACECURVE_DIMCOUNT-1);
  } while( ( bit -= CC_SPACECURVE_DIMCOUNT ) >= 0 );
  for( bit = CC_SPACECURVE_DIMCOUNT ; bit < totalbits ; bit <<= 1 )
    pointindex ^= pointindex >> bit;

  p0 = pointindex & 0x1;
  p1 = pointindex & 0x2;
  p2 = pointindex & 0x4;
  p3 = pointindex & 0x8;
  for( bit = 1 ; bit < mapbits ; bit++ )
  {
    pointindex >>= CC_SPACECURVE_DIMCOUNT;
    p0 |= ( pointindex & 0x1 ) << bit;
    p1 |= ( pointindex & 0x2 ) << bit;
    p2 |= ( pointindex & 0x4 ) << bit;
    p3 |= ( pointindex & 0x8 ) << bit;
  }
  point[0] = p0 >> 0;
  point[1] = p1 >> 1;
  point[2] = p2 >> 2;
  point[3] = p3 >> 3;

  return;
}


////


#ifdef CC_SPACECURVE_DIMCOUNT
 #undef CC_SPACECURVE_DIMCOUNT
#endif
#ifdef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING1
#endif
#ifdef CC_SPACECURVE_MAPBITS
 #undef CC_SPACECURVE_MAPBITS
#endif
#define CC_SPACECURVE_DIMCOUNT (4)
#define CC_SPACECURVE_DIMSPACING (64/CC_SPACECURVE_DIMCOUNT)
#define CC_SPACECURVE_DIMSPACING1 (CC_SPACECURVE_DIMSPACING-1)


uint64_t ccSpaceCurveMap4D64( int mapbits, uint64_t *point )
{
  int bit, rotation, totalbits;
  uint64_t curveindex, pointindex, indexpart;
  uint64_t addmask, addbits;

  pointindex = point[3] ^ ( point[3] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[2] ^ ( point[2] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[1] ^ ( point[1] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[0] ^ ( point[0] >> 1 );

  rotation = 0;
  addmask = 0;
  curveindex = 0;
  bit = mapbits - 1;
  do
  {
    indexpart = pointindex >> bit;
    addmask ^= ( indexpart & 0x1 ) | ( ( indexpart >> (1*CC_SPACECURVE_DIMSPACING1) ) & 0x2 ) | ( ( indexpart >> (2*CC_SPACECURVE_DIMSPACING1) ) & 0x4 ) | ( ( indexpart >> (3*CC_SPACECURVE_DIMSPACING1) ) & 0x8 );
    addbits = ( ( addmask >> rotation ) | ( addmask << ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    curveindex = ( curveindex << CC_SPACECURVE_DIMCOUNT ) | addbits;
    addmask = 1 << rotation;
    /* Magic number! */
    rotation = ( rotation + ( ( 0x23242321 >> ( ( addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ) << 2 ) ) & 0x7 ) ) & (CC_SPACECURVE_DIMCOUNT-1);
  } while( --bit >= 0 );

  totalbits = CC_SPACECURVE_DIMCOUNT * mapbits;
  curveindex ^= (uint64_t)0x8888888888888888 & ( ( ( (uint64_t)1 ) << ( totalbits - CC_SPACECURVE_DIMCOUNT ) ) - 1 );
  for( bit = 1 ; bit < totalbits ; bit <<= 1 )
    curveindex ^= curveindex >> bit;
  return curveindex;
}

void ccSpaceCurveUnmap4D64( int mapbits, uint64_t curveindex, uint64_t *point )
{
  int bit, rotation, totalbits;
  uint64_t pointindex;
  uint64_t addmask, addbits;
  uint64_t p0, p1, p2, p3;

  totalbits = CC_SPACECURVE_DIMCOUNT * mapbits;
  rotation = 0;
  addmask = 0;
  curveindex ^= ( curveindex ^ ( ( (uint64_t)0x1111111111111111 ) & ( ( ( (uint64_t)2 ) << ( totalbits - CC_SPACECURVE_DIMCOUNT ) ) - 1 ) ) ) >> 1;
  pointindex = 0;
  bit = totalbits - CC_SPACECURVE_DIMCOUNT;
  do
  {
    addbits = ( curveindex >> bit ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    pointindex <<= CC_SPACECURVE_DIMCOUNT;
    pointindex |= addmask ^ ( ( ( addbits << rotation ) | ( addbits >> ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 ) );
    addmask = 1 << rotation;
    /* Magic number! */
    rotation = ( rotation + ( ( 0x23242321 >> ( ( addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ) << 2 ) ) & 0x7 ) ) & (CC_SPACECURVE_DIMCOUNT-1);
  } while( ( bit -= CC_SPACECURVE_DIMCOUNT ) >= 0 );
  for( bit = CC_SPACECURVE_DIMCOUNT ; bit < totalbits ; bit <<= 1 )
    pointindex ^= pointindex >> bit;

  p0 = pointindex & 0x1;
  p1 = pointindex & 0x2;
  p2 = pointindex & 0x4;
  p3 = pointindex & 0x8;
  for( bit = 1 ; bit < mapbits ; bit++ )
  {
    pointindex >>= CC_SPACECURVE_DIMCOUNT;
    p0 |= ( pointindex & 0x1 ) << bit;
    p1 |= ( pointindex & 0x2 ) << bit;
    p2 |= ( pointindex & 0x4 ) << bit;
    p3 |= ( pointindex & 0x8 ) << bit;
  }
  point[0] = p0 >> 0;
  point[1] = p1 >> 1;
  point[2] = p2 >> 2;
  point[3] = p3 >> 3;

  return;
}


////


#ifdef CC_SPACECURVE_DIMCOUNT
 #undef CC_SPACECURVE_DIMCOUNT
#endif
#ifdef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING1
#endif
#ifdef CC_SPACECURVE_MAPBITS
 #undef CC_SPACECURVE_MAPBITS
#endif
#define CC_SPACECURVE_DIMCOUNT (5)
#define CC_SPACECURVE_DIMSPACING (CC_SPACECURVE_BITS_PER_INT/CC_SPACECURVE_DIMCOUNT)
#define CC_SPACECURVE_DIMSPACING1 (CC_SPACECURVE_DIMSPACING-1)


unsigned int ccSpaceCurveMap5D( int mapbits, unsigned int *point )
{
  int bit, rotation, totalbits;
  unsigned int curveindex, pointindex, indexpart;
  unsigned int addmask, addbits;

  pointindex = point[4] ^ ( point[4] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[3] ^ ( point[3] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[2] ^ ( point[2] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[1] ^ ( point[1] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[0] ^ ( point[0] >> 1 );

  rotation = 0;
  addmask = 0;
  curveindex = 0;
  bit = mapbits - 1;
  do
  {
    indexpart = pointindex >> bit;
    addmask ^= ( indexpart & 0x1 ) | ( ( indexpart >> (1*CC_SPACECURVE_DIMSPACING1) ) & 0x2 ) | ( ( indexpart >> (2*CC_SPACECURVE_DIMSPACING1) ) & 0x4 ) | ( ( indexpart >> (3*CC_SPACECURVE_DIMSPACING1) ) & 0x8 ) | ( ( indexpart >> (4*CC_SPACECURVE_DIMSPACING1) ) & 0x10 );
    addbits = ( ( addmask >> rotation ) | ( addmask << ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    curveindex = ( curveindex << CC_SPACECURVE_DIMCOUNT ) | addbits;
    addmask = 1 << rotation;
    for( addbits &= -addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ; addbits ; addbits >>= 1 )
      rotation++;
    if( ++rotation >= CC_SPACECURVE_DIMCOUNT )
      rotation -= CC_SPACECURVE_DIMCOUNT;
  } while( --bit >= 0 );

  totalbits = CC_SPACECURVE_DIMCOUNT * mapbits;
  curveindex ^= ( ( ( ( (unsigned int)1 ) << totalbits ) - 1 ) / ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 ) ) >> 1;
  for( bit = 1 ; bit < totalbits ; bit <<= 1 )
    curveindex ^= curveindex >> bit;
  return curveindex;
}

void ccSpaceCurveUnmap5D( int mapbits, unsigned int curveindex, unsigned int *point )
{
  int bit, rotation, totalbits;
  unsigned int pointindex;
  unsigned int addmask, addbits;
  unsigned int p0, p1, p2, p3, p4;

  totalbits = CC_SPACECURVE_DIMCOUNT * mapbits;
  rotation = 0;
  addmask = 0;
  curveindex ^= ( curveindex ^ ( ( ( ( (unsigned int)1 ) << totalbits ) - 1 ) / ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 ) ) ) >> 1;
  pointindex = 0;
  bit = totalbits - CC_SPACECURVE_DIMCOUNT;
  do
  {
    addbits = ( curveindex >> bit ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    pointindex <<= CC_SPACECURVE_DIMCOUNT;
    pointindex |= addmask ^ ( ( ( addbits << rotation ) | ( addbits >> ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 ) );
    addmask = 1 << rotation;
    for( addbits &= -addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ; addbits ; addbits >>= 1 )
      rotation++;
    if( ++rotation >= CC_SPACECURVE_DIMCOUNT )
      rotation -= CC_SPACECURVE_DIMCOUNT;
  } while( ( bit -= CC_SPACECURVE_DIMCOUNT ) >= 0 );
  for( bit = CC_SPACECURVE_DIMCOUNT ; bit < totalbits ; bit <<= 1 )
    pointindex ^= pointindex >> bit;

  p0 = pointindex & 0x1;
  p1 = pointindex & 0x2;
  p2 = pointindex & 0x4;
  p3 = pointindex & 0x8;
  p4 = pointindex & 0x10;
  for( bit = 1 ; bit < mapbits ; bit++ )
  {
    pointindex >>= CC_SPACECURVE_DIMCOUNT;
    p0 |= ( pointindex & 0x1 ) << bit;
    p1 |= ( pointindex & 0x2 ) << bit;
    p2 |= ( pointindex & 0x4 ) << bit;
    p3 |= ( pointindex & 0x8 ) << bit;
    p4 |= ( pointindex & 0x10 ) << bit;
  }
  point[0] = p0 >> 0;
  point[1] = p1 >> 1;
  point[2] = p2 >> 2;
  point[3] = p3 >> 3;
  point[4] = p4 >> 4;

  return;
}


////


#ifdef CC_SPACECURVE_DIMCOUNT
 #undef CC_SPACECURVE_DIMCOUNT
#endif
#ifdef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING
 #undef CC_SPACECURVE_DIMSPACING1
#endif
#ifdef CC_SPACECURVE_MAPBITS
 #undef CC_SPACECURVE_MAPBITS
#endif
#define CC_SPACECURVE_DIMCOUNT (5)
#define CC_SPACECURVE_DIMSPACING (64/CC_SPACECURVE_DIMCOUNT)
#define CC_SPACECURVE_DIMSPACING1 (CC_SPACECURVE_DIMSPACING-1)


uint64_t ccSpaceCurveMap5D64( int mapbits, uint64_t *point )
{
  int bit, rotation, totalbits;
  uint64_t curveindex, pointindex, indexpart;
  uint64_t addmask, addbits;

  pointindex = point[4] ^ ( point[4] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[3] ^ ( point[3] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[2] ^ ( point[2] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[1] ^ ( point[1] >> 1 );
  pointindex <<= CC_SPACECURVE_DIMSPACING;
  pointindex |= point[0] ^ ( point[0] >> 1 );

  rotation = 0;
  addmask = 0;
  curveindex = 0;
  bit = mapbits - 1;
  do
  {
    indexpart = pointindex >> bit;
    addmask ^= ( indexpart & 0x1 ) | ( ( indexpart >> (1*CC_SPACECURVE_DIMSPACING1) ) & 0x2 ) | ( ( indexpart >> (2*CC_SPACECURVE_DIMSPACING1) ) & 0x4 ) | ( ( indexpart >> (3*CC_SPACECURVE_DIMSPACING1) ) & 0x8 ) | ( ( indexpart >> (4*CC_SPACECURVE_DIMSPACING1) ) & 0x10 );
    addbits = ( ( addmask >> rotation ) | ( addmask << ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    curveindex = ( curveindex << CC_SPACECURVE_DIMCOUNT ) | addbits;
    addmask = 1 << rotation;
    for( addbits &= -addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ; addbits ; addbits >>= 1 )
      rotation++;
    if( ++rotation >= CC_SPACECURVE_DIMCOUNT )
      rotation -= CC_SPACECURVE_DIMCOUNT;
  } while( --bit >= 0 );

  totalbits = CC_SPACECURVE_DIMCOUNT * mapbits;
  curveindex ^= ( ( ( ( (uint64_t)1 ) << totalbits ) - 1 ) / ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 ) ) >> 1;
  for( bit = 1 ; bit < totalbits ; bit <<= 1 )
    curveindex ^= curveindex >> bit;
  return curveindex;
}

void ccSpaceCurveUnmap5D64( int mapbits, uint64_t curveindex, uint64_t *point )
{
  int bit, rotation, totalbits;
  uint64_t pointindex;
  uint64_t addmask, addbits;
  uint64_t p0, p1, p2, p3, p4;

  totalbits = CC_SPACECURVE_DIMCOUNT * mapbits;
  rotation = 0;
  addmask = 0;
  curveindex ^= ( curveindex ^ ( ( ( ( (uint64_t)1 ) << totalbits ) - 1 ) / ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 ) ) ) >> 1;
  pointindex = 0;
  bit = totalbits - CC_SPACECURVE_DIMCOUNT;
  do
  {
    addbits = ( curveindex >> bit ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 );
    pointindex <<= CC_SPACECURVE_DIMCOUNT;
    pointindex |= addmask ^ ( ( ( addbits << rotation ) | ( addbits >> ( CC_SPACECURVE_DIMCOUNT - rotation ) ) ) & ( ( 1 << CC_SPACECURVE_DIMCOUNT ) - 1 ) );
    addmask = 1 << rotation;
    for( addbits &= -addbits & ( ( 1 << (CC_SPACECURVE_DIMCOUNT-1) ) - 1 ) ; addbits ; addbits >>= 1 )
      rotation++;
    if( ++rotation >= CC_SPACECURVE_DIMCOUNT )
      rotation -= CC_SPACECURVE_DIMCOUNT;
  } while( ( bit -= CC_SPACECURVE_DIMCOUNT ) >= 0 );
  for( bit = CC_SPACECURVE_DIMCOUNT ; bit < totalbits ; bit <<= 1 )
    pointindex ^= pointindex >> bit;

  p0 = pointindex & 0x1;
  p1 = pointindex & 0x2;
  p2 = pointindex & 0x4;
  p3 = pointindex & 0x8;
  p4 = pointindex & 0x10;
  for( bit = 1 ; bit < mapbits ; bit++ )
  {
    pointindex >>= CC_SPACECURVE_DIMCOUNT;
    p0 |= ( pointindex & 0x1 ) << bit;
    p1 |= ( pointindex & 0x2 ) << bit;
    p2 |= ( pointindex & 0x4 ) << bit;
    p3 |= ( pointindex & 0x8 ) << bit;
    p4 |= ( pointindex & 0x10 ) << bit;
  }
  point[0] = p0 >> 0;
  point[1] = p1 >> 1;
  point[2] = p2 >> 2;
  point[3] = p3 >> 3;
  point[4] = p4 >> 4;

  return;
}


