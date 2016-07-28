#pragma once

#include "AABB.h"
#include "SBVH.h"
#include "base/Math.hpp"
#include "MemoryAllignedAllocator.h"
#include <vector>


namespace FW {

	struct GPUBvh_Buffers {

		GPUBvh_Buffers(GLuint nodesSSBO, GLuint trianglesSSBO, GLuint indicesSSBO, GLuint materialsSSBO) :
			m_nodes_ssbo(nodesSSBO),
			m_triangles_ssbo(trianglesSSBO),
			m_indices_ssbo(indicesSSBO),
			m_materials_ssbo(materialsSSBO)
		{}

		GPUBvh_Buffers() :
			m_nodes_ssbo(0),
			m_triangles_ssbo(0),
			m_indices_ssbo(0),
			m_materials_ssbo(0)
		{

		}

		void bindAll() {
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, m_triangles_ssbo);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, m_materials_ssbo);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, m_indices_ssbo);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, m_nodes_ssbo);
		}

		GLuint m_nodes_ssbo;
		GLuint m_triangles_ssbo;
		GLuint m_indices_ssbo;
		GLuint m_materials_ssbo;

	};

	struct GPU_Material {
		// 4 * 4 bytes
		Vec3f diffuse;
		int diffuseTexIndex;

		// 4 * 4 bytes
		Vec3f specular;
		int normalMapTexIndex;

	};

	struct GPU_Triangle {
		Vec4f v1;
		Vec4f v2;
		Vec4f v3;
		Vec4f uv1;
		Vec4f uv2;
	};

	class _MM_ALIGN16 GPUBvhNode {

		friend class GPUBvh;
		friend class GPURenderer;

	public:
		GPUBvhNode() {};

		void setAABB(const AABB & aabb) {
			m_maxbb.x = aabb.max.x;
			m_maxbb.y = aabb.max.y;
			m_maxbb.z = aabb.max.z;

			m_minbb.x = aabb.min.x;
			m_minbb.y = aabb.min.y;
			m_minbb.z = aabb.min.z;
		}
		void setSegmentStart(int i) {
			m_segmentStart = i;
		}
		void setSegmentEnd(int i) {
			m_segmentEnd = i;
		}
		void setHit(int i) {
			m_hitLink = i;
		}
		void setMiss(int i) {
			m_missLink = i;
		}

		void setParent(int parent) {
			m_parentLink = parent;
		}

		void setLeftChildIndex(int index) {
			m_left = index;
		}

		AABB getAABB() const {
			return AABB(m_minbb, m_maxbb);
		}
		int getSegmentStart() const {
			return m_segmentStart;
		}
		int getSegmentEnd() const {
			return m_segmentEnd;
		}
		int getHit() const {
			return m_hitLink;
		}
		int getMiss() const {
			return m_missLink;
		}

	private:

		Vec3f m_minbb;
		int m_segmentStart;
		Vec3f m_maxbb;
		int m_segmentEnd;

		// hit , miss , parent, split axis
		int m_hitLink;
		int m_missLink;
		int m_parentLink;
		int m_left;
	};

	class GPUBvh {

		friend class GPURenderer;

	public:

		GPUBvh(const SBVH * sbvh);

		GPUBvh_Buffers genBuffer(const std::vector<MeshBase::Material*> & materials);

		void print();

	private:

		std::vector<GPUBvhNode, aligned_allocator<GPUBvhNode, 16> > nodes_;
		std::vector<unsigned> indices_;

		const std::vector<RTTriangle> * triangles_;

		void inOrderIndex(const SBVH * sbvh, std::vector<int> & inOrderIndices);
		void reverseOrderIndex(const SBVH * sbvh, std::vector<int> & reverseOrderIndices);

		void print(unsigned int nodeIndex);


	};

	

};