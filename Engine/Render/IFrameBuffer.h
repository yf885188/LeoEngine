/*! \file Engine\Render\IFrameBuffer.h
\ingroup Engine
\brief ��Ⱦ��������
*/
#ifndef LE_RENDER_IFrameBuffer_h
#define LE_RENDER_IFrameBuffer_h 1

#include "IRenderView.h"
#include "../Core/ViewPort.h"
#include <vector>
#include <memory>

namespace platform {
	namespace Render {

		class FrameBuffer {
		public:
			virtual ~FrameBuffer();

			virtual void OnBind();
			virtual void OnUnBind();
		protected:
			std::vector<std::shared_ptr<RenderTargetView>> clr_views;
			std::shared_ptr<DepthStencilView> ds_view;
			std::vector<std::shared_ptr<UnorderedAccessView>> uav_views;

			ViewPort viewport;
		};
	}
}

#endif