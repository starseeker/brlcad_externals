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

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
  double progress;
  int stage;
  const char *stagename;
  long trianglecount;
} mdStatus;

enum
{
  MD_STATUS_STAGE_INIT,
  MD_STATUS_STAGE_BUILDVERTICES,
  MD_STATUS_STAGE_BUILDTRIANGLES,
  MD_STATUS_STAGE_BUILDTRIREFS,
  MD_STATUS_STAGE_BUILDQUEUE,
  MD_STATUS_STAGE_DECIMATION,
  MD_STATUS_STAGE_STORE,
  MD_STATUS_STAGE_DONE,

  MD_STATUS_STAGE_COUNT
};


typedef struct
{
  /* Input vertex data */
  size_t vertexcount;
  void *vertex;
  /* Supported vertex formats: MD_FORMAT_FLOAT, MD_FORMAT_DOUBLE, MD_FORMAT_SHORT, MD_FORMAT_INT, MD_FORMAT_INT16, MD_FORMAT_INT32 */
  int vertexformat;
  size_t vertexstride;
  /* Can allocate more vertices than the initial count, in order to have spare vertices for cloning when computing normals */
  size_t vertexalloc;

  /* Input indices data */
  void *indices;
  /* Supported index formats: MD_FORMAT_UBYTE, MD_FORMAT_USHORT, MD_FORMAT_UINT, MD_FORMAT_UINT8, MD_FORMAT_UINT16, MD_FORMAT_UINT32, MD_FORMAT_UINT64 */
  int indicesformat;
  size_t indicesstride;

  /* Optional per-triangle custom data */
  size_t tricount;
  void *tridata;
  size_t tridatasize;

  /* Optional callback, if tridata is provided, can return a non-zero edge factor between two connected triangles */
  double (*edgeweight)( void *tridata0, void *tridata1 );

  /* Optional callback, merge attributes for two vertices with the given blending factors */
  void (*vertexmerge)( void *mergecontext, int dstindex, int srcindex, double weight0, double weight1 );
  void *mergecontext;

  /* Optional callback, copy custom vertex attributes, only used when asked to compute vertex normals */
  void (*vertexcopy)( void *mergecontext, int dstindex, int srcindex );
  void *copycontext;

  /* Feature size */
  double decimationstrength;

  /* To compute vertex normals */
  void *normalbase;
  /* Supported normal formats: MD_FORMAT_FLOAT, MD_FORMAT_DOUBLE, MD_FORMAT_BYTE, MD_FORMAT_SHORT, MD_FORMAT_INT8, MD_FORMAT_INT16, MD_FORMAT_INT_2_10_10_10_REV */
  int normalformat;
  size_t normalstride;

  /* Output: Count of edge reductions */
  long decimationcount;
  /* Output: Count of edges that were reused by triangles */
  /* Any non-zero count indicates mesh topology errors in the input data */
  long collisioncount;

  /* Output: Time spent performing the decimation */
  long msecs;

  /* Status callback */
  long statusmilliseconds;
  void *statuscontext;
  void (*statuscallback)( void *statuscontext, const mdStatus *status );

  /* Advanced configuration options */
  double compactnesstarget;
  double compactnesspenalty;
  /* Factor by which to increase the stiffness of boundaries, edges without opposite edges */
  double boundaryweight;
  /* How many ops are executed before updating the evaluations of all ops that require it */
  int syncstepcount;
  /* For optional normal smoothing, maximum angle in degrees for merged smoothing */
  double normalsearchangle;
  /* Maximum allowed memory usage, mdMeshDecimation() can return zero and fail if that's too low to work */
  size_t maxmemorysize;

} mdOperation;


enum
{
  /* Type format for vertexformat, indicesformat, normalformat */
  MD_FORMAT_FLOAT,
  MD_FORMAT_DOUBLE,
  MD_FORMAT_BYTE,
  MD_FORMAT_SHORT,
  MD_FORMAT_INT,
  MD_FORMAT_UBYTE,
  MD_FORMAT_USHORT,
  MD_FORMAT_UINT,
  MD_FORMAT_INT8,
  MD_FORMAT_INT16,
  MD_FORMAT_INT32,
  MD_FORMAT_INT64,
  MD_FORMAT_UINT8,
  MD_FORMAT_UINT16,
  MD_FORMAT_UINT32,
  MD_FORMAT_UINT64,
  MD_FORMAT_INT_2_10_10_10_REV
};


/* Initialize mdOperation with default values */
void mdOperationInit( mdOperation *op );

/* Set vertex and indices input data */
void mdOperationData( mdOperation *op, size_t vertexcount, void *vertex, int vertexformat, size_t vertexstride, size_t tricount, void *indices, int indicesformat, size_t indicesstride );

/* Set decimation strength, feature size proportional to scale of model */
void mdOperationStrength( mdOperation *op, double decimationstrength );

/* Set optional per-triangle data and callback to return a edge weight between two triangles */
void mdOperationTriData( mdOperation *op, void *tridata, size_t tridatasize, double (*edgeweight)( void *tridata0, void *tridata1 ) );

/* Set optional callback to copy vertex attributes (excluding vertex position) */
void mdOperationVertexCopy( mdOperation *op, void (*vertexcopy)( void *copycontext, int dstindex, int srcindex ), void *copycontext );

/* Set optional callback to blend vertex attributes (exclude vertex position) */
void mdOperationVertexMerge( mdOperation *op, void (*vertexmerge)( void *mergecontext, int dstindex, int srcindex, double weight0, double weight1 ), void *mergecontext );

/* Set optional computation and storage of normals */
void mdOperationComputeNormals( mdOperation *op, void *base, int format, size_t stride );

/* Set optional callback to receive progress updates */
void mdOperationStatusCallback( mdOperation *op, void (*statuscallback)( void *statuscontext, const mdStatus *status ), void *statuscontext, long milliseconds );


/* Decimate the mesh specified by the mdOperation struct */
int mdMeshDecimation( mdOperation *operation, int threadcount, int flags );


/* Slightly increase the quality of aggressive mesh decimations, about 50% slower (or >100% slower without SSE) */
#define MD_FLAGS_CONTINUOUS_UPDATE (0x1)
/* Do not pack vertices, leave unused vertices in place */
#define MD_FLAGS_NO_VERTEX_PACKING (0x2)
/* When recomputing normals, allow cloning/splitting of vertices with diverging normals */
#define MD_FLAGS_NORMAL_VERTEX_SPLITTING (0x4)
/* Define orientation of triangles when rebuilding normals */
#define MD_FLAGS_TRIANGLE_WINDING_CW (0x8)
#define MD_FLAGS_TRIANGLE_WINDING_CCW (0x10)
/* Do not actually do any decimation, in case all you actually want to do is to recompute normals */
#define MD_FLAGS_NO_DECIMATION (0x20)


#ifdef __cplusplus
}
#endif

