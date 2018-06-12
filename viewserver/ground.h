#pragma once
#define WIN32_LEAN_AND_MEAN  
#include <windows.h>
#include <windowsx.h>
#include <malloc.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <time.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <fstream>
using namespace std;
#include <direct.h>
#include <commdlg.h>
#include <malloc.h>
#include <cmath>
#include <string.h>
#include <tchar.h>

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>

struct PosTexVertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};
struct PosTexNormVertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
	XMFLOAT3 Norm;
};



XMFLOAT2 operator+(XMFLOAT2 lhs, XMFLOAT2 rhs);
XMFLOAT3 operator+(XMFLOAT3 lhs, XMFLOAT3 rhs);

bool LoadOBJ(char * filename, ID3D11Device * g_pd3dDevice, ID3D11Buffer ** ppVertexBuffer, int * vertex_count);


//===== ADDED
//
////for matrix and vector operations, some handy functions:
//float length(const XMFLOAT3 &v);
//XMFLOAT3 mul(const XMMATRIX &M, const XMFLOAT3 &rhs);
//XMMATRIX mul(const XMMATRIX &lhs, const XMMATRIX &rhs);
//float dot(XMFLOAT3 a, XMFLOAT3 b);
//XMFLOAT3 cross(XMFLOAT3 a, XMFLOAT3 b);
//XMFLOAT3 normalize(const  XMFLOAT3 &a);
//XMFLOAT3 operator+(const XMFLOAT3 lhs, const XMFLOAT3 rhs);
//XMFLOAT2 operator+(const XMFLOAT2 lhs, const XMFLOAT2 rhs);
//XMFLOAT3 operator-(const XMFLOAT3 lhs, const XMFLOAT3 rhs);
//
//class camera
//{
//public:
//	float angle_y;
//	float angle_x;
//	XMFLOAT3 pos;
//	int ak, sk, dk, wk;//stores if a key is pressed
//	int ek, qk;
//	camera()
//	{
//		angle_y = 0;
//		pos = XMFLOAT3(0, 0, 0);
//		ek = qk = ak = sk = dk = wk = 0;//0 not pressed, 1 pressed
//	}
//	XMMATRIX calculate_view(XMMATRIX &V)
//	{
//		//calculate the matrices Ry, Tz
//		XMMATRIX Ry, Rx, T;
//
//		float anglespeed = 0.0006;
//		float speed = 0.003;
//		//rotation
//		if (ak == 1)
//		{
//			angle_y = angle_y + anglespeed;
//		}
//		else if (dk == 1)
//		{
//			angle_y = angle_y - anglespeed;
//		}
//		if (qk == 1)
//		{
//			angle_x = angle_x + anglespeed;
//		}
//		else if (ek == 1)
//		{
//			angle_x = angle_x - anglespeed;
//		}
//		Ry = XMMatrixRotationY(angle_y);
//		Rx = XMMatrixRotationX(angle_x);
//		//translation
//		XMFLOAT3 lookat_worldcoord = XMFLOAT3(0, 0, speed);
//		XMMATRIX R = Rx*Ry;
//		XMFLOAT3 lookat_viewcoord = mul(R, lookat_worldcoord);
//		if (wk == 1)
//		{
//			pos = pos - lookat_viewcoord;
//		}
//		else if (sk == 1)
//		{
//			pos = pos + lookat_viewcoord;
//		}
//		T = XMMatrixTranslation(-pos.x, -pos.y, pos.z);
//
//		return T*V*Ry*Rx;
//
//	}
//};