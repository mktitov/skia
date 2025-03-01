/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlResourceProvider.h"

#include "experimental/graphite/include/BackendTexture.h"
#include "experimental/graphite/src/GraphicsPipelineDesc.h"
#include "experimental/graphite/src/mtl/MtlBuffer.h"
#include "experimental/graphite/src/mtl/MtlCommandBuffer.h"
#include "experimental/graphite/src/mtl/MtlGpu.h"
#include "experimental/graphite/src/mtl/MtlGraphicsPipeline.h"
#include "experimental/graphite/src/mtl/MtlTexture.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

ResourceProvider::ResourceProvider(const skgpu::Gpu* gpu)
    : skgpu::ResourceProvider(gpu) {
}

const Gpu* ResourceProvider::mtlGpu() {
    return static_cast<const Gpu*>(fGpu);
}

sk_sp<skgpu::CommandBuffer> ResourceProvider::createCommandBuffer() {
    return CommandBuffer::Make(this->mtlGpu());
}

sk_sp<skgpu::GraphicsPipeline> ResourceProvider::onCreateGraphicsPipeline(
        const GraphicsPipelineDesc& desc) {
    return GraphicsPipeline::Make(this->mtlGpu(), desc);
}

sk_sp<skgpu::Texture> ResourceProvider::createTexture(SkISize dimensions,
                                                      const skgpu::TextureInfo& info) {
    return Texture::Make(this->mtlGpu(), dimensions, info);
}

sk_sp<skgpu::Texture> ResourceProvider::createWrappedTexture(const BackendTexture& texture) {
    sk_cfp<mtl::Handle> mtlHandleTexture = texture.getMtlTexture();
    if (!mtlHandleTexture) {
        return nullptr;
    }
    sk_cfp<id<MTLTexture>> mtlTexture((id<MTLTexture>)mtlHandleTexture.release());
    return Texture::MakeWrapped(texture.dimensions(), texture.info(), std::move(mtlTexture));
}


sk_sp<skgpu::Buffer> ResourceProvider::createBuffer(size_t size,
                                                    BufferType type,
                                                    PrioritizeGpuReads prioritizeGpuReads) {
    return Buffer::Make(this->mtlGpu(), size, type, prioritizeGpuReads);
}

} // namespace skgpu::mtl
