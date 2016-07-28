#include "GPUBvh.h"
#include "gpu/GLContext.hpp"
#include <iostream>
#include <unordered_map>
namespace FW {

	static struct GPUStackEntry {

		GPUStackEntry() {}

		GPUStackEntry(int index, int parentSibling, bool isLeft) :
			index_(index),
			parentSibling_(parentSibling),
			isLeft_(isLeft) {

		}

		int index_;
		int parentSibling_;
		bool isLeft_;



	};

	void GPUBvh::print() {
		print(0);
	}

	void GPUBvh::print(unsigned int nodeIndex) {

		if (nodeIndex == -1) return;

		const auto & node = nodes_[nodeIndex];

		std::cout << nodeIndex << " -> HIT: " << node.getHit() << ", MISS: " << node.getMiss() << std::endl;
		std::cout << "\t -> HIT: " << node.getHit() << ", MISS: " << node.getMiss() << std::endl;

		print(node.getHit());

	}

	GPUBvh::GPUBvh(const SBVH * sbvh) : triangles_(sbvh->m_triangles) {

		indices_.resize(sbvh->m_numIndices);
		nodes_.resize(sbvh->m_numNodes);

		// in order traversal to index nodes
		std::vector<int> inOrderIndices;
		std::vector<int> reverseOrderIndices;
		inOrderIndex(sbvh, inOrderIndices);
		reverseOrderIndex(sbvh, reverseOrderIndices);

		GPUStackEntry st[128];
		int stSize = 1;
		st[0] = GPUStackEntry(0, -1, false);

		int cIdx = 0;
		int order = 0;
		nodes_[0].setParent(-1);
		while (stSize != 0) {

			GPUStackEntry topEntry = st[--stSize];
			int top = topEntry.index_;
			const auto & sbvhNode = sbvh->m_nodes[top];
			auto & gpuNode = nodes_[top];

			gpuNode.setAABB(sbvhNode.m_aabb.toAABB());
			gpuNode.setLeftChildIndex(sbvhNode.getLeft());

			if (order + 1 == nodes_.size()) {
				gpuNode.setHit(-1);
			}
			else {
				gpuNode.setHit(inOrderIndices[order + 1]);
			}


			if (sbvhNode.isLeaf()) {
				// if leaf, copy the triangle indices
				if (order + 1 == nodes_.size()) {
					gpuNode.setMiss(-1);
				}
				else {
					gpuNode.setMiss(inOrderIndices[order + 1]);
				}
				gpuNode.setSegmentStart(cIdx);
				gpuNode.setSegmentEnd(cIdx + sbvhNode.getReferencesSize());

				for (int i = 0; i < sbvhNode.getReferencesSize(); ++i) {
					indices_[cIdx++] = sbvh->m_references[sbvhNode.m_referenceIndices[i]].m_triangleIndex;
				}


			}
			else {
				gpuNode.setSegmentStart(-1);


				gpuNode.setMiss(topEntry.parentSibling_);


				st[stSize++] = GPUStackEntry(sbvhNode.getRight(), topEntry.parentSibling_, false);
				st[stSize++] = GPUStackEntry(sbvhNode.getLeft(), sbvhNode.getRight(), true);
				nodes_[sbvhNode.getRight()].setParent(top);
				nodes_[sbvhNode.getLeft()].setParent(top);
			}

			++order;

		}

	}

	void GPUBvh::inOrderIndex(const SBVH * sbvh, std::vector<int> & inOrderIndices) {

		inOrderIndices.resize(sbvh->m_nodes.size());

		int st[128];
		int stSize = 1;
		st[0] = 0;
		int inOrderIndex = 0;

		while (stSize != 0) {

			int top = st[--stSize];
			const auto & sbvhNode = sbvh->m_nodes[top];

			inOrderIndices[inOrderIndex++] = top;

			if (!sbvhNode.isLeaf()) {

				st[stSize++] = sbvhNode.getRight();
				st[stSize++] = sbvhNode.getLeft();

			}


		}

	}

	void GPUBvh::reverseOrderIndex(const SBVH * sbvh, std::vector<int> & reverseOrderIndices) {
		reverseOrderIndices.resize(sbvh->m_nodes.size());

		int st[128];
		int stSize = 1;
		st[0] = 0;
		int inOrderIndex = 0;

		while (stSize != 0) {

			int top = st[--stSize];
			const auto & sbvhNode = sbvh->m_nodes[top];

			reverseOrderIndices[inOrderIndex++] = top;

			if (!sbvhNode.isLeaf()) {
				st[stSize++] = sbvhNode.getLeft();
				st[stSize++] = sbvhNode.getRight();
			}


		}
	}

	GPUBvh_Buffers GPUBvh::genBuffer(const std::vector<MeshBase::Material*> & inMaterials) {
		GPUBvh_Buffers buffer;
		glGenBuffers(1, &buffer.m_nodes_ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.m_nodes_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, nodes_.size() * sizeof(GPUBvhNode), nodes_.data(), GL_DYNAMIC_COPY);

		glGenBuffers(1, &buffer.m_indices_ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.m_indices_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, indices_.size() * sizeof(int), indices_.data(), GL_DYNAMIC_COPY);
		
		glGenBuffers(1, &buffer.m_triangles_ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.m_triangles_ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, triangles_->size() * sizeof(GPU_Triangle), NULL, GL_DYNAMIC_COPY);

		GPU_Triangle * triangles = (GPU_Triangle*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, triangles_->size() * sizeof(GPU_Triangle), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		for (int i = 0; i < triangles_->size(); ++i) {
			const auto & trig = (*triangles_)[i];
			triangles[i].v1 = Vec4f(trig.m_vertices[0].p, 0.0f);
			triangles[i].v2 = Vec4f(trig.m_vertices[1].p, 0.0f);
			triangles[i].v3 = Vec4f(trig.m_vertices[2].p, 0.0f);
			triangles[i].uv1 = Vec4f(trig.m_vertices[0].t, trig.m_vertices[1].t);
			triangles[i].uv2 = Vec4f(trig.m_vertices[2].t, 0.0f, 0.0f);
		}
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		
		return buffer;
	}

};