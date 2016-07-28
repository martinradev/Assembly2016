#pragma once

#include <base/Main.hpp>
#include <io/StateDump.hpp>
#include "3d/CameraControls.hpp"
#include <base/Math.hpp>
#include <base/Random.hpp>
#include <3d/Mesh.hpp>
#include <string>
#include <vector>

FW::String readShaderSource(const std::string & source);

FW::GLContext::Program * loadShader(FW::GLContext * gl, const std::vector<std::string> & vertexShader, const std::vector<std::string> & pixelShader, const std::string & programName);
FW::GLContext::Program * loadShader(FW::GLContext * gl, const std::vector<std::string> & vertexShader, const std::vector<std::string> & pixelShader, const std::vector<std::string> & geometryShader, const std::string & programName);
FW::GLContext::Program * loadShader(FW::GLContext * gl, const std::vector<std::string> & computeShader, const std::string & programName, bool print = false);
FW::GLContext::Program * loadShader(FW::GLContext * gl, const std::vector<std::string> & vertexShader, const std::vector<std::string> & tessControlShader, const std::vector<std::string> & tessEvalShader, const std::vector<std::string> & pixelShader, const std::string & programName);

FW::GLContext::Program * loadShader(FW::GLContext * gl, const std::string & vertexShader, const std::string & pixelShader, const std::string & programName);
FW::GLContext::Program * loadShader(FW::GLContext * gl, const std::string & vertexShader, const std::string & tessControlShader, const std::string & tessEvalShader, const std::string & pixelShader, const std::string & programName);
FW::GLContext::Program * loadShader(
	FW::GLContext * gl, 
	const std::string & vertexShader, 
	const std::string & tessControlShader, 
	const std::string & tessEvalShader,
	GLenum geomInputType, GLenum geomOutputType, int geomVerticesOut,
	const std::string & geometryShader, 
	const std::string & pixelShader, 
	const std::string & programName);
FW::GLContext::Program * loadShader(FW::GLContext * gl, const std::string & vertexShader, const std::string & pixelShader, const std::string & geometryShader, const std::string & programName);
FW::GLContext::Program * loadShader(FW::GLContext * gl, const std::string & computeShader, const std::string & programName);