#include "IFrameBuffer.h"

namespace platform::Render {

	FrameBuffer::~FrameBuffer() = default;

	void FrameBuffer::OnBind()
	{
	}

	void FrameBuffer::OnUnBind()
	{
	}

	leo::observer_ptr<GPUView> FrameBuffer::Attached(Attachment which) const
	{
		switch (which)	
		{
		case platform::Render::FrameBuffer::DepthStencil:
			return leo::make_observer(ds_view.get());
		default:
			leo::uint32 clr_index = which - Target0;
			if (clr_index < clr_views.size())
				return leo::make_observer(clr_views[which].get());
			else
				return {};
			break;
		}
		throw std::invalid_argument("don't support enum argument");
	}

	std::shared_ptr<GPUView> FrameBuffer::Attach(Attachment which, const std::shared_ptr<RenderTargetView>& view)
	{
		switch (which) {
		case DepthStencil:
			throw std::invalid_argument("can' bind rtv to DepthStencil slot");
		default:
			//TODO Check max_simultaneous_rts support
			leo::uint32 clr_index = which - Target0;
			if ((clr_index < clr_views.size()) && clr_views[clr_index])
				this->Detach(which);
			if (clr_views.size() < clr_index + 1)
				clr_views.resize(clr_index + 1);
			clr_views[clr_index] = view;

			//find viewport
			auto min_clr_index = clr_index;
			for (auto i = 0; i != clr_index; ++i)
				if (clr_views[i])
					min_clr_index = i;
			if (min_clr_index == clr_index)
			{
				viewport.left = 0;
				viewport.top = 0;
				viewport.width = view->Width();
				viewport.height = view->Height();
			}
			break;
		}
		return view;
	}

	std::shared_ptr<GPUView> FrameBuffer::Attach(Attachment which, const std::shared_ptr<DepthStencilView>& view) {
		switch (which)
		{
		case DepthStencil:
			if (ds_view)
				this->Detach(which);
			ds_view = view;
			break;
		default:
			throw std::invalid_argument("dsv restrict bind on DepthStencil slot");
		}
		return view;
	}

	std::shared_ptr<GPUView> FrameBuffer::Attach(Attachment which, const std::shared_ptr<UnorderedAccessView>& view) {
		leo::uint8 index = which;
		//TODO Check max_simultaneous_uavs support
		if ((index < uav_views.size()) && uav_views[index])
			this->DetachUAV(index);
		if (uav_views.size() < index + 1)
			uav_views.resize(index + 1);
		uav_views[index] = view;
		return view;
	}

	void FrameBuffer::Detach(Attachment which)
	{
		switch (which) {
		case DepthStencil:
			ds_view.reset();
			break;
		default:
			leo::uint32 clr_index = which - Target0;
			if ((clr_index < clr_views.size()) && clr_views[clr_index])
				clr_views[clr_index].reset();
			break;
		}
	}

	void FrameBuffer::DetachUAV(leo::uint8 which)
	{
		leo::uint8 index = which;
		if ((index < uav_views.size()) && uav_views[index])
			uav_views[index].reset();
	}
}


