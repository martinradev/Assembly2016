#pragma once

#include "3d/Mesh.hpp"
#include "base/Random.hpp"
#include "curve.h"
#include "surf.h"

#include <vector>

class Object {

public:

	virtual void render(FW::GLContext* gl, FW::GLContext::Program * program, const FW::Mat4f& posToCamera, const FW::Mat4f& projection) const = 0;
	virtual ~Object() {};

	void setToWorldTransformation(const FW::Mat4f & toWorld) {
		toWorld_ = toWorld;
	}

	FW::Mat4f getToWorldTransformation() const {
		return toWorld_;
	}

protected:

	FW::Mat4f toWorld_;

};

class CurveObject : public Object {

public:
	
	CurveObject() = delete;

	CurveObject(const Curve & curve);
	~CurveObject();
	CurveObject & operator =(const CurveObject & rht) = delete;
	CurveObject(const CurveObject & rht) = delete;
	CurveObject(CurveObject && rht) = delete;
	CurveObject & operator =(CurveObject && rht) = delete;

	void render(FW::GLContext* gl, FW::GLContext::Program * program, const FW::Mat4f& posToCamera, const FW::Mat4f& projection) const;

private:
	size_t numberOfPoints_;
	GLuint vao_;
	GLuint vbo_;

};

class SimpleObject : public Object {

public:
	SimpleObject() = delete;

	SimpleObject(FW::MeshBase * mesh);
	~SimpleObject();
	SimpleObject & operator =(const SimpleObject & rht);
	SimpleObject(const SimpleObject & rht);
	SimpleObject(SimpleObject && rht);
	SimpleObject & operator =(SimpleObject && rht);

	void render(FW::GLContext* gl, FW::GLContext::Program * program, const FW::Mat4f& posToCamera, const FW::Mat4f& projection) const;

private:
	FW::MeshBase * mesh_;
};

class PointCloud : public Object {

public:

	PointCloud() : vao_(0), vbo_(0), numberOfElements_(0) {};
	~PointCloud();

	void setupMarchingCubes(const FW::Vec3f & from, const FW::Vec3f & to, float cubeLen);

	void render(FW::GLContext* gl, FW::GLContext::Program * program, const FW::Mat4f& posToCamera, const FW::Mat4f& projection) const;

private:
	GLuint vao_;
	GLuint vbo_;
	size_t numberOfElements_;
};

class SurfaceObject : public Object{
public:
	SurfaceObject(const float * meshBuf, const int * indBuf, int indBufSz, GLuint texObject);
	SurfaceObject(const Surface & surface, GLuint texObject);
	SurfaceObject(const Surface & surface, GLuint texObject, const FW::Mat4f & toWorldTransformation);
	~SurfaceObject();

	SurfaceObject() = delete;
	SurfaceObject & operator =(const SurfaceObject & rht) = delete;
	SurfaceObject(const SurfaceObject & rht) = delete;
	SurfaceObject(SurfaceObject && rht) = delete;
	SurfaceObject & operator =(SurfaceObject && rht) = delete;

	void render(FW::GLContext* gl, FW::GLContext::Program * program, const FW::Mat4f& posToCamera, const FW::Mat4f& projection) const;

private:
	GLuint vao_;
	GLuint vbo_;
	GLuint textureObject_;

	size_t numberOfElements_;

};

class PlaneTestObject : public Object {

public:

	PlaneTestObject();
	~PlaneTestObject();

	PlaneTestObject & operator =(const PlaneTestObject & rht) = delete;
	PlaneTestObject(const PlaneTestObject & rht) = delete;
	PlaneTestObject(const PlaneTestObject && rht) = delete;

	void render(FW::GLContext* gl, FW::GLContext::Program * program, const FW::Mat4f& posToCamera, const FW::Mat4f& projection) const;

private:

	GLuint vao_;
	GLuint vbo_;

};