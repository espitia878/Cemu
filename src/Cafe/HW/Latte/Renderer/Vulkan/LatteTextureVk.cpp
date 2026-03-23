#include "Cafe/HW/Latte/Renderer/Vulkan/LatteTextureVk.h"
#include "Cafe/HW/Latte/Renderer/Vulkan/LatteTextureViewVk.h"
#include "Cafe/HW/Latte/Renderer/Vulkan/VulkanRenderer.h"
#include "Cafe/HW/Latte/Renderer/Vulkan/VulkanAPI.h"
#include <algorithm>

LatteTextureVk::LatteTextureVk(VulkanRenderer* vkRenderer, Latte::E_DIM dim, MPTR physAddress, MPTR physMipAddress, Latte::E_GX2SURFFMT format, uint32 width, uint32 height, uint32 depth, uint32 pitch, uint32 mipLevels, uint32 swizzle, Latte::E_HWTILEMODE tileMode, bool isDepth)
	: LatteTexture(dim, physAddress, physMipAddress, format, width, height, depth, pitch, mipLevels, swizzle, tileMode, isDepth), m_vkr(vkRenderer)
{
	vkObjTex = new VKRObjectTexture();

	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	
	sint32 effectiveBaseWidth = width;
	sint32 effectiveBaseHeight = height;
	sint32 effectiveBaseDepth = depth;
	if (overwriteInfo.hasResolutionOverwrite)
	{
		effectiveBaseWidth = overwriteInfo.width;
		effectiveBaseHeight = overwriteInfo.height;
		effectiveBaseDepth = overwriteInfo.depth;
	}
	effectiveBaseDepth = (effectiveBaseDepth > 1) ? effectiveBaseDepth : 1;

	imageInfo.extent.width = (uint32)effectiveBaseWidth;
	imageInfo.extent.height = (uint32)effectiveBaseHeight;
	imageInfo.mipLevels = mipLevels;
	
	// --- MALI IMMORTALIS STABILITY PATCH ---
	// Iniciamos con bits de solo lectura para estabilidad en formatos 0x3b/0x38
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	// Lista negra para formatos detectados en Logcat (Zombies/Mali Error)
	bool isMaliProhibited = (format == (Latte::E_GX2SURFFMT)0x3b || format == (Latte::E_GX2SURFFMT)0x38 || Latte::IsCompressedFormat(format));

	if (!isMaliProhibited)
	{
		imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
	}
	
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	if (dim == Latte::E_DIM::DIM_3D)
	{
		imageInfo.extent.depth = (uint32)effectiveBaseDepth;
		imageInfo.arrayLayers = 1;
		imageInfo.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
	}
	else
	{
		imageInfo.extent.depth = 1;
		imageInfo.arrayLayers = (uint32)effectiveBaseDepth;
		if (dim != Latte::E_DIM::DIM_1D && (effectiveBaseDepth % 6) == 0 && effectiveBaseWidth == effectiveBaseHeight)
			imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}
	
	VulkanRenderer::FormatInfoVK texFormatInfo;
	vkRenderer->GetTextureFormatInfoVK(format, isDepth, dim, effectiveBaseWidth, effectiveBaseHeight, &texFormatInfo);
	imageInfo.format = texFormatInfo.vkImageFormat;
	vkObjTex->m_imageAspect = texFormatInfo.vkImageAspect;
	
	if (isDepth == false && texFormatInfo.isCompressed)
	{
		imageInfo.flags |= VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT;
		imageInfo.flags |= VK_IMAGE_CREATE_EXTENDED_USAGE_BIT;
	}
    
	if (isDepth == false)
	{
		imageInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
		if (!isMaliProhibited)
		{
			imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
	}

	if (isDepth)
	{
		imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	else if(!isMaliProhibited && texFormatInfo.vkImageFormat != VK_FORMAT_R4G4_UNORM_PACK8) 
	{
		imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	imageInfo.imageType = VK_IMAGE_TYPE_2D; 
	if (dim == Latte::E_DIM::DIM_1D) imageInfo.imageType = VK_IMAGE_TYPE_1D;
	else if (dim == Latte::E_DIM::DIM_3D) imageInfo.imageType = VK_IMAGE_TYPE_3D;

	if (vkCreateImage(m_vkr->GetLogicalDevice(), &imageInfo, nullptr, &vkObjTex->m_image) != VK_SUCCESS)
		m_vkr->UnrecoverableError("Failed to create texture image");
	
	vkObjTex->m_flags = imageInfo.flags;
	vkObjTex->m_format = imageInfo.format;

	// Correcion final para linea 97 (evita errores de std namespace)
	m_layoutsMips = (mipLevels > 1u) ? mipLevels : 1u; 
	m_layoutsDepth = (depth > 1u) ? depth : 1u;
	
	if (Is3DTexture())
	{
		m_layouts.resize(m_layoutsMips, VK_IMAGE_LAYOUT_UNDEFINED); 
	}
	else
	{
		m_layouts.resize(m_layoutsMips * m_layoutsDepth, VK_IMAGE_LAYOUT_UNDEFINED); 
	}
}
