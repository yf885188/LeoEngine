/*! \file Engine\Render\D3D12\Context.h
\ingroup Engine
\brief 绘制创建封装。
*/
#ifndef LE_RENDER_D3D12_Context_h
#define LE_RENDER_D3D12_Context_h 1

#include "../IContext.h"
#include "Texture.h"
#include "Adapter.h"
#include "Display.h"
#include <UniversalDXSDK/d3d12.h>

namespace platform_ex {
	namespace Windows {
		namespace D3D12 {
			class Device : platform::Render::Device {
			public:
				Device(DXGI::Adapter& adapter);


				D3D12_CPU_DESCRIPTOR_HANDLE AllocDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type);
				void DeallocDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, D3D12_CPU_DESCRIPTOR_HANDLE Handle);
				
				ID3D12Device* operator->() lnoexcept;


			public:
				friend class Context;
			
				//@{
				//\brief 使用者可以修改这些值满足特定需求
				lconstexpr static UINT const NUM_MAX_RENDER_TARGET_VIEWS = 1024+Display::NUM_BACK_BUFFERS;
				lconstexpr static UINT const NUM_MAX_DEPTH_STENCIL_VIEWS = 128;
				lconstexpr static UINT const NUM_MAX_CBV_SRV_UAVS = 4 * 1024;

				//@}
				D3D12_CPU_DESCRIPTOR_HANDLE null_srv_handle,null_uav_handle;

			private:
				void DeviceEx(ID3D12Device* device, ID3D12CommandQueue* cmd_queue, D3D_FEATURE_LEVEL feature_level);
			private:
				//@{
				//\brief base object for swapchain
				COMPtr<ID3D12Device> d3d_device;
				COMPtr<ID3D12CommandQueue> d3d_cmd_queue;
				//@}

				D3D_FEATURE_LEVEL d3d_feature_level;

				//@{
				//\brief object for create object
				enum CommandType {
					Command_Render = 0,
					//Command_Compute,
					//Command_Copy,
					CommandTypeCount
				};

				array<COMPtr<ID3D12CommandAllocator>, CommandTypeCount> d3d_cmd_allocators;
				//COMPtr<ID3D12CommandQueue> d3d_cmd_compute_quque;
				//COMPtr<ID3D12CommandQueue> d3d_cmd_copy_quque;

				array<COMPtr<ID3D12DescriptorHeap>, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> d3d_desc_heaps;
				array<UINT, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> d3d_desc_incres_sizes;
				//@{
				//\brief host'memory so large can enough to store many bool
				array<bool, NUM_MAX_RENDER_TARGET_VIEWS> rtv_heap_flag;
				array<bool, NUM_MAX_DEPTH_STENCIL_VIEWS> dsv_heap_flag;
				array<bool, NUM_MAX_CBV_SRV_UAVS> cbv_srv_uav_heap_flag;
				//@}

				//@}
			};

			class Context : public platform::Render::Context {
			public:
				Context();

				DXGI::Adapter& DefaultAdapter();

				DefGetter(const lnothrow, IDXGIFactory4*, DXGIFactory4, adapter_list.GetDXGIFactory4());
				DefGetter(, Device&, Device, *device);
				friend class Device;
			private:
				void ContextEx(ID3D12Device* device, ID3D12CommandQueue* cmd_queue);
				void CreateDeviceAndDisplay() override;
			private:

				DXGI::AdapterList adapter_list;

				shared_ptr<Device> device;
				shared_ptr<Display> display;

				array<COMPtr<ID3D12GraphicsCommandList>,Device::CommandTypeCount> d3d_cmd_lists;

			public:
				static Context& Instance();
			};
		}
	}
}

#endif