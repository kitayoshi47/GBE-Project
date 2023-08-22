#include "window/MainWindowClass.h"

#include <vector>
#include <d3d11.h>
#include <d3dx11.h>
#include <directxmath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")

#define SCREEN_WIDTH  160
#define SCREEN_HEIGHT 144
#define SCREEN_SIZE   3

#define WINDOW_TITLE    L"00_Test"
#define WINDOW_WIDTH    SCREEN_WIDTH * SCREEN_SIZE
#define WINDOW_HEIGHT   SCREEN_HEIGHT * SCREEN_SIZE
#define SAFE_RELEASE(x) if(x) x->Release();

// �v���g�^�C�v�錾
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam); // �E�B���h�E�v���V�[�W��
HRESULT OnInit(HWND hWnd);     // ������
HRESULT InitDevice(HWND hWnd); // �f�o�C�X�֘A������
VOID    InitView();            // �r���[�֘A������
HRESULT InitShader();          // �V�F�[�_�֘A������
HRESULT InitBuffer();          // �o�b�t�@�֘A������
VOID    OnUpdate();            // �X�V
VOID    OnRender();            // �`��
VOID    OnDestroy();           // ���������

// using�f�B���N�e�B�u
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// �p�C�v���C���I�u�W�F�N�g
ID3D11Device*           g_device;           // �f�o�C�X�C���^�[�t�F�C�X
ID3D11DeviceContext*    g_context;          // �R���e�L�X�g
IDXGISwapChain*         g_swapChain;        // �X���b�v�`�F�C���C���^�[�t�F�C�X
ID3D11RenderTargetView* g_renderTargetView; // �����_�[�^�[�Q�b�g�r���[
ID3D11InputLayout*      g_layout;           // �C���v�b�g���C�A�E�g
ID3D11VertexShader*     g_vertexShader;     // ���_�V�F�[�_
ID3D11PixelShader*      g_pixelShader;      // �s�N�Z���V�F�[�_
ID3D11Buffer*           g_vertexBuffer;     // ���_�o�b�t�@

// �E�C���h�E
MainWindowClass g_window;

// ���_�\����
struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT4 color;
};

// ��`���_�\����
struct QuadVertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Uv;
};

// �\���̈�̐��@(�A�X�y�N�g��)
FLOAT g_aspectRatio = static_cast<FLOAT>(WINDOW_WIDTH) / static_cast<FLOAT>(WINDOW_HEIGHT);

// ���C���֐�
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    // �E�B���h�E����
    g_window.Initialize(
        hInstance,
        WindowProc,
        WINDOW_TITLE,
        WINDOW_WIDTH,
        WINDOW_HEIGHT);

    MSG	msg;
    ZeroMemory(&msg, sizeof(msg));
    if (SUCCEEDED(OnInit(g_window.GetHandle()))) // DirectX�̏�����
    {
        // �E�B���h�E�̕\��
        ShowWindow(g_window.GetHandle(), SW_SHOW);

        // ���C�����[�v
        while (msg.message != WM_QUIT)
        {
            // �L���[���̃��b�Z�[�W������
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                OnUpdate(); // �X�V
                OnRender(); // �`��
            }
        }
    }

    // WM_QUIT���b�Z�[�W�̕�����Windows�ɕԂ�
    return static_cast<char>(msg.wParam);
}

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    switch (nMsg)
    {
    case WM_DESTROY:    // �I����
        OnDestroy();    // ���������
        PostQuitMessage(0);
        return 0;
    }

    // switch�����������Ȃ��������b�Z�[�W������
    return DefWindowProc(hWnd, nMsg, wParam, lParam);
}

// ������
HRESULT OnInit(HWND hWnd)
{
    // �f�o�C�X�֘A������
    if (FAILED(InitDevice(hWnd))) return E_FAIL;

    // �r���[�֘A������
    InitView();

    // �V�F�[�_�֘A������
    if (FAILED(InitShader())) return E_FAIL;

    // �o�b�t�@�֘A������
    if (FAILED(InitBuffer())) return E_FAIL;

    if (g_device)
    {
        HRESULT hr = S_OK;

        // 2�����e�N�X�`���̐ݒ�
        D3D11_TEXTURE2D_DESC texDesc;
        memset(&texDesc, 0, sizeof(texDesc));
        texDesc.Usage              = D3D11_USAGE_DEFAULT;
        texDesc.Format             = DXGI_FORMAT_R8G8B8A8_TYPELESS;
        texDesc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.Width              = 128;
        texDesc.Height             = 128;
        texDesc.CPUAccessFlags     = 0;
        texDesc.MipLevels          = 1;
        texDesc.ArraySize          = 1;
        texDesc.SampleDesc.Count   = 1;
        texDesc.SampleDesc.Quality = 0;

        // 2�����e�N�X�`���̐���
        ID3D11Texture2D* pRenderTex = NULL; // �����_�[�e�N�X�`��
        hr = g_device->CreateTexture2D(&texDesc, NULL, &pRenderTex);
        if (FAILED(hr)) {
            return E_FAIL;
        }

        // �����_�[�^�[�Q�b�g�r���[�̐ݒ�
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        memset(&rtvDesc, 0, sizeof(rtvDesc));
        rtvDesc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

        // �����_�[�^�[�Q�b�g�r���[�̐���
        ID3D11RenderTargetView* pRenderTargetView = NULL; // �����_�[�^�[�Q�b�g�r���[
        hr = g_device->CreateRenderTargetView(pRenderTex, &rtvDesc, &pRenderTargetView);
        if (FAILED(hr)) {
        	return false;
        }

	    // �T���v���X�e�[�g�̐ݒ�
	    D3D11_SAMPLER_DESC smpDesc;
	    memset( &smpDesc, 0, sizeof( smpDesc ) );
	    smpDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	    smpDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
	    smpDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
	    smpDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
	    smpDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	    smpDesc.MinLOD         = 0;
	    smpDesc.MaxLOD         = D3D11_FLOAT32_MAX;

	    // �T���v���X�e�[�g����
        ID3D11SamplerState* pSamplerState; // �T���v���[�X�e�[�g
	    hr = g_device->CreateSamplerState(&smpDesc, &pSamplerState);
	    if (FAILED(hr)) {
		    return false;
	    }

	    // ���_�f�[�^�̐ݒ�
	    QuadVertex vertices[] = {
		    { XMFLOAT3(-0.25f, -0.25f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		    { XMFLOAT3(-0.25f, -1.00f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		    { XMFLOAT3(-1.00f, -1.00f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

   		    { XMFLOAT3(-0.25f, -0.25f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		    { XMFLOAT3(-1.00f, -1.00f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		    { XMFLOAT3(-1.00f, -0.25f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
	    };

	    // �X�g���C�h�̐ݒ�
        uint32_t mQuadStride;
	    mQuadStride = sizeof(QuadVertex);

	    // ���_�o�b�t�@�̐ݒ�
	    D3D11_BUFFER_DESC bufDesc;
	    memset(&bufDesc, 0, sizeof(bufDesc));
	    bufDesc.Usage          = D3D11_USAGE_DEFAULT;
	    bufDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
	    bufDesc.CPUAccessFlags = 0;
	    bufDesc.ByteWidth      = sizeof(QuadVertex) * 6;

	    // �T�u���\�[�X�f�[�^�̐ݒ�
	    D3D11_SUBRESOURCE_DATA initData;
	    memset(&initData, 0, sizeof(initData));
	    initData.pSysMem = vertices;

	    // ���_�o�b�t�@����
        ID3D11Buffer* pQuadVertexBuffer; // �`��p���_�o�b�t�@
	    hr = g_device->CreateBuffer(&bufDesc, &initData, &pQuadVertexBuffer);
	    if (FAILED(hr)) {
		    return false;
	    }

        // ��n��
        SAFE_RELEASE(pRenderTex);
        SAFE_RELEASE(pRenderTargetView);
        SAFE_RELEASE(pSamplerState);
        SAFE_RELEASE(pQuadVertexBuffer);
    }

    return S_OK;
}

// �f�o�C�X�֘A������
HRESULT InitDevice(HWND hWnd)
{
    // �h���C�o�[��ʂ��`
    std::vector<D3D_DRIVER_TYPE> driverTypes
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
        D3D_DRIVER_TYPE_SOFTWARE,
    };

    // �X���b�v�`�F�C���̍쐬
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferDesc.Width = WINDOW_WIDTH;
    swapChainDesc.BufferDesc.Height = WINDOW_HEIGHT;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.Windowed = TRUE;

    // �h���C�o�[��ʂ��ォ�猟�؂��I��
    // �I�������f�o�C�X��p���ĕ`�悷��
    HRESULT hr = 0;
    for (size_t i = 0; i < driverTypes.size(); i++)
    {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            driverTypes[i],
            nullptr,
            0,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &swapChainDesc,
            &g_swapChain,
            &g_device,
            nullptr,
            &g_context
        );
        if (SUCCEEDED(hr)) break;
    }
    if (FAILED(hr))
    {
        MessageBox(NULL, L"DirectX11�ɑΉ����Ă��Ȃ��f�o�C�X�ł��B", g_window.GetTitle(), MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    return S_OK;
}

// �r���[�֘A������
VOID InitView()
{
    // �\���̈���쐬
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(viewport));
    viewport.Width = WINDOW_WIDTH;
    viewport.Height = WINDOW_HEIGHT;
    viewport.MinDepth = D3D11_MIN_DEPTH;    // 0.0f
    viewport.MaxDepth = D3D11_MAX_DEPTH;    // 1.0f
    g_context->RSSetViewports(1, &viewport);

    // �o�b�N�o�b�t�@���쐬
    ID3D11Texture2D* backBuffer;
    g_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    g_device->CreateRenderTargetView(backBuffer, nullptr, &g_renderTargetView);
    SAFE_RELEASE(backBuffer);

    // �����_�[�^�[�Q�b�g���o�b�N�o�b�t�@�ɐݒ�
    g_context->OMSetRenderTargets(1, &g_renderTargetView, nullptr);
}

// �V�F�[�_�֘A������
HRESULT InitShader()
{
    ID3DBlob* vertexShader, * pixelShader;

    // �����̃V�F�[�_�����[�h���R���p�C��
    if (FAILED(D3DX11CompileFromFile(L"resource/shader/shaders.hlsl", NULL, NULL, "vsMain", "vs_4_0", NULL, NULL, NULL, &vertexShader, NULL, NULL)))
    {
        MessageBox(NULL, L"���_�V�F�[�_��ǂݍ��߂܂���ł����B", g_window.GetTitle(), MB_OK | MB_ICONERROR);
        return E_FAIL;
    }
    if (FAILED(D3DX11CompileFromFile(L"resource/shader/shaders.hlsl", NULL, NULL, "psMain", "ps_4_0", NULL, NULL, NULL, &pixelShader, NULL, NULL)))
    {
        MessageBox(NULL, L"�s�N�Z���V�F�[�_��ǂݍ��߂܂���ł����B", g_window.GetTitle(), MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // �J�v�Z����
    g_device->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), NULL, &g_vertexShader);
    g_device->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), NULL, &g_pixelShader);

    // ���_�C���v�b�g���C�A�E�g���`
    D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    // �C���v�b�g���C�A�E�g�̃T�C�Y
    UINT numElements = sizeof(inputElementDescs) / sizeof(inputElementDescs[0]);

    // ���_�C���v�b�g���C�A�E�g���쐬
    if (FAILED(g_device->CreateInputLayout(inputElementDescs, numElements, vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), &g_layout)))
    {
        MessageBox(NULL, L"���_�C���v�b�g���C�A�E�g�̒�`���Ԉ���Ă��܂��B", g_window.GetTitle(), MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    SAFE_RELEASE(vertexShader);
    SAFE_RELEASE(pixelShader);

    // ���_�C���v�b�g���C�A�E�g���Z�b�g
    g_context->IASetInputLayout(g_layout);

    return S_OK;
}

// �o�b�t�@�֘A������
HRESULT InitBuffer()
{
    // �O�p�`�̃W�I���g�����`
    Vertex vertices[] =
    {
        { { 0.0f,   0.25f * g_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },    // ��-��
        { { 0.25f, -0.25f * g_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },    // �E-��
        { {-0.25f, -0.25f * g_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },    // ��-��
    };

    // �o�b�t�@���쐬
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));

    // ���_�o�b�t�@�̐ݒ�
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;                 // CPU�����GPU�ɂ�鏑�����݃A�N�Z�X
    bufferDesc.ByteWidth = sizeof(Vertex) * 3;              // �T�C�Y��Vertex�\���́~3
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;        // ���_�o�b�t�@���g�p
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;     // CPU���o�b�t�@�ɏ������ނ��Ƃ�����

    // ���_�o�b�t�@���쐬
    if (FAILED(g_device->CreateBuffer(&bufferDesc, nullptr, &g_vertexBuffer)))
    {
        MessageBox(NULL, L"���_�o�b�t�@���쐬�ł��܂���ł����B", g_window.GetTitle(), MB_OK | MB_ICONERROR);
        return E_FAIL;
    }

    // �\�����钸�_�o�b�t�@��I��
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    g_context->IASetVertexBuffers(0, 1, &g_vertexBuffer, &stride, &offset);

    // �g�p����v���~�e�B�u�^�C�v��ݒ�
    g_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // ���_�o�b�t�@�ɒ��_�f�[�^���R�s�[
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    g_context->Map(g_vertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedResource);   // �o�b�t�@�̃}�b�s���O
    memcpy(mappedResource.pData, vertices, sizeof(vertices));                               // �f�[�^���R�s�[
    g_context->Unmap(g_vertexBuffer, NULL);

    return S_OK;
}

// �X�V
VOID OnUpdate()
{
}

// �`��
VOID OnRender()
{
    // �����_�[�^�[�Q�b�g�r���[���w�肵���F�ŃN���A
    FLOAT clearColor[4] = { 0.0f, 0.5f, 0.5f, 1.0f };
    g_context->ClearRenderTargetView(g_renderTargetView, clearColor);

    // �V�F�[�_�I�u�W�F�N�g���Z�b�g
    g_context->VSSetShader(g_vertexShader, nullptr, 0);
    g_context->PSSetShader(g_pixelShader, nullptr, 0);

    // ���_�o�b�t�@���o�b�N�o�b�t�@�ɕ`��
    g_context->Draw(3, 0);

    // �t���[�����ŏI�o��
    g_swapChain->Present(0, 0); // �t���b�v
}

// ���������
VOID OnDestroy()
{
    SAFE_RELEASE(g_device);
    SAFE_RELEASE(g_context);
    SAFE_RELEASE(g_swapChain);
    SAFE_RELEASE(g_renderTargetView);
    SAFE_RELEASE(g_layout);
    SAFE_RELEASE(g_vertexShader);
    SAFE_RELEASE(g_pixelShader);
    SAFE_RELEASE(g_vertexBuffer);
}
