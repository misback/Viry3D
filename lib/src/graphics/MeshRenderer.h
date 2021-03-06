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

#include "Renderer.h"

namespace Viry3D
{
    class Mesh;

    class MeshRenderer : public Renderer
    {
    public:
        MeshRenderer();
        virtual ~MeshRenderer();
        virtual Ref<BufferObject> GetVertexBuffer() const;
        virtual Ref<BufferObject> GetIndexBuffer() const;
#if VR_VULKAN
        virtual Ref<BufferObject> GetDrawBuffer() const { return m_draw_buffer; }
#elif VR_GLES
        virtual const DrawBuffer& GetDrawBuffer() const { return m_draw_buffer; }
#endif
        const Ref<Mesh>& GetMesh() const { return m_mesh; }
        int GetSubmesh() const { return m_submesh; }
        void SetMesh(const Ref<Mesh>& mesh, int submesh = 0);

    private:
        Ref<Mesh> m_mesh;
        int m_submesh;
#if VR_VULKAN
        Ref<BufferObject> m_draw_buffer;
#elif VR_GLES
        DrawBuffer m_draw_buffer;
#endif
    };
}
