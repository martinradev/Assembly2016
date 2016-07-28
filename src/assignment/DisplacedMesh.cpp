#include "DisplacedMesh.h"
#include "ShaderSetup.h"
#include "MeshRenderHelper.h"

#include "Globals.h"

namespace FW {

	DisplacedMesh::DisplacedMesh(
		GLContext * gl,
		Mesh<VertexPNTC> * mesh, 
		const std::string & vertexShader, 
		const std::string & tessControlShader,
		const std::string & tessEvalShader, 
		const std::string & fragmentShader,
		const std::string & programName)
		:
		m_vertexShader(vertexShader),
		m_tessControlShader(tessControlShader),
		m_tessEvalShader(tessEvalShader),
		m_fragmentShader(fragmentShader),
		m_program(programName),
		m_mesh(mesh)
	{

		loadShaders(gl);

	}
	void DisplacedMesh::render(GBuffer * gbuffer, Window & wnd, const CameraControls & camera, const Mat4f & toWorld) {

		GLContext * gl = wnd.getGL();

		GLContext::Program * prog = gl->getProgram(m_program.c_str());
		prog->use();

		Mat4f toCamera = camera.getWorldToCamera();
		Mat4f toScreen = camera.getWorldToClip();

		gl->setUniform(prog->getUniformLoc("time"), GLOBAL_TIMER.getElapsed());
		gl->setUniform(prog->getUniformLoc("toCamera"), toCamera);
		gl->setUniform(prog->getUniformLoc("toWorld"), toWorld);
		gl->setUniform(prog->getUniformLoc("toScreen"), toScreen);
		gl->setUniform(prog->getUniformLoc("normalToCamera"), toCamera.inverted().transposed());
		gl->setUniform(prog->getUniformLoc("dispUniform"), Vec3f(m_disp1, m_disp2, m_disp3));
		gl->setUniform(prog->getUniformLoc("cameraPos"), camera.getPosition());

		glBindFragDataLocation(prog->getHandle(), 0, "diffuse");
		glBindFragDataLocation(prog->getHandle(), 1, "normal");
		glBindFragDataLocation(prog->getHandle(), 2, "position");
		glBindFragDataLocation(prog->getHandle(), 3, "depth");

		renderMeshTessellation(gl, m_mesh, prog, 3);

	}

	void DisplacedMesh::loadShaders(GLContext * gl) {

	
		const char declShader[] = "shaders/raymarch/decl.glsl";
		const char noiseShader[] = "shaders/raymarch/noise.glsl"; // noise utilities
		const char knobsShader[] = "shaders/raymarch/knobs.glsl"; // knob uniforms

		loadShader(gl, { m_vertexShader }, { m_tessControlShader },
		{
			declShader,
			noiseShader,
			knobsShader,
			m_tessEvalShader
		},
		{ m_fragmentShader }, m_program);
		gl->checkErrors();

	}

};