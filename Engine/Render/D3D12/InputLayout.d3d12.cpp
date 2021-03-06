#include "InputLayout.hpp"
#include "Convert.h"
#include "GraphicsBuffer.hpp"
#include "Context.h"
namespace platform_ex::Windows::D3D12 {

	InputLayout::InputLayout() = default;

	const std::vector<D3D12_INPUT_ELEMENT_DESC>& InputLayout::GetInputDesc() const
	{
		if (vertex_elems.empty()) {
			std::vector<D3D12_INPUT_ELEMENT_DESC> elems;

			WORD input_slot = 0;
			for (auto &vertex_stream : vertex_streams) {
				auto stream_elems = Convert(vertex_stream);
				for (auto& elem : stream_elems)
					elem.InputSlot = input_slot;
				++input_slot;
				elems.insert(elems.end(), stream_elems.begin(), stream_elems.end());
			}

			vertex_elems.swap(elems);
		}
		return vertex_elems;
	}

	void InputLayout::Active() const
	{
		auto num_vertex_streams = static_cast<UINT>(vertex_streams.size());

		std::vector<D3D12_VERTEX_BUFFER_VIEW> vbvs;
		vbvs.resize(num_vertex_streams);

		for (auto i = 0; i != num_vertex_streams; ++i) {
			auto& vb = *static_cast<GraphicsBuffer*>(vertex_streams[i].stream.get());
			vbvs[i].BufferLocation = vb.Resource()->GetGPUVirtualAddress();
			vbvs[i].SizeInBytes = vb.GetSize();
			vbvs[i].StrideInBytes = vertex_streams[i].vertex_size;
		}

		auto & cmd_list = Context::Instance().GetCommandList(Device::Command_Render);
		if (num_vertex_streams)
			cmd_list->IASetVertexBuffers(0, num_vertex_streams, vbvs.data());
		if (GetNumIndices()) {
			D3D12_INDEX_BUFFER_VIEW ibv;
			auto& ib = *static_cast<GraphicsBuffer*>(index_stream.get());
			ibv.BufferLocation = ib.Resource()->GetGPUVirtualAddress();
			ibv.SizeInBytes = ib.GetSize();
			ibv.Format = Convert(index_format);
			cmd_list->IASetIndexBuffer(&ibv);
		}
	}
}