

//���͗p
struct vertexIn
{
	float3 pos : POSITION0;
	float4 col : COLOR0;
	float2 tex : TEXCOORD0;
};

//�o�͗p
struct vertexOut
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD0;
};
typedef vertexOut pixcelIn;

//�ϊ��p�s��
cbuffer ConstantBuffer : register(b0)
{
	matrix World;		//���[���h�ϊ��s��
	matrix View;		//�r���[�ϊ��s��
	matrix Projection;	//�����ˉe�ϊ��s��
}

//���_�V�F�[�_�[
vertexOut VS(vertexIn IN)
{
	vertexOut OUT;

	OUT.pos = mul(IN.pos, World);		//���[���h�ϊ�
	OUT.pos = mul(OUT.pos, View);		//�r���[�ϊ�
	OUT.pos = mul(OUT.pos, Projection);	//�����ˉe�ϊ�
	OUT.col = IN.col;
	OUT.tex = IN.tex;

	return OUT;
}
//struct pixcelIn
//{
//	float4 pos : SV_POSITION;
//	float4 col : COLOR;
//	float2 tex : TEXCOORD0;
//};

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

float4 PS(pixcelIn IN) : SV_Target
{
	pixcelIn OUT;

//OUT.col = txDiffuse.Sample(samLinear, IN.tex);
OUT.col = IN.col;
return OUT.col;
}

/*
//���_�V�F�[�_�[
float4 VS(float4 Pos : POSITION) : SV_POSITION
{
	return Pos;
}
//�s�N�Z���V�F�[�_�[
float4 PS(float4 Pos : SV_POSITION) : SV_Target
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}*/