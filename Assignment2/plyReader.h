// A simnple reader for triangle ply files. Includes an opengl renderer.
// Load and Draw are really the only methods you need to call.
// I think this has memory leaks.

class Model_PLY
{
public:
	int Load(char *filename); // load the ply file given by filename
	void Draw();
	float* calculateNormal(float *coord1, float *coord2, float *coord3);
	Model_PLY();

	float* Faces_Triangles;
	float* Faces_Quads;
	float* Vertex_Buffer;
	float* Normals;

	int TotalConnectedTriangles;
	int TotalConnectedQuads;
	int TotalConnectedPoints;
	int TotalFaces;


};

