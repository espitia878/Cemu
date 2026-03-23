#include "Cafe/HW/Latte/Renderer/Vulkan/LatteTextureViewVk.h"
#include "Cafe/HW/Latte/Renderer/Vulkan/LatteTextureVk.h"
#include "Cafe/HW/Latte/Renderer/Vulkan/VulkanRenderer.h"
#include <algorithm>

LatteTextureViewVk::LatteTextureViewVk(VkDevice device, LatteTextureVk* tex, Latte::E_DIM dim, Latte::E_GX2SURFFMT format, uint32 firstMip, uint32 mipCount, uint32 firstSlice, uint32 sliceCount)
	: LatteTextureView(tex, dim, format, firstMip, mipCount, firstSlice, sliceCount)
	, m_device(device)
{
	// Obtenemos el objeto de textura (confirmado en LatteTextureVk.h)
	auto vkObj = tex->GetImageObj();
	m_format = vkObj->m_format;
	m_uniqueId = 0; 

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = vkObj->m_image; // Nombre confirmado en VKRBase.h
	
	switch (dim)
	{
		case Latte::E_DIM::DIM_1D: viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D; break;
		case Latte::E_DIM::DIM_2D: viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; break;
		case Latte::E_DIM::DIM_2D_MSAA: viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; break;
		case Latte::E_DIM::DIM_3D: viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D; break;
		case Latte::E_DIM::DIM_CUBE: viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE; break;
		default: viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; break;
	}

	viewInfo.format = m_format;
	viewInfo.subresourceRange.aspectMask = vkObj->m_imageAspect; // Nombre confirmado en VKRBase.h
	viewInfo.subresourceRange.baseMipLevel = (uint32)firstMip;
	viewInfo.subresourceRange.levelCount = (uint32)mipCount;
	viewInfo.subresourceRange.baseArrayLayer = (uint32)firstSlice;
	viewInfo.subresourceRange.layerCount = (uint32)sliceCount;

	// Intentar crear la vista
	if (vkCreateImageView(m_device, &viewInfo, nullptr, &m_view) != VK_SUCCESS)
	{
		m_view = VK_NULL_HANDLE;
	}
}

LatteTextureViewVk::~LatteTextureViewVk()
{
	if (m_view != VK_NULL_HANDLE)
	{
		vkDestroyImageView(m_device, m_view, nullptr);
		m_view = VK_NULL_HANDLE;
	}
}

void LatteTextureViewVk::AddDescriptorSetReference(struct VkDescriptorSetInfo* dsInfo) {
	if (std::find(list_descriptorSets.begin(), list_descriptorSets.end(), dsInfo) == list_descriptorSets.end()) 
		list_descriptorSets.emplace_back(dsInfo);
}

void LatteTextureViewVk::RemoveDescriptorSetReference(struct VkDescriptorSetInfo* dsInfo) {
	auto it = std::find(list_descriptorSets.begin(), list_descriptorSets.end(), dsInfo);
	if (it != list_descriptorSets.end())
		list_descriptorSets.erase(it);
}
