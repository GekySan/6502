#include "Renderer.hpp"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

Renderer::Renderer()
    : swapchain(nullptr), dev(nullptr), devcon(nullptr), backbuffer(nullptr),
    pFrameBufferTexture(nullptr), pTextureView(nullptr), pSamplerLinear(nullptr),
    pVS(nullptr), pPS(nullptr), pLayout(nullptr), pVertexBuffer(nullptr), pIndexBuffer(nullptr)
{
}

Renderer::~Renderer()
{
    CleanD3D();
}

HRESULT Renderer::InitD3D(HWND hwnd)
{
    DXGI_SWAP_CHAIN_DESC scd = {};

    scd.BufferCount = 1;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.Width = 640;
    scd.BufferDesc.Height = 640;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        0,
        NULL,
        0,
        D3D11_SDK_VERSION,
        &scd,
        &swapchain,
        &dev,
        NULL,
        &devcon
    );
    if (FAILED(hr))
        return hr;

    ID3D11Texture2D* pBackBuffer;
    hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
        return hr;

    hr = dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
    pBackBuffer->Release();
    if (FAILED(hr))
        return hr;

    devcon->OMSetRenderTargets(1, &backbuffer, NULL);

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = 640;
    viewport.Height = 640;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    devcon->RSSetViewports(1, &viewport);

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = 32;
    desc.Height = 32;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = dev->CreateTexture2D(&desc, NULL, &pFrameBufferTexture);
    if (FAILED(hr))
        return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = desc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = dev->CreateShaderResourceView(pFrameBufferTexture, &srvDesc, &pTextureView);
    if (FAILED(hr))
        return hr;

    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = dev->CreateSamplerState(&sampDesc, &pSamplerLinear);
    if (FAILED(hr))
        return hr;

    ID3DBlob* pVSBlob = nullptr;
    ID3DBlob* pPSBlob = nullptr;
    ID3DBlob* pErrorBlob = nullptr;

    const char* szVertexShader =
        "struct VS_INPUT { float3 Pos : POSITION; float2 Tex : TEXCOORD0; };"
        "struct PS_INPUT { float4 Pos : SV_POSITION; float2 Tex : TEXCOORD0; };"
        "PS_INPUT VS(VS_INPUT input) {"
        "   PS_INPUT output;"
        "   output.Pos = float4(input.Pos, 1.0f);"
        "   output.Tex = input.Tex;"
        "   return output;"
        "}";

    const char* szPixelShader =
        "Texture2D shaderTexture : register(t0);"
        "SamplerState SampleType : register(s0);"
        "struct PS_INPUT { float4 Pos : SV_POSITION; float2 Tex : TEXCOORD0; };"
        "float4 PS(PS_INPUT input) : SV_Target {"
        "   return shaderTexture.Sample(SampleType, input.Tex);"
        "}";

    hr = D3DCompile(szVertexShader, strlen(szVertexShader), NULL, NULL, NULL, "VS", "vs_4_0", 0, 0, &pVSBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
            pErrorBlob->Release();
        }
        return hr;
    }

    hr = D3DCompile(szPixelShader, strlen(szPixelShader), NULL, NULL, NULL, "PS", "ps_4_0", 0, 0, &pPSBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
            pErrorBlob->Release();
        }
        return hr;
    }

    hr = dev->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &pVS);
    if (FAILED(hr))
    {
        pVSBlob->Release();
        return hr;
    }

    hr = dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &pPS);
    if (FAILED(hr))
    {
        pPSBlob->Release();
        return hr;
    }

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                             D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = dev->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &pLayout);
    pVSBlob->Release();
    pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    devcon->IASetInputLayout(pLayout);

    Vertex vertices[] =
    {
        { -1.0f, 1.0f, 0.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 0.0f, 1.0f, 0.0f },
        { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f },
        { 1.0f, -1.0f, 0.0f, 1.0f, 1.0f }
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = vertices;

    hr = dev->CreateBuffer(&bd, &InitData, &pVertexBuffer);
    if (FAILED(hr))
        return hr;

    DWORD indices[] =
    {
        0,1,2,
        2,1,3
    };

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(indices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;

    hr = dev->CreateBuffer(&bd, &InitData, &pIndexBuffer);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

void Renderer::RenderFrame(const std::vector<uint8_t>& screen_state)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = devcon->Map(pFrameBufferTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr))
    {
        uint8_t* pTexels = (uint8_t*)mappedResource.pData;

        for (int y = 0; y < 32; y++)
        {
            for (int x = 0; x < 32; x++)
            {
                int index = (y * 32 + x) * 3;
                int texelIndex = y * mappedResource.RowPitch + x * 4;

                pTexels[texelIndex] = screen_state[index];
                pTexels[texelIndex + 1] = screen_state[index + 1];
                pTexels[texelIndex + 2] = screen_state[index + 2];
                pTexels[texelIndex + 3] = 255;
            }
        }

        devcon->Unmap(pFrameBufferTexture, 0);
    }

    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    devcon->ClearRenderTargetView(backbuffer, clearColor);

    DrawTexture();

    swapchain->Present(1, 0);
}

void Renderer::DrawTexture()
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    devcon->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
    devcon->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    devcon->VSSetShader(pVS, 0, 0);
    devcon->PSSetShader(pPS, 0, 0);

    devcon->PSSetShaderResources(0, 1, &pTextureView);
    devcon->PSSetSamplers(0, 1, &pSamplerLinear);

    devcon->DrawIndexed(6, 0, 0);
}

void Renderer::CleanD3D()
{
    if (pSamplerLinear) { pSamplerLinear->Release(); pSamplerLinear = nullptr; }
    if (pTextureView) { pTextureView->Release(); pTextureView = nullptr; }
    if (pFrameBufferTexture) { pFrameBufferTexture->Release(); pFrameBufferTexture = nullptr; }
    if (pVS) { pVS->Release(); pVS = nullptr; }
    if (pPS) { pPS->Release(); pPS = nullptr; }
    if (pLayout) { pLayout->Release(); pLayout = nullptr; }
    if (pVertexBuffer) { pVertexBuffer->Release(); pVertexBuffer = nullptr; }
    if (pIndexBuffer) { pIndexBuffer->Release(); pIndexBuffer = nullptr; }
    if (backbuffer) { backbuffer->Release(); backbuffer = nullptr; }
    if (swapchain) { swapchain->Release(); swapchain = nullptr; }
    if (devcon) { devcon->Release(); devcon = nullptr; }
    if (dev) { dev->Release(); dev = nullptr; }
}
