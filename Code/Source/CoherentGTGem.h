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
		Coherent::UIGT::UISystem* m_UISystem;
		SimpleLogger m_LogHandler;

		int m_width;
		int m_height;
		int m_texid;


		ITexture* temp;
		ID3D11Texture2D* m_texture;
		ID3D11Texture2D* m_stencilTexture;
		ID3D11DepthStencilView* m_stencilView;

		std::unique_ptr<renoir::RendererBackend> m_Backend;

		Coherent::UIGT::UISystemRenderer* m_UISystemRenderer;

		Coherent::UIGT::View* m_CurrentView;
		Coherent::UIGT::ViewRenderer* m_viewRenderer;
    };
} // namespace CoherentGT
