#include "Mesh.h"
#include "../Render/IContext.h"
#include "AssetResourceScheduler.h"
#include "../Asset/MeshX.h"

namespace platform {
	using namespace Render;

	Mesh::Mesh(const asset::MeshAsset & asset, const std::string & _name)
		:name(_name), sub_meshes(asset.GetSubMeshDesces())
	{
		auto& device = Context::Instance().GetDevice();
		input_layout = unique_raw(device.CreateInputLayout());

		for (std::size_t i = 0; i != asset.GetVertexElements().size(); ++i) {
			auto& element = asset.GetVertexElements()[i];
			auto& stream = asset.GetVertexStreams()[i];
			auto vertex_stream = leo::share_raw(
				device.CreateVertexBuffer(
					Buffer::Usage::Static,
					EAccessHint::EA_GPURead | EAccessHint::EA_Immutable,
					element.GetElementSize()*asset.GetVertexCount(),
					element.format, stream.get()));
			input_layout->BindVertexStream(vertex_stream, { element });
		}

		auto index_stream = leo::share_raw(
			device.CreateIndexBuffer(
				Buffer::Usage::Static, EAccessHint::EA_GPURead | EAccessHint::EA_Immutable,
				NumFormatBytes(asset.GetIndexFormat())*asset.GetIndexCount(),
				asset.GetIndexFormat(), asset.GetIndexStreams().get()));
		input_layout->BindIndexStream(index_stream, asset.GetIndexFormat());

		//Topo
		input_layout->SetTopoType(InputLayout::TriangleList);
	}
	const asset::MeshAsset::SubMeshDescrption::LodDescription & Mesh::GetSubMeshCurretnLodDescription(int submesh_index)
	{
		return GetSubMeshLodDescription(submesh_index, mesh_lod);
	}
	const asset::MeshAsset::SubMeshDescrption::LodDescription & Mesh::GetSubMeshLodDescription(int submesh_index, int lod_index)
	{
		return sub_meshes[submesh_index].LodsDescription[mesh_lod];
	}
	leo::uint8 Mesh::GetSubMeshMaterialIndex(int submesh_index)
	{
		return sub_meshes[submesh_index].MaterialIndex;
	}

	const std::string& Mesh::GetName() const lnothrow {
		return name;
	}


	MeshesHolder::MeshesHolder()
		:loaded_meshes(pool_resource.upstream_resource())
	{
	}
	platform::MeshesHolder::~MeshesHolder()
	{
	}
	std::shared_ptr<void> platform::MeshesHolder::FindResource(const leo::any & key)
	{
		if (auto ptuple = leo::any_cast<std::tuple<std::shared_ptr<asset::MeshAsset>, std::string>>(&key))
			return FindResource(std::get<0>(*ptuple), std::get<1>(*ptuple));
		return {};
	}
	std::shared_ptr<Mesh> platform::MeshesHolder::FindResource(const std::shared_ptr<asset::MeshAsset>& asset, const std::string & name)
	{
		for (auto& pair : loaded_meshes)
		{
			if (auto sp = pair.first.lock())
				if (sp == asset && pair.second->GetName() == name)
					return pair.second;
		}
		return {};
	}
	void platform::MeshesHolder::Connect(const std::shared_ptr<asset::MeshAsset>& asset, const std::shared_ptr<Mesh>& mesh)
	{
		auto use_count = mesh.use_count();
		auto insert_itr = std::find_if(loaded_meshes.begin(), loaded_meshes.end(),[&](auto& pair) { return pair.second.use_count() == use_count; });
		loaded_meshes.emplace(insert_itr, asset, mesh);
		auto erase_count = 0;
		auto itr = loaded_meshes.rbegin();
		while (itr != loaded_meshes.rend() && itr->second.use_count() == 1)
			++erase_count,++itr;
		loaded_meshes.resize(loaded_meshes.size() - erase_count);
	}
	MeshesHolder & platform::MeshesHolder::Instance()
	{
		static MeshesHolder instance;
		return instance;
	}

	template<>
	std::shared_ptr<Mesh> AssetResourceScheduler::SyncSpawnResource<Mesh,const X::path&,const std::string&>(const X::path& path, const std::string & name) {
		auto pAsset = X::LoadMeshAsset(path);
		if (!pAsset)
			return {};
		if (auto pMesh = MeshesHolder::Instance().FindResource(pAsset,name))
			return pMesh;
		auto pMesh = std::make_shared<Mesh>(*pAsset,name);
		MeshesHolder::Instance().Connect(pAsset, pMesh);
		return pMesh;
	}

	template std::shared_ptr<Mesh> AssetResourceScheduler::SyncSpawnResource<Mesh, const X::path&, const std::string&>(const X::path& path, const std::string & name);
}
