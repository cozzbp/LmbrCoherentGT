#include "StdAfx.h"
#include <platform_impl.h>
#include "CoherentGTGem.h"
#include <d3d11.h>

#include "RenoirBackends/Common/ComHelpers.h"
#include <FlowSystem/Nodes/FlowBaseNode.h>

CoherentGT::CoherentGTGem::CoherentGTGem() { }
CoherentGT::CoherentGTGem::~CoherentGTGem() { }


typedef HRESULT(WINAPI *D3DCompileProc)(
	__in_bcount(SrcDataSize) LPCVOID pSrcData,
	__in SIZE_T SrcDataSize,
	__in_opt LPCSTR pSourceName,
	__in_xcount_opt(pDefines->Name != NULL) CONST D3D_SHADER_MACRO* pDefines,
	__in_opt ID3DInclude* pInclude,
	__in LPCSTR pEntrypoint,
	__in LPCSTR pTarget,
	__in UINT Flags1,
	__in UINT Flags2,
	__out ID3DBlob** ppCode,
	__out_opt ID3DBlob** ppErrorMsgs);

D3DCompileProc D3DCompileProcPtr = nullptr;
HMODULE D3DCompilerModule = nullptr;


void CoherentGT::CoherentGTGem::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
    using namespace CoherentGT;

    switch (event)
    {
    case ESYSTEM_EVENT_FLOW_SYSTEM_REGISTER_EXTERNAL_NODES:
        RegisterFlowNodes();
        break;

    case ESYSTEM_EVENT_GAME_POST_INIT:
        // Put your init code here
        // All other Gems will exist at this point
		Init();
        break;


    case ESYSTEM_EVENT_FULL_SHUTDOWN:
    case ESYSTEM_EVENT_FAST_SHUTDOWN:
        // Put your shutdown code here
        // Other Gems may have been shutdown already, but none will have destructed
        break;
    }
}

void CoherentGT::CoherentGTGem::Init()
{

	ID3D11Device* pDevice = static_cast<ID3D11Device*>(gEnv->pRenderer->GetRendDevice());

	//Init d3d stuff
	m_SwapChain = static_cast<IDXGISwapChain*>(gEnv->pRenderer->GetSwapChain());
	HRESULT hr;

	m_width = gEnv->pRenderer->GetWidth();
	m_height = gEnv->pRenderer->GetHeight();

	pDevice->GetImmediateContext(&m_ImmediateContext);

	CryLogAlways("INIT COHERENT GT GEM");
	Coherent::UIGT::SystemSettings settings;
	settings.DebuggerPort = 9999;
	m_UISystem = InitializeUIGTSystem(COHERENT_UI_GT_LICENSE, settings, Coherent::LoggingGT::Trace,
		&m_LogHandler);

	FileResourceHandler fileHandler;
	Coherent::UIGT::ViewInfo info;
	info.Width = m_width;
	info.Height = m_height;
	info.IsTransparent = true;
	info.ResourceHandlerInstance = &fileHandler;
	info.ViewListenerInstance = new SimpleViewListener();

	m_Backend.reset(new renoir::Dx11Backend(static_cast<ID3D11Device*>(gEnv->pRenderer->GetRendDevice()), true));

	string path;
	AZ::IO::FileIOBase* fileIO = AZ::IO::FileIOBase::GetInstance();
	if (fileIO)
	{
		const char* aliasPath = fileIO->GetAlias("@assets@");
		if (aliasPath && aliasPath[0] != '\0')
		{
			path += aliasPath;
		}
	}
	path += "\\Shaders";
	const char* dx11ShadersPath = path;
	static_cast<renoir::Dx11Backend*>(m_Backend.get())->InitializeStaticResources(dx11ShadersPath);

	uint32 flags = FT_DONT_STREAM | FT_USAGE_RENDERTARGET | FT_DONT_RELEASE | FT_DONT_RESIZE;
	m_crytex = gEnv->pRenderer->EF_CreateTexture(m_width, m_height, 1, 0, eTF_R8G8B8A8, flags);

	m_UISystemRenderer = m_UISystem->CreateRenderer(m_Backend.get());
	m_CurrentView = m_UISystem->CreateView(info, "http://www.liminalgames.com/ui/index.html");

	int sampleCount = 1;

	auto tex = gEnv->pRenderer->EF_GetTextureByID(m_crytex);
	auto rtv = static_cast<ID3D11RenderTargetView*>(tex->GetTextureRTV());

	m_stencilTexture = nullptr;
	m_stencilView = nullptr;

	D3D11_TEXTURE2D_DESC descDepth;
	std::memset(&descDepth, 0, sizeof(descDepth));
	descDepth.Width = m_width;
	descDepth.Height = m_height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = sampleCount;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D *texture;
	if (FAILED(pDevice->CreateTexture2D(&descDepth,
		nullptr,
		&texture)))
	{
		CryLogAlways("FAILED TO CREATE TEX");
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	std::memset(&descDSV, 0, sizeof(descDSV));
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	ID3D11DepthStencilView *dsv;
	if (FAILED(pDevice->CreateDepthStencilView(
		texture,
		&descDSV,
		&dsv)))
	{
		CryLogAlways("FAILED TO CREATE STENCIL VIEW");
	}

	Coherent::UIGT::NativeRenderTarget rt = { nullptr, nullptr };
	// Get a pointer to the native texture
	rt.Texture = (void*)rtv;
	rt.DepthStencilTexture = (void*)dsv;

	m_viewRenderer = m_UISystemRenderer->CreateViewRenderer(
		m_CurrentView,
		rt,
		m_width,
		m_height, sampleCount);

	gEnv->pGame->GetIGameFramework()->RegisterListener(this, "CoherentGT", FRAMEWORKLISTENERPRIORITY_HUD);

}

void CoherentGT::CoherentGTGem::OnPostUpdate(float fDeltaTime)
{
	m_UISystem->Advance();
	if (m_CurrentView)
		m_CurrentView->Layout();

	m_viewRenderer->Paint();
	////Drawing goes here

	gEnv->pRenderer->SetCullMode(R_CULL_DISABLE);
	gEnv->pRenderer->Set2DMode(true, m_width, m_height);
	gEnv->pRenderer->SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0, true);
	gEnv->pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
	gEnv->pRenderer->Draw2dImageStretchMode(true);
	gEnv->pRenderer->DrawImage(0, 0, m_width, m_height, m_crytex, 0, 1, 1, 0, 1, 1, 1, 1, false);

	gEnv->pRenderer->Draw2dImageStretchMode(false);
	gEnv->pRenderer->Set2DMode(false, 0, 0);
}

GEM_REGISTER(CoherentGT::CoherentGTGem)
