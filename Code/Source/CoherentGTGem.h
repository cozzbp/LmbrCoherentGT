#pragma once

#include <IGameFramework.h>

#include "CoherentGT/ICoherentGTGem.h"

#include <Coherent/UIGT/UISystem.h>
#include <Coherent/UIGT/View.h>
#include <Coherent/UIGT/ViewInfo.h>
#include <Coherent/UIGT/LicenseGT.h>



#include <Coherent/UIGT/UISystem.h>

#include <Coherent/UIGT/ResourceResponseUIGT.h>
#include <Coherent/UIGT/ResourceRequestUIGT.h>



#include <RenoirBackend/RendererBackend.h>

#include "Helpers/FileResourceHandler.h"
#include "Helpers/SimpleLogger.h"
#include "Helpers/SimpleViewListener.h"

#include "Backends/Dx11Backend.h"


const char SCREEN_QUAD_SHADER[] = ""
"														  \n"
"Texture2D txDiffuse : register(t0);					  \n"
"														  \n"
"SamplerState samLinear : register(s0);					  \n"
"														  \n"
"struct VSScreenQuadInput								  \n"
"{														  \n"
"	float4 Position : POSITION0;						  \n"
"	float2 TexCoords0 : TEXCOORD0;						  \n"
"};														  \n"
"														  \n"
"struct VSScreenQuadOutput								  \n"
"{														  \n"
"	float4 Position : SV_POSITION;						  \n"
"	float2 TexCoords0 : TEXCOORD0;						  \n"
"};														  \n"
"														  \n"
"VSScreenQuadOutput VS(VSScreenQuadInput input)			  \n"
"{														  \n"
"	VSScreenQuadOutput output = (VSScreenQuadOutput)0;	  \n"
"														  \n"
"	input.Position.xy = sign(input.Position.xy);		  \n"
"	output.Position = float4(input.Position.xy, 0, 1);	  \n"
"														  \n"
"	output.TexCoords0 = input.TexCoords0;				  \n"
"														  \n"
"	return output;										  \n"
"}														  \n"
"														  \n"
"float4 PS(VSScreenQuadOutput input) : SV_Target		  \n"
"{														  \n"
"	return txDiffuse.Sample(samLinear, input.TexCoords0); \n"
"}														  \n"
"														  \n";


namespace CoherentGT
{
	class CoherentGTGem : public ICoherentGTGem, public IGameFrameworkListener
    {
        GEM_IMPLEMENT_WITH_INTERFACE(CoherentGTGem, ICoherentGTGem, 0xf97cc6afbf8e470a, 0xbe8b6c966c6c6cca)

    public:
        void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;

		void OnPostUpdate(float fDeltaTime) override;

		void Init();
	private:

		int m_width;
		int m_height;
		int m_crytex;


		Coherent::UIGT::UISystem* m_UISystem;
		SimpleLogger m_LogHandler;

		ID3D11Texture2D* m_texture;
		ID3D11Texture2D* m_stencilTexture;
		ID3D11DepthStencilView* m_stencilView;

		std::unique_ptr<renoir::RendererBackend> m_Backend;

		Coherent::UIGT::UISystemRenderer* m_UISystemRenderer;

		Coherent::UIGT::View* m_CurrentView;
		Coherent::UIGT::ViewRenderer* m_viewRenderer;

		IDXGISwapChain* m_SwapChain;

		ID3D11RenderTargetView* m_BackBufferView;

		ID3D11Texture2D* m_DepthStencil;
		ID3D11DepthStencilView* m_BackDepthStencilView;

		ID3D11DeviceContext* m_ImmediateContext;
		ID3D11DepthStencilState* m_DefaultDepthStencilState;


		//Screen quad
		ID3D11VertexShader* m_VS;
		ID3D11PixelShader* m_PS;
		ID3D11InputLayout* m_InputLayout;
		ID3D11Buffer* m_VB;
		ID3D11Buffer* m_VBFlipped;
		ID3D11Buffer* m_IndexBuffer;
		ID3D11SamplerState* m_LinearSampler;

		ID3D11RenderTargetView* m_RTV;


    };
} // namespace CoherentGT
