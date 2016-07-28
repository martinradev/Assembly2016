#pragma once

#include "Scene.h"

namespace FW {

	struct HeightMapVertex {

		Vec4f m_position;
		Vec4f m_normal;

		HeightMapVertex() {};
		HeightMapVertex(const Vec4f & position, const Vec4f & normal) :
			m_position(position),
			m_normal(normal) {};

	};

	struct HeightMapModel {

	
		HeightMapModel() {};
		HeightMapModel(const std::vector<HeightMapVertex> & vertices)
			:
			m_vertices(vertices)
		{

		}
	
		std::vector<HeightMapVertex> m_vertices;
	};
	
	class HeightMapScene : public Scene {

	public:

		HeightMapScene(const std::string & sceneShader, const std::string & heightMapFile, GLContext * ctx);

		void render(Window & wnd, const CameraControls & camera);
		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);

		HeightMapModel toModel(GLContext * ctx);


	private:

		void loadShaders(GLContext * ctx);
		void saveModel(const HeightMapModel & model, const std::string & fileName);
		void updateUniforms(Window & wnd, const CameraControls & camera);


		std::string m_sceneShader;
		std::string m_programName; // terrain march program
		std::string m_programCSName; // compute shader program for triangulation

		GLuint m_heightMapTex;
		GLuint m_terrainTrigSSBO;

		// knobs
		Vec4f m_knobs[10];

		Vec2f m_planeSize;
		Vec2i m_numTriangles;

	};

};