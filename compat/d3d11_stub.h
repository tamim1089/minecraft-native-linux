// compat/d3d11_stub.h — minimal opaque D3D11 types so 4J_Render.h parses on Linux.
// We never call real D3D; the render backend is stubbed at link time (see render_stub.cpp).
// Real values used for the blend/comparison enums so any state logic stays sane.
#pragma once
#ifndef PORT_D3D11_STUB_H
#define PORT_D3D11_STUB_H

// Opaque COM-ish interfaces — only ever used as pointers in our compiled code.
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11SamplerState;
struct ID3D11BlendState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11InputLayout;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct IDXGISwapChain;
struct IDXGIFactory;

typedef struct _D3D11_RECT { long left, top, right, bottom; } D3D11_RECT;
typedef struct _D3D11_VIEWPORT { float TopLeftX, TopLeftY, Width, Height, MinDepth, MaxDepth; } D3D11_VIEWPORT;

// D3D11_BLEND — real DXGI enum values.
enum D3D11_BLEND {
    D3D11_BLEND_ZERO = 1,
    D3D11_BLEND_ONE = 2,
    D3D11_BLEND_SRC_COLOR = 3,
    D3D11_BLEND_INV_SRC_COLOR = 4,
    D3D11_BLEND_SRC_ALPHA = 5,
    D3D11_BLEND_INV_SRC_ALPHA = 6,
    D3D11_BLEND_DEST_ALPHA = 7,
    D3D11_BLEND_INV_DEST_ALPHA = 8,
    D3D11_BLEND_DEST_COLOR = 9,
    D3D11_BLEND_INV_DEST_COLOR = 10,
    D3D11_BLEND_SRC_ALPHA_SAT = 11,
    D3D11_BLEND_BLEND_FACTOR = 14,
    D3D11_BLEND_INV_BLEND_FACTOR = 15,
};

// D3D11_COMPARISON_FUNC — real values.
enum D3D11_COMPARISON_FUNC {
    D3D11_COMPARISON_NEVER = 1,
    D3D11_COMPARISON_LESS = 2,
    D3D11_COMPARISON_EQUAL = 3,
    D3D11_COMPARISON_LESS_EQUAL = 4,
    D3D11_COMPARISON_GREATER = 5,
    D3D11_COMPARISON_NOT_EQUAL = 6,
    D3D11_COMPARISON_GREATER_EQUAL = 7,
    D3D11_COMPARISON_ALWAYS = 8,
};

#endif // PORT_D3D11_STUB_H
