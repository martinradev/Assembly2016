#pragma once

#include "Scene.h"
#include "ProceduralTextureScene.h"
namespace FW {
	
	/**
		vertex information

	*/
	struct RayMarchScene_Vertex {
		
		RayMarchScene_Vertex() {};

		RayMarchScene_Vertex(const Vec3f & position, const Vec3f & normal, const Vec2f & uv) :
			m_position(position, 0.0f),
			m_normal(normal, 0.0f),
			m_data(uv, 0.0f, 0.0f) {

		}

		Vec4f m_position;
		Vec4f m_normal;
		Vec4f m_data; // (x,y) -> uv coords

	};

	struct RayMarchScene_Face {

		RayMarchScene_Face() {};

		RayMarchScene_Face(const RayMarchScene_Vertex & v1, const RayMarchScene_Vertex & v2, const RayMarchScene_Vertex & v3) {
			m_vertices[0] = v1;
			m_vertices[1] = v2;
			m_vertices[2] = v3;

			// compute type

			if (m_vertices[1].m_position.w == m_vertices[2].m_position.w) {
				m_vertices[0].m_position.w = m_vertices[2].m_position.w;
			}

		}

		RayMarchScene_Vertex m_vertices[3];

		int getType() const {

			return int(m_vertices[0].m_position.w);

		}

	};

	/**
		triangulated model from the ray march scene, generated using i.e. marching tetrahedra
	*/
	class RayMarchScene_TriangulatedModel {
		friend class RayMarchScene;
	public:

	private:

		std::vector<RayMarchScene_Vertex> m_vertices; // every 3 vertices will generate a triangle

	};

	class RayMarchScene : public Scene {

	public:

		RayMarchScene(const std::string & sceneShader, GLContext * ctx);

		void render(Window & wnd, const CameraControls & camera);
		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);
		void setTexture(unsigned index, ProceduralTextureScene * texture) {
			if (index < 10) {
				m_proceduralTextures[index] = texture;
			}
			else {
				::printf("ERROR: Cannot set texture at %u", index);
			}
		}
		/**
			Outputs the model in the scene.
			Likely, tetrahedra marching on the gpu is in use.
			Check implementation for more information.
		*/
		RayMarchScene_TriangulatedModel toModel(GLContext * ctx);

	private:

		void loadShaders(GLContext * ctx);

		void loadTextures();

		void updateUniforms(Window & wnd, const CameraControls & camera);

		void saveMesh(const RayMarchScene_TriangulatedModel & mesh, const std::string & fileName);

		std::string m_sceneShader;
		std::string m_programName;
		std::string m_tmProgramName; // tetrahedra march program name
		std::string m_tmNormalProgramName; // tetrahedra march (normal) program name

		GLuint m_csVertexBufferSSBO;
		GLuint m_csPrefixSumSSBO;
		GLuint m_brickTexture;
		GLuint m_concreteTexture;

		ProceduralTextureScene * m_proceduralTextures[10];

		// knobs
		Vec4f m_knobs[10];

		// volume sliders
		bool m_showVolumeSliders;
		Vec4f m_volumeStart; // (x,y,z) -> start coordinate of the volume we want to march on, (w) cube length
		Vec4i m_volumeNumBlocks; // (x,y,z) -> num blocks in the volume

	};

};