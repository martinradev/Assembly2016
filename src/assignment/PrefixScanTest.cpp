#include "PrefixScanTests.h"

#include "GPUPrefixScan.h"

#include <vector>
#include <cstdio>


namespace FW {

	void benchmarkPrefixScan(GLContext * gl) {

		int numValues = 1024*1024;
		std::vector<int> values(numValues);
		int totalSum = 0;
		for (int i = 0; i < numValues; ++i) {
		values[i] = rand()%32;
		totalSum += values[i];
		}

		// compute prefix scan
		std::vector<int> prefix(numValues);
		prefix[0] = 0;
		for (int i = 1; i < numValues; ++i) {
		prefix[i] = prefix[i - 1] + values[i-1];
		}

		GLuint vBuffer;
		GLuint blockBuffer;
		glGenBuffers(1, &vBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, vBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * numValues, values.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		int numBlocks = (numValues+1023) / 1024;
		glGenBuffers(1, &blockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 1025, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);



		GPUPrefixScan::loadProgram(gl, "shaders/compute/prefix_scan.glsl", "shaders/compute/prefix_block_add.glsl");
		GLint elapsedTime;
		GLuint elapsedBuf;


		for (int i = 0; i < 10; ++i) {
		glGenQueries(1, &elapsedBuf);
		glBeginQuery(GL_TIME_ELAPSED, elapsedBuf);

		GPUPrefixScan::scan(gl, vBuffer, blockBuffer, numValues);

		glEndQuery(GL_TIME_ELAPSED);

		int stopTimerAvailable = 0;
		while (!stopTimerAvailable) {
		glGetQueryObjectiv(elapsedBuf,
		GL_QUERY_RESULT_AVAILABLE,
		&stopTimerAvailable);
		}

		glGetQueryObjectui64v(elapsedBuf, GL_QUERY_RESULT, &elapsedTime);
		printf("Time spent on the GPU: %lf ms\n", double(elapsedTime) / 1000000.0);

		}
		std::vector<int> prefixGPU(numValues);
		std::vector<int> blockValues(1025);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, vBuffer);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numValues * sizeof(int), prefixGPU.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockBuffer);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 1025 * sizeof(int), blockValues.data());

		

	}

	void becnhmarkWorkEfficientPrefixScan(GLContext * gl) {
		int numValues = 1024;
		std::vector<int> values(numValues);
		int totalSum = 0;
		for (int i = 0; i < numValues; ++i) {
			values[i] = rand() % 32;
			totalSum += values[i];
		}

		// compute prefix scan
		std::vector<int> prefix(numValues);
		prefix[0] = 0;
		for (int i = 1; i < numValues; ++i) {
			prefix[i] = prefix[i - 1] + values[i - 1];
		}

		GLuint inBuffer;
		GLuint outBuffer;
		GLuint inBlockBuffer;
		GLuint outBlockBuffer;
		glGenBuffers(1, &inBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, inBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * numValues, values.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		glGenBuffers(1, &outBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, outBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * numValues, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glGenBuffers(1, &inBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, inBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 1025, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glGenBuffers(1, &outBlockBuffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, outBlockBuffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 1025, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		GPUPrefixScan::loadProgram(gl, "shaders/compute/prefix_scan_work_efficient.glsl", "shaders/compute/prefix_block_add.glsl");

		GLint elapsedTime;
		GLuint elapsedBuf;


		for (int i = 0; i < 10; ++i) {
			glGenQueries(1, &elapsedBuf);
			glBeginQuery(GL_TIME_ELAPSED, elapsedBuf);

			GPUPrefixScan::scanWorkEfficient(gl, inBuffer, outBuffer, inBlockBuffer, outBlockBuffer, numValues);

			glEndQuery(GL_TIME_ELAPSED);

			int stopTimerAvailable = 0;
			while (!stopTimerAvailable) {
				glGetQueryObjectiv(elapsedBuf,
					GL_QUERY_RESULT_AVAILABLE,
					&stopTimerAvailable);
			}

			glGetQueryObjectui64v(elapsedBuf, GL_QUERY_RESULT, &elapsedTime);
			printf("Time spent on the GPU: %lf ms\n", double(elapsedTime) / 1000000.0);

			std::vector<int> prefixGPU(numValues);
			std::vector<int> blockValues(1025);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, outBuffer);
			glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, numValues * sizeof(int), prefixGPU.data());

			for (int i = 0; i < numValues; ++i) {
				if (prefixGPU[i] != prefix[i]) printf("error\n");
			}

		}

		

		


	}

}