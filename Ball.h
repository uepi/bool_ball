
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
		XMVECTOR p;			// 球の位置
		XMVECTOR Pre_p;		// 1つ前の円の位置
		XMVECTOR v;			// 速度ベクトル
		XMVECTOR a;			// 加速度ベクトル
		float r = 0.036f + 0.0001f;				// 半径
		float w=1.0;				// 質量
		float scale=1.0;			// スケール
		int verocity;
		int angle;
		int power;
		ID3D11Buffer* VertexBuffer = nullptr;
		ID3D11Buffer* IndexBuffer = nullptr;
		XMMATRIX World= XMMatrixIdentity();
		

		//vector<Ball*> balls;
};