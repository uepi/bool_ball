#include "Ball.h"
#include <vector>


using namespace std;


extern void  CreateVertexData(pmd* _model, vector<Vertex3D>& _data, unsigned short** _index);
extern void ballCreateVertexBuffer(vector<Vertex3D>& _data, Ball& balls);
extern void ballCreateIndexBuffer(unsigned short* _index, pmd* _model, Ball& balls);
extern void createBall(Vector3 positions);



Ball::Ball(pmd* model, Vector3 position)
{

	this->p = XMVectorSet(position.x,position.y,position.z,1.0f);
	vector<Vertex3D>TYOUTEN;
	unsigned short* hIndexData = nullptr;
	CreateVertexData(model, TYOUTEN, &hIndexData);
	ballCreateVertexBuffer(TYOUTEN, *this);
	ballCreateIndexBuffer(hIndexData, model, *this);
	XMMATRIX hTrans;
	hTrans = XMMatrixTranslation(position.x*10.0f, position.y, position.z*10.0f);
	this->World = XMMatrixMultiply(this->World, hTrans);
}
void Ball::Update()
{
	XMMATRIX AA = XMMatrixIdentity();
	XMMATRIX Trans;
	Trans = XMMatrixTranslation(XMVectorGetX(this->p), XMVectorGetY(this->p), XMVectorGetZ(this->p));
	AA =  XMMatrixMultiply(AA, Trans);

	this->World = AA;
	
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


