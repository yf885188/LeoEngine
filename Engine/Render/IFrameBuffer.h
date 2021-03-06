/*! \file Engine\Render\IFrameBuffer.h
\ingroup Engine
\brief ��Ⱦ��������
*/
#ifndef LE_RENDER_IFrameBuffer_h
#define LE_RENDER_IFrameBuffer_h 1

#include "IRenderView.h"
#include "../Core/ViewPort.h"

#include <LBase/lmathtype.hpp>

#include <vector>
#include <memory>

namespace platform {
	namespace Render {

		class FrameBuffer {
		public:
			enum Attachment : uint8{
				Target0,
				Target1,
				Target2,
				Target3,
				Target4,
				Target5,
				Target6,
				Target7,
				DepthStencil,
			};

			enum ClearFlag : uint8 {
				Color = 1<<0,
				Depth = 1<<1,
				Stencil = 1<<2,
			};

			virtual ~FrameBuffer();

			virtual void OnBind();
			virtual void OnUnBind();

			leo::observer_ptr<GPUView> Attached(Attachment which) const;

			std::shared_ptr<GPUView> Attach(Attachment which, const std::shared_ptr<RenderTargetView>& view);
			std::shared_ptr<GPUView> Attach(Attachment which, const std::shared_ptr<DepthStencilView>& view);
			std::shared_ptr<GPUView> Attach(Attachment which, const std::shared_ptr<UnorderedAccessView>& view);

			void Detach(Attachment which);
			void DetachUAV(leo::uint8 which);

			virtual void Clear(leo::uint32 flags, const leo::math::float4  & clr, float depth, leo::int32 stencil) = 0;
		protected:
			std::vector<std::shared_ptr<RenderTargetView>> clr_views;
			std::shared_ptr<DepthStencilView> ds_view;
			std::vector<std::shared_ptr<UnorderedAccessView>> uav_views;

			ViewPort viewport;
		};
	}
}

#endif