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

#include "Renderer.h"
#include "Camera.h"
#include "Material.h"
#include "Shader.h"
#include "BufferObject.h"
#include "Debug.h"

namespace Viry3D
{
    Renderer::Renderer():
		m_camera(nullptr),
        m_model_matrix_dirty(true)
    {
    
    }

    Renderer::~Renderer()
    {
    
    }

    void Renderer::SetMaterial(const Ref<Material>& material)
    {
        if (m_material)
        {
            m_material->OnUnSetRenderer(this);
        }

        m_material = material;
        this->MarkRendererOrderDirty();

        if (m_material)
        {
            m_material->OnSetRenderer(this);
        }

#if VR_VULKAN
        this->MarkInstanceCmdDirty();
#endif

        if (m_instance_material)
        {
            if (m_material)
            {
                Map<String, MaterialProperty> properties = m_instance_material->GetProperties();

                m_instance_material = RefMake<Material>(m_material->GetShader());
                for (const auto& i : properties)
                {
                    switch (i.second.type)
                    {
                        case MaterialProperty::Type::Color:
                            m_instance_material->SetColor(i.second.name, *(Color*) &i.second.data);
                            break;
                        case MaterialProperty::Type::Vector:
                            m_instance_material->SetVector(i.second.name, *(Vector4*) &i.second.data);
                            break;
                        case MaterialProperty::Type::Float:
                            m_instance_material->SetFloat(i.second.name, *(float*) &i.second.data);
                            break;
                        case MaterialProperty::Type::Texture:
                            m_instance_material->SetTexture(i.second.name, i.second.texture);
                            break;
                        case MaterialProperty::Type::Matrix:
                            m_instance_material->SetMatrix(i.second.name, *(Matrix4x4*) &i.second.data);
                            break;
                        case MaterialProperty::Type::VectorArray:
                            m_instance_material->SetVectorArray(i.second.name, i.second.vector_array);
                            break;
                        case MaterialProperty::Type::Int:
                            m_instance_material->SetInt(i.second.name, *(int*) &i.second.data);
                            break;
                    }
                }
            }
            else
            {
                m_instance_material.reset();
            }
        }

        if (m_material)
        {
            if (m_camera)
            {
                m_material->SetMatrix(VIEW_MATRIX, m_camera->GetViewMatrix());
                m_material->SetMatrix(PROJECTION_MATRIX, m_camera->GetProjectionMatrix());
            }
        }

        m_model_matrix_dirty = true;
    }

    void Renderer::OnAddToCamera(Camera* camera)
    {
		assert(m_camera == nullptr);
		m_camera = camera;
    }

    void Renderer::OnRemoveFromCamera(Camera* camera)
    {
		assert(m_camera == camera);
		m_camera = nullptr;
    }

    void Renderer::MarkRendererOrderDirty()
    {
		if (m_camera)
		{
			m_camera->MarkRendererOrderDirty();
		}
    }

#if VR_VULKAN
    void Renderer::MarkInstanceCmdDirty()
    {
		if (m_camera)
		{
			m_camera->MarkInstanceCmdDirty(this);
		}
    }
#endif

    void Renderer::OnMatrixDirty()
    {
        m_model_matrix_dirty = true;
    }

    void Renderer::Update()
    {
        if (m_model_matrix_dirty)
        {
            m_model_matrix_dirty = false;
            this->SetInstanceMatrix(MODEL_MATRIX, this->GetLocalToWorldMatrix());
        }

#if VR_VULKAN
        if (m_material)
        {
            m_material->UpdateUniformSets();
        }

        if (m_instance_material)
        {
            m_instance_material->UpdateUniformSets();
        }
#endif
    }

    void Renderer::SetInstanceMatrix(const String& name, const Matrix4x4& mat)
    {
        if (m_material)
        {
            if (!m_instance_material)
            {
                m_instance_material = RefMake<Material>(m_material->GetShader());
            }
            
            m_instance_material->SetMatrix(name, mat);
        }
    }

    void Renderer::SetInstanceVectorArray(const String& name, const Vector<Vector4>& array)
    {
        if (m_material)
        {
            if (!m_instance_material)
            {
                m_instance_material = RefMake<Material>(m_material->GetShader());
            }

            m_instance_material->SetVectorArray(name, array);
        }
    }

#if VR_GLES
    void Renderer::OnDraw()
    {
        const Ref<Material>& material = this->GetMaterial();
        Ref<BufferObject> vertex_buffer = this->GetVertexBuffer();
        Ref<BufferObject> index_buffer = this->GetIndexBuffer();
        const DrawBuffer& draw_buffer = this->GetDrawBuffer();

        if (!material || !vertex_buffer || !index_buffer || draw_buffer.index_count == 0)
        {
            return;
        }

        const Ref<Shader>& shader = material->GetShader();
        if (!shader->Use())
        {
            return;
        }

        vertex_buffer->Bind();
        index_buffer->Bind();
        shader->EnableVertexAttribs();
        shader->ApplyRenderState();
        material->ApplyUniforms();

        const Ref<Material>& instance_material = this->GetInstanceMaterial();
        if (instance_material)
        {
            instance_material->ApplyUniforms();
        }

        glDrawElements(GL_TRIANGLES, draw_buffer.index_count, GL_UNSIGNED_SHORT, (const void*) (draw_buffer.first_index * sizeof(unsigned short)));

        shader->DisableVertexAttribs();
        vertex_buffer->Unind();
        index_buffer->Unind();

        LogGLError();
    }
#endif
}
