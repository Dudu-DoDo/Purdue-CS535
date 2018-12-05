#pragma once

#include <Cg/cgGL.h>
#include <Cg/cg.h>
#include <string>
#include <vector>
#include <memory>

class TM;

struct uniformVariables
{
	int is_cube;
	int is_cubemap;
	int is_box;
	int is_gnd;
	std::string tex;
	std::shared_ptr<TM> box0;		
	std::shared_ptr<TM> box1;
	std::shared_ptr<TM> box2;		
};

// two classes defining the interface between the CPU and GPU
// models part of the CPU-GPU interface that is independent of specific shaders
class CGInterface
{
public:
	CGprofile vertexCGprofile; // vertex shader profile of GPU
	CGprofile geometryCGprofile; // geometry shader profile of GPU
	CGprofile pixelCGprofile; // pixel shader profile of GPU
	CGcontext cgContext;
	void PerSessionInit(); // per session initialization
	CGInterface(); // constructor
	void EnableProfiles(); // enable GPU rendering
	void DisableProfiles(); // disable GPU rendering
};


// models the part of the CPU-GPU interface for a specific shader
//        here there is a single shader "ShaderOne"
//  a shader consists of a vertex, a geometry, and a pixel (fragment) shader
// fragment == pixel; shader == program; e.g. pixel shader, pixel program, fragment shader, fragment program, vertex shader, etc.
class ShaderOneInterface
{
	CGprogram geometryProgram; // the geometry shader to be used for the "ShaderOne"
	CGprogram vertexProgram;
	CGprogram fragmentProgram;
	// uniform parameters, i.e parameters that have the same value for all geometry rendered
	CGparameter vertexModelViewProj; // a matrix combining projection and modelview matrices
	CGparameter vertexMorphFraction;

	CGparameter geometryModelViewProj; // geometry shader
	CGparameter fragmentKa; // ambient coefficient for fragment shader
	CGparameter fragmentLightPos, fragmentPPCC;
	CGparameter fragmentIsST;
	CGparameter fragmentTex0;
	CGparameter fragmentCubemapTex;
	CGparameter fragmentIsCubemap;

	// Hard coded
	// The other two boxes
	CGparameter fragmentBox0;
	CGparameter fragmentBox1;
	CGparameter fragmentBox2;
	CGparameter fragmentIsGround;
	CGparameter fragmetGroundHeight;
	CGparameter fragmentTopTex;
	CGparameter fragmentBox0Color;
	CGparameter fragmentBox1Color;
	CGparameter fragmentBox2Color;

public:
	ShaderOneInterface()
	{
	};
	bool PerSessionInit(CGInterface* cgi, const std::string shaderOneFile); // per session initialization
	void BindPrograms(); // enable geometryProgram, vertexProgram, fragmentProgram
	void PerFrameInit(uniformVariables &uniforms); // set uniform parameter values, etc.

	void PerFrameDisable(); // disable programs
};
