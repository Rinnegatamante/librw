#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../rwbase.h"
#include "../rwerror.h"
#include "../rwplg.h"
#include "../rwrender.h"
#include "../rwpipeline.h"
#include "../rwobjects.h"
#include "../rwengine.h"
#ifdef RW_OPENGL
// #include <GL/glew.h>
#include "rwgl3.h"
#include "rwgl3impl.h"
#include "rwgl3shader.h"

#include "psp2_shaders.h"

extern float *gVertexBufferIm2D;
extern uint16_t *gIndicesIm2D;
extern float *gVertexBufferIm3D;
extern uint16_t *gIndicesIm3D;
extern uint16_t *gConstIndices;

namespace rw {
namespace gl3 {

// uint32 im2DVbo, im2DIbo;
// #ifdef RW_GL_USE_VAOS
// uint32 im2DVao;
// #endif

Shader *im2dOverrideShader;

static int32 u_xform;

#define STARTINDICES 10000
#define STARTVERTICES 10000

static Shader *im2dShader;
static AttribDesc im2dattribDesc[3] = {
	{ ATTRIB_POS,        GL_FLOAT,         GL_FALSE, 4,
		sizeof(Im2DVertex), 0 },
	{ ATTRIB_COLOR,      GL_UNSIGNED_BYTE, GL_TRUE,  4,
		sizeof(Im2DVertex), offsetof(Im2DVertex, r) },
	{ ATTRIB_TEXCOORDS0, GL_FLOAT,         GL_FALSE, 2,
		sizeof(Im2DVertex), offsetof(Im2DVertex, u) },
};

static int primTypeMap[] = {
	GL_POINTS,	// invalid
	GL_LINES,
	GL_LINE_STRIP,
	GL_TRIANGLES,
	GL_TRIANGLE_STRIP,
	GL_TRIANGLE_FAN,
	GL_POINTS
};

void
openIm2D(void)
{
	u_xform = registerUniform("u_xform");

#include "shaders/im2d_gl.h"
#include "shaders/simple_fs_gl.h"
	const char *vs[] = { (const char*)im2d_v, (const char*)&size_im2d_v, nil };
	const char *fs[] = { (const char*)simple_f, (const char*)&size_simple_f, nil };
	im2dShader = Shader::create(vs, fs, true);
	assert(im2dShader);

	// glGenBuffers(1, &im2DIbo);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, im2DIbo);
	// glBufferData(GL_ELEMENT_ARRAY_BUFFER, STARTINDICES*2, nil, GL_STREAM_DRAW);

	// glGenBuffers(1, &im2DVbo);
	// glBindBuffer(GL_ARRAY_BUFFER, im2DVbo);
	// glBufferData(GL_ARRAY_BUFFER, STARTVERTICES*sizeof(Im2DVertex), nil, GL_STREAM_DRAW);

// #ifdef RW_GL_USE_VAOS
	// glGenVertexArrays(1, &im2DVao);
	// glBindVertexArray(im2DVao);
	// setAttribPointers(im2dattribDesc, 3);
// #endif
}

void
closeIm2D(void)
{
	// glDeleteBuffers(1, &im2DIbo);
	// glDeleteBuffers(1, &im2DVbo);
// #ifdef RW_GL_USE_VAOS
	// glDeleteVertexArrays(1, &im2DVao);
// #endif
	im2dShader->destroy();
	im2dShader = nil;
}

static Im2DVertex tmpprimbuf[3];

void
im2DRenderLine(void *vertices, int32 numVertices, int32 vert1, int32 vert2)
{
	Im2DVertex *verts = (Im2DVertex*)vertices;
	tmpprimbuf[0] = verts[vert1];
	tmpprimbuf[1] = verts[vert2];
	im2DRenderPrimitive(PRIMTYPELINELIST, tmpprimbuf, 2);
}

void
im2DRenderTriangle(void *vertices, int32 numVertices, int32 vert1, int32 vert2, int32 vert3)
{
	Im2DVertex *verts = (Im2DVertex*)vertices;
	tmpprimbuf[0] = verts[vert1];
	tmpprimbuf[1] = verts[vert2];
	tmpprimbuf[2] = verts[vert3];
	im2DRenderPrimitive(PRIMTYPETRILIST, tmpprimbuf, 3);
}

void
im2DRenderPrimitive(PrimitiveType primType, void *vertices, int32 numVertices)
{
	GLfloat xform[4];
	Camera *cam;
	cam = (Camera*)engine->currentCamera;

// #ifdef RW_GL_USE_VAOS
	// glBindVertexArray(im2DVao);
// #endif

	// glBindBuffer(GL_ARRAY_BUFFER, im2DVbo);
	// glBufferData(GL_ARRAY_BUFFER, STARTVERTICES*sizeof(Im2DVertex), nil, GL_STREAM_DRAW);
	// glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices*sizeof(Im2DVertex), vertices);

	xform[0] = 2.0f/cam->frameBuffer->width;
	xform[1] = -2.0f/cam->frameBuffer->height;
	xform[2] = -1.0f;
	xform[3] = 1.0f;

	if(im2dOverrideShader)
		im2dOverrideShader->use();
	else
		im2dShader->use();
// #ifndef RW_GL_USE_VAOS
	// setAttribPointers(im2dattribDesc, 3);
// #endif

	flushCache();
	glUniform4fv(currentShader->uniformLocations[u_xform], 1, xform);
	
	sceClibMemcpy(gVertexBufferIm2D, vertices, numVertices*sizeof(Im2DVertex));
	vglVertexAttribPointerMapped(0, gVertexBufferIm2D);
	vglIndexPointerMapped(gConstIndices);
	gVertexBufferIm2D += numVertices*(sizeof(Im2DVertex)/sizeof(float));
	
	vglDrawObjects(primTypeMap[primType], numVertices, GL_FALSE);
	
// #ifndef RW_GL_USE_VAOS
	// disableAttribPointers(im2dattribDesc, 3);
// #endif
}

void
im2DRenderIndexedPrimitive(PrimitiveType primType,
	void *vertices, int32 numVertices,
	void *indices, int32 numIndices)
{
	GLfloat xform[4];
	Camera *cam;
	cam = (Camera*)engine->currentCamera;

// #ifdef RW_GL_USE_VAOS
	// glBindVertexArray(im2DVao);
// #endif

	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, im2DIbo);
	// glBufferData(GL_ELEMENT_ARRAY_BUFFER, STARTINDICES*2, nil, GL_STREAM_DRAW);
	// glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, numIndices*2, indices);

	// glBindBuffer(GL_ARRAY_BUFFER, im2DVbo);
	// glBufferData(GL_ARRAY_BUFFER, STARTVERTICES*sizeof(Im2DVertex), nil, GL_STREAM_DRAW);
	// glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices*sizeof(Im2DVertex), vertices);

	xform[0] = 2.0f/cam->frameBuffer->width;
	xform[1] = -2.0f/cam->frameBuffer->height;
	xform[2] = -1.0f;
	xform[3] = 1.0f;

	if(im2dOverrideShader)
		im2dOverrideShader->use();
	else
		im2dShader->use();
// #ifndef RW_GL_USE_VAOS
	// setAttribPointers(im2dattribDesc, 3);
// #endif

	flushCache();
	glUniform4fv(currentShader->uniformLocations[u_xform], 1, xform);
	
	sceClibMemcpy(gIndicesIm2D, indices, numIndices * 2);
	vglIndexPointerMapped(gIndicesIm2D);
	gIndicesIm2D += numIndices;
	
	sceClibMemcpy(gVertexBufferIm2D, vertices, numVertices*sizeof(Im2DVertex));
	vglVertexAttribPointerMapped(0, gVertexBufferIm2D);
	gVertexBufferIm2D += numVertices*(sizeof(Im2DVertex)/sizeof(float));
		
	vglDrawObjects(primTypeMap[primType], numIndices, GL_FALSE);
	
// #ifndef RW_GL_USE_VAOS
	// disableAttribPointers(im2dattribDesc, 3);
// #endif
}


// Im3D


static Shader *im3dShader;
static AttribDesc im3dattribDesc[3] = {
	{ ATTRIB_POS,        GL_FLOAT,         GL_FALSE, 3,
		sizeof(Im3DVertex), 0 },
	{ ATTRIB_COLOR,      GL_UNSIGNED_BYTE, GL_TRUE,  4,
		sizeof(Im3DVertex), offsetof(Im3DVertex, r) },
	{ ATTRIB_TEXCOORDS0, GL_FLOAT,         GL_FALSE, 2,
		sizeof(Im3DVertex), offsetof(Im3DVertex, u) },
};
static uint32 im3DVbo, im3DIbo;
#ifdef RW_GL_USE_VAOS
static uint32 im3DVao;
#endif
static int32 num3DVertices;	// not actually needed here

void
openIm3D(void)
{
#include "shaders/im3d_gl.h"
#include "shaders/simple_fs_gl.h"
	const char *vs[] = { (const char*)im3d_v, (const char*)&size_im3d_v, nil };
	const char *fs[] = { (const char*)simple_f, (const char*)&size_simple_f, nil };
	im3dShader = Shader::create(vs, fs, false);
	assert(im3dShader);

	// glGenBuffers(1, &im3DIbo);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, im3DIbo);
	// glBufferData(GL_ELEMENT_ARRAY_BUFFER, STARTINDICES*2, nil, GL_STREAM_DRAW);

	// glGenBuffers(1, &im3DVbo);
	// glBindBuffer(GL_ARRAY_BUFFER, im3DVbo);
	// glBufferData(GL_ARRAY_BUFFER, STARTVERTICES*sizeof(Im3DVertex), nil, GL_STREAM_DRAW);

// #ifdef RW_GL_USE_VAOS
	// glGenVertexArrays(1, &im3DVao);
	// glBindVertexArray(im3DVao);
	// setAttribPointers(im3dattribDesc, 3);
// #endif
}

void
closeIm3D(void)
{
	// glDeleteBuffers(1, &im3DIbo);
	// glDeleteBuffers(1, &im3DVbo);
// #ifdef RW_GL_USE_VAOS
	// glDeleteVertexArrays(1, &im3DVao);
// #endif
	im3dShader->destroy();
	im3dShader = nil;
}

void
im3DTransform(void *vertices, int32 numVertices, Matrix *world, uint32 flags)
{
	if(world == nil){
		static Matrix ident;
		ident.setIdentity();
		world = &ident;
	}
	setWorldMatrix(world);
	im3dShader->use();

	if((flags & im3d::VERTEXUV) == 0)
		SetRenderStatePtr(TEXTURERASTER, nil);

	sceClibMemcpy(gVertexBufferIm3D, vertices, numVertices*sizeof(Im3DVertex));
// #ifdef RW_GL_USE_VAOS
	// glBindVertexArray(im2DVao);
// #endif

	// glBindBuffer(GL_ARRAY_BUFFER, im3DVbo);
	// glBufferData(GL_ARRAY_BUFFER, STARTVERTICES*sizeof(Im3DVertex), nil, GL_STREAM_DRAW);
	// glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices*sizeof(Im3DVertex), vertices);
// #ifndef RW_GL_USE_VAOS
	// setAttribPointers(im3dattribDesc, 3);
// #endif
	num3DVertices = numVertices;
}

void
im3DRenderPrimitive(PrimitiveType primType)
{
	flushCache();
	vglIndexPointerMapped(gConstIndices);
	vglVertexAttribPointerMapped(0, gVertexBufferIm3D);
	gVertexBufferIm3D += num3DVertices*(sizeof(Im3DVertex)/sizeof(float));
	vglDrawObjects(primTypeMap[primType], num3DVertices, GL_FALSE);
}

void
im3DRenderIndexedPrimitive(PrimitiveType primType, void *indices, int32 numIndices)
{
	flushCache();
	sceClibMemcpy(gIndicesIm3D, indices, numIndices * 2);
	vglIndexPointerMapped(gIndicesIm3D);
	vglVertexAttribPointerMapped(0, gVertexBufferIm3D);
	gVertexBufferIm3D += num3DVertices*(sizeof(Im3DVertex)/sizeof(float));
	gIndicesIm3D += numIndices;
	vglDrawObjects(primTypeMap[primType], numIndices, GL_FALSE);
}

void
im3DEnd(void)
{
// #ifndef RW_GL_USE_VAOS
	// disableAttribPointers(im3dattribDesc, 3);
// #endif
}

}
}

#endif
