//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include "XUSGShader.h"
#include "XUSGDescriptor.h"

namespace XUSG
{
	enum class DescriptorType : uint8_t
	{
		SRV,
		UAV,
		CBV,
		SAMPLER,
		CONSTANT,

		NUM
	};

	class PipelineLayoutCache;

	struct DescriptorRange
	{
		DescriptorType	ViewType;
		uint32_t NumDescriptors;
		uint32_t BaseBinding;
		uint32_t Space;
		uint8_t Flags;
	};

	namespace Util
	{
		class PipelineLayout
		{
		public:
			PipelineLayout();
			virtual ~PipelineLayout();

			void SetShaderStage(uint32_t index, Shader::Stage stage);
			void SetRange(uint32_t index, DescriptorType type, uint32_t num, uint32_t baseBinding,
				uint32_t space = 0, uint8_t flags = 0);
			void SetConstants(uint32_t index, uint32_t num32BitValues, uint32_t binding,
				uint32_t space = 0, Shader::Stage stage = Shader::Stage::ALL);

			XUSG::PipelineLayout CreatePipelineLayout(PipelineLayoutCache &pipelineLayoutCache, uint8_t flags,
				const wchar_t *name = nullptr);
			XUSG::PipelineLayout GetPipelineLayout(PipelineLayoutCache &pipelineLayoutCache, uint8_t flags,
				const wchar_t *name = nullptr);

			DescriptorTableLayout CreateDescriptorTableLayout(uint32_t index, PipelineLayoutCache &pipelineLayoutCache) const;
			DescriptorTableLayout GetDescriptorTableLayout(uint32_t index, PipelineLayoutCache &pipelineLayoutCache) const;

			const std::vector<std::string> &GetDescriptorTableLayoutKeys() const;
			std::string &GetPipelineLayoutKey(PipelineLayoutCache *pPipelineLayoutCache);

		protected:
			std::string &checkKeySpace(uint32_t index);

			std::vector<std::string> m_descriptorTableLayoutKeys;
			std::string m_pipelineLayoutKey;

			bool m_tableLayoutsCompleted;
		};
	}

	class PipelineLayoutCache
	{
	public:
		PipelineLayoutCache();
		PipelineLayoutCache(const Device &device);
		virtual ~PipelineLayoutCache();

		void SetDevice(const Device &device);
		void SetPipelineLayout(const std::string &key, const PipelineLayout &pipelineLayout);

		PipelineLayout CreatePipelineLayout(Util::PipelineLayout &util, uint8_t flags,
			const wchar_t *name = nullptr);
		PipelineLayout GetPipelineLayout(Util::PipelineLayout &util, uint8_t flags,
			const wchar_t *name = nullptr, bool needCreate = true);

		DescriptorTableLayout CreateDescriptorTableLayout(uint32_t index, const Util::PipelineLayout &util);
		DescriptorTableLayout GetDescriptorTableLayout(uint32_t index, const Util::PipelineLayout &util);

	protected:
		PipelineLayout createPipelineLayout(const std::string &key, const wchar_t *name) const;
		PipelineLayout getPipelineLayout(const std::string &key, const wchar_t *name, bool needCreate);

		DescriptorTableLayout createDescriptorTableLayout(const std::string &key);
		DescriptorTableLayout getDescriptorTableLayout(const std::string &key);

		Device m_device;

		std::unordered_map<std::string, PipelineLayout> m_pipelineLayouts;
		std::unordered_map<std::string, DescriptorTableLayout> m_descriptorTableLayouts;
	};
}
