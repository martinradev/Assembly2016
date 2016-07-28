#include "ProceduralTexturing.h"
#include "ShaderSetup.h"

using namespace FW;

void ProceduralTexture::loadShaders() {
	loadShader(gl_, shadersInfo_[0], shadersInfo_[1], shadersInfo_[2]);
}

ProceduralTexture::ProceduralTexture(
	FW::GLContext * gl,
	PlaneTestObject * planeObject, 
	const std::string & vShader, const std::string & pShader, const std::string & programName) :
gl_(gl),
planeObject_(planeObject)
{
	setProgramInfo(vShader, pShader, programName);
	loadShaders();
}

void ProceduralTexture::start() {
	
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.3f, 0.5f, 0.8f, 1.0f);
	timer_.start();

}

void ProceduralTexture::render(GenericBuffer * buffer) const {

	updateProgramUniforms();

	GLContext::Program * prog = gl_->getProgram(shadersInfo_[2].c_str());

	FW::Mat4f ident;
	ident.setIdentity();

	if (buffer != nullptr) {
		buffer->activate();
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	planeObject_->render(gl_, prog, ident, ident);

	if (buffer != nullptr) {
		buffer->deactivate();
	}

}

void ProceduralTexture::updateProgramUniforms() const {
	GLContext::Program * prog = gl_->getProgram(shadersInfo_[2].c_str());
	prog->use();

	gl_->setUniform(prog->getUniformLoc("windowSize"), windowSize_);
	gl_->setUniform(prog->getUniformLoc("time"), timer_.getElapsed());
}