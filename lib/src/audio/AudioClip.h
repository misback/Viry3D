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
#include "memory/ByteBuffer.h"

namespace Viry3D
{
    class AudioClip : public Object
    {
    public:
        static Ref<AudioClip> LoadWaveFromFile(const String& path);
        virtual ~AudioClip();

    private:
        AudioClip();

    private:
        int m_channel;
        int m_sample_rate;
        int m_byte_rate;
        int m_sample_bits;
        ByteBuffer m_samples;
    };
}
