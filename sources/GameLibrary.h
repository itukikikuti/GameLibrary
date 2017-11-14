// (c) 2017 Naoki Nakagawa
#pragma once
#include <string>
#include <windows.h>
#include <wrl.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define Main() WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#define PUBLIC public:
#define PRIVATE private:
#define PROTECTED protected:

namespace GameLibrary {
	class Game {
		PUBLIC Game() = delete;
		PUBLIC static HWND GetWindow() {
			static HWND window = nullptr;

			if (window == nullptr) {
				HINSTANCE instance = GetModuleHandleA(nullptr);

				WNDCLASSEXA windowClass = {};
				windowClass.cbSize = sizeof(WNDCLASSEXA);
				windowClass.style = CS_HREDRAW | CS_VREDRAW;
				windowClass.lpfnWndProc = ProcessWindow;
				windowClass.cbClsExtra = 0;
				windowClass.cbWndExtra = 0;
				windowClass.hInstance = instance;
				windowClass.hIcon = nullptr;
				windowClass.hCursor = nullptr;
				windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
				windowClass.lpszMenuName = nullptr;
				windowClass.lpszClassName = "GameLibrary";
				windowClass.hIconSm = nullptr;
				if (!RegisterClassExA(&windowClass)) return nullptr;

				window = CreateWindowA("GameLibrary", "GameLibrary", WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, nullptr, nullptr, instance, nullptr);

				SetSize(1280, 720);

				ShowWindow(window, SW_SHOWNORMAL);
			}

			return window;
		}
		PUBLIC static DirectX::XMINT2 GetSize() {
			return Size();
		}
		PUBLIC static void SetSize(int width, int height) {
			Size() = DirectX::XMINT2(width, height);

			RECT windowRect = {};
			RECT clientRect = {};
			GetWindowRect(GetWindow(), &windowRect);
			GetClientRect(GetWindow(), &clientRect);

			int w = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left) + width;
			int h = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top) + height;

			int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
			int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

			SetWindowPos(GetWindow(), nullptr, x, y, w, h, SWP_NOZORDER);
		}
		PUBLIC static char* GetTitle() {
			char* title = nullptr;
			GetWindowTextA(GetWindow(), title, GetWindowTextLengthA(GetWindow()));
			return title;
		}
		PUBLIC static void SetTitle(const char* title) {
			SetWindowTextA(GetWindow(), title);
		}
		PUBLIC static ID3D11Device& GetDevice() {
			static Microsoft::WRL::ComPtr<ID3D11Device> device = nullptr;

			if (device == nullptr) {
				int createDeviceFlag = 0;
#if defined(_DEBUG)
				createDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif

				D3D_DRIVER_TYPE driverTypes[] = {
					D3D_DRIVER_TYPE_HARDWARE,
					D3D_DRIVER_TYPE_WARP,
					D3D_DRIVER_TYPE_REFERENCE,
				};
				int driverTypeCount = sizeof(driverTypes) / sizeof(driverTypes[0]);

				D3D_FEATURE_LEVEL featureLevels[] = {
					D3D_FEATURE_LEVEL_11_0,
					D3D_FEATURE_LEVEL_10_1,
					D3D_FEATURE_LEVEL_10_0,
				};
				int featureLevelCount = sizeof(featureLevels) / sizeof(featureLevels[0]);

				for (int i = 0; i < driverTypeCount; i++) {
					HRESULT result = D3D11CreateDevice(nullptr, driverTypes[i], nullptr, createDeviceFlag, featureLevels, featureLevelCount, D3D11_SDK_VERSION, device.GetAddressOf(), nullptr, nullptr);

					if (SUCCEEDED(result)) {
						break;
					}
				}
			}

			return *device.Get();
		}
		PUBLIC static IDXGISwapChain& GetSwapChain() {
			static Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain = nullptr;

			if (swapChain == nullptr) {
				const int SWAP_CHAIN_COUNT = 2;
				const DXGI_FORMAT SWAP_CHAIN_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
				const int MULTI_SAMPLE_COUNT = 1;
				const int MULTI_SAMPLE_QUALITY = 0;

				IDXGIDevice1* dxgiDevice = nullptr;
				IDXGIAdapter* adapter = nullptr;
				IDXGIFactory* factory = nullptr;

				GetDevice().QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);
				dxgiDevice->GetAdapter(&adapter);
				adapter->GetParent(__uuidof(IDXGIFactory), (void**)&factory);

				DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
				swapChainDesc.BufferCount = SWAP_CHAIN_COUNT;
				swapChainDesc.BufferDesc.Width = GetSize().x;
				swapChainDesc.BufferDesc.Height = GetSize().y;
				swapChainDesc.BufferDesc.Format = SWAP_CHAIN_FORMAT;
				swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
				swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
				swapChainDesc.OutputWindow = GetWindow();
				swapChainDesc.SampleDesc.Count = MULTI_SAMPLE_COUNT;
				swapChainDesc.SampleDesc.Quality = MULTI_SAMPLE_QUALITY;
				swapChainDesc.Windowed = true;

				factory->CreateSwapChain(&GetDevice(), &swapChainDesc, swapChain.GetAddressOf());

				factory->Release();
				adapter->Release();
				dxgiDevice->Release();
			}

			return *swapChain.Get();
		}
		PUBLIC static ID3D11DeviceContext& GetDeviceContext() {
			static Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext = nullptr;
			static Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader = nullptr;
			static Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader = nullptr;
			static Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout = nullptr;

			if (deviceContext == nullptr) {
				GetDevice().GetImmediateContext(&deviceContext);

				D3D11_VIEWPORT viewPort = {};
				viewPort.Width = (float)GetSize().x;
				viewPort.Height = (float)GetSize().y;
				viewPort.MinDepth = 0.0f;
				viewPort.MaxDepth = 1.0f;
				viewPort.TopLeftX = 0;
				viewPort.TopLeftY = 0;
				deviceContext->RSSetViewports(1, &viewPort);

				ID3DBlob *vertexShaderBlob = nullptr;
				CompileShader(nullptr, "VS", "vs_4_0", &vertexShaderBlob);
				GetDevice().CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
				deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);

				ID3DBlob *pixelShaderBlob = nullptr;
				CompileShader(nullptr, "PS", "ps_4_0", &pixelShaderBlob);
				GetDevice().CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
				pixelShaderBlob->Release();
				deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);

				D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				};
				int inputElementDescCount = sizeof(inputElementDesc) / sizeof(inputElementDesc[0]);

				GetDevice().CreateInputLayout(inputElementDesc, inputElementDescCount, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), inputLayout.GetAddressOf());
				vertexShaderBlob->Release();
				deviceContext->IASetInputLayout(inputLayout.Get());

				ID3D11BlendState* blendState = nullptr;
				D3D11_BLEND_DESC blendDesc = {};
				blendDesc.AlphaToCoverageEnable = false;
				blendDesc.IndependentBlendEnable = false;
				blendDesc.RenderTarget[0].BlendEnable = true;
				blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
				blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
				blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
				blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
				blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
				blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
				blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

				float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
				GetDevice().CreateBlendState(&blendDesc, &blendState);
				deviceContext->OMSetBlendState(blendState, blendFactor, 0xffffffff);
				blendState->Release();
			}

			return *deviceContext.Get();
		}
		PUBLIC static ID3D11RenderTargetView& GetRenderTargetView() {
			static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView = nullptr;
			static Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTargetTexture = nullptr;
			static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> renderTargetShaderResourceView = nullptr;

			if (renderTargetView == nullptr) {
				GetSwapChain().GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)renderTargetTexture.GetAddressOf());

				GetDevice().CreateRenderTargetView(renderTargetTexture.Get(), nullptr, renderTargetView.GetAddressOf());

				GetDevice().CreateShaderResourceView(renderTargetTexture.Get(), nullptr, renderTargetShaderResourceView.GetAddressOf());

				GetDeviceContext().OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);
			}

			return *renderTargetView.Get();
		}
		PUBLIC static DirectX::XMINT2 GetMousePosition() {
			return MousePosition();
		}
		PUBLIC static bool GetKey(int VK_CODE) {
			return KeyState()[VK_CODE] & 0x80;
		}
		PUBLIC static bool GetKeyUp(int VK_CODE) {
			return !(KeyState()[VK_CODE] & 0x80) && (PreKeyState()[VK_CODE] & 0x80);
		}
		PUBLIC static bool GetKeyDown(int VK_CODE) {
			return (KeyState()[VK_CODE] & 0x80) && !(PreKeyState()[VK_CODE] & 0x80);
		}
		PUBLIC static float GetTime() {
			return Time();
		}
		PUBLIC static float GetDeltaTime() {
			return DeltaTime();
		}
		PUBLIC static void AddFont(const char* path) {
			AddFontResourceExA(path, FR_PRIVATE, nullptr);
		}
		PUBLIC static bool Loop() {
			static float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

			GetSwapChain().Present(0, 0);

			if (!ProcessResponse()) {
				return false;
			}

			ProcessSize();
			ProcessMousePosition();
			ProcessKey();
			PrecessDeltaTime();
			PrecessTime();

			GetDeviceContext().ClearRenderTargetView(&GetRenderTargetView(), color);

			return true;
		}
		PRIVATE static DirectX::XMINT2& Size() {
			static DirectX::XMINT2 size;
			return size;
		}
		PRIVATE static DirectX::XMINT2& MousePosition() {
			static DirectX::XMINT2 mousePosition;
			return mousePosition;
		}
		PRIVATE static BYTE* PreKeyState() {
			static BYTE preKeyState[256];
			return preKeyState;
		}
		PRIVATE static BYTE* KeyState() {
			static BYTE keyState[256];
			return keyState;
		}
		PRIVATE static float& Time() {
			static float time = 0.0f;
			return time;
		}
		PRIVATE static float& DeltaTime() {
			static float deltaTime = 0.0f;
			return deltaTime;
		}
		PRIVATE static void CompileShader(const char* path, const char* entryPoint, const char* shaderModel, ID3DBlob** out) {
			DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
			shaderFlags |= D3DCOMPILE_DEBUG;
#endif

			ID3DBlob *errorBlob = nullptr;

			if (path == nullptr) {
				char* shader = "\
cbuffer CB:register(b0){matrix W;matrix V;matrix P;float4 C;};\
Texture2D Tex:register(t0);SamplerState S:register(s0);\
struct VO{float4 pos:SV_POSITION;float4 c:COLOR;float2 uv:TEXCOORD;};\
VO VS(float4 v:POSITION,float2 uv:TEXCOORD){VO o=(VO)0;o.pos=mul(W,v);o.pos=mul(V,o.pos);o.pos=mul(P,o.pos);o.c=C;o.uv=uv;return o;}\
float4 PS(VO o):SV_TARGET{return Tex.Sample(S,o.uv)*o.c;}";

				D3DCompile(shader, strlen(shader), nullptr, nullptr, nullptr, entryPoint, shaderModel, shaderFlags, 0, out, &errorBlob);
			}
			else {
				size_t length = strlen(path) + 1;
				wchar_t* wstring = new wchar_t[length];
				mbstowcs_s(nullptr, wstring, length, path, _TRUNCATE);
				D3DCompileFromFile(wstring, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel, shaderFlags, 0, out, &errorBlob);
				delete wstring;
			}

			if (errorBlob != nullptr) {
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				MessageBoxA(GetWindow(), (char*)errorBlob->GetBufferPointer(), "Shader Error", MB_OK);
				errorBlob->Release();
			}
		}
		PRIVATE static void ProcessSize() {
			RECT clientRect = {};
			GetClientRect(GetWindow(), &clientRect);

			Size() = DirectX::XMINT2(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
		}
		PRIVATE static void ProcessMousePosition() {
			POINT point = {};
			GetCursorPos(&point);

			ScreenToClient(GetWindow(), &point);
			MousePosition() = DirectX::XMINT2(point.x, point.y);
		}
		PRIVATE static void ProcessKey() {
			for (int i = 0; i < 256; i++) {
				PreKeyState()[i] = KeyState()[i];
			}

			GetKeyboardState(KeyState());
		}
		PRIVATE static LARGE_INTEGER GetCounter() {
			LARGE_INTEGER counter;
			QueryPerformanceCounter(&counter);
			return counter;
		}
		PRIVATE static LARGE_INTEGER GetCountFrequency() {
			LARGE_INTEGER frequency;
			QueryPerformanceFrequency(&frequency);
			return frequency;
		}
		PRIVATE static void PrecessTime() {
			Time() += GetDeltaTime();
		}
		PRIVATE static void PrecessDeltaTime() {
			static LARGE_INTEGER preTime = GetCounter();
			static LARGE_INTEGER frequency = GetCountFrequency();

			LARGE_INTEGER time = GetCounter();
			DeltaTime() = (float)(time.QuadPart - preTime.QuadPart) / frequency.QuadPart;
			preTime = GetCounter();
		}
		PRIVATE static bool ProcessResponse() {
			static MSG response = {};

			while (response.message != WM_QUIT) {
				if (PeekMessageA(&response, nullptr, 0, 0, PM_REMOVE)) {
					TranslateMessage(&response);
					DispatchMessageA(&response);
				}
				else {
					return true;
				}
			}

			return false;
		}
		PRIVATE static LRESULT WINAPI ProcessWindow(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
			if (message == WM_DESTROY) {
				PostQuitMessage(0);
			}
			return DefWindowProcA(window, message, wParam, lParam);
		}
	};
}

// (c) 2017 Naoki Nakagawa
#pragma once
#include <wincodec.h>

namespace GameLibrary {
	class Sprite {
		PROTECTED struct Constant {
			DirectX::XMMATRIX world;
			DirectX::XMMATRIX view;
			DirectX::XMMATRIX projection;
			DirectX::XMFLOAT4 color;
		};
		PRIVATE struct Vertex {
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT2 uv;
		};

		PROTECTED DirectX::XMFLOAT2 position;
		PROTECTED float angle;
		PROTECTED DirectX::XMFLOAT2 scale;
		PROTECTED DirectX::XMFLOAT4 color;
		PROTECTED UINT width;
		PROTECTED UINT height;
		PROTECTED ID3D11Texture2D* texture;
		PROTECTED Constant constant;
		PRIVATE int indexCount;
		PRIVATE ID3D11Buffer* vertexBuffer;
		PRIVATE ID3D11Buffer* indexBuffer;
		PRIVATE ID3D11Buffer* constantBuffer;
		PRIVATE ID3D11ShaderResourceView* shaderResourceView;
		PRIVATE ID3D11SamplerState* samplerState;

		PUBLIC Sprite(const char* path) {
			Game::GetWindow();
			IWICImagingFactory* factory = nullptr;
			CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
			IWICBitmapDecoder* decoder = nullptr;
			size_t length = strlen(path) + 1;
			wchar_t* wstring = new wchar_t[length];
			mbstowcs_s(nullptr, wstring, length, path, _TRUNCATE);
			factory->CreateDecoderFromFilename(wstring, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
			delete wstring;
			IWICBitmapFrameDecode* frame = nullptr;
			decoder->GetFrame(0, &frame);
			frame->GetSize(&width, &height);
			WICPixelFormatGUID pixelFormat;
			frame->GetPixelFormat(&pixelFormat);
			BYTE* textureBuffer = new BYTE[width * height * 4];

			if (pixelFormat != GUID_WICPixelFormat32bppRGBA) {
				IWICFormatConverter* formatConverter = nullptr;
				factory->CreateFormatConverter(&formatConverter);

				formatConverter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);

				formatConverter->CopyPixels(0, width * 4, width * height * 4, textureBuffer);
			}
			else {
				frame->CopyPixels(0, width * 4, width * height * 4, textureBuffer);
			}

			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width = width;
			textureDesc.Height = height;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA textureSubresourceData;
			textureSubresourceData.pSysMem = textureBuffer;
			textureSubresourceData.SysMemPitch = width * 4;
			textureSubresourceData.SysMemSlicePitch = width * height * 4;
			Game::GetDevice().CreateTexture2D(&textureDesc, &textureSubresourceData, &texture);

			delete[] textureBuffer;

			Initialize();

			Position() = DirectX::XMFLOAT2(0.0f, 0.0f);
			Angle() = 0.0f;
			Scale() = DirectX::XMFLOAT2(1.0f, 1.0f);
			Color() = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		PUBLIC virtual ~Sprite() {
			if (texture)
				texture->Release();

			if (shaderResourceView)
				shaderResourceView->Release();

			if (samplerState)
				samplerState->Release();

			if (vertexBuffer)
				vertexBuffer->Release();

			if (indexBuffer)
				indexBuffer->Release();

			if (constantBuffer)
				constantBuffer->Release();
		}
		PUBLIC DirectX::XMINT2 GetSize() {
			return DirectX::XMINT2(width, height);
		}
		PUBLIC void SetScale(float scale) {
			Scale() = DirectX::XMFLOAT2(scale, scale);
		}
		PUBLIC DirectX::XMFLOAT2& Position() {
			return position;
		}
		PUBLIC float& Angle() {
			return angle;
		}
		PUBLIC DirectX::XMFLOAT2& Scale() {
			return scale;
		}
		PUBLIC DirectX::XMFLOAT4& Color() {
			return color;
		}
		PUBLIC void Draw() {
			constant.world = DirectX::XMMatrixIdentity();
			constant.world *= DirectX::XMMatrixScaling(width * Scale().x, height * Scale().y, 1.0f);
			constant.world *= DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(-Angle()));
			constant.world *= DirectX::XMMatrixTranslation(Position().x, -Position().y, 0.0f);
			constant.color = Color();

			Game::GetDeviceContext().UpdateSubresource(constantBuffer, 0, nullptr, &constant, 0, 0);

			Game::GetDeviceContext().VSSetConstantBuffers(0, 1, &constantBuffer);
			Game::GetDeviceContext().PSSetShaderResources(0, 1, &shaderResourceView);
			Game::GetDeviceContext().PSSetSamplers(0, 1, &samplerState);

			Game::GetDeviceContext().DrawIndexed(indexCount, 0, 0);
		}
		PROTECTED Sprite() {
		}
		PROTECTED void Initialize() {
			Vertex quad[] = {
				{ DirectX::XMFLOAT3(-0.5f, 0.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
				{ DirectX::XMFLOAT3(0.5f, 0.5f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
				{ DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
				{ DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
			};
			int vertexCount = sizeof(quad) / sizeof(quad[0]);

			int index[] = {
				0, 1, 2,
				3, 2, 1,
			};
			indexCount = sizeof(index) / sizeof(index[0]);

			constant.view = DirectX::XMMatrixLookAtLH(DirectX::XMVectorSet(Game::GetSize().x / 2.0f, -Game::GetSize().y / 2.0f, 0.0f, 0.0f), DirectX::XMVectorSet(Game::GetSize().x / 2.0f, -Game::GetSize().y / 2.0f, 1.0f, 0.0f), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			constant.projection = DirectX::XMMatrixOrthographicLH(Game::GetSize().x * 1.0f, Game::GetSize().y * 1.0f, -1.0f, 1.0f);

			D3D11_BUFFER_DESC vertexBufferDesc = {};
			vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertexCount;
			vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertexBufferDesc.CPUAccessFlags = 0;
			D3D11_SUBRESOURCE_DATA vertexSubresourceData = {};
			vertexSubresourceData.pSysMem = quad;
			Game::GetDevice().CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &vertexBuffer);

			UINT stride = sizeof(Vertex);
			UINT offset = 0;
			Game::GetDeviceContext().IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
			Game::GetDeviceContext().IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			D3D11_BUFFER_DESC indexBufferDesc = {};
			indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			indexBufferDesc.ByteWidth = sizeof(int) * indexCount;
			indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indexBufferDesc.CPUAccessFlags = 0;
			D3D11_SUBRESOURCE_DATA indexSubresourceData = {};
			indexSubresourceData.pSysMem = index;
			Game::GetDevice().CreateBuffer(&indexBufferDesc, &indexSubresourceData, &indexBuffer);

			Game::GetDeviceContext().IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			D3D11_BUFFER_DESC constantBufferDesc = {};
			constantBufferDesc.ByteWidth = sizeof(Constant);
			constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			constantBufferDesc.CPUAccessFlags = 0;
			Game::GetDevice().CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);

			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
			shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceViewDesc.Texture2D.MipLevels = 1;
			Game::GetDevice().CreateShaderResourceView(texture, &shaderResourceViewDesc, &shaderResourceView);

			D3D11_SAMPLER_DESC samplerDesc;
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			samplerDesc.BorderColor[0] = 0;
			samplerDesc.BorderColor[1] = 0;
			samplerDesc.BorderColor[2] = 0;
			samplerDesc.BorderColor[3] = 0;
			samplerDesc.MinLOD = 0;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
			Game::GetDevice().CreateSamplerState(&samplerDesc, &samplerState);
		}
	};
}

// (c) 2017 Naoki Nakagawa
#pragma once
#include <strsafe.h>

namespace GameLibrary {
	class Text : public Sprite {
		PUBLIC Text(const char* text, const char* fontFamily = "�l�r ����") {
			LOGFONTA logFont = {};
			logFont.lfHeight = 256;
			logFont.lfWidth = 0;
			logFont.lfEscapement = 0;
			logFont.lfOrientation = 0;
			logFont.lfWeight = 0;
			logFont.lfItalic = false;
			logFont.lfUnderline = false;
			logFont.lfStrikeOut = false;
			logFont.lfCharSet = SHIFTJIS_CHARSET;
			logFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;
			logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			logFont.lfQuality = PROOF_QUALITY;
			logFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
			StringCchCopyA(logFont.lfFaceName, 32, fontFamily);
			HFONT font = CreateFontIndirectA(&logFont);

			HDC dc = GetDC(nullptr);
			HFONT oldFont = (HFONT)SelectObject(dc, font);

			UINT code = 0;
			if (IsDBCSLeadByte(*text)) {
				code = (BYTE)text[0] << 8 | (BYTE)text[1];
			}
			else {
				code = text[0];
			}

			TEXTMETRICA textMetrics = {};
			GetTextMetricsA(dc, &textMetrics);
			GLYPHMETRICS glyphMetrics;
			const MAT2 matrix = { { 0, 1 },{ 0, 0 },{ 0, 0 },{ 0, 1 } };
			DWORD size = GetGlyphOutlineA(dc, code, GGO_GRAY4_BITMAP, &glyphMetrics, 0, nullptr, &matrix);
			BYTE* ptr = new BYTE[size];
			GetGlyphOutlineA(dc, code, GGO_GRAY4_BITMAP, &glyphMetrics, size, ptr, &matrix);

			SelectObject(dc, oldFont);
			DeleteObject(font);
			ReleaseDC(nullptr, dc);

			width = glyphMetrics.gmCellIncX;
			height = textMetrics.tmHeight;

			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width = width;
			textureDesc.Height = height;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DYNAMIC;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			textureDesc.MiscFlags = 0;

			Game::GetDevice().CreateTexture2D(&textureDesc, nullptr, &texture);

			D3D11_MAPPED_SUBRESOURCE mapped;
			Game::GetDeviceContext().Map(texture, D3D11CalcSubresource(0, 0, 1), D3D11_MAP_WRITE_DISCARD, 0, &mapped);

			BYTE* bits = (BYTE*)mapped.pData;
			DirectX::XMINT2 origin;
			origin.x = glyphMetrics.gmptGlyphOrigin.x;
			origin.y = textMetrics.tmAscent - glyphMetrics.gmptGlyphOrigin.y;
			DirectX::XMINT2 bitmapSize;
			bitmapSize.x = glyphMetrics.gmBlackBoxX + (4 - (glyphMetrics.gmBlackBoxX % 4)) % 4;
			bitmapSize.y = glyphMetrics.gmBlackBoxY;
			const int LEVEL = 17;
			memset(bits, 0, mapped.RowPitch * textMetrics.tmHeight);

			for (int y = origin.y; y < origin.y + bitmapSize.y; y++) {
				for (int x = origin.x; x < origin.x + bitmapSize.x; x++) {
					DWORD alpha = (255 * ptr[x - origin.x + bitmapSize.x * (y - origin.y)]) / (LEVEL - 1);
					DWORD color = 0x00ffffff | (alpha << 24);
					memcpy((BYTE*)bits + mapped.RowPitch * y + 4 * x, &color, sizeof(DWORD));
				}
			}

			Game::GetDeviceContext().Unmap(texture, D3D11CalcSubresource(0, 0, 1));
			delete[] ptr;

			Initialize();

			Position() = DirectX::XMFLOAT2(0.0f, 0.0f);
			Angle() = 0.0f;
			Scale() = DirectX::XMFLOAT2(1.0f, 1.0f);
			Color() = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		}
	};
}
