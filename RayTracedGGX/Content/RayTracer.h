//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include "Core/XUSG.h"
#include "RayTracing/XUSGRayTracing.h"

class RayTracer
{
public:
	enum MeshIndex : uint8_t
	{
		GROUND,
		MODEL_OBJ,

		NUM_MESH
	};

	enum RayTracingPipeline : uint8_t
	{
		TEST,
		GGX,

		NUM_RAYTRACE_PIPELINE
	};

	RayTracer(const XUSG::RayTracing::Device &device, const XUSG::RayTracing::CommandList &commandList);
	virtual ~RayTracer();

	bool Init(uint32_t width, uint32_t height, XUSG::Resource *vbUploads, XUSG::Resource *ibUploads,
		XUSG::RayTracing::Geometry *geometries, const char *fileName, const DirectX::XMFLOAT4 &posScale
		= DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	void SetPipeline(RayTracingPipeline pipeline);
	void UpdateFrame(uint32_t frameIndex, DirectX::CXMVECTOR eyePt, DirectX::CXMMATRIX viewProj);
	void Render(uint32_t frameIndex);
	void ClearHistory();

	const XUSG::Texture2D &GetOutputView(uint32_t frameIndex, XUSG::ResourceState dstState = XUSG::ResourceState(0));

	static const uint32_t FrameCount = 3;

protected:
	enum PipelineLayoutIndex : uint8_t
	{
		GLOBAL_LAYOUT,
		RAY_GEN_LAYOUT,
		HIT_GROUP_LAYOUT,
		GBUFFER_PASS_LAYOUT,
		TEMPORAL_SS_LAYOUT,

		NUM_PIPELINE_LAYOUT
	};

	enum GlobalPipelineLayoutSlot : uint8_t
	{
		OUTPUT_VIEW,
		SHADER_RESOURCES,
		ACCELERATION_STRUCTURE = SHADER_RESOURCES,
		SAMPLER,
		INDEX_BUFFERS,
		VERTEX_BUFFERS
	};

	enum PipelineIndex : uint8_t
	{
		GBUFFER_PASS,
		TEMPORAL_SS,
		TEMPORAL_AA,

		NUM_PIPELINE
	};

	enum SRVTable : uint8_t
	{
		SRV_TABLE_IB,
		SRV_TABLE_VB,
		SRV_TABLE_TS,

		NUM_SRV_TABLE = SRV_TABLE_TS + FrameCount
	};

	enum UAVTable : uint8_t
	{
		UAV_TABLE_OUTPUT,
		UAV_TABLE_TSAMP,

		NUM_UAV_TABLE
	};

	struct RayGenConstants
	{
		DirectX::XMMATRIX	ProjToWorld;
		DirectX::XMVECTOR	EyePt;
		DirectX::XMFLOAT2	Jitter;
	};

	struct HitGroupConstants
	{
		DirectX::XMMATRIX	Normal;
		DirectX::XMFLOAT2	Hammersley;
	};

	struct BasePassConstants
	{
		DirectX::XMFLOAT4X4	WorldViewProj;
		DirectX::XMFLOAT4X4	WorldViewProjPrev;
	};

	bool createVB(uint32_t numVert, uint32_t stride, const uint8_t *pData, XUSG::Resource &vbUpload);
	bool createIB(uint32_t numIndices, const uint32_t *pData, XUSG::Resource &ibUpload);
	bool createGroundMesh(XUSG::Resource &vbUpload, XUSG::Resource &ibUpload);
	bool createInputLayout();
	bool createPipelineLayouts();
	bool createPipelines();
	bool createDescriptorTables();
	bool buildAccelerationStructures(XUSG::RayTracing::Geometry *geometries);
	bool buildShaderTables();

	void updateAccelerationStructures(uint32_t frameIndex);
	void rayTrace(uint32_t frameIndex);
	void gbufferPass(uint32_t frameIndex);
	void temporalSS(uint32_t frameIndex);

	XUSG::RayTracing::Device m_device;
	XUSG::RayTracing::CommandList m_commandList;

	uint32_t m_numIndices[NUM_MESH];

	DirectX::XMUINT2	m_viewport;
	DirectX::XMFLOAT4	m_posScale;
	DirectX::XMFLOAT4X4 m_worlds[NUM_MESH];
	BasePassConstants	m_cbBasePass[NUM_MESH];

	static const uint32_t NumUAVs = NUM_MESH + 1 + FrameCount * NUM_UAV_TABLE;
	XUSG::RayTracing::BottomLevelAS m_bottomLevelASs[NUM_MESH];
	XUSG::RayTracing::TopLevelAS m_topLevelAS;

	XUSG::InputLayout			m_inputLayout;
	XUSG::PipelineLayout		m_pipelineLayouts[NUM_PIPELINE_LAYOUT];
	XUSG::RayTracing::Pipeline	m_rayTracingPipelines[NUM_RAYTRACE_PIPELINE];
	XUSG::Pipeline				m_pipelines[NUM_PIPELINE];

	XUSG::DescriptorTable	m_srvTables[NUM_SRV_TABLE];
	XUSG::DescriptorTable	m_uavTables[FrameCount][NUM_UAV_TABLE];
	XUSG::DescriptorTable	m_samplerTable;
	XUSG::RenderTargetTable	m_rtvTables[FrameCount];

	XUSG::VertexBuffer		m_vertexBuffers[NUM_MESH];
	XUSG::IndexBuffer		m_indexBuffers[NUM_MESH];

	XUSG::Texture2D			m_outputViews[FrameCount][NUM_UAV_TABLE];
	XUSG::RenderTarget		m_velocities[FrameCount];
	XUSG::DepthStencil		m_depth;

	XUSG::Resource			m_scratch;
	XUSG::Resource			m_instances[FrameCount];

	RayTracingPipeline		m_pipeIndex;

	// Shader tables
	static const wchar_t *HitGroupName;
	static const wchar_t *RaygenShaderName;
	static const wchar_t *ClosestHitShaderName;
	static const wchar_t *MissShaderName;
	XUSG::RayTracing::ShaderTable	m_missShaderTables[NUM_RAYTRACE_PIPELINE];
	XUSG::RayTracing::ShaderTable	m_hitGroupShaderTables[FrameCount][NUM_RAYTRACE_PIPELINE];
	XUSG::RayTracing::ShaderTable	m_rayGenShaderTables[FrameCount][NUM_RAYTRACE_PIPELINE];

	XUSG::ShaderPool				m_shaderPool;
	XUSG::RayTracing::PipelineCache	m_rayTracingPipelineCache;
	XUSG::Graphics::PipelineCache	m_graphicsPipelineCache;
	XUSG::Compute::PipelineCache	m_computePipelineCache;
	XUSG::PipelineLayoutCache		m_pipelineLayoutCache;
	XUSG::DescriptorTableCache		m_descriptorTableCache;
};
