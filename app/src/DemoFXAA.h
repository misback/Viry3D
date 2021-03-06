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

#include "DemoMesh.h"

#define FXAA_QUALITY_FAST		10
#define FXAA_QUALITY_DEFAULT	12
#define FXAA_QUALITY_HIGH		29
#define FXAA_QUALITY_EXTREME	39

namespace Viry3D
{
    class DemoFXAA : public DemoMesh
    {
    public:
        Camera* m_blit_camera = nullptr;

        void InitRenderTexture()
        {
            auto color_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                TextureFormat::R8G8B8A8,
                true,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);
            auto depth_texture = Texture::CreateRenderTexture(
                Display::Instance()->GetWidth(),
                Display::Instance()->GetHeight(),
                Texture::ChooseDepthFormatSupported(true),
                true,
                FilterMode::Linear,
                SamplerAddressMode::ClampToEdge);
            m_camera->SetRenderTarget(color_texture, depth_texture);

            RenderState render_state;
            render_state.cull = RenderState::Cull::Off;
            render_state.zTest = RenderState::ZTest::Off;
            render_state.zWrite = RenderState::ZWrite::Off;

#if VR_VULKAN
            String vs = R"(
Input(0) vec4 a_pos;
Input(2) vec2 a_uv;

Output(0) vec2 v_uv;

void main()
{
	gl_Position = a_pos;
	v_uv = a_uv;

	vulkan_convert();
}
)";
            String fs = R"(
precision highp float;
      
UniformTexture(0, 0) uniform sampler2D u_texture;

UniformBuffer(0, 1) uniform UniformBuffer01
{
    vec4 u_rcp_frame;
} buf_0_1;

Input(0) vec2 v_uv;

Output(0) vec4 o_frag;

void main()
{
    o_frag = FxaaPixelShader(v_uv,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsolePosPos,
        u_texture,							    // FxaaTex tex,
        u_texture,							    // FxaaTex fxaaConsole360TexExpBiasNegOne,
        u_texture,							    // FxaaTex fxaaConsole360TexExpBiasNegTwo,
        buf_0_1.u_rcp_frame.xy,					// FxaaFloat2 fxaaQualityRcpFrame,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsoleRcpFrameOpt,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsoleRcpFrameOpt2,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsole360RcpFrameOpt2,
        0.75f,									// FxaaFloat fxaaQualitySubpix,
        0.166f,									// FxaaFloat fxaaQualityEdgeThreshold,
        0.0833f,								// FxaaFloat fxaaQualityEdgeThresholdMin,
        0.0f,									// FxaaFloat fxaaConsoleEdgeSharpness,
        0.0f,									// FxaaFloat fxaaConsoleEdgeThreshold,
        0.0f,									// FxaaFloat fxaaConsoleEdgeThresholdMin,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f)		// FxaaFloat fxaaConsole360ConstDir,
    );
}
)";
            
            auto shader = RefMake<Shader>(
                "",
                Vector<String>(),
                vs,
                String::Format(
                    "#extension GL_ARB_gpu_shader5: enable\n"
                    "#define FXAA_QUALITY__PRESET %d\n"
                    "#define FXAA_GLSL_130 1",
                    FXAA_QUALITY_EXTREME),
                Vector<String>({ "FXAA.in" }),
                fs,
                render_state);
#elif VR_GLES
            String vs = R"(
in vec4 a_pos;
in vec2 a_uv;

out vec2 v_uv;

void main()
{
    gl_Position = a_pos;
    v_uv = vec2(a_uv.x, 1.0 - a_uv.y);
}
)";
            String fs = R"(
precision highp float;

uniform sampler2D u_texture;

uniform vec4 u_rcp_frame;

in vec2 v_uv;

out vec4 o_frag;

void main()
{
    o_frag = FxaaPixelShader(v_uv,
        FxaaFloat4(0.0, 0.0, 0.0, 0.0),             // FxaaFloat4 fxaaConsolePosPos,
        u_texture,                                  // FxaaTex tex,
        u_texture,                                  // FxaaTex fxaaConsole360TexExpBiasNegOne,
        u_texture,                                  // FxaaTex fxaaConsole360TexExpBiasNegTwo,
        u_rcp_frame.xy,                             // FxaaFloat2 fxaaQualityRcpFrame,
        FxaaFloat4(0.0, 0.0, 0.0, 0.0),             // FxaaFloat4 fxaaConsoleRcpFrameOpt,
        FxaaFloat4(0.0, 0.0, 0.0, 0.0),             // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
        FxaaFloat4(0.0, 0.0, 0.0, 0.0),             // FxaaFloat4 fxaaConsole360RcpFrameOpt2,
        0.75,                                       // FxaaFloat fxaaQualitySubpix,
        0.166,                                      // FxaaFloat fxaaQualityEdgeThreshold,
        0.0833,                                     // FxaaFloat fxaaQualityEdgeThresholdMin,
        0.0,                                        // FxaaFloat fxaaConsoleEdgeSharpness,
        0.0,                                        // FxaaFloat fxaaConsoleEdgeThreshold,
        0.0,                                        // FxaaFloat fxaaConsoleEdgeThresholdMin,
        FxaaFloat4(0.0, 0.0, 0.0, 0.0)              // FxaaFloat fxaaConsole360ConstDir,
        );
}
)";
            
            auto shader = RefMake<Shader>(
#if VR_IOS || VR_ANDROID || VR_UWP
                "#version 300 es",
#else
                "#version 330",
#endif
                Vector<String>(),
                vs,
                String::Format(
#if VR_IOS || VR_ANDROID || VR_UWP
                    "#version 300 es\n"
#else
                    "#version 330\n"
#endif
#if VR_WINDOWS
                    "#extension GL_ARB_gpu_shader5: enable\n"
#endif
                    "#define FXAA_QUALITY__PRESET %d\n"
                    "#define FXAA_GLSL_130 1",
                    FXAA_QUALITY_EXTREME),
                Vector<String>({ "FXAA.in" }),
                fs,
                render_state);
#endif

            auto material = RefMake<Material>(shader);
            material->SetVector("u_rcp_frame", Vector4(1.0f / Display::Instance()->GetWidth(), 1.0f / Display::Instance()->GetHeight()));

            // color -> window
            m_blit_camera = Display::Instance()->CreateBlitCamera(1, color_texture, material);

            m_ui_camera->SetDepth(2);
        }

        virtual void Init()
        {
            DemoMesh::Init();

            this->InitRenderTexture();
        }

        virtual void Done()
        {
            Display::Instance()->DestroyCamera(m_blit_camera);
            m_blit_camera = nullptr;

            DemoMesh::Done();
        }

        virtual void Update()
        {
            DemoMesh::Update();
        }
    };
}
