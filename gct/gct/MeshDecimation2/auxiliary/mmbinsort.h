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

void *mmBinSortInit( size_t itemlistoffset, int rootbucketcount, int groupbucketcount, double rootmin, double rootmax, int groupthreshold, double (*itemvaluecallback)( void *item ), int maxdepth, int numanodeindex );
void mmBinSortFree( void *binsort );

void mmBinSortAdd( void *binsort, void *item, double itemvalue );
void mmBinSortRemove( void *binsort, void *item, double itemvalue );
void mmBinSortUpdate( void *binsort, void *item, double olditemvalue, double newitemvalue );

void *mmBinSortGetRootBucket( void *binsort, int bucketindex, int *itemcount );
void *mmBinSortGetGroupBucket( void *binsortgroup, int bucketindex, int *itemcount );

void *mmBinSortGetFirstItem( void *binsort, double failmax );

