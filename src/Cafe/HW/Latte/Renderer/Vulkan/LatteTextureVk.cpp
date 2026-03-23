#include "Cafe/HW/Latte/Renderer/Vulkan/LatteTextureVk.h"
#include "Cafe/HW/Latte/Renderer/Vulkan/LatteTextureViewVk.h"
#include "Cafe/HW/Latte/Renderer/Vulkan/VulkanRenderer.h"
#include "Cafe/HW/Latte/Renderer/Vulkan/VulkanAPI.h"

LatteTextureVk::LatteTextureVk(VulkanRenderer* vkRenderer, Latte::E_DIM dim, MPTR physAddress, MPTR physMipAddress, Latte::E_GX2SURFFMT format, uint32 width, uint32 height, uint32 depth, uint32 pitch, uint32 mipLevels, uint32 swizzle, Latte::E_HWTILEMODE tileMode, bool isDepth)
	: LatteTexture(dim, physAddress, physMipAddress, format, width, height, depth, pitch, mipLevels, swizzle, tileMode, isDepth), m_vkr(vkRenderer)
{
	vkObjTex = new VKRObjectTexture();
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	
	uint32 effWidth = (overwriteInfo.hasResolutionOverwrite) ? overwriteInfo.width : width;
	uint32 effHeight = (overwriteInfo.hasResolutionOverwrite) ? overwriteInfo.height : height;
	uint32 effDepth = (overwriteInfo.hasResolutionOverwrite) ? overwriteInfo.depth : depth;
	effDepth = (effDepth > 1) ? effDepth : 1;

	imageInfo.extent.width = effWidth;
	imageInfo.extent.height = effHeight;
	imageInfo.mipLevels = mipLevels;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	// Parche Mali Immortalis
	bool isForbidden = (format == (Latte::E_GX2SURFFMT)0x3b || format == (Latte::E_GX2SURFFMT)0x38);
	if (!isForbidden) imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	if (dim == Latte::E_DIM::DIM_3D) {
		imageInfo.extent.depth = effDepth;
		imageInfo.arrayLayers = 1;
		imageInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
	} else {
		imageInfo.extent.depth = 1;
		imageInfo.arrayLayers = effDepth;
	}
	
	VulkanRenderer::FormatInfoVK fInfo;
	m_vkr->GetTextureFormatInfoVK(format, isDepth, dim, effWidth, effHeight, &fInfo);
	imageInfo.format = fInfo.vkImageFormat;
	vkObjTex->m_imageAspect = fInfo.vkImageAspect;
	
	if (!isDepth) {
		imageInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
		if (!isForbidden) imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	} else {
		imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	imageInfo.imageType = (dim == Latte::E_DIM::DIM_1D) ? VK_IMAGE_TYPE_1D : ((dim == Latte::E_DIM::DIM_3D) ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D);

	if (vkCreateImage(m_vkr->GetLogicalDevice(), &imageInfo, nullptr, &vkObjTex->m_image) != VK_SUCCESS)
		m_vkr->UnrecoverableError("Failed to create image");
	
	vkObjTex->m_flags = imageInfo.flags;
	vkObjTex->m_format = imageInfo.format;

	m_layoutsMips = (mipLevels > 1) ? mipLevels : 1;
	m_layoutsDepth = (effDepth > 1) ? effDepth : 1;
	m_layouts.resize(m_layoutsMips * m_layoutsDepth, VK_IMAGE_LAYOUT_UNDEFINED);
}

LatteTextureVk::~LatteTextureVk() {
	if (vkObjTex) delete vkObjTex;
}

void LatteTextureVk::AllocateOnHost() { }

LatteTextureView* LatteTextureVk::CreateView(Latte::E_DIM dim, Latte::E_GX2SURFFMT format, uint32 baseMip, uint32 mipCount, uint32 firstSlice, uint32 sliceCount) {
    return new LatteTextureViewVk(m_vkr->GetLogicalDevice(), this, dim, format, (int32)baseMip, (int32)mipCount, (int32)firstSlice, (int32)sliceCount);
}
