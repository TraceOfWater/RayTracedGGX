//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "DXFrameworkHelper.h"
#include "XUSGRayTracingPipelineLayout.h"

using namespace std;
using namespace XUSG;
using namespace XUSG::RayTracing;

RayTracing::PipelineLayout::PipelineLayout() :
	Util::PipelineLayout()
{
}

RayTracing::PipelineLayout::~PipelineLayout()
{
}

XUSG::PipelineLayout RayTracing::PipelineLayout::CreatePipelineLayout(const Device &device,
	PipelineLayoutCache &pipelineLayoutCache, uint8_t flags, uint32_t numUAVs, const wchar_t *name)
{
	m_pipelineLayoutKey[0] = flags;
	const auto &key = m_pipelineLayoutKey;

	const auto numLayouts = static_cast<uint32_t>((key.size() - 1) / sizeof(void*));
	const auto pDescriptorTableLayoutPtrs = reinterpret_cast<DescriptorTableLayout::element_type* const*>(&key[1]);

	vector<D3D12_ROOT_PARAMETER1> descriptorTableLayouts(numLayouts);
	for (auto i = 0u; i < numLayouts; ++i)
		descriptorTableLayouts[i] = *pDescriptorTableLayoutPtrs[i];

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC layoutDesc;
	layoutDesc.Init_1_1(numLayouts, descriptorTableLayouts.data(), 0, nullptr, static_cast<D3D12_ROOT_SIGNATURE_FLAGS>(flags));

	XUSG::PipelineLayout layout;
	Blob signature, error;
	if (device.RaytracingAPI == RayTracing::API::FallbackLayer)
	{
		H_RETURN(device.Fallback->D3D12SerializeRootSignature(&layoutDesc.Desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1,
			&signature, &error, numUAVs), cerr, reinterpret_cast<wchar_t*>(error->GetBufferPointer()), nullptr);
		V_RETURN(device.Fallback->CreateRootSignature(1, signature->GetBufferPointer(), signature->GetBufferSize(),
			IID_PPV_ARGS(&layout)), cerr, nullptr);
	}
	else // DirectX Raytracing
	{
		H_RETURN(D3D12SerializeRootSignature(&layoutDesc.Desc_1_0, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error),
			cerr, reinterpret_cast<wchar_t*>(error->GetBufferPointer()), nullptr);
		V_RETURN(device.Native->CreateRootSignature(1, signature->GetBufferPointer(), signature->GetBufferSize(),
			IID_PPV_ARGS(&layout)), cerr, nullptr);
	}
	if (name) layout->SetName(name);

	return layout;
}

XUSG::PipelineLayout RayTracing::PipelineLayout::GetPipelineLayout(const Device &device,
	PipelineLayoutCache &pipelineLayoutCache, uint8_t flags, uint32_t numUAVs, const wchar_t *name)
{
	auto layout = pipelineLayoutCache.GetPipelineLayout(*this, flags, name, false);

	if (!layout)
	{
		layout = CreatePipelineLayout(device, pipelineLayoutCache, flags, numUAVs, name);
		pipelineLayoutCache.SetPipelineLayout(m_pipelineLayoutKey, layout);
	}

	return layout;
}
