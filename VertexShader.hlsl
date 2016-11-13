

//入力用
struct vertexIn
{
	float3 pos : POSITION0;
	float4 col : COLOR0;
	float2 tex : TEXCOORD0;
};

//出力用
struct vertexOut
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
	float2 tex : TEXCOORD0;
};
typedef vertexOut pixcelIn;

//変換用行列
cbuffer ConstantBuffer : register(b0)
{
	matrix World;		//ワールド変換行列
	matrix View;		//ビュー変換行列
	matrix Projection;	//透視射影変換行列
}

//頂点シェーダー
vertexOut VS(vertexIn IN)
{
	vertexOut OUT;

	OUT.pos = mul(IN.pos, World);		//ワールド変換
	OUT.pos = mul(OUT.pos, View);		//ビュー変換
	OUT.pos = mul(OUT.pos, Projection);	//透視射影変換
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
//頂点シェーダー
float4 VS(float4 Pos : POSITION) : SV_POSITION
{
	return Pos;
}
//ピクセルシェーダー
float4 PS(float4 Pos : SV_POSITION) : SV_Target
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}*/