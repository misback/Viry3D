/*
* Viry3D
* Copyright 2014-2018 by Stack - stackos@qq.com
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once

#include "Object.h"
#include "Display.h"
#include "thread/ThreadPool.h"

namespace Viry3D
{
    enum class CubemapFace
    {
        Unknown = -1,

        PositiveX,
        NegativeX,
        PositiveY,
        NegativeY,
        PositiveZ,
        NegativeZ,

        Count
    };

    enum class TextureFormat
    {
        None,
        R8,
        R8G8,
        R8G8B8A8,
        D16,
        D24X8,
        D32,
        D16S8,
        D24S8,
        D32S8,
        S8,
    };

    enum class FilterMode
    {
        None = -1,
        Nearest,
        Linear,
        Trilinear,
    };

    enum class SamplerAddressMode
    {
        None = -1,
        Repeat,
        ClampToEdge,
        Mirror,
        MirrorOnce,
    };

    class Texture : public Object
    {
    private:
        friend class DisplayPrivate;

    public:
        static ByteBuffer LoadImageFromFile(const String& path, int& width, int& height, int& bpp);
        static Ref<Texture> LoadTexture2DFromFile(
            const String& path,
            FilterMode filter_mode,
            SamplerAddressMode wrap_mode,
            bool gen_mipmap);
        static Ref<Texture> CreateTexture2DFromMemory(
            const ByteBuffer& pixels,
            int width,
            int height,
            TextureFormat format,
            FilterMode filter_mode,
            SamplerAddressMode wrap_mode,
            bool gen_mipmap,
            bool dynamic);
        static Ref<Texture> CreateCubemap(
            int size,
            TextureFormat format,
            FilterMode filter_mode,
            SamplerAddressMode wrap_mode,
            bool mipmap);
        static Ref<Texture> CreateRenderTexture(
            int width,
            int height,
            TextureFormat format,
            bool create_sampler,
            FilterMode filter_mode,
            SamplerAddressMode wrap_mode);
        static Ref<Texture> CreateTexture2DArrayFromMemory(
            const Vector<ByteBuffer>& pixels,
            int width,
            int height,
            int layer_count,
            TextureFormat format,
            FilterMode filter_mode,
            SamplerAddressMode wrap_mode,
            bool gen_mipmap,
            bool dynamic);
        static TextureFormat ChooseDepthFormatSupported(bool sample);
		static Ref<Texture> GetSharedWhiteTexture();
		static Ref<Texture> GetSharedBlackTexture();
		static Ref<Texture> GetSharedNormalTexture();
		static Ref<Texture> GetSharedCubemap();
		static void Done();
        virtual ~Texture();
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }
        int GetMipmapLevelCount() const { return m_mipmap_level_count; }
        void UpdateTexture2D(const ByteBuffer& pixels, int x, int y, int w, int h);
        void UpdateCubemap(const ByteBuffer& pixels, CubemapFace face, int level);
        void UpdateTexture2DArray(const ByteBuffer& pixels, int layer, int level);
        void GenMipmaps();
        void CopyTexture(
            const Ref<Texture>& src_texture,
            int src_layer, int src_level,
            int src_x, int src_y,
            int layer, int level,
            int x, int y,
            int w, int h);
        void CopyToMemory(ByteBuffer& pixels, int layer, int level);
#if VR_VULKAN
        VkFormat GetFormat() const { return m_format; }
        VkImage GetImage() const { return m_image; }
        VkImageView GetImageView() const { return m_image_view; }
        VkSampler GetSampler() const { return m_sampler; }
#elif VR_GLES
        GLuint GetTexture() const { return m_texture; }
        GLenum GetTarget() const { return m_target; }
        void Bind() const { glBindTexture(m_target, m_texture); }
        void Unbind() const { glBindTexture(m_target, 0); }
        bool IsRenderTexture() const { return m_render_texture; }
#endif

    private:
#if VR_VULKAN
        void CopyBufferToImageBegin();
        void CopyBufferToImage(const Ref<BufferObject>& image_buffer, int x, int y, int w, int h, int face, int level);
        void CopyBufferToImageEnd();
#elif VR_GLES
        static Ref<Texture> CreateTexture(
            GLenum target,
            int width,
            int height,
            TextureFormat format,
            int mipmap_level_count);
        void CreateSampler(FilterMode filter_mode, SamplerAddressMode wrap_mode);
#endif
        Texture();
        int GetLayerCount();

    private:
		static Ref<Texture> m_shared_white_texture;
		static Ref<Texture> m_shared_black_texture;
		static Ref<Texture> m_shared_normal_texture;
		static Ref<Texture> m_shared_cubemap;
#if VR_VULKAN
        VkFormat m_format;
        VkImage m_image;
        VkImageView m_image_view;
        VkDeviceMemory m_memory;
        VkMemoryAllocateInfo m_memory_info;
        VkSampler m_sampler;
        Ref<BufferObject> m_image_buffer;
#elif VR_GLES
        GLuint m_texture;
        GLuint m_target;
        GLint m_internal_format;
        GLenum m_format;
        GLenum m_pixel_type;
        bool m_have_storage;
        GLuint m_copy_framebuffer;
        bool m_render_texture;
        bool m_depth_texture;
#endif
        int m_width;
        int m_height;
        int m_mipmap_level_count;
        bool m_dynamic;
        bool m_cubemap;
        int m_array_size;
    };
}
