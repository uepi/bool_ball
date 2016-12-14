
#include"pmd.h"
#include"d3d11.h"
#include <directxmath.h>
using namespace DirectX;
struct Vertex3D {
	float pos[3];	//x-y-z
	float col[4];	//r-g-b-a
	float tex[2];
};
struct Vector3 {
	float x;
	float y;
	float z;
};
class Ball {
	public:
		Ball(pmd* model,Vector3 potision);
		void Update();
		void Draw();
		int id;
		//vector<Vertex3D> TYOUTEN;
		//pmd* model;
		void Transform();
		void rigidBody();
		void CollisionWall();
		void CollisionBall();
		int verocity;
		int angle;
		int power;
		float r;
		ID3D11Buffer* VertexBuffer = nullptr;
		ID3D11Buffer* IndexBuffer = nullptr;
		XMMATRIX World;

		//vector<Ball*> balls;
};