#pragma once

#include "gpu/GLContext.hpp"
#include <string>

namespace FW {
	struct SDFGeometryDescription {

		SDFGeometryDescription(const std::string & sdfSource, const Vec3f & start, const Vec3f & numSteps, float cubeStep, const Vec3f & position, const Vec3f & rotation, const Vec3f & scale)
			: mSDFSource(sdfSource),
			mStart(start),
			mNumSteps(numSteps),
			mCubeStep(cubeStep) {
		
			Mat3f rotR3 = Mat3f::rotation(Vec3f(1,0,0), rotation.x) 
				* Mat3f::rotation(Vec3f(0, 1, 0), rotation.y) 
				* Mat3f::rotation(Vec3f(0, 0, 1), rotation.z);
			Mat4f rot;
			rot.setRow(0, Vec4f(rotR3.getRow(0), 0.0f));
			rot.setRow(1, Vec4f(rotR3.getRow(1), 0.0f));
			rot.setRow(2, Vec4f(rotR3.getRow(2), 0.0f));
			rot.setRow(3, Vec4f(0.0f, 0.0f, 0.0f, 1.0f));

			toWorld = Mat4f::translate(position) * rot * Mat4f::scale(scale);
			toWorldNormal = toWorld.inverted().transposed();
		}

		Mat4f toWorld;
		Mat4f toWorldNormal;
		std::string mSDFSource;
		Vec3f mStart;
		Vec3i mNumSteps;
		float mCubeStep;

	};

	struct SDFGeometryOutput {

		Mat4f toWorld;
		Mat4f toWorldNormal;
		GLuint mVao;
		GLuint mVbo;
		GLuint numTriangles;

	};

	enum SDFGeometryFactoryBuffers : int {

		SDF_GEOM_INDEX_BUFFER = 0,
		SDF_GEOM_BLOCK_BUFFER,
		SDF_GEOM_BUFFERS_COUNT

	};

	class SDFGeometryFactory {

	public:

		SDFGeometryFactory(GLContext * gl);
		void produceGeometry(GLContext * gl, const SDFGeometryDescription & description, SDFGeometryOutput * geometryResult);
		void resetBuffers();

	private:

		void loadCommonShaders(GLContext * gl);
		GLuint getProgram(const std::string & sdfShader);

		GLuint mBuffers[SDFGeometryFactoryBuffers::SDF_GEOM_BUFFERS_COUNT];

		GLuint mTetrahedraMarchComputeShader;
		GLuint mDeclarationShader;
		GLuint mNoiseShader;
		GLuint mHGSDFShader;

	};

};