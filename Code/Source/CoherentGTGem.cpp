#include "StdAfx.h"
#include <platform_impl.h>
#include "CoherentGTGem.h"
#include <FlowSystem/Nodes/FlowBaseNode.h>

CoherentGT::CoherentGTGem::CoherentGTGem() { }
CoherentGT::CoherentGTGem::~CoherentGTGem() { }

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

	temp = gEnv->pRenderer->EF_LoadTexture("EngineAssets/Textures/startscreen.tif", FT_DONT_STREAM | FT_DONT_RESIZE | FT_NOMIPS | FT_USAGE_ALLOWREADSRGB);
	CryLogAlways("INIT COHERENT GT GEM");
	Coherent::UIGT::SystemSettings settings;
	settings.DebuggerPort = 9999;
	m_UISystem = InitializeUIGTSystem(COHERENT_UI_GT_LICENSE, settings, Coherent::LoggingGT::Trace,
		&m_LogHandler);


	m_width = gEnv->pRenderer->GetWidth();
	m_height = gEnv->pRenderer->GetHeight();

	FileResourceHandler fileHandler;
	Coherent::UIGT::ViewInfo info;
	info.Width = m_width;
	info.Height = m_height;
	info.IsTransparent = true;
	info.ResourceHandlerInstance = &fileHandler;
	info.ViewListenerInstance = new SimpleViewListener();

	m_Backend.reset(new renoir::Dx11Backend(static_cast<ID3D11Device*>(gEnv->pRenderer->GetRendDevice()), true));

	char rootpath[_MAX_PATH];
	CryGetCurrentDirectory(sizeof(rootpath), rootpath);

	string path = rootpath;
	path += "\\";
	path += gEnv->pConsole->GetCVar("sys_game_folder")->GetString();
	path += "\\Shaders";

	//This is what we'll use as soon as we get Gem Asset caching working
	/*AZ::IO::FileIOBase* fileIO = AZ::IO::FileIOBase::GetInstance();
	if (fileIO)
	{
		const char* aliasPath = fileIO->GetAlias("@assets@");
		if (aliasPath && aliasPath[0] != '\0')
		{
			path += aliasPath;
		}
	}*/

	const char* dx11ShadersPath = path;
	static_cast<renoir::Dx11Backend*>(m_Backend.get())->InitializeStaticResources(dx11ShadersPath);

	m_UISystemRenderer = m_UISystem->CreateRenderer(m_Backend.get());
	m_CurrentView = m_UISystem->CreateView(info, "http://www.liminalgames.com/ui/index.html");

	int sampleCount = 1;

	uint32 flags = FT_DONT_STREAM | FT_USAGE_RENDERTARGET | FT_DONT_RELEASE | FT_DONT_RESIZE;
	m_texid = gEnv->pRenderer->EF_CreateTexture(m_width, m_height, 1, 0, eTF_R8G8B8A8, flags);
	auto tex = gEnv->pRenderer->EF_GetTextureByID(m_texid);
	m_texture = static_cast<ID3D11Texture2D*>(tex->GetDeviceTexture());

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

	ID3D11Device* pDevice = static_cast<ID3D11Device*>(gEnv->pRenderer->GetRendDevice());

	ID3D11Texture2D *texture;
	HRESULT hr = pDevice->CreateTexture2D(&descDepth, nullptr, &texture);
	CRY_ASSERT(SUCCEEDED(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	std::memset(&descDSV, 0, sizeof(descDSV));
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = (sampleCount == 1) ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
	descDSV.Texture2D.MipSlice = 0;

	ID3D11DepthStencilView *dsv;
	hr = pDevice->CreateDepthStencilView(texture, &descDSV, &dsv);
	CRY_ASSERT(SUCCEEDED(hr));

	m_stencilTexture = texture;
	m_stencilView = dsv;

	Coherent::UIGT::NativeRenderTarget rt = { nullptr, nullptr };
	// Get a pointer to the native texture
	rt.Texture = m_texture;
	rt.DepthStencilTexture = m_stencilTexture;

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

	//gEnv->pRenderer->SetCullMode(R_CULL_DISABLE);
	//gEnv->pRenderer->Set2DMode(true, m_width, m_height);
	//gEnv->pRenderer->SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0, true);
	//gEnv->pRenderer->SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
	//gEnv->pRenderer->Draw2dImageStretchMode(true);
	//gEnv->pRenderer->DrawImage(0, 0, m_width, m_height, temp->GetTextureID(), 0, 1, 1, 0, 1, 1, 1, 1, false);

	//gEnv->pRenderer->Draw2dImageStretchMode(false);
	//gEnv->pRenderer->Set2DMode(false, 0, 0);
}

GEM_REGISTER(CoherentGT::CoherentGTGem)
