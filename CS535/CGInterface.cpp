//#define GEOM_SHADER

#include "CGInterface.h"
#include "v3.h"
#include "m33.h"
#include "scene.h"

#include <iostream>
#include "global_varibles.h"
#include <GL/glext.h>
using namespace std;
#define GEOM_SHADER

CGInterface::CGInterface()
{
};

void CGInterface::PerSessionInit()
{
	glEnable(GL_DEPTH_TEST);

	CGprofile latestVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
	CGprofile latestGeometryProfile = cgGLGetLatestProfile(CG_GL_GEOMETRY);
	CGprofile latestPixelProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

#ifdef GEOM_SHADER
	if (latestGeometryProfile == CG_PROFILE_UNKNOWN)
	{
		cerr << "ERROR: geometry profile is not available" << endl;
		exit(0);
	}

	cgGLSetOptimalOptions(latestGeometryProfile);
	CGerror Error = cgGetError();
	if (Error)
	{
		cerr << "CG ERROR: " << cgGetErrorString(Error) << endl;
	}
#endif

	cout << "Info: Latest GP Profile Supported: " << cgGetProfileString(latestGeometryProfile) << endl;

	geometryCGprofile = latestGeometryProfile;

	cout << "Info: Latest VP Profile Supported: " << cgGetProfileString(latestVertexProfile) << endl;
	cout << "Info: Latest FP Profile Supported: " << cgGetProfileString(latestPixelProfile) << endl;

	vertexCGprofile = latestVertexProfile;
	pixelCGprofile = latestPixelProfile;
	cgContext = cgCreateContext();
}

bool ShaderOneInterface::PerSessionInit(CGInterface* cgi, const std::string shaderOneFile)
{
	if (shaderOneFile.empty())
	{
		cerr << "Shader one file has not set up! \n";
		return false;
	}

#ifdef GEOM_SHADER
	geometryProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE,
	                                          shaderOneFile.c_str(), cgi->geometryCGprofile, "GeometryMain", nullptr);
	if (geometryProgram == nullptr)
	{
		CGerror Error = cgGetError();
		cerr << "Shader One Geometry Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
		cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
		return false;
	}
#endif

	vertexProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE,
	                                        shaderOneFile.c_str(), cgi->vertexCGprofile, "VertexMain", nullptr);
	if (vertexProgram == nullptr)
	{
		CGerror Error = cgGetError();
		cerr << "Shader One Vertex Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
		cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
		return false;
	}

	fragmentProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE,
	                                          shaderOneFile.c_str(), cgi->pixelCGprofile, "FragmentMain", nullptr);
	if (fragmentProgram == nullptr)
	{
		CGerror Error = cgGetError();
		cerr << "Shader One Fragment Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
		cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
		return false;
	}

	// load programs
#ifdef GEOM_SHADER
	cgGLLoadProgram(geometryProgram);
#endif
	cgGLLoadProgram(vertexProgram);
	cgGLLoadProgram(fragmentProgram);

	// Vertex shader
	vertexModelViewProj = cgGetNamedParameter(vertexProgram, "modelViewProj");
	vertexMorphFraction = cgGetNamedParameter(vertexProgram, "Mf");

	// Geometry shader
	geometryModelViewProj = cgGetNamedParameter(geometryProgram, "modelViewProj");

	// Fragment shader
	fragmentPPCC = cgGetNamedParameter(fragmentProgram, "ppc_C");
	fragmentLightPos = cgGetNamedParameter(fragmentProgram, "light_position");
	fragmentIsST = cgGetNamedParameter(fragmentProgram, "hasST");
	fragmentTex0 = cgGetNamedParameter(fragmentProgram, "tex");
	fragmentCubemapTex = cgGetNamedParameter(fragmentProgram, "env");
	fragmentIsCubemap = cgGetNamedParameter(fragmentProgram, "isCubemap");
	fragmentIsGround = cgGetNamedParameter(fragmentProgram, "isGround");
	fragmentBox0 = cgGetNamedParameter(fragmentProgram, "box0");
	fragmentBox1 = cgGetNamedParameter(fragmentProgram, "box1");
	fragmentBox2 = cgGetNamedParameter(fragmentProgram, "box2");
	fragmentTopTex = cgGetNamedParameter(fragmentProgram, "topTex");
	fragmetGroundHeight = cgGetNamedParameter(fragmentProgram, "groundHeight");
	fragmentBox0Color = cgGetNamedParameter(fragmentProgram, "box0Color");
	fragmentBox1Color = cgGetNamedParameter(fragmentProgram, "box1Color");
	fragmentBox2Color = cgGetNamedParameter(fragmentProgram, "box2Color");

	return true;
}

void ShaderOneInterface::PerFrameInit(uniformVariables &uniforms)
{
	//set parameters
	if(uniforms.is_cubemap)
	{
		cgGLSetStateMatrixParameter(vertexModelViewProj,
			CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

		cgGLSetStateMatrixParameter(
			geometryModelViewProj,
			CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
	}
	else
	{
		cgGLSetStateMatrixParameter(vertexModelViewProj,
			CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

		cgGLSetStateMatrixParameter(
			geometryModelViewProj,
			CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
	}

	// Fragment Shader
	cgSetParameter3fv(fragmentLightPos, reinterpret_cast<float*>(&cur_scene->lightPPCs[0]->C));
	cgSetParameter3fv(fragmentPPCC, reinterpret_cast<float*>(&cur_scene->ppc->C));
	cgSetParameter1i(fragmentIsST, uniforms.is_cube);
	cgSetParameter1i(fragmentIsCubemap, uniforms.is_cubemap);
	cgSetParameter1i(fragmentIsGround, uniforms.is_gnd);

	auto[corner0, x0, y0, z0] = uniforms.box0->get_corners_axis();
	auto[corner1, x1, y1, z1] = uniforms.box1->get_corners_axis();
	auto[corner2, x2, y2, z2] = uniforms.box2->get_corners_axis();
	V3 box0Color = uniforms.box0->colors[0];
	V3 box1Color = uniforms.box1->colors[1];
	V3 box2Color = uniforms.box2->colors[2];

	vector<float> box0Info{ corner0.x(), corner0.y(), corner0.z(), 0.0f,
							x0.x(), x0.y(), x0.z(),0.0f,
							y0.x(), y0.y(), y0.z(),0.0f,
							z0.x(), z0.y(), z0.z(),0.0f };
	vector<float> box1Info{ corner1.x(), corner1.y(), corner1.z(), 0.0f,
						x1.x(), x1.y(), x1.z(),0.0f,
						y1.x(), y1.y(), y1.z(),0.0f,
						z1.x(), z1.y(), z1.z(),0.0f };
	vector<float> box2Info{ corner2.x(), corner2.y(), corner2.z(), 0.0f,
							x2.x(), x2.y(), x2.z(), 0.0f,
							y2.x(), y2.y(), y2.z(), 0.0f,
							z2.x(), z2.y(), z2.z(), 0.0f };
	
	cgSetParameter3fv(fragmentBox0Color, &box0Color[0]);
	cgSetParameter3fv(fragmentBox1Color, &box1Color[0]);
	cgSetParameter3fv(fragmentBox2Color, &box2Color[0]);

	cgSetMatrixParameterfr(fragmentBox0, &box0Info[0]);
	cgSetMatrixParameterfr(fragmentBox1, &box1Info[0]);
	cgSetMatrixParameterfr(fragmentBox2, &box2Info[0]);
	cgSetParameter1f(fragmetGroundHeight, cur_scene->meshes[3]->verts[0].y());	

	if(uniforms.is_cube)
	{
		cgGLSetTextureParameter(fragmentTex0, FrameBuffer::gpu_tex_ID.at(uniforms.tex));
		cgGLEnableTextureParameter(fragmentTex0);
	}

	cgGLSetTextureParameter(fragmentCubemapTex, FrameBuffer::gpu_tex_ID.at(cubemap_files[0]));
	cgGLEnableTextureParameter(fragmentCubemapTex);

	cgGLSetTextureParameter(fragmentTopTex, FrameBuffer::gpu_tex_ID.at("images/tex.tiff"));
	cgGLEnableTextureParameter(fragmentTopTex);
}

void ShaderOneInterface::PerFrameDisable()
{
}


void ShaderOneInterface::BindPrograms()
{
#ifdef GEOM_SHADER
	cgGLBindProgram(geometryProgram);
#endif
	cgGLBindProgram(vertexProgram);
	cgGLBindProgram(fragmentProgram);
}

void CGInterface::DisableProfiles()
{
	cgGLDisableProfile(vertexCGprofile);
#ifdef GEOM_SHADER
	cgGLDisableProfile(geometryCGprofile);
#endif
	cgGLDisableProfile(pixelCGprofile);
}

void CGInterface::EnableProfiles()
{
	cgGLEnableProfile(vertexCGprofile);
#ifdef GEOM_SHADER
	cgGLEnableProfile(geometryCGprofile);
#endif
	cgGLEnableProfile(pixelCGprofile);
}

