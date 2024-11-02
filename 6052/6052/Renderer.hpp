#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "Color.hpp"

#include <cstdint>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>
#include <Windows.h>

struct Vertex {
    float x, y, z;
    float u, v;
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    HRESULT InitD3D(HWND hwnd);
    void RenderFrame(const std::vector<uint8_t>& screen_state);
    void CleanD3D();

private:
    void DrawTexture();

    IDXGISwapChain* swapchain;
    ID3D11Device* dev;
    ID3D11DeviceContext* devcon;
    ID3D11RenderTargetView* backbuffer;
    ID3D11Texture2D* pFrameBufferTexture;
    ID3D11ShaderResourceView* pTextureView;
    ID3D11SamplerState* pSamplerLinear;
    ID3D11VertexShader* pVS;
    ID3D11PixelShader* pPS;
    ID3D11InputLayout* pLayout;
    ID3D11Buffer* pVertexBuffer;
    ID3D11Buffer* pIndexBuffer;
};

#endif
