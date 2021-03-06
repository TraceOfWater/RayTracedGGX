//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include "XUSGRayTracingType.h"
#include "Core/XUSGPipelineLayout.h"

namespace XUSG
{
	namespace RayTracing
	{
		class PipelineCache;

		class State
		{
		public:
			struct KeyHeader
			{
				void	*ShaderLib;
				void	*GlobalPipelineLayout;
				uint32_t NumHitGroups;
				uint32_t NumLocalPipelineLayouts;
				uint32_t MaxPayloadSize;
				uint32_t MaxAttributeSize;
				uint32_t MaxRecursionDepth;
			};

			struct KeyHitGroup
			{
				const void *HitGroup;
				const void *ClosestHitShader;
				const void *AnyHitShader;
				const void *IntersectionShader;
				uint8_t Type;
			};

			struct KeyLocalPipelineLayoutHeader
			{
				void *PipelineLayout;
				uint32_t NumShaders;
			};

			struct KeyLocalPipelineLayout
			{
				KeyLocalPipelineLayoutHeader Header;
				std::vector<void*> Shaders;
			};

			State();
			virtual ~State();

			void SetShaderLibrary(Blob shaderLib);
			void SetHitGroup(uint32_t index, const void *hitGroup, const void *closestHitShader,
				const void *anyHitShader = nullptr, const void *intersectionShader = nullptr,
				uint8_t type = 0);
			void SetShaderConfig(uint32_t maxPayloadSize, uint32_t maxAttributeSize);
			void SetLocalPipelineLayout(uint32_t index, const XUSG::PipelineLayout &layout,
				uint32_t numShaders, const void **pShaders);
			void SetGlobalPipelineLayout(const XUSG::PipelineLayout &layout);
			void SetMaxRecursionDepth(uint32_t depth);

			Pipeline CreatePipeline(PipelineCache &pipelineCache, const wchar_t *name = nullptr);
			Pipeline GetPipeline(PipelineCache &pipelineCache, const wchar_t *name = nullptr);

			const std::string &GetKey();

		protected:
			void complete();

			KeyHeader *m_pKeyHeader;
			std::string m_key;

			std::vector<KeyHitGroup> m_keyHitGroups;
			std::vector<KeyLocalPipelineLayout> m_keyLocalPipelineLayouts;
			
			bool m_isComplete;
		};

		class PipelineCache
		{
		public:
			PipelineCache();
			PipelineCache(const Device &device);
			virtual ~PipelineCache();

			void SetDevice(const Device &device);
			void SetPipeline(const std::string &key, const Pipeline &pipeline);

			Pipeline CreatePipeline(State &state, const wchar_t *name = nullptr);
			Pipeline GetPipeline(State &state, const wchar_t *name = nullptr);

		protected:
			Pipeline createPipeline(const std::string &key, const wchar_t *name);
			Pipeline getPipeline(const std::string &key, const wchar_t *name);

			Device m_device;

			std::unordered_map<std::string, Pipeline> m_pipelines;
		};
	}
}
