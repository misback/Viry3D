/*
 * Viry3D
 * Copyright 2014-2017 by Stack - stackos@qq.com
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

#include "ARScene.h"
#include "Debug.h"
#include "gles/gles_include.h"
#include "graphics/Texture2D.h"
#import <ARKit/ARKit.h>

struct CapturedTexture
{
    GLuint texture_y;
    int width_y;
    int height_y;
    GLuint texture_uv;
    int width_uv;
    int height_uv;
};

API_AVAILABLE(ios(11.0))
@interface SessionDelegate : NSObject <ARSessionDelegate>

@end

@implementation SessionDelegate
{
    ARSession* m_session;
    CVOpenGLESTextureRef m_captured_texture_y;
    CVOpenGLESTextureRef m_captured_texture_uv;
    CVOpenGLESTextureCacheRef m_captured_texture_cache;
}

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        m_session = [ARSession new];
        m_session.delegate = self;
        
        auto context = [EAGLContext currentContext];
        CVOpenGLESTextureCacheCreate(NULL, NULL, context, NULL, &m_captured_texture_cache);
    }
    
    return self;
}

- (void)dealloc {
    CVBufferRelease(m_captured_texture_y);
    CVBufferRelease(m_captured_texture_uv);
}

- (void)run
{
    ARWorldTrackingConfiguration* configuration = [ARWorldTrackingConfiguration new];
    configuration.planeDetection = ARPlaneDetectionHorizontal;
    
    [m_session runWithConfiguration:configuration];
}

- (void)pause
{
    [m_session pause];
}

- (void)update:(CapturedTexture*)texture anchors:(Viry3D::Vector<Viry3D::ARAnchor>&)anchors
{
    ARFrame* frame = m_session.currentFrame;
    memset(texture, 0, sizeof(CapturedTexture));
    
    if (frame == nil)
    {
        return;
    }
    
    CVPixelBufferRef pixel_buffer = frame.capturedImage;
    
    if (CVPixelBufferGetPlaneCount(pixel_buffer) < 2)
    {
        return;
    }
    
    [self _updateCapturedTexture:pixel_buffer texture:texture];
    [self _updateAnchors:frame anchors:anchors];
}

- (void)_updateCapturedTexture:(CVPixelBufferRef)pixel_buffer texture:(CapturedTexture*)texture
{
    int width_y;
    int height_y;
    int width_uv;
    int height_uv;
    
    CVBufferRelease(m_captured_texture_y);
    CVBufferRelease(m_captured_texture_uv);
    m_captured_texture_y = [self _createTextureFromPixelBuffer:pixel_buffer internalFormat:GL_LUMINANCE format:GL_LUMINANCE type:GL_UNSIGNED_BYTE planeIndex:0 width:&width_y height:&height_y];
    m_captured_texture_uv = [self _createTextureFromPixelBuffer:pixel_buffer internalFormat:GL_RG8 format:GL_RG type:GL_UNSIGNED_BYTE planeIndex:1 width:&width_uv height:&height_uv];
    
    if (m_captured_texture_y != nil &&
        m_captured_texture_uv != nil)
    {
        texture->texture_y = CVOpenGLESTextureGetName(m_captured_texture_y);
        texture->width_y = width_y;
        texture->height_y = height_y;
        texture->texture_uv = CVOpenGLESTextureGetName(m_captured_texture_uv);
        texture->width_uv = width_uv;
        texture->height_uv = height_uv;
    }
}

- (CVOpenGLESTextureRef)_createTextureFromPixelBuffer:(CVPixelBufferRef)pixel_buffer internalFormat:(GLint)internal_format format:(GLint)format type:(GLenum)type planeIndex:(NSInteger)plane_index width:(int*)width height:(int*)height {
    *width = (int) CVPixelBufferGetWidthOfPlane(pixel_buffer, plane_index);
    *height = (int) CVPixelBufferGetHeightOfPlane(pixel_buffer, plane_index);
    
    CVOpenGLESTextureRef texture = nil;
    CVReturn status = CVOpenGLESTextureCacheCreateTextureFromImage(NULL, m_captured_texture_cache, pixel_buffer, NULL, GL_TEXTURE_2D, internal_format, *width, *height, format, type, plane_index, &texture);
    if (status != kCVReturnSuccess)
    {
        CVBufferRelease(texture);
        texture = nil;
    }
    
    return texture;
}

- (void)_updateAnchors:(ARFrame*)frame anchors:(Viry3D::Vector<Viry3D::ARAnchor>&)anchors
{
    NSArray<ARAnchor*>* frame_anchors = frame.anchors;
    int anchor_count = (int) [frame_anchors count];
    
    anchors.Clear();
    
    for (int i = 0; i < anchor_count; i++)
    {
        ARAnchor* anchor = [frame_anchors objectAtIndex:i];
        if ([anchor isKindOfClass:[ARPlaneAnchor class]])
        {
            ARPlaneAnchor* plane = (ARPlaneAnchor*) anchor;

            Viry3D::ARAnchor a;
            a.id = plane.identifier.UUIDString.UTF8String;
            a.transform = Viry3D::Matrix4x4(
                plane.transform.columns[0].x, plane.transform.columns[0].y, plane.transform.columns[0].z, plane.transform.columns[0].w,
                plane.transform.columns[1].x, plane.transform.columns[1].y, plane.transform.columns[1].z, plane.transform.columns[1].w,
                plane.transform.columns[2].x, plane.transform.columns[2].y, plane.transform.columns[2].z, plane.transform.columns[2].w,
                plane.transform.columns[3].x, plane.transform.columns[3].y, plane.transform.columns[3].z, plane.transform.columns[3].w);
            a.center = Viry3D::Vector3(plane.center.x, plane.center.y, plane.center.z);
            a.extent = Viry3D::Vector3(plane.extent.x, plane.extent.y, plane.extent.z);
            
            anchors.Add(a);
        }
    }
}

- (void)session:(ARSession *)session didFailWithError:(NSError *)error
{
    Log("ARSession didFailWithError:%s", [[error localizedDescription] UTF8String]);
}

- (void)sessionWasInterrupted:(ARSession *)session
{
    // Inform the user that the session has been interrupted, for example, by presenting an overlay
    Log("ARSession sessionWasInterrupted");
}

- (void)sessionInterruptionEnded:(ARSession *)session
{
    // Reset tracking and/or remove existing anchors if consistent tracking is required
    Log("ARSession sessionInterruptionEnded");
}

@end

namespace Viry3D
{
    static SessionDelegate* g_session = nil;
    
	bool ARScene::IsSupported()
    {
        return ARConfiguration.isSupported;
    }
    
    ARScene::ARScene()
    {
        g_session = [[SessionDelegate alloc] init];
    }
    
    ARScene::~ARScene()
    {
        g_session = nil;
    }
    
    void ARScene::RunSession()
    {
        if (g_session != nil)
        {
            [g_session run];
        }
    }
    
    void ARScene::PauseSession()
    {
        if (g_session != nil)
        {
            [g_session pause];
        }
    }
    
    void ARScene::UpdateSession()
    {
        if (g_session != nil)
        {
            CapturedTexture background;
            
            [g_session update:&background anchors:m_anchors];
            
            if (background.texture_y > 0 &&
                background.texture_uv > 0)
            {
                if (m_background_texture_y)
                {
                    m_background_texture_y->UpdateExternalTexture((void*) (size_t) background.texture_y);
                }
                else
                {
                    m_background_texture_y = Texture2D::CreateExternalTexture(background.width_y, background.height_y, TextureFormat::Alpha8, false, (void*) (size_t) background.texture_y);
                }
                
                if (m_background_texture_uv)
                {
                    m_background_texture_uv->UpdateExternalTexture((void*) (size_t) background.texture_uv);
                }
                else
                {
                     m_background_texture_uv = Texture2D::CreateExternalTexture(background.width_uv, background.height_uv, TextureFormat::RG16, false, (void*) (size_t) background.texture_uv);
                }
            }
        }
    }
}
