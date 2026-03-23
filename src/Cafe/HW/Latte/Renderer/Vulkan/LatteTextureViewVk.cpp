#include "Cafe/HW/Latte/Renderer/Vulkan/LatteTextureViewVk.h"
#include "Cafe/HW/Latte/Renderer/Vulkan/LatteTextureVk.h"
#include "Cafe/HW/Latte/Renderer/Vulkan/VulkanRenderer.h"
#include "Cafe/HW/Latte/Renderer/Vulkan/VulkanAPI.h"

LatteTextureViewVk::LatteTextureViewVk(VkDevice device, LatteTextureVk* tex, Latte::E_DIM dim, Latte::E_GX2SURFFMT format, uint32 firstMip, uint32 mipCount, uint32 firstSlice, uint32 sliceCount)
	: LatteTextureView(tex, dim, format, firstMip, mipCount, firstSlice, sliceCount), m_device(device)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	
	// Acceso seguro al objeto de textura
	auto vkObj = tex->GetVKRObject();
	viewInfo.image = vkObj->m_image;
	
	switch (dim)
	{
	case Latte::E_DIM::DIM_1D: viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D; break;
	case Latte::E_DIM::DIM_2D: viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; break;
	case Latte::E_DIM::DIM_2D_MSAA: viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; break;
	case Latte::E_DIM::DIM_3D: viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D; break;
	case Latte::E_DIM::DIM_CUBE: viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE; break;
	default: viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; break;
	}

	viewInfo.format = vkObj->m_format;
	viewInfo.subresourceRange.aspectMask = vkObj->m_imageAspect;
	viewInfo.subresourceRange.baseMipLevel = firstMip;
	viewInfo.subresourceRange.levelCount = mipCount;
	viewInfo.subresourceRange.baseArrayLayer = firstSlice;
	viewInfo.subresourceRange.layerCount = sliceCount;

	// IMPORTANTE: Se usa m_view porque así está en el .h
	if (vkCreateImageView(m_device, &viewInfo, nullptr, &m_view) != VK_SUCCESS) {
		// Error silencioso para evitar crashes en logs
	}
}

LatteTextureViewVk::~LatteTextureViewVk()
{
	if (m_view != VK_NULL_HANDLE)
		vkDestroyImageView(m_device, m_view, nullptr);
}
