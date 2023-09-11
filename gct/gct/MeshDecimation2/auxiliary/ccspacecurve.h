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


unsigned int ccSpaceCurveMap2D( int mapbits, unsigned int *point );
void ccSpaceCurveUnmap2D( int mapbits, unsigned int curveindex, unsigned int *point );
uint64_t ccSpaceCurveMap2D64( int mapbits, uint64_t *point );
void ccSpaceCurveUnmap2D64( int mapbits, uint64_t curveindex, uint64_t *point );


unsigned int ccSpaceCurveMap3D( int mapbits, unsigned int *point );
void ccSpaceCurveUnmap3D( int mapbits, unsigned int curveindex, unsigned int *point );
uint64_t ccSpaceCurveMap3D64( int mapbits, uint64_t *point );
void ccSpaceCurveUnmap3D64( int mapbits, uint64_t curveindex, uint64_t *point );

unsigned int ccSpaceCurveMap3D10Bits( unsigned int *point );
void ccSpaceCurveUnmap3D10Bits( unsigned int curveindex, unsigned int *point );
uint64_t ccSpaceCurveMap3D21Bits64( uint64_t *point );
void ccSpaceCurveUnmap3D21Bits64( uint64_t curveindex, uint64_t *point );


unsigned int ccSpaceCurveMap4D( int mapbits, unsigned int *point );
void ccSpaceCurveUnmap4D( int mapbits, unsigned int curveindex, unsigned int *point );
uint64_t ccSpaceCurveMap4D64( int mapbits, uint64_t *point );
void ccSpaceCurveUnmap4D64( int mapbits, uint64_t curveindex, uint64_t *point );


unsigned int ccSpaceCurveMap5D( int mapbits, unsigned int *point );
void ccSpaceCurveUnmap5D( int mapbits, unsigned int curveindex, unsigned int *point );
uint64_t ccSpaceCurveMap5D64( int mapbits, uint64_t *point );
void ccSpaceCurveUnmap5D64( int mapbits, uint64_t curveindex, uint64_t *point );

