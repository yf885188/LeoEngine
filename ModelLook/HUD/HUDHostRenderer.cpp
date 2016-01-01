
#include "HUDHostRenderer.h"
#include "Widget.h"
#include "../UI/Blit.h"

LEO_BEGIN
HUD_BEGIN

HostRenderer::~HostRenderer()
{
}

void HostRenderer::SetSize(const Size & s)
{
	BufferedRenderer::SetSize(s);
	window->Resize({ static_cast<uint16>(s.GetWidth()),static_cast<uint16>(s.GetHeight())});
}

void HostRenderer::Render(std::pair<uint16, uint16> hostsize)
{
	if(!window->IsMined())
	{
		auto& wgt(widget.get());

 		CommitInvalidation(wgt.GetBox());
		//AdjustSize();

		auto b = false;
		{
			
			//context must have this local scope
			const auto g(GetContext());
			const auto r(GetInvalidatedArea());

			Drawing::ClearImage(*g);
			b = bool(Validate(wgt, wgt, { *g,{},r }));
		}
		if(b)
			window->Render(hostsize);
	}
}

void HostRenderer::InitWidgetView()
{
	//Converte widget to a WidgetCCV
	
	auto& pos(widget.get().GetLocationOf());

	window->RePos({static_cast<uint16>(pos.GetX()),static_cast<uint16>(pos.GetY())});
}

HUD_END
LEO_END


