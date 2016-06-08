#pragma once

#include <Coherent/UIGT/ViewListener.h>

class SimpleViewListener : public Coherent::UIGT::ViewListener
{
	virtual void OnViewCreated(Coherent::UIGT::View* view) COHERENT_OVERRIDE
	{
		m_View = view;
	}
private:
	Coherent::UIGT::View* m_View;
};