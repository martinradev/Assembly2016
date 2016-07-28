#include "ShaderSetup.h"
#include "io/File.hpp"
#include <string>
#include <cassert>
#include <cstdio>
#include <vector>
#include <iostream>

using namespace std;
using namespace FW;

String readShaderSource(const std::string & source) {
	File vsf(source.c_str(), File::Read);
	char* vsa = new char[vsf.getSize() + 1];
	vsf.readFully(vsa, (S32)vsf.getSize());
	vsa[vsf.getSize()] = 0;
	String vs(vsa);
	delete[] vsa;
	return vs;
}

GLContext::Program* loadShader(FW::GLContext * gl,
	const std::vector<std::string> & vertexShader,
	const std::vector<std::string> & pixelShader,
	const std::string & programName) {

	String vs;
	String ps;

	for (size_t i = 0; i < vertexShader.size(); ++i) {
		vs.append(readShaderSource(vertexShader[i]));
	}

	for (size_t i = 0; i < pixelShader.size(); ++i) {
		ps.append(readShaderSource(pixelShader[i]));
	}

	GLContext::Program* pProgram = nullptr;
	try
	{
		pProgram = new GLContext::Program(vs, ps, true);
		gl->setProgram(programName.c_str(), pProgram);
	}
	catch (GLContext::Program::ShaderCompilationException& e)
	{
		::printf("Could not compile shader %s:\n%s\n", programName.c_str(), e.msg_.getPtr());

		fail("Fatal, need a shader to draw!");
	}
	return pProgram;
}



GLContext::Program* loadShader(
	FW::GLContext * gl,
	const std::vector<std::string> & vertexShader,
	const std::vector<std::string> & pixelShader,
	const std::vector<std::string> & geometryShader,
	const std::string & programName) {

	String vs, ps, gs;

	for (size_t i = 0; i < vertexShader.size(); ++i) {
		vs.append(readShaderSource(vertexShader[i]));
	}

	for (size_t i = 0; i < pixelShader.size(); ++i) {
		ps.append(readShaderSource(pixelShader[i]));
	}

	for (size_t i = 0; i < geometryShader.size(); ++i) {
		gs.append(readShaderSource(geometryShader[i]));
	}

	GLContext::Program* pProgram = nullptr;
	try
	{
		pProgram = new GLContext::Program(vs, GL_POINTS, GL_TRIANGLE_STRIP, 36, gs, ps, true);
		gl->setProgram(programName.c_str(), pProgram);
	}
	catch (GLContext::Program::ShaderCompilationException& e)
	{
		::printf("Could not compile shader %s:\n%s\n", programName.c_str(), e.msg_.getPtr());

		fail("Fatal, need a shader to draw!");

	}

	return pProgram;
}

GLContext::Program* loadShader(
	FW::GLContext * gl,
	const std::vector<std::string> & vertexShader,
	const std::vector<std::string> & tessControlShader,
	const std::vector<std::string> & tessEvalShader,
	const std::vector<std::string> & pixelShader,
	const std::string & programName) {

	String vs, ps, tcs, tes;

	for (size_t i = 0; i < vertexShader.size(); ++i) {
		vs.append(readShaderSource(vertexShader[i]));
	}

	for (size_t i = 0; i < pixelShader.size(); ++i) {
		ps.append(readShaderSource(pixelShader[i]));
	}

	for (size_t i = 0; i < tessControlShader.size(); ++i) {
		tcs.append(readShaderSource(tessControlShader[i]));
	}
	for (size_t i = 0; i < tessEvalShader.size(); ++i) {
		tes.append(readShaderSource(tessEvalShader[i]));
	}

	GLContext::Program* pProgram = nullptr;
	try
	{
		pProgram = new GLContext::Program(vs, tcs, tes, ps, true);
		gl->setProgram(programName.c_str(), pProgram);
	}
	catch (GLContext::Program::ShaderCompilationException& e)
	{
		::printf("Could not compile shader %s:\n%s\n", programName.c_str(), e.msg_.getPtr());

		fail("Fatal, need a shader to draw!");

	}

	return pProgram;
}

GLContext::Program* loadShader(
	FW::GLContext * gl,
	const std::vector<std::string> & computeShader,
	const std::string & programName, bool print) {

	String vs;

	for (size_t i = 0; i < computeShader.size(); ++i) {
		vs.append(readShaderSource(computeShader[i]));
	}

	GLContext::Program* pProgram = nullptr;
	try
	{
		pProgram = new GLContext::Program(vs, true);
		gl->setProgram(programName.c_str(), pProgram);
	}
	catch (GLContext::Program::ShaderCompilationException& e)
	{
		::printf("Could not compile shader %s:\n%s\n", programName.c_str(), e.msg_.getPtr());

		fail("Fatal, need a shader to draw!");

	}

	return pProgram;
}


GLContext::Program* loadShader(
	FW::GLContext * gl, 
	const std::string & vertexShader, 
	const std::string & pixelShader, 
	const std::string & programName) {
	

	String vs = readShaderSource(vertexShader);
	String ps = readShaderSource(pixelShader);

	GLContext::Program* pProgram = nullptr;
	try
	{
		pProgram = new GLContext::Program(vs, ps, true);
		gl->setProgram(programName.c_str(), pProgram);
	}
	catch (GLContext::Program::ShaderCompilationException& e)
	{
		::printf("Could not compile shader %s:\n%s\n", programName.c_str(), e.msg_.getPtr());

		fail("Fatal, need a shader to draw!");
	}

	return pProgram;
}

GLContext::Program* loadShader(
	FW::GLContext * gl,
	const std::string & vertexShader,
	const std::string & tessControlShader,
	const std::string & tessEvalShader,
	const std::string & pixelShader,
	const std::string & programName) {


	String vs = readShaderSource(vertexShader);
	String tcs = readShaderSource(tessControlShader);
	String tes = readShaderSource(tessEvalShader);
	String ps = readShaderSource(pixelShader);

	GLContext::Program* pProgram = nullptr;
	try
	{
		pProgram = new GLContext::Program(vs, tcs, tes, ps, true);
		gl->setProgram(programName.c_str(), pProgram);
	}
	catch (GLContext::Program::ShaderCompilationException& e)
	{
		::printf("Could not compile shader %s:\n%s\n", programName.c_str(), e.msg_.getPtr());

		fail("Fatal, need a shader to draw!");

	}

	return pProgram;
}

FW::GLContext::Program * loadShader(
	FW::GLContext * gl, 
	const std::string & vertexShader, 
	const std::string & tessControlShader, 
	const std::string & tessEvalShader,
	GLenum geomInputType, GLenum geomOutputType, int geomVerticesOut,
	const std::string & geometryShader, 
	const std::string & pixelShader, 
	const std::string & programName)
{
	String vs = readShaderSource(vertexShader);
	String tcs = readShaderSource(tessControlShader);
	String tes = readShaderSource(tessEvalShader);
	String gs = readShaderSource(geometryShader);
	String ps = readShaderSource(pixelShader);

	GLContext::Program* pProgram = nullptr;
	try
	{
		pProgram = new GLContext::Program(vs, tcs, tes, geomInputType, geomOutputType, geomVerticesOut, gs, ps, true);
		gl->setProgram(programName.c_str(), pProgram);
	}
	catch (GLContext::Program::ShaderCompilationException& e)
	{
		::printf("Could not compile shader %s:\n%s\n", programName.c_str(), e.msg_.getPtr());

		fail("Fatal, need a shader to draw!");

	}

	return pProgram;
}

GLContext::Program* loadShader(
	FW::GLContext * gl,
	const std::string & vertexShader,
	const std::string & pixelShader,
	const std::string & geometryShader,
	const std::string & programName) {

	String vs = readShaderSource(vertexShader);
	String ps = readShaderSource(pixelShader);
	String gs = readShaderSource(geometryShader);

	GLContext::Program* pProgram = nullptr;
	try
	{
		pProgram = new GLContext::Program(vs, GL_POINTS, GL_TRIANGLE_STRIP, 36, gs, ps, true);
		gl->setProgram(programName.c_str(), pProgram);
	}
	catch (GLContext::Program::ShaderCompilationException& e)
	{
		::printf("Could not compile shader %s:\n%s\n", programName.c_str(), e.msg_.getPtr());

		fail("Fatal, need a shader to draw!");

	}

	return pProgram;
}

GLContext::Program* loadShader(
	FW::GLContext * gl,
	const std::string & computeShader,
	const std::string & programName) {

	String vs = readShaderSource(computeShader);

	GLContext::Program* pProgram = nullptr;
	try
	{
		pProgram = new GLContext::Program(vs, true);
		gl->setProgram(programName.c_str(), pProgram);
	}
	catch (GLContext::Program::ShaderCompilationException& e)
	{
		::printf("Could not compile shader %s:\n%s\n", programName.c_str(), e.msg_.getPtr());

		fail("Fatal, need a shader to draw!");
	}

	return pProgram;
}