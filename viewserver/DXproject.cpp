/*
CST 325
Phillip T Emmons
FINAL PROJECT
12-9-17
Create a game that let you explore virtual world in which you fly over heightmap-generated terrain.
IMPORTANT: you have to have separate vertex and pixel shaders for each flying object, skybox, billboard clouds, grassy terrain heightmapping, and wavy water.
	User keys: awsd

Comment source: msdn.Mircosoft.com - PRECISE CLEAR EXPLANATIONS...WELL SOMETIMES...
*/
//=====		see load.cpp  !important!
#include "ground.h"
//=====		defines
#define MAX_LOADSTRING 1000
#define TIMER1 111
//=====		STANDARD DIRECTX GLOBAL VARS
HINSTANCE hInst;											//	program number = instance
TCHAR szTitle[MAX_LOADSTRING];								//	name in window title
TCHAR szWindowClass[MAX_LOADSTRING];						//	class name of window
HWND hMain = NULL;											//	number of windows = handle window = hwnd
static char MainWin[] = "MainWin";							//	class name
HBRUSH  hWinCol = CreateSolidBrush(RGB(180, 180, 180));		//	a color
HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

//--------------------------------------------------------------------------------------
// Global Variables -  POINTERS...but what do they really do?
//--------------------------------------------------------------------------------------
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;

//=====		DEVICE CONTEXT								// allows CPU to divide the workload
ID3D11Device*           g_pd3dDevice = NULL;			// is for initialization and loading things (pictures, models, ...) <- InitDevice()
ID3D11DeviceContext*    g_pImmediateContext = NULL;		// is for render your models w/ pics on the screen					<- Render()

//=====		 PAGE FLIPPING -  interface implements one or more surfaces for storing rendered data before presenting it to an output.
IDXGISwapChain*         g_pSwapChain = NULL;

//=====		 USER SCREEN - A render-target-view interface identifies the render-target subresources that can be accessed during rendering.
ID3D11RenderTargetView* g_pRenderTargetView = NULL;

//=====		GPU MAPPING FROM BUFFER -  interface holds a definition of how to feed vertex data that is laid out in memory
ID3D11InputLayout*      g_pVertexLayout = NULL;
ID3D11InputLayout*		planeLayout = NULL;

//=====		DIRECTX INDIVIDUAL 
ID3D11Buffer*			g_pConstantMatrixBuffer = NULL;// - interface accesses a buffer resource, which is unstructured memory. Buffers typically store vertex or index data.

ID3D11SamplerState*			samplerState = NULL;// - interface holds a description for sampler state that you can bind to any shader stage of the pipeline for reference by texture sample operations.

ID3D11Texture2D*            g_pDepthStencil = NULL;// - interface manages texel data, which is structured memory.
ID3D11DepthStencilView*     g_pDepthStencilView = NULL;// -  interface accesses a texture resource during depth-stencil testing.
ID3D11DepthStencilState		*ds_on, *ds_off;// -  interface holds a description for depth-stencil state that you can bind to the output-merger stage.
											   
ID3D11RasterizerState		*rs_CW, *rs_CCW, *rs_NO, *rs_Wire;// - interface holds a description for rasterizer state that you can bind to the rasterizer stage.
															  
ID3D11BlendState*			g_BlendState;//OMG transparency!!! -  interface holds a description for blending state that you can bind to the output-merger stage.
	
//=====		TERRAIN
ID3D11Buffer*			terrainBuffer = NULL;// - interface accesses a buffer resource, which is unstructured memory. Buffers typically store vertex or index data.
int						terrianVertCount = 0;
ID3D11VertexShader*		terrainVertexShader = NULL;// - interface manages an executable program (a vertex shader) that controls the vertex-shader stage.
ID3D11PixelShader*		terrainPixelShader = NULL;// -  interface manages an executable program (a pixel shader) that controls the pixel-shader stage.

ID3D11ShaderResourceView*	terrainTexture = NULL;// - interface specifies the subresources a shader can access during rendering.
ID3D11ShaderResourceView*	rockTexture = NULL;		// eg - constant buffer, a texture buffer, and a texture.


//=====		DEATHSTAR
ID3D11Buffer*			deathstarBuffer = NULL;
int						deathstarVertCount = 0;
ID3D11VertexShader*     deathstarVertexShader = NULL;
ID3D11PixelShader*      deathstarPixelShader = NULL;

ID3D11ShaderResourceView*	deathstarTexture = NULL;

//=====		WATER
ID3D11Buffer*			waterBuffer = NULL;
int						waterVertCount = 0;
ID3D11VertexShader*		waterVertexShader = NULL;
ID3D11PixelShader*		waterPixelShader  = NULL;

ID3D11ShaderResourceView*   waveOneTexture = NULL;
ID3D11ShaderResourceView*   waveTwoTexture = NULL;

//=====		SKYBOX
ID3D11Buffer*           skyboxBuffer = NULL; 
int						skyboxVertCount = 36;
ID3D11VertexShader*		skyboxVertexShader = NULL;
ID3D11PixelShader*		skyboxPixelShader = NULL;

ID3D11ShaderResourceView*	skyboxTexture = NULL;

//=====		STATIONARY BILLBOARDS(PLANETS/2DSHIPS/BLACK HOLE)
ID3D11Buffer*           billboardBuffer = NULL; 
int						billboardVertCount = 6;
ID3D11VertexShader*		billboardVertexShader = NULL;
ID3D11PixelShader*		billboardPixelShader = NULL;

ID3D11ShaderResourceView*	billboardTextureP1 = NULL;
ID3D11ShaderResourceView*	billboardTextureP2 = NULL;
ID3D11ShaderResourceView*	billboardTextureP3 = NULL;
ID3D11ShaderResourceView*	billboardTextureP4 = NULL;
ID3D11ShaderResourceView*	billboardTextureP5 = NULL;
ID3D11ShaderResourceView*	billboardTextureMF = NULL;
ID3D11ShaderResourceView*	billboardTextureTF = NULL;
ID3D11ShaderResourceView*	billboardTextureE = NULL;
ID3D11ShaderResourceView*	billboardTextureBH = NULL;

//=====		BILLBOARDS(NEBULAS)

ID3D11ShaderResourceView*	billboardTextureN1 = NULL;
ID3D11ShaderResourceView*	billboardTextureN2 = NULL;


XMMATRIX nebulaBillBoard1[30]; //allows creation of 'cluster' of billboards
XMMATRIX nebulaBillBoard2[20];

int nebulaBillBoardCount = 30;//identifies the array index

//=====		CLASSIC THREE - 
XMMATRIX g_world;//model: per object position and rotation and scaling of the object
XMMATRIX g_view;//camera: position and rotation of the camera
XMMATRIX g_projection;//perspective: angle of view, near plane / far plane


//=====		CPU CONSTANT BUFFER - must match the GPU constant buffer; allows communication with the GPU
struct CONSTANT_MATRIX_BUFFER
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;

	XMFLOAT4 offsets;// camera perspectives
	XMFLOAT4 camPos;
	XMFLOAT4 waveOffset;
};
CONSTANT_MATRIX_BUFFER MatrixBuffer;

//=====		OVERIDE OPERATORS FUNCTIONS
XMFLOAT2 operator+(XMFLOAT2 lhs, XMFLOAT2 rhs)
{
	return XMFLOAT2(
		lhs.x + rhs.x,
		lhs.y + rhs.y
	);
}
XMFLOAT2 operator*(float lhs, XMFLOAT2 rhs)
{
	return XMFLOAT2(
		lhs * rhs.x,
		lhs * rhs.y
	);
}
//conflicting mul functions, hence the naming
XMMATRIX mulQS(const XMMATRIX &lhs, const XMMATRIX &rhs)
{
	XMMATRIX M = XMMatrixMultiply(lhs, rhs);
	return M;
}
//=====		QUICKSORT fn FROM VIDEO
//USE 'MARIO' OR A COLORFUL IMAGE FOR DEBUGGING
//'mulQS' fn allows for const mul
int partition(XMMATRIX *arr, const int left, const int right, XMMATRIX &view) {
	const int mid = left + (right - left) / 2;
	//"...pivot is that element you took out, and to which you're comparing the other elements in the list."
	//https://www.quora.com/What-is-the-pivot-in-quicksort
	const float pivot = mulQS(arr[mid], view)._43;
	std::swap(arr[mid], arr[left]); // move the midpoint value to the front.
	int i = left + 1;
	int j = right;

	/* partition */

	while (i <= j) {
		//no '>' exist for a XMMATRIX and float
		float arri = mulQS(arr[i], view)._43;
		while (i <= j && arri >= pivot) {
			i++;
			arri = mulQS(arr[i], view)._43;
		}
		float arrj = mulQS(arr[j], view)._43;
		while (i <= j && arrj < pivot) {
			j--;
			arrj = mulQS(arr[j], view)._43;
		}
		if (i < j) {
			std::swap(arr[i], arr[j]);
		}
	}
	std::swap(arr[i - 1], arr[left]);
	return i - 1;
}

void quicksort(XMMATRIX *arr, const int left, const int right, const int sz, XMMATRIX &view) {
	if (left >= right) {
		return;
	}
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	RECT rc;
//=====		getting the windows size into a RECT structure
	GetClientRect(hMain, &rc);	
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
//=====		DIRECTX COMPATABILITY
	D3D_DRIVER_TYPE driverTypes[] =
	{
	D3D_DRIVER_TYPE_HARDWARE,
	D3D_DRIVER_TYPE_WARP,
	D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

//====		INITIALIZE/CREATE 'FLIP FLOP'
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hMain;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

//=====		 CREATE A RENDER TARGET VIEW - converts 3d to 2d 
	ID3D11Texture2D* pBackBuffer = NULL;// -  interface manages texel data, which is structured memory.
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

//=====		INITIALLY SET CURRRENT RENDER TARGET VIEW
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);

//=====		 SETUP THE VIEWPORT
	D3D11_VIEWPORT vp;// - Defines the dimensions of a viewport.
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

///=====		COMPILE and CREATE THE POS TEX NORM VERTEX SHADERS - ALWAYS IN PAIRS: COMPILE&CREATE

//=====		COMPILE VS DEATHSTAR
	ID3DBlob* pVSBlob = NULL;// - interface is used to return data of arbitrary length. 
	hr = CompileShaderFromFile(L"shader.fx", "vs_deathstar", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

//=====		CREATE VS DEATHSTAR
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &deathstarVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}


//=====		LAYOUT #1 WITH NORMS - remember 4bytes per float...
//=====		DEFINE INPUT LAYOUT - A description of a single element for the input-assembler stage.
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);

//=====		SET INPUT LAYOUT - &planeLayout is the key to the layout
	hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &planeLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;


///=====		COMPILE and CREATE THE POS TEX VERTEX SHADERS -  ALWAYS IN PAIRS COMPILE&CREATE

//=====		COMPILE VS TERRAIN
	hr = CompileShaderFromFile(L"shader.fx", "vs_terrain", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

//=====		CREATE VS TERRAIN
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &terrainVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

//=====		COMPILE VS WATER
	hr = CompileShaderFromFile(L"shader.fx", "vs_water", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

//=====		CREATE VS WATER
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &waterVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

//=====		COMPILE VS SKYBOX
	hr = CompileShaderFromFile(L"shader.fx", "vs_skybox", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

//=====		CREATE VS SKYBOX
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &skyboxVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

//=====		COMPILE VS BILLBOARD
	hr = CompileShaderFromFile(L"shader.fx", "vs_billboard", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

//=====		CREATE VS BILLBOARD
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &billboardVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

//=====		LAYOUT #2  
//=====		DEFINE INPUT LAYOUT
	D3D11_INPUT_ELEMENT_DESC layout2[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	 numElements = ARRAYSIZE(layout2);

//=====		SET INPUT LAYOUT - &g_pVertexLayout is the key to the layout
	hr = g_pd3dDevice->CreateInputLayout(layout2, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

//=====		SET CURRRENT INPUT LAYOUT
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);


///=====		COMPILE and CREATE THE PIXEL SHADERS - ALWAYS IN PAIRS: COMPILE & CREATE

//=====		COMPILE PS DEATHSTAR
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(L"shader.fx", "ps_deathstar", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

//=====		CREATE PS DEATHSTAR
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &deathstarPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

//=====		COMPILE PS TERRAIN
	hr = CompileShaderFromFile(L"shader.fx", "ps_terrain", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

//=====		CREATE PS TERRAIN
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &terrainPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

//=====		COMPILE PS WATER
	hr = CompileShaderFromFile(L"shader.fx", "ps_water", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

//=====		CREATE PS WATER
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &waterPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

//=====		COMPILE PS SKYBOX
	hr = CompileShaderFromFile(L"shader.fx", "ps_skybox", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

//=====		CREATE PS SKYBOX
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &skyboxPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

//=====		COMPILE PS BILLBOARD
	hr = CompileShaderFromFile(L"shader.fx", "ps_billboard", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

//=====		CREATE PS BILLBOARD
	hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &billboardPixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

//=====		END SHADERS

//===========================	MESH - DEFINE DATA TYPE/ INITIALIZE / ADD TO BUFFER

	int meshX =		500, 
		meshZ =		500, 
		fraction =	500;

//=====		TERRAIN MESH - 25k triangles
	terrianVertCount = meshX * meshZ * 6;
	//dynamic memory allocation
	PosTexVertex* terrainData = new (nothrow) PosTexVertex[terrianVertCount];
	if (terrainData == nullptr) {
		//error assigning memory. Take measures.
	}

	float step = 1.0f / fraction;

	for (int jj = 0; jj < meshZ; ++jj)
	{
		for (int ii = 0; ii < meshX; ++ii)
		{
			unsigned int offset = (jj * 500 + ii) * 6;
			XMFLOAT3 squareOffset = XMFLOAT3(ii, 0, jj);
			XMFLOAT2 texOffset = XMFLOAT2(step * ii, step * jj);

			//triangle 1
			terrainData[offset + 0].Pos = XMFLOAT3(0, 0, 0)		+ squareOffset;
			terrainData[offset + 1].Pos = XMFLOAT3(0, 0, 1)		+ squareOffset;
			terrainData[offset + 2].Pos = XMFLOAT3(1, 0, 0)		+ squareOffset;
			
			terrainData[offset + 0].Tex = (XMFLOAT2(0, 0)		+ texOffset);
			terrainData[offset + 1].Tex = (XMFLOAT2(0, step)	+ texOffset);
			terrainData[offset + 2].Tex = (XMFLOAT2(step, 0)	+ texOffset);

			//triangle 2
			terrainData[offset + 3].Pos = XMFLOAT3(1, 0, 0)		+ squareOffset;
			terrainData[offset + 4].Pos = XMFLOAT3(0, 0, 1)		+ squareOffset;
			terrainData[offset + 5].Pos = XMFLOAT3(1, 0, 1)		+ squareOffset;

			terrainData[offset + 3].Tex = (XMFLOAT2(step, 0)	+ texOffset);
			terrainData[offset + 4].Tex = (XMFLOAT2(0, step)	+ texOffset);
			terrainData[offset + 5].Tex = (XMFLOAT2(step, step) + texOffset);
		}
	}
	
//=====		DESCRIBE/DEFINE/CREATE BUFFER
	D3D11_BUFFER_DESC bd;// - Describes a buffer resource.
	D3D11_SUBRESOURCE_DATA InitData;// - Specifies data for initializing a subresource.

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(PosTexVertex) * terrianVertCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = terrainData;

	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &terrainBuffer);
	if (FAILED(hr))
		return hr;
//=====		RETURN MEMORY
	delete[] terrainData;

//=====		WATER MESH

	waterVertCount = meshX * meshZ * 6;

	PosTexVertex *waterData = new (nothrow) PosTexVertex[waterVertCount];
	if (waterData == nullptr) {
		// error assigning memory. Take measures.
	}

	int count = 0;
	float ft = 1.0 / fraction;
	for (int zz = 0; zz< meshZ; zz++)
		for (int xx = 0; xx < meshX; xx++)
		{
			//xz plane to give it depth; xy plane would have the mesh in font of me

			waterData[count + 0].Pos = XMFLOAT3(0, 0, 1) + XMFLOAT3(xx, 0, zz);		//left top
			waterData[count + 1].Pos = XMFLOAT3(1, 0, 0) + XMFLOAT3(xx, 0, zz);		//right bottom
			waterData[count + 2].Pos = XMFLOAT3(0, 0, 0) + XMFLOAT3(xx, 0, zz);		//left bottom

			waterData[count + 0].Tex = XMFLOAT2(0.0f, ft) + XMFLOAT2(ft*xx, ft*zz);
			waterData[count + 1].Tex = XMFLOAT2(ft, 0.0f) + XMFLOAT2(ft*xx, ft*zz);
			waterData[count + 2].Tex = XMFLOAT2(0.0f, 0.0f) + XMFLOAT2(ft*xx, ft*zz);

			waterData[count + 3].Pos = XMFLOAT3(0, 0, 1) + XMFLOAT3(xx, 0, zz);		//left top
			waterData[count + 4].Pos = XMFLOAT3(1, 0, 1) + XMFLOAT3(xx, 0, zz);		//right top
			waterData[count + 5].Pos = XMFLOAT3(1, 0, 0) + XMFLOAT3(xx, 0, zz);		//right bottom

			waterData[count + 3].Tex = XMFLOAT2(0.0f, ft) + XMFLOAT2(ft*xx, ft*zz);
			waterData[count + 4].Tex = XMFLOAT2(ft, ft) + XMFLOAT2(ft*xx, ft*zz);
			waterData[count + 5].Tex = XMFLOAT2(ft, 0.0f) + XMFLOAT2(ft*xx, ft*zz);

			count = count + 6;
		}
//=====	BUFFER
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(PosTexVertex) * waterVertCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = waterData;

	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &waterBuffer);
	if (FAILED(hr))
		return hr;

//=====		RETURN MEMORY
	delete[] waterData;

//=====		SKYBOX MESH
	float texWide = (1.0f / 4.0f);
	float texHeight = (1.0f / 3.0f);

	PosTexVertex skyboxData[36];
	//front
	skyboxData[0].Pos = XMFLOAT3(-1, 1, 1);		skyboxData[0].Tex = XMFLOAT2(1 * texWide, 1 * texHeight);
	skyboxData[1].Pos = XMFLOAT3(-1, -1, 1);	skyboxData[1].Tex = XMFLOAT2(1 * texWide, 2 * texHeight);
	skyboxData[2].Pos = XMFLOAT3(1, -1, 1);		skyboxData[2].Tex = XMFLOAT2(2 * texWide, 2 * texHeight);

	skyboxData[3].Pos = XMFLOAT3(1, -1, 1);		skyboxData[3].Tex = XMFLOAT2(2 * texWide, 2 * texHeight);
	skyboxData[4].Pos = XMFLOAT3(1, 1, 1);		skyboxData[4].Tex = XMFLOAT2(2 * texWide, 1 * texHeight);
	skyboxData[5].Pos = XMFLOAT3(-1, 1, 1);		skyboxData[5].Tex = XMFLOAT2(1 * texWide, 1 * texHeight);
	//left
	skyboxData[6].Pos = XMFLOAT3(-1, 1, -1);	skyboxData[6].Tex = XMFLOAT2(0 * texWide, 1 * texHeight);
	skyboxData[7].Pos = XMFLOAT3(-1, -1, -1);	skyboxData[7].Tex = XMFLOAT2(0 * texWide, 2 * texHeight);
	skyboxData[8].Pos = XMFLOAT3(-1, -1, 1);	skyboxData[8].Tex = XMFLOAT2(1 * texWide, 2 * texHeight);

	skyboxData[9].Pos = XMFLOAT3(-1, -1, 1);	skyboxData[9].Tex = XMFLOAT2(1 * texWide, 2 * texHeight);
	skyboxData[10].Pos = XMFLOAT3(-1, 1, 1);	skyboxData[10].Tex = XMFLOAT2(1 * texWide, 1 * texHeight);
	skyboxData[11].Pos = XMFLOAT3(-1, 1, -1);	skyboxData[11].Tex = XMFLOAT2(0 * texWide, 1 * texHeight);
	//back
	skyboxData[12].Pos = XMFLOAT3(1, 1, -1);	skyboxData[12].Tex = XMFLOAT2(3 * texWide, 1 * texHeight);
	skyboxData[13].Pos = XMFLOAT3(1, -1, -1);	skyboxData[13].Tex = XMFLOAT2(3 * texWide, 2 * texHeight);
	skyboxData[14].Pos = XMFLOAT3(-1, -1, -1);	skyboxData[14].Tex = XMFLOAT2(4 * texWide, 2 * texHeight);

	skyboxData[15].Pos = XMFLOAT3(-1, -1, -1);	skyboxData[15].Tex = XMFLOAT2(4 * texWide, 2 * texHeight);
	skyboxData[16].Pos = XMFLOAT3(-1, 1, -1);	skyboxData[16].Tex = XMFLOAT2(4 * texWide, 1 * texHeight);
	skyboxData[17].Pos = XMFLOAT3(1, 1, -1);	skyboxData[17].Tex = XMFLOAT2(3 * texWide, 1 * texHeight);
	//right
	skyboxData[18].Pos = XMFLOAT3(1, -1, 1);	skyboxData[18].Tex = XMFLOAT2(2 * texWide, 2 * texHeight);
	skyboxData[19].Pos = XMFLOAT3(1, -1, -1);	skyboxData[19].Tex = XMFLOAT2(3 * texWide, 2 * texHeight);
	skyboxData[20].Pos = XMFLOAT3(1, 1, -1);	skyboxData[20].Tex = XMFLOAT2(3 * texWide, 1 * texHeight);

	skyboxData[21].Pos = XMFLOAT3(1, 1, -1);	skyboxData[21].Tex = XMFLOAT2(3 * texWide, 1 * texHeight);
	skyboxData[22].Pos = XMFLOAT3(1, 1, 1);		skyboxData[22].Tex = XMFLOAT2(2 * texWide, 1 * texHeight);
	skyboxData[23].Pos = XMFLOAT3(1, -1, 1);	skyboxData[23].Tex = XMFLOAT2(2 * texWide, 2 * texHeight);
	////top
	skyboxData[24].Pos = XMFLOAT3(-1, 1, 1);	skyboxData[24].Tex = XMFLOAT2(1 * texWide, 1 * texHeight);
	skyboxData[25].Pos = XMFLOAT3(1, 1, 1);		skyboxData[25].Tex = XMFLOAT2(2 * texWide, 1 * texHeight);
	skyboxData[26].Pos = XMFLOAT3(1, 1, -1);	skyboxData[26].Tex = XMFLOAT2(2 * texWide, 0 * texHeight);

	skyboxData[27].Pos = XMFLOAT3(1, 1, -1);		skyboxData[27].Tex = XMFLOAT2(2 * texWide, 0 * texHeight);
	skyboxData[28].Pos = XMFLOAT3(-1, 1, -1);		skyboxData[28].Tex = XMFLOAT2(1 * texWide, 0 * texHeight);
	skyboxData[29].Pos = XMFLOAT3(-1, 1, 1);		skyboxData[29].Tex = XMFLOAT2(1 * texWide, 1 * texHeight);
	//bottom
	skyboxData[30].Pos = XMFLOAT3(-1, -1, 1);		skyboxData[30].Tex = XMFLOAT2(1 * texWide, 2 * texHeight);
	skyboxData[31].Pos = XMFLOAT3(-1, -1, -1);		skyboxData[31].Tex = XMFLOAT2(1 * texWide, 3 * texHeight);
	skyboxData[32].Pos = XMFLOAT3(1, -1, -1);		skyboxData[32].Tex = XMFLOAT2(2 * texWide, 3 * texHeight);

	skyboxData[33].Pos = XMFLOAT3(1, -1, -1);		skyboxData[33].Tex = XMFLOAT2(2 * texWide, 3 * texHeight);
	skyboxData[34].Pos = XMFLOAT3(1, -1, 1);		skyboxData[34].Tex = XMFLOAT2(2 * texWide, 2 * texHeight);
	skyboxData[35].Pos = XMFLOAT3(-1, -1, 1);		skyboxData[35].Tex = XMFLOAT2(1 * texWide, 2 * texHeight);

	//=====	BUFFER
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(PosTexVertex) * 36;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = skyboxData;

	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &skyboxBuffer);
	if (FAILED(hr))
		return hr;

//=====		BILLBOARD MESH
	PosTexVertex billboardData[6];

	billboardData[0].Pos = XMFLOAT3(-1, 1, 1);	//left top
	billboardData[1].Pos = XMFLOAT3(1, -1, 1);	//right bottom
	billboardData[2].Pos = XMFLOAT3(-1, -1, 1); //left bottom

	billboardData[0].Tex = XMFLOAT2(0.0f, 0.0f);
	billboardData[1].Tex = XMFLOAT2(1.0f, 1.0f);
	billboardData[2].Tex = XMFLOAT2(0.0f, 1.0f);

	billboardData[3].Pos = XMFLOAT3(-1, 1, 1);	//left top
	billboardData[4].Pos = XMFLOAT3(1, 1, 1);	//right top
	billboardData[5].Pos = XMFLOAT3(1, -1, 1);	//right bottom

	billboardData[3].Tex = XMFLOAT2(0.0f, 0.0f);			
	billboardData[4].Tex = XMFLOAT2(1.0f, 0.0f);			
	billboardData[5].Tex = XMFLOAT2(1.0f, 1.0f);	

//=====	BUFFER
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(PosTexVertex) * 6;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = billboardData;
	hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &billboardBuffer);
	if (FAILED(hr))
		return hr;
	
//=====		DESCRIBE/DEFINE/CREATE CONSTANT_MATRIX_BUFFER
	D3D11_BUFFER_DESC cbDesc;

	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.ByteWidth = sizeof(CONSTANT_MATRIX_BUFFER);
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = &MatrixBuffer;

	hr = g_pd3dDevice->CreateBuffer(&cbDesc, &InitData, &g_pConstantMatrixBuffer);
	if (FAILED(hr))
		return hr;

//=====		TEXTURE SAMPLERS
//=====		INITIALIZE SAMPLER
	D3D11_SAMPLER_DESC sampDesc;

	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
//=====		CREATE SAMPLER
	hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &samplerState);
	if (FAILED(hr))
	{
		return hr;
	}


//=====		BLENDSTATE - MUST HAVE FOR TRANSPARENCY 
	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.AlphaToCoverageEnable = FALSE;
	blendStateDesc.IndependentBlendEnable = FALSE;
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;
	g_pd3dDevice->CreateBlendState(&blendStateDesc, &g_BlendState);


//=====		SET CURRENT STATE
	g_pImmediateContext->VSSetSamplers(0, 1, &samplerState);
	g_pImmediateContext->PSSetSamplers(0, 1, &samplerState);

//===== INITIALIZE TEXTURES
	//ASTEROID TERRAIN HEIGHTMAP
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"heightmap.png", NULL, NULL, &terrainTexture, NULL);
	if (FAILED(hr))
		return hr;
	//ROCKY TEXTURE
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"rock.jpg", NULL, NULL, &rockTexture, NULL);
	if (FAILED(hr))
		return hr;
	//DEATHSTAR TEXTURE
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"ds_S.png", NULL, NULL, &deathstarTexture, NULL);
	if (FAILED(hr))
		return hr;
	//WATER ONE TEXTURE
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"wave1.jpg", NULL, NULL, &waveOneTexture, NULL);
	if (FAILED(hr))
		return hr;
	//WATER TWO TEXTURE
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"wave2.jpg", NULL, NULL, &waveTwoTexture, NULL);
	if (FAILED(hr))
		return hr;
	//SKYBOX TEXTURE
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"skybox.png", NULL, NULL, &skyboxTexture, NULL);
	if (FAILED(hr))
		return hr;
	//BILLBOARD TEXTURES
	//PLANET 1-5
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"p1.png", NULL, NULL, &billboardTextureP1, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"p2.png", NULL, NULL, &billboardTextureP2, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"p3.png", NULL, NULL, &billboardTextureP3, NULL);
	if (FAILED(hr))
		return hr;

	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"p4.png", NULL, NULL, &billboardTextureP4, NULL);
	if (FAILED(hr))
		return hr;
	 
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"p5.png", NULL, NULL, &billboardTextureP5, NULL);
	if (FAILED(hr))
		return hr;
	//MILLIeNUM FALCON
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"mf.png", NULL, NULL, &billboardTextureMF, NULL);
	if (FAILED(hr))
		return hr;
	//TIE FIGHTER
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"tf.png", NULL, NULL, &billboardTextureTF, NULL);
	if (FAILED(hr))
		return hr;
	//BLACKHOLE
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"bh.png", NULL, NULL, &billboardTextureBH, NULL);
	if (FAILED(hr))
		return hr;
	//ENTERPRISE
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"e.png", NULL, NULL, &billboardTextureE, NULL);
	if (FAILED(hr))
		return hr;
	//NEBULA1
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"n1.png", NULL, NULL, &billboardTextureN1, NULL);
	if (FAILED(hr))
		return hr;
	//NEBULA2
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"n2.png", NULL, NULL, &billboardTextureN2, NULL);
	if (FAILED(hr))
		return hr;
	
//===== END TEXTURES

//=====		DEPTH BUFFER
//=====		INITIALIZE 
	D3D11_TEXTURE2D_DESC descDepth;

	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_R32_TYPELESS;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
//=====		CREATE DEPTH TEXTURE
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
	if (FAILED(hr))
		return hr;

//=====		DEPTH STENCIL VIEW BUFFER
//=====		INITIALIZE 
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;

	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
//=====		CREATE STENCIL VIEW BUFFER
	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
	if (FAILED(hr))
		return hr;

//=====		RASTERIZER STATES - CLOCKWISE/COUNTERCLOCKWISE/WIRE
	D3D11_RASTERIZER_DESC			RS_CW, RS_Wire;

	RS_CW.AntialiasedLineEnable = FALSE;
	RS_CW.CullMode = D3D11_CULL_BACK;
	RS_CW.DepthBias = 0;
	RS_CW.DepthBiasClamp = 0.0f;
	RS_CW.DepthClipEnable = true;
	RS_CW.FillMode = D3D11_FILL_SOLID;
	RS_CW.FrontCounterClockwise = false;
	RS_CW.MultisampleEnable = FALSE;
	RS_CW.ScissorEnable = false;
	RS_CW.SlopeScaledDepthBias = 0.0f;

	//rasterizer state clockwise triangles
	g_pd3dDevice->CreateRasterizerState(&RS_CW, &rs_CW);

	//rasterizer state counterclockwise triangles
	RS_CW.CullMode = D3D11_CULL_FRONT;
	g_pd3dDevice->CreateRasterizerState(&RS_CW, &rs_CCW);
	RS_Wire = RS_CW;
	RS_Wire.CullMode = D3D11_CULL_NONE;

	//rasterizer state seeing both sides of the triangle
	g_pd3dDevice->CreateRasterizerState(&RS_Wire, &rs_NO);

	//rasterizer state wirefrime
	RS_Wire.FillMode = D3D11_FILL_WIREFRAME;
	g_pd3dDevice->CreateRasterizerState(&RS_Wire, &rs_Wire);

//=====		INITIALIZE DEPTH STATS - RENDERS OUT OF ORDER OF DRAW
	//create the depth stencil states for turning the depth buffer on and of:
	D3D11_DEPTH_STENCIL_DESC		DS_ON, DS_OFF;
	DS_ON.DepthEnable = true;
	DS_ON.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DS_ON.DepthFunc = D3D11_COMPARISON_LESS;

	//Stencil test parameters
	DS_ON.StencilEnable = true;
	DS_ON.StencilReadMask = 0xFF;
	DS_ON.StencilWriteMask = 0xFF;

	//Stencil operations if pixel is front-facing
	DS_ON.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	DS_ON.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//Stencil operations if pixel is back-facing
	DS_ON.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	DS_ON.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DS_ON.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

//=====		CREATE STENCIL STATES
	DS_OFF = DS_ON;
	DS_OFF.DepthEnable = false;
	g_pd3dDevice->CreateDepthStencilState(&DS_ON, &ds_on);
	g_pd3dDevice->CreateDepthStencilState(&DS_OFF, &ds_off);

//=====		LOADOBJ AND LOAD3DS OBJECTS
	LoadOBJ("ds.obj", g_pd3dDevice, &deathstarBuffer, &deathstarVertCount);

//=====		CREATE GROUP OF 'MARIOS' or NEBULAS - FROM VIDEO - 

	for (int i = 0; i < nebulaBillBoardCount; i++) {
		nebulaBillBoard1[i] = XMMatrixTranslation(2 + (rand() % 6 + ((rand() % 10) / 10.0)), rand() % 4 + ((rand() % 10) / 10.0), 5 + rand() % 6 + ((rand() % 10) / 10.0) );
	}
	for (int i = 0; i < nebulaBillBoardCount-10; i++) {
		nebulaBillBoard2[i] = XMMatrixTranslation( 5 + rand() % 6 + ((rand() % 10) / 10.0), 3 + rand() % 4 + ((rand() % 10) / 10.0), 2 + (rand() % 6 + ((rand() % 10) / 10.0)) );
	}

//=====		CLASSIC MATRIX INITIALIZE

	g_world = XMMatrixIdentity();

		XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);//camera position
		XMVECTOR At = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);//look at
		XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);// normal vector on at vector (always up)
	g_view = XMMatrixLookAtLH(Eye, At, Up);

	g_projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.01f, 10000.0f);

//=====		BUFFER DATA
	MatrixBuffer.world = g_world;
	MatrixBuffer.view = g_view;
	MatrixBuffer.projection = g_projection;
	//MatrixBuffer.offsets = XMFLOAT4(0, 0, 0, 0);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Render function
//--------------------------------------------------------------------------------------

float angle = 0;// - ROTATION 'Y' AXIS
float angleSpeed = 0.01;// - ROTATION SPEED
float accel = 0.0001;// - 'W' 'S' FORWARD BACK 
float camSpeed = 0.001;// - SPEED OF THE MOVING TEXTURES 
//DEFAULT KEYBOARD INPUT
bool a_key = false;
bool d_key = false;
bool w_key = false;
bool s_key = false;
//USED TO STORE THE VALUE OF OFFSETS IN VS
static XMFLOAT4 waveOffset = { 0.0f, 0.0f, 0.0f, 0.0f };

XMFLOAT2 camPos = XMFLOAT2(0, 0); //<-- camera position in x-z plane. note that z is y here


void Render()
{
//USER KEYBOARD INPUT 
	if (a_key && !d_key)
	{
		angle -= angleSpeed;
	}
	else if (d_key && !a_key)
	{
		angle += angleSpeed;
	}
	else if (w_key && !s_key)
	{
		camSpeed += accel;
	}
	else if (s_key && !w_key)
	{
		camSpeed -= accel;
	}
	else
	{//DEFAULT 
		camSpeed = 0.001;
	}

//WATER - WAVE OFFSET
	waveOffset.x += 0.000001;
	waveOffset.y += 0.000001;
	waveOffset.z += 0.000001;
	waveOffset.w += 0.000001;

//===== CONSTANT DATA BUFFER - SETS Y VECTOR STATIC - NO UP/DOWN
	{
		XMVECTOR forward = XMVectorSet(0, 0, 1, 0);
		XMMATRIX rotation = XMMatrixRotationY(angle);
		forward = XMVector3Transform(forward, rotation);
		forward = camSpeed * XMVector3Normalize(forward);
		camPos.x += XMVectorGetX(forward);//PASSES VALUE INTO VS TERRAIN/WATER TEXTURE
		camPos.y += XMVectorGetZ(forward);

		MatrixBuffer.offsets.x = camPos.x;
		MatrixBuffer.offsets.y = camPos.y;

		MatrixBuffer.waveOffset = waveOffset;
	}

//===== VERTICIES STATE - EG POINTLIST,TRIANGLELIST
	g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

//===== CLEAR THE BACK BUFFER
	float ClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };//DEFAULT COLOR HELPS WITH DEBUGGING
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

	//INIT & CREATE BLENDER
	float blendFactor[] = { 0, 0, 0, 0 };
	UINT sampleMask = 0xffffffff;
	g_pImmediateContext->OMSetBlendState(g_BlendState, blendFactor, sampleMask);

//=====	TESTING
		//g_pImmediateContext->RSSetState(rs_Wire);

//=======================================		SKYBOX MESH & TEXTURE
	g_pImmediateContext->RSSetState(rs_CW);

//===== SET SHADERS
	g_pImmediateContext->VSSetShader(skyboxVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(skyboxPixelShader, NULL, 0);

//===== SCALE ROTATE TRANSLATE
	MatrixBuffer.world = XMMatrixRotationY(-angle); ;
	MatrixBuffer.view = g_view;

//===== UPDATE RESOURCES
	g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, 0, &MatrixBuffer, 0, 0);

//===== SET VS PS CONSTANTBUFFERS
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);

//===== SET SHADER RESOURCES
	g_pImmediateContext->PSSetShaderResources(0, 1, &skyboxTexture);

//===== SET STRIDE AND OFFSET
	UINT stride = sizeof(PosTexVertex);
	UINT offset = 0;

//===== IASET VBUFFER AND INPUT LAYOUT
	g_pImmediateContext->IASetVertexBuffers(0, 1, &skyboxBuffer, &stride, &offset);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

//=====	SEE IT FROM THE INSIDE/ NO DEPTH WRITING
	g_pImmediateContext->RSSetState(rs_CCW);
	g_pImmediateContext->OMSetDepthStencilState(ds_off, 1);

//=====	DRAW
	g_pImmediateContext->Draw(36, 0);

//=====	RESTORE TO DEFAULT
	g_pImmediateContext->RSSetState(rs_CW);
	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);

//=======================================	WATER MESH & TEXTURE

//=====		SET SHADERS
	g_pImmediateContext->VSSetShader(waterVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(waterPixelShader, NULL, 0);

//=====		SCALE ROTATE TRANSLATE
	MatrixBuffer.world = XMMatrixTranslation(-250, -10, -250);
	MatrixBuffer.view = XMMatrixRotationY(-angle);
	//no change MatrixBuffer.projection 

//=====		UPDATE RESOURCES
	g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, 0, &MatrixBuffer, 0, 0);

//===== SET VS PS CONSTANTBUFFER
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);

//===== SET SHADER RESOURCES
	g_pImmediateContext->VSSetShaderResources(0, 1, &waveOneTexture);
	g_pImmediateContext->VSSetShaderResources(1, 1, &waveTwoTexture);

	g_pImmediateContext->PSSetShaderResources(0, 1, &waveOneTexture);
	g_pImmediateContext->PSSetShaderResources(1, 1, &waveTwoTexture);

//=====	SET STRIDE AND OFFSET
	stride = sizeof(PosTexVertex);


//=====	IASET VBUFFER AND INPUT LAYOUT
	g_pImmediateContext->IASetVertexBuffers(0, 1, &waterBuffer, &stride, &offset);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

// TESTING
	//g_pImmediateContext->RSSetState(rs_Wire);

//=====	DRAW
	g_pImmediateContext->Draw(waterVertCount, 0);

	////////////////////////  RINSE REPEAT  /////////////////////////////

//=======================================		TERRAIN MESH & TEXTURE
//===== SET SHADERS
	g_pImmediateContext->VSSetShader(terrainVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(terrainPixelShader, NULL, 0);

//===== SCALE ROTATE TRANSLATE 
	MatrixBuffer.world = XMMatrixTranslation(-250, -10, -250);
	MatrixBuffer.view = XMMatrixRotationY(-angle);
	//no change MatrixBuffer.projection

//===== UPDATE RESOURCES
	g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, 0, &MatrixBuffer, 0, 0);

//===== SET VS PS CONSTANTBUFFERS
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);

//===== SET SHADER RESOURCES
	g_pImmediateContext->VSSetShaderResources(0, 1, &terrainTexture);

	g_pImmediateContext->PSSetShaderResources(0, 1, &terrainTexture);
	g_pImmediateContext->PSSetShaderResources(1, 1, &rockTexture);

//===== SET STRIDE AND OFFSET
	stride = sizeof(PosTexVertex);

//===== IASET VBUFFER AND INPUT LAYOUT
	g_pImmediateContext->IASetVertexBuffers(0, 1, &terrainBuffer, &stride, &offset);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

//=====	DRAW
	g_pImmediateContext->Draw(terrianVertCount, 0);

//=====	TESTING
		//g_pImmediateContext->RSSetState(rs_Wire);

	//////////////////////	RINSE REPEAT  /////////////////////////////

//=======================================		DEATHSTAR PLANE MESH & TEXTURE
	static float spin = 0.0f;
	spin += 0.001;

//===== SET SHADERS
	g_pImmediateContext->VSSetShader(deathstarVertexShader, 0, 0);
	g_pImmediateContext->PSSetShader(deathstarPixelShader, 0, 0);

//===== SCALE ROTATE TRANSLATE	

	//NOTE: there is a mystery translation in the rotation - in an small orbit

	XMMATRIX T = XMMatrixTranslation(0, -3, 20) * XMMatrixRotationY(angle);
	XMMATRIX Rz = XMMatrixRotationZ(XM_PIDIV4 + (XM_PIDIV2 / 6.0f));
	XMMATRIX Rx = XMMatrixRotationX((XM_PIDIV2) / 3.0f);
	XMMATRIX Ry = XMMatrixRotationY(spin);

	MatrixBuffer.world = Rz*Rx*Ry*T;


//===== UPDATE RESOURCES
	g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, 0, &MatrixBuffer, 0, 0);

//===== SET VS PS BUFFERS
		//NONE

//===== SET SHADER RESOURCES
	g_pImmediateContext->PSSetShaderResources(0, 1, &deathstarTexture);

//===== SET STRIDE AND OFFSET
	stride = sizeof(PosTexNormVertex);

//===== IASET VBUFFER AND INPUT LAYOUT
	g_pImmediateContext->IASetVertexBuffers(0, 1, &deathstarBuffer, &stride, &offset);
	g_pImmediateContext->IASetInputLayout(planeLayout);

//=====	DRAW
	g_pImmediateContext->Draw(deathstarVertCount, 0);

	//////////////////////	RINSE REPEAT  /////////////////////////////

//=======================================		BILLBOARD MESH & TEXTURES
	XMMATRIX Vc = g_view;
	Vc._41 = 0;
	Vc._42 = 0;
	Vc._43 = 0;
	XMVECTOR f;
	Vc = XMMatrixInverse(&f, Vc);

//===== SET SHADERS
	g_pImmediateContext->VSSetShader(billboardVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(billboardPixelShader, NULL, 0);

//===== SCALE ROTATE TRANSLATE 
	T = XMMatrixTranslation(-30, 14, 40);
	Ry = XMMatrixRotationY(angle);
	XMMATRIX S = XMMatrixScaling(3, 3, 3);
	MatrixBuffer.world = S*Vc*Ry*T;

//===== UPDATE RESOURCES
	g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, 0, &MatrixBuffer, 0, 0);

//===== SET VS PS CONSTANTBUFFERS
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);

//===== SET SHADER RESOURCES
	g_pImmediateContext->PSSetShaderResources(0, 1, &billboardTextureP1);


//===== SET STRIDE AND OFFSET
	stride = sizeof(PosTexVertex);

//===== IASET VBUFFER AND INPUT LAYOUT
	g_pImmediateContext->IASetVertexBuffers(0, 1, &billboardBuffer, &stride, &offset);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

//=====	DRAW
	g_pImmediateContext->Draw(6, 0);

//=====	TESTING
	//g_pImmediateContext->RSSetState(rs_Wire);

	//////////////////////	RINSE REPEAT  /////////////////////////////

//===== SET SHADERS
	g_pImmediateContext->VSSetShader(billboardVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(billboardPixelShader, NULL, 0);

//===== SCALE ROTATE TRANSLATE 
	T = XMMatrixTranslation(50, 14, 10);
	Ry = XMMatrixRotationY(angle);
	S = XMMatrixScaling(1, 1, 1);
	MatrixBuffer.world = S*Vc*Ry*T;

//===== UPDATE RESOURCES
	g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, 0, &MatrixBuffer, 0, 0);

//===== SET VS PS CONSTANTBUFFERS
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);

//===== SET SHADER RESOURCES
	g_pImmediateContext->PSSetShaderResources(0, 1, &billboardTextureP2);


//===== SET STRIDE AND OFFSET
	stride = sizeof(PosTexVertex);

//===== IASET VBUFFER AND INPUT LAYOUT
	g_pImmediateContext->IASetVertexBuffers(0, 1, &billboardBuffer, &stride, &offset);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

//=====	DRAW
	g_pImmediateContext->Draw(6, 0);

//=====	TESTING
	//g_pImmediateContext->RSSetState(rs_Wire);

	//////////////////////	RINSE REPEAT  /////////////////////////////

//===== SET SHADERS
	g_pImmediateContext->VSSetShader(billboardVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(billboardPixelShader, NULL, 0);

//===== SCALE ROTATE TRANSLATE 
	T = XMMatrixTranslation(10, 15, -50);// *XMMatrixRotationY(angle);
	Ry = XMMatrixRotationY(angle);
	S = XMMatrixScaling(1, 1, 1);
	MatrixBuffer.world = S*Vc*Ry*T;

//===== UPDATE RESOURCES
	g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, 0, &MatrixBuffer, 0, 0);

//===== SET VS PS CONSTANTBUFFERS
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);

//===== SET SHADER RESOURCES
	g_pImmediateContext->PSSetShaderResources(0, 1, &billboardTextureP3);


//===== SET STRIDE AND OFFSET
	stride = sizeof(PosTexVertex);

//===== IASET VBUFFER AND INPUT LAYOUT
	g_pImmediateContext->IASetVertexBuffers(0, 1, &billboardBuffer, &stride, &offset);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

//=====	DRAW
	g_pImmediateContext->Draw(6, 0);

//=====	TESTING
	//g_pImmediateContext->RSSetState(rs_Wire);

	//////////////////////	RINSE REPEAT  /////////////////////////////
	//===== SET SHADERS
	g_pImmediateContext->VSSetShader(billboardVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(billboardPixelShader, NULL, 0);

//===== SCALE ROTATE TRANSLATE 
	T = XMMatrixTranslation(-40, 9, 0);// *XMMatrixRotationY(angle);
	Ry = XMMatrixRotationY(angle);
	S = XMMatrixScaling(1.2, 1.2, 1.2);
	MatrixBuffer.world = S*Vc*Ry*T;

//===== UPDATE RESOURCES
	g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, 0, &MatrixBuffer, 0, 0);

//===== SET VS PS CONSTANTBUFFERS
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);

//===== SET SHADER RESOURCES
	g_pImmediateContext->PSSetShaderResources(0, 1, &billboardTextureP4);


//===== SET STRIDE AND OFFSET
	stride = sizeof(PosTexVertex);

//===== IASET VBUFFER AND INPUT LAYOUT
	g_pImmediateContext->IASetVertexBuffers(0, 1, &billboardBuffer, &stride, &offset);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

//=====	DRAW
	g_pImmediateContext->Draw(6, 0);

//=====	TESTING
	//g_pImmediateContext->RSSetState(rs_Wire);

	//////////////////////	RINSE REPEAT  /////////////////////////////
//===== SET SHADERS
	g_pImmediateContext->VSSetShader(billboardVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(billboardPixelShader, NULL, 0);

//===== SCALE ROTATE TRANSLATE 
	T = XMMatrixTranslation(25, 7, 40);// *XMMatrixRotationY(angle);
	Ry = XMMatrixRotationY(angle);
	S = XMMatrixScaling(2, 2, 2);
	MatrixBuffer.world = S*Vc*Ry*T;

//===== UPDATE RESOURCES
	g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, 0, &MatrixBuffer, 0, 0);

//===== SET VS PS CONSTANTBUFFERS
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);

//===== SET SHADER RESOURCES
	g_pImmediateContext->PSSetShaderResources(0, 1, &billboardTextureP5);


//===== SET STRIDE AND OFFSET
	stride = sizeof(PosTexVertex);

//===== IASET VBUFFER AND INPUT LAYOUT
	g_pImmediateContext->IASetVertexBuffers(0, 1, &billboardBuffer, &stride, &offset);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

//=====	DRAW
	g_pImmediateContext->Draw(6, 0);

//=====	TESTING
	//g_pImmediateContext->RSSetState(rs_Wire);

	//////////////////////	RINSE REPEAT  /////////////////////////////
//===== SET SHADERS
	g_pImmediateContext->VSSetShader(billboardVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(billboardPixelShader, NULL, 0);

//===== SCALE ROTATE TRANSLATE 
	T = XMMatrixTranslation(45, 5, -40);// *XMMatrixRotationY(angle);
	Ry = XMMatrixRotationY(angle);
	S = XMMatrixScaling(1, 1, 1);
	MatrixBuffer.world = S*Vc*Ry*T;

//===== UPDATE RESOURCES
	g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, 0, &MatrixBuffer, 0, 0);

//===== SET VS PS CONSTANTBUFFERS
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);

//===== SET SHADER RESOURCES
	g_pImmediateContext->PSSetShaderResources(0, 1, &billboardTextureMF);


//===== SET STRIDE AND OFFSET
	stride = sizeof(PosTexVertex);

//===== IASET VBUFFER AND INPUT LAYOUT
	g_pImmediateContext->IASetVertexBuffers(0, 1, &billboardBuffer, &stride, &offset);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

//=====	DRAW
	g_pImmediateContext->Draw(6, 0);

//=====	TESTING
	//g_pImmediateContext->RSSetState(rs_Wire);

	//////////////////////	RINSE REPEAT  /////////////////////////////
//===== SET SHADERS
	g_pImmediateContext->VSSetShader(billboardVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(billboardPixelShader, NULL, 0);

//===== SCALE ROTATE TRANSLATE 
	T = XMMatrixTranslation(50, 7, -40);
	Ry = XMMatrixRotationY(angle);
	S = XMMatrixScaling(1, 1, 1);
	MatrixBuffer.world = S*Vc*Ry*T;

//===== UPDATE RESOURCES
	g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, 0, &MatrixBuffer, 0, 0);

//===== SET VS PS CONSTANTBUFFERS
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);

//===== SET SHADER RESOURCES
	g_pImmediateContext->PSSetShaderResources(0, 1, &billboardTextureTF);


//===== SET STRIDE AND OFFSET
	stride = sizeof(PosTexVertex);

//===== IASET VBUFFER AND INPUT LAYOUT
	g_pImmediateContext->IASetVertexBuffers(0, 1, &billboardBuffer, &stride, &offset);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

//=====	DRAW
	g_pImmediateContext->Draw(6, 0);

//=====	TESTING
	//g_pImmediateContext->RSSetState(rs_Wire);

	//////////////////////	RINSE REPEAT  /////////////////////////////
//===== SET SHADERS
	g_pImmediateContext->VSSetShader(billboardVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(billboardPixelShader, NULL, 0);

//===== SCALE ROTATE TRANSLATE 
	T = XMMatrixTranslation(-40, 8, -40);// *XMMatrixRotationY(angle);
	Ry = XMMatrixRotationY(angle);
	S = XMMatrixScaling(2, 2, 2);
	MatrixBuffer.world = S*Vc*Ry*T;

//===== UPDATE RESOURCES
	g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, 0, &MatrixBuffer, 0, 0);

//===== SET VS PS CONSTANTBUFFERS
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);

//===== SET SHADER RESOURCES
	g_pImmediateContext->PSSetShaderResources(0, 1, &billboardTextureBH);


//===== SET STRIDE AND OFFSET
	stride = sizeof(PosTexVertex);

//===== IASET VBUFFER AND INPUT LAYOUT
	g_pImmediateContext->IASetVertexBuffers(0, 1, &billboardBuffer, &stride, &offset);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

//=====	DRAW
	g_pImmediateContext->Draw(6, 0);

//=====	TESTING
	//g_pImmediateContext->RSSetState(rs_Wire);

	//////////////////////	RINSE REPEAT  /////////////////////////////
//===== SET SHADERS
	g_pImmediateContext->VSSetShader(billboardVertexShader, NULL, 0);
	g_pImmediateContext->PSSetShader(billboardPixelShader, NULL, 0);

//===== SCALE ROTATE TRANSLATE 
	T = XMMatrixTranslation(-45, 7, -35);// *XMMatrixRotationY(angle);
	Ry = XMMatrixRotationY(angle);
	S = XMMatrixScaling(2.5, 2.5, 2.5);
	MatrixBuffer.world = S*Vc*Ry*T;

//===== UPDATE RESOURCES
	g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, 0, &MatrixBuffer, 0, 0);

//===== SET VS PS CONSTANTBUFFERS
	g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);
	g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);

//===== SET SHADER RESOURCES
	g_pImmediateContext->PSSetShaderResources(0, 1, &billboardTextureE);


//===== SET STRIDE AND OFFSET
	stride = sizeof(PosTexVertex);

//===== IASET VBUFFER AND INPUT LAYOUT
	g_pImmediateContext->IASetVertexBuffers(0, 1, &billboardBuffer, &stride, &offset);
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

//=====	DRAW
	g_pImmediateContext->Draw(6, 0);

//=====	TESTING
	//g_pImmediateContext->RSSetState(rs_Wire);

//////////////////////	RINSE REPEAT  /////////////////////////////

//===== NEBULA MESH AND TEXTURE

	quicksort(nebulaBillBoard1, 0, nebulaBillBoardCount - 1, nebulaBillBoardCount, g_view);

//=====	RESOLVES THE OVERLAPPING CLIPPING **********
	g_pImmediateContext->OMSetDepthStencilState(ds_off, 1);//allows for overlapping images to be rendered

	for (int i = 0; i < nebulaBillBoardCount; i++) {
//===== SET SHADERS
		g_pImmediateContext->VSSetShader(billboardVertexShader, NULL, 0);
		g_pImmediateContext->PSSetShader(billboardPixelShader, NULL, 0);

//===== SCALE ROTATE TRANSLATE 
		S = XMMatrixScaling(1.5,1.5,1.5);
		T = nebulaBillBoard1[i];
		XMMATRIX T2 = XMMatrixTranslation(0, 4, 20);
		Ry = XMMatrixRotationY(angle);
		MatrixBuffer.world = S*Vc*Ry*T*T2;
		
//===== UPDATE RESOURCES
		g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, 0, &MatrixBuffer, 0, 0);

//===== SET VS PS CONSTANTBUFFERS
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);
		g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);

//===== SET SHADER RESOURCES
		g_pImmediateContext->PSSetShaderResources(0, 1, &billboardTextureN1);


//===== SET STRIDE AND OFFSET
		stride = sizeof(PosTexVertex);

//===== IASET VBUFFER AND INPUT LAYOUT
		g_pImmediateContext->IASetVertexBuffers(0, 1, &billboardBuffer, &stride, &offset);
		g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

//=====	DRAW
		g_pImmediateContext->Draw(6, 0);

//=====	TESTING
		//g_pImmediateContext->RSSetState(rs_Wire);
	}

	//depth writing ON  
	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);

	//////////////////////	RINSE REPEAT  /////////////////////////////

	quicksort(nebulaBillBoard2, 0, nebulaBillBoardCount - 1, nebulaBillBoardCount, g_view);

	//=====	RESOLVES THE OVERLAPPING CLIPPING **********
	g_pImmediateContext->OMSetDepthStencilState(ds_off, 1);//allows for overlapping images to be rendered

	for (int i = 0; i < nebulaBillBoardCount; i++) {
		//===== SET SHADERS
		g_pImmediateContext->VSSetShader(billboardVertexShader, NULL, 0);
		g_pImmediateContext->PSSetShader(billboardPixelShader, NULL, 0);

		//===== SCALE ROTATE TRANSLATE 
		S = XMMatrixScaling(1.5,1.5,1.5);
		T = nebulaBillBoard2[i];
		XMMATRIX T2 = XMMatrixTranslation(-15, 5, -40);
		Ry = XMMatrixRotationY(angle);
		MatrixBuffer.world = S*Vc*Ry*T*T2;

		//===== UPDATE RESOURCES
		g_pImmediateContext->UpdateSubresource(g_pConstantMatrixBuffer, 0, 0, &MatrixBuffer, 0, 0);

		//===== SET VS PS CONSTANTBUFFERS
		g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);
		g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantMatrixBuffer);

		//===== SET SHADER RESOURCES
		g_pImmediateContext->PSSetShaderResources(0, 1, &billboardTextureN2);


		//===== SET STRIDE AND OFFSET
		stride = sizeof(PosTexVertex);

		//===== IASET VBUFFER AND INPUT LAYOUT
		g_pImmediateContext->IASetVertexBuffers(0, 1, &billboardBuffer, &stride, &offset);
		g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

		//=====	DRAW
		g_pImmediateContext->Draw(6, 0);

		//=====	TESTING
		//g_pImmediateContext->RSSetState(rs_Wire);
	}

	//depth writing ON  
	g_pImmediateContext->OMSetDepthStencilState(ds_on, 1);

	//////////////////////	RINSE REPEAT  /////////////////////////////

//===== SWAP CHAIN - 
	g_pSwapChain->Present(1, 0);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);	//message loop function (containing all switch-case statements

int WINAPI wWinMain(				//	the main function in a window program. program starts here
	HINSTANCE hInstance,			//	here the program gets its own number
	HINSTANCE hPrevInstance,		//	in case this program is called from within another program
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	hInst = hInstance;												//						save in global variable for further use
	MSG msg;

	// Globale Zeichenfolgen initialisieren
	LoadString(hInstance, 103, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, 104, szWindowClass, MAX_LOADSTRING);
	//register Window															<<<<<<<<<<  STEP ONE: REGISTER WINDOW						
	WNDCLASSEX wcex;												//						=> Filling out struct WNDCLASSEX
	BOOL Result = TRUE;
	wcex.cbSize = sizeof(WNDCLASSEX);								//						size of this struct (don't know why
	wcex.style = CS_HREDRAW | CS_VREDRAW;							//						?
	wcex.lpfnWndProc = (WNDPROC)WndProc;							//						The corresponding Proc File -> Message loop switch-case file
	wcex.cbClsExtra = 0;											//
	wcex.cbWndExtra = 0;											//
	wcex.hInstance = hInstance;										//						The number of the program
	wcex.hIcon = LoadIcon(hInstance, NULL);							//
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);						//
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);				//						Background color
	wcex.lpszMenuName = NULL;										//
	wcex.lpszClassName = L"TutorialWindowClass";					//						Name of the window (must the the same as later when opening the window)
	wcex.hIconSm = LoadIcon(wcex.hInstance, NULL);					//
	Result = (RegisterClassEx(&wcex) != 0);							//						Register this struct in the OS

				//													STEP TWO: OPENING THE WINDOW with x,y position and xlen, ylen 
	RECT rc = { 0, 0, 1920, 1080 };//640,480 ... 1280,720
	hMain = CreateWindow(L"TutorialWindowClass", L"Direct3D 11 Tutorial 2: Rendering a Triangle",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL);
	if (hMain == 0)	return 0;

	ShowWindow(hMain, nCmdShow);
	UpdateWindow(hMain);


	if (FAILED(InitDevice()))
	{
		return 0;
	}

	//=====					STEP THREE: Going into the infinity message loop							

	// Main message loop
	msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}

	return (int)msg.wParam;
}
///////////////////////////////////////////////////
void redr_win_full(HWND hwnd, bool erase)
{
	RECT rt;
	GetClientRect(hwnd, &rt);
	InvalidateRect(hwnd, &rt, erase);
}

///////////////////////////////////
//		This Function is called every time the Left Mouse Button is down
///////////////////////////////////
void OnLBD(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
}
///////////////////////////////////
//		This Function is called every time the Right Mouse Button is down
///////////////////////////////////
void OnRBD(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
}
///////////////////////////////////
//		This Function is called every time a character key is pressed
///////////////////////////////////
void OnChar(HWND hwnd, UINT ch, int cRepeat)
{
}
///////////////////////////////////
//		This Function is called every time the Left Mouse Button is up
///////////////////////////////////
void OnLBU(HWND hwnd, int x, int y, UINT keyFlags)
{
}
///////////////////////////////////
//		This Function is called every time the Right Mouse Button is up
///////////////////////////////////
void OnRBU(HWND hwnd, int x, int y, UINT keyFlags)
{
}
///////////////////////////////////
//		This Function is called every time the Mouse Moves
///////////////////////////////////
void OnMM(HWND hwnd, int x, int y, UINT keyFlags)
{
}
///////////////////////////////////
//		This Function is called once at the begin of a program
///////////////////////////////////
#define TIMER1 1

BOOL OnCreate(HWND hwnd, CREATESTRUCT FAR* lpCreateStruct)
{
	hMain = hwnd;
	return TRUE;
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	HWND hwin;
	switch (id)
	{
	default:
		break;
	}

}
//************************************************************************
void OnTimer(HWND hwnd, UINT id)
{
}
//************************************************************************

///////////////////////////////////
//		This Function is called every time the window has to be painted again
///////////////////////////////////

void OnPaint(HWND hwnd)
{
}
//****************************************************************************

//*************************************************************************
void OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	switch (vk)
	{
	case 65://a LEFT
		a_key = true;
		break;
	case 68://d RIGHT
		d_key = true;
		break;
	case 87://w FORWARD
		w_key = true;
		break;
	case 83://s BACKWARD
		s_key = true;
		break;
	default:
		break;
	}
}

//*************************************************************************
void OnKeyUp(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	switch (vk)
	{
	case 65://a
		a_key = false;
		break;
	case 68://d
		d_key = false;
		break;
	case 87://w
		w_key = false;
		break;
	case 83://s
		s_key = false;
		break;
	default:
		break;
	}
}

//=============================================================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{	/*
		#define HANDLE_MSG(hwnd, message, fn)    \
		case (message): return HANDLE_##message((hwnd), (wParam), (lParam), (fn))
		*/
		HANDLE_MSG(hwnd, WM_CHAR, OnChar);			// when a key is pressed and its a character
		HANDLE_MSG(hwnd, WM_LBUTTONDOWN, OnLBD);	// when pressing the left button
		HANDLE_MSG(hwnd, WM_LBUTTONUP, OnLBU);		// when releasing the left button
		HANDLE_MSG(hwnd, WM_MOUSEMOVE, OnMM);		// when moving the mouse inside your window
		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);		// called only once when the window is created
		//HANDLE_MSG(hwnd, WM_PAINT, OnPaint);		// drawing stuff
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);	// not used
		HANDLE_MSG(hwnd, WM_KEYDOWN, OnKeyDown);	// press a keyboard key
		HANDLE_MSG(hwnd, WM_KEYUP, OnKeyUp);		// release a keyboard key
		HANDLE_MSG(hwnd, WM_TIMER, OnTimer);		// timer
	case WM_PAINT:
		hdc = BeginPaint(hMain, &ps);
		EndPaint(hMain, &ps);
		break;
	case WM_ERASEBKGND:
		return (LRESULT)1;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			MessageBoxA(NULL, (char*)pErrorBlob->GetBufferPointer(), "Error", MB_OK);
		}
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}