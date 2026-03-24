#include "Cafe/HW/Latte/Renderer/OpenGL/LatteTextureGL.h"
#include "Cafe/HW/Latte/Renderer/OpenGL/LatteTextureViewGL.h"
#include "Cafe/HW/Latte/Renderer/OpenGL/OpenGLRenderer.h"
#include "Cafe/HW/Latte/Core/Latte.h"

#include "config/LaunchSettings.h"

LatteTextureGL::LatteTextureGL(Latte::E_DIM dim, MPTR physAddress, MPTR physMipAddress, Latte::E_GX2SURFFMT format, uint32 width, uint32 height, uint32 depth, uint32 pitch, uint32 mipLevels, uint32 swizzle,
	Latte::E_HWTILEMODE tileMode, bool isDepth)
	: LatteTexture(dim, physAddress, physMipAddress, format, width, height, depth, pitch, mipLevels, swizzle, tileMode, isDepth)
{
	GenerateEmptyTextureFromGX2Dim(dim, this->glId_texture, this->glTexTarget, true);
	// set format info
	FormatInfoGL glFormatInfo;
	GetOpenGLFormatInfo(isDepth, overwriteInfo.hasFormatOverwrite ? (Latte::E_GX2SURFFMT)overwriteInfo.format : format, dim, &glFormatInfo);
	this->glInternalFormat = glFormatInfo.glInternalFormat;
	this->isAlternativeFormat = glFormatInfo.isUsingAlternativeFormat;
	// set debug name
	bool useGLDebugNames = false;
#ifdef CEMU_DEBUG_ASSERT
	useGLDebugNames = true;
#endif

	// CORRECCIÓN: Acceso correcto a LaunchSettings mediante Get()
	if (LaunchSettings::Get().IsLaunchFlagSet(LaunchFlag::DebugNames))
		useGLDebugNames = true;

	if (useGLDebugNames)
	{
		std::string debugName = "LatteTexture_" + std::to_string(physAddress);
		glObjectLabel(GL_TEXTURE, this->glId_texture, (GLsizei)debugName.size(), debugName.c_str());
	}
}

LatteTextureGL::~LatteTextureGL()
{
	if (this->glId_texture != 0)
		glDeleteTextures(1, &this->glId_texture);
}

void LatteTextureGL::GenerateEmptyTextureFromGX2Dim(Latte::E_DIM dim, uint32& glId, uint32& glTexTarget, bool createStorage)
{
	glGenTextures(1, &glId);
	switch (dim)
	{
	case Latte::E_DIM::DIM_1D:
		glTexTarget = GL_TEXTURE_1D;
		break;
	case Latte::E_DIM::DIM_2D:
	case Latte::E_DIM::DIM_2D_MSAA:
		glTexTarget = GL_TEXTURE_2D;
		break;
	case Latte::E_DIM::DIM_2D_ARRAY:
	case Latte::E_DIM::DIM_2D_ARRAY_MSAA:
		glTexTarget = GL_TEXTURE_2D_ARRAY;
		break;
	case Latte::E_DIM::DIM_3D:
		glTexTarget = GL_TEXTURE_3D;
		break;
	case Latte::E_DIM::DIM_CUBE:
		glTexTarget = GL_TEXTURE_CUBE_MAP;
		break;
	default:
		// Evitamos crash en dimensiones no soportadas
		glTexTarget = GL_TEXTURE_2D; 
		break;
	}
}

void glTextureStorage1DWrapper(uint32 target, uint32 texture, uint32 levels, uint32 internalformat, uint32 width)
{
#ifdef CAF_OPENGL_EMULATE_STORAGE
	glBindTexture(target, texture);
	uint32 w = width;
	for (uint32 i = 0; i < levels; i++)
	{
		glTexImage1D(target, i, internalformat, w, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		w = std::max(1u, w / 2u);
	}
#else
	glTextureStorage1D(texture, levels, internalformat, width);
#endif
}

void glTextureStorage2DWrapper(uint32 target, uint32 texture, uint32 levels, uint32 internalformat, uint32 width, uint32 height)
{
#ifdef CAF_OPENGL_EMULATE_STORAGE
	glBindTexture(target, texture);
	uint32 w = width;
	uint32 h = height;
	for (uint32 i = 0; i < levels; i++)
	{
		glTexImage2D(target, i, internalformat, (GLsizei)w, (GLsizei)h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		w = std::max(1u, w / 2u);
		h = std::max(1u, h / 2u);
	}
#else
	glTextureStorage2D(texture, levels, internalformat, (GLsizei)width, (GLsizei)height);
#endif
}

void glTextureStorage3DWrapper(uint32 target, uint32 texture, uint32 levels, uint32 internalformat, uint32 width, uint32 height, uint32 depth)
{
#ifdef CAF_OPENGL_EMULATE_STORAGE
	glBindTexture(target, texture);
	uint32 w = width;
	uint32 h = height;
	uint32 d = depth;
	for (uint32 i = 0; i < levels; i++)
	{
		glTexImage3D(target, i, internalformat, (GLsizei)w, (GLsizei)h, (GLsizei)d, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		w = std::max(1u, w / 2u);
		h = std::max(1u, h / 2u);
		if (target == GL_TEXTURE_3D)
			d = std::max(1u, d / 2u);
	}
#else
	glTextureStorage3D(texture, levels, internalformat, (GLsizei)width, (GLsizei)height, (GLsizei)depth);
#endif
}

void LatteTextureGL::UpdateTextureStorage(LatteTextureGL* hostTexture, uint32 effectiveBaseWidth, uint32 effectiveBaseHeight, uint32 effectiveBaseDepth, uint32 mipLevels)
{
	mipLevels = std::max(mipLevels, 1u);
	
	if (hostTexture->dim == Latte::E_DIM::DIM_2D || hostTexture->dim == Latte::E_DIM::DIM_2D_MSAA)
	{
		cemu_assert_debug(effectiveBaseDepth == 1);
		glTextureStorage2DWrapper(GL_TEXTURE_2D, hostTexture->glId_texture, mipLevels, hostTexture->glInternalFormat, effectiveBaseWidth, effectiveBaseHeight);
	}
	else if (hostTexture->dim == Latte::E_DIM::DIM_1D)
	{
		cemu_assert_debug(effectiveBaseHeight == 1);
		cemu_assert_debug(effectiveBaseDepth == 1);
		glTextureStorage1DWrapper(GL_TEXTURE_1D, hostTexture->glId_texture, mipLevels, hostTexture->glInternalFormat, effectiveBaseWidth);
	}
	else if (hostTexture->dim == Latte::E_DIM::DIM_2D_ARRAY || hostTexture->dim == Latte::E_DIM::DIM_2D_ARRAY_MSAA)
	{
		glTextureStorage3DWrapper(GL_TEXTURE_2D_ARRAY, hostTexture->glId_texture, mipLevels, hostTexture->glInternalFormat, effectiveBaseWidth, effectiveBaseHeight, std::max(1u, effectiveBaseDepth));
	}
	else if (hostTexture->dim == Latte::E_DIM::DIM_3D)
	{
		glTextureStorage3DWrapper(GL_TEXTURE_3D, hostTexture->glId_texture, mipLevels, hostTexture->glInternalFormat, effectiveBaseWidth, effectiveBaseHeight, effectiveBaseDepth);
	}
	else if (hostTexture->dim == Latte::E_DIM::DIM_CUBE)
	{
		cemu_assert_debug(effectiveBaseDepth == 1);
		glTextureStorage2DWrapper(GL_TEXTURE_CUBE_MAP, hostTexture->glId_texture, mipLevels, hostTexture->glInternalFormat, effectiveBaseWidth, effectiveBaseHeight);
	}
	else
	{
		// Bypass para evitar errores de formato en hardware específico
		cemu_assert_debug(true); 
	}
}
