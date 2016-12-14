#include "Ball.h"
#include <vector>


using namespace std;


extern void  CreateVertexData(pmd* _model, vector<Vertex3D>& _data, unsigned short** _index);
extern void ballCreateVertexBuffer(vector<Vertex3D>& _data, Ball& balls);
extern void ballCreateIndexBuffer(unsigned short* _index, pmd* _model, Ball& balls);
extern void createBall(Vector3 positions);



Ball::Ball(pmd* model, Vector3 position)
{
	
	vector<Vertex3D>TYOUTEN;
	unsigned short* hIndexData = nullptr;
	CreateVertexData(model, TYOUTEN, &hIndexData);
	ballCreateVertexBuffer(TYOUTEN,*this);
	ballCreateIndexBuffer(hIndexData,model,*this);
	XMMATRIX hTrans;
	hTrans = XMMatrixTranslation(position.x, 0.0f, position.z);
	this->World = XMMatrixMultiply(this->World, hTrans);
}
void Ball::Update()
{
}

void Ball::Draw()
{
}

void Ball::Transform()
{
	
}

void Ball::rigidBody()
{
}

void Ball::CollisionWall()
{

}


