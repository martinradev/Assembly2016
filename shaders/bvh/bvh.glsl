

struct Triangle {
	vec4 v1;
	vec4 v2;
	vec4 v3;
	vec4 uv1;
	vec4 uv2; // uv2[2] = materialIndex
};

struct BVH_Node {
	vec3 minbb;
	int segmentStart;
	vec3 maxbb;
	int segmentEnd;
	
	int hitLink;
	int missLink;
	int parentLink;
	int leftChild;
};

struct Material {
	vec3 diffuse;
	int diffuseTextureIndex;
	vec3 specular;
	int normalTextureIndex;
};

layout (std430, binding = 10) buffer TriangleBuffer_Declaration {
	
	Triangle triangles[];
	
} triangle_buffer;

layout (std430, binding = 13) buffer Material_Declaration {
	
	Material materials[];
	
} material_buffer;


layout (std430, binding = 12) buffer BVHBuffer_Declaration {
	
	BVH_Node nodes[];
	
} bvh_buffer;

layout (std430, binding = 11) buffer BVHBuffer_Indices_Declaration {
	
	int indices[];
	
} bvh_indices_buffer;


// http://www.ci.i.u-tokyo.ac.jp/~hachisuka/tdf2015.pdf
bool triangle_intersect(const in Triangle trig, const in vec3 orig, const in vec3 dir, out float u, out float v, inout float t) {
	vec3 e1 = (trig.v2 - trig.v1).xyz;
	vec3 e2 = (trig.v3 - trig.v1).xyz;
	
	vec3 pv = cross(dir, e2);
	float det = dot(e1, pv);
	
	vec3 tv = orig - trig.v1.xyz;
	vec3 qv = cross(tv, e1);
	
	vec4 uvt;
	uvt.x = dot(tv, pv);
	uvt.y = dot(dir, qv);
	uvt.z = dot(e2, qv);
	
	uvt.xyz = uvt.xyz / det;
	
	uvt.w = 1.0 - uvt.x - uvt.y;
	
	if (all(greaterThanEqual(uvt,vec4(0.0))) && (uvt.z < t)) {
		
		t = uvt.z;
		u = uvt.x;
		v = uvt.y;
		return true;
	}

	return false;
}

bool box_intersect(const in vec3 minbb, const in vec3 maxbb, const in vec3 orig, const in vec3 dir, const in float t, inout float tbegin) {
	
	vec3 tmin = (minbb - orig) / dir;
	vec3 tmax = (maxbb - orig) / dir;
	
	vec3 t1 = min(tmin, tmax);
	vec3 t2 = max(tmin, tmax);
	
	float ts = max(t1.x, max(t1.y, t1.z));
	float te = min(t2.x, min(t2.y, t2.z));

	if (te < 0.0 || ts > t || ts > te) return false;
	tbegin = max(0.0, ts);
	
	return true;
}


bool bvh_intersect(const in vec3 orig, const in vec3 dir, out float u, out float v, inout float t, out int found) {
	
	float tt = 1.0;
	
	int top = 0;
	
	found = -1;
	
	while(top != -1) {
		

		BVH_Node node = bvh_buffer.nodes[top];
		if (box_intersect(node.minbb, node.maxbb, orig, dir, t, tt)) {
			
			if (node.segmentStart != -1) {
				// if it is a leaf check triangles
				int sbegin = node.segmentStart;
				int send = node.segmentEnd;
				for (int i = sbegin; i < send; ++i) {
					const int k = bvh_indices_buffer.indices[i];
					
					Triangle trig = triangle_buffer.triangles[k];
					
					if (triangle_intersect(trig, orig, dir, u, v, t)) {
						found = k;
					}
				
				}
			}
			
			// hit
			
			top = node.hitLink;
		} else {
			// miss
			top = node.missLink;
		}
		
	
	}
	
	return found != -1;
}


bool bvh_intersect_bitstack(const in vec3 orig, const in vec3 dir, out float u, out float v, inout float t, out int found, in bool isShadowRay = false) {
	
	float tt = 1.0;
	
	found = -1;
	
	int top = 0;
	int lstack = 0;
	int rstack = 0;
	
	while(top != -1) {
		

		BVH_Node node = bvh_buffer.nodes[top];
		
		bool trackback = false;
		
		if (node.segmentStart != -1) {
			// if it is a leaf check triangles
			int sbegin = node.segmentStart;
			int send = node.segmentEnd;
			for (int i = sbegin; i < send; ++i) {
				const int k = bvh_indices_buffer.indices[i];
					
				Triangle trig = triangle_buffer.triangles[k];
					
				if (triangle_intersect(trig, orig, dir, u, v, t)) {
					found = k;
					if (isShadowRay) return true;
				}
				
			}
			trackback = true;
			
		} else {
			
			// check left and right children
			
			BVH_Node lNode = bvh_buffer.nodes[node.leftChild];
			BVH_Node rNode = bvh_buffer.nodes[node.leftChild+1];
			
			float t1, t2;
			bool r1 = box_intersect(lNode.minbb, lNode.maxbb, orig, dir, t, t1);
			bool r2 = box_intersect(rNode.minbb, rNode.maxbb, orig, dir, t, t2);
			
			if (r1 && r2) {
				
				if (t1 <= t2) {
					// first left
					top = node.leftChild;
					lstack = (lstack|1)<<1;
					rstack <<= 1;
				} else {
					// first right
					top = node.leftChild+1;
					rstack = (rstack|1)<<1;
					lstack <<= 1;
				}
			
				
			} else if(r1) {
				top = node.leftChild;
				lstack <<= 1;
				rstack <<= 1;
			} else if(r2) {
				top = node.leftChild+1;
				lstack <<= 1;
				rstack <<= 1;
			} else {
				trackback = true;
			}
			
		}
		
		if (trackback) {
			
			bool f = false;
			
			while(lstack != 0 || rstack != 0) {
				node = bvh_buffer.nodes[top];
				if ((lstack & 1) != 0) {
					// visit right node
					top = node.leftChild+1;
					lstack &= ~1;
					lstack <<= 1;
					rstack <<= 1;
					f = true;
					break;
				} else if((rstack & 1) != 0) {
					// visit left node
					top = node.leftChild;
					rstack &= ~1;
					lstack <<= 1;
					rstack <<= 1;
					f = true;
					break;
				}
				top = node.parentLink; // go to parent
				lstack >>= 1;
				rstack >>= 1;
			}
			
			if (!f) break;
			
		}
	
	}
	
	return found != -1;
}

vec3 getTrigNormal(in const Triangle trig) {
	vec3 e1 = trig.v3.xyz - trig.v1.xyz;
	vec3 e2 = trig.v2.xyz - trig.v1.xyz;
	return normalize(cross(e1, e2));
}

