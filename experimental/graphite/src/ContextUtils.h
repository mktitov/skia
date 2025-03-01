/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ContextUtils_DEFINED
#define skgpu_ContextUtils_DEFINED

#include "experimental/graphite/include/Context.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTileMode.h"

namespace skgpu {

class PaintParams;
class Uniform;
class UniformCache;

// A single, fully specified combination resulting from a PaintCombo (i.e., it corresponds to a
// specific skgpu::PaintParams object (a subset of SkPaint))
struct Combination {
    bool operator==(const Combination& other) const {
        return fShaderType == other.fShaderType &&
               fTileMode == other.fTileMode &&
               fBlendMode == other.fBlendMode;
    }

    uint32_t key() const {
        return (static_cast<int>(fShaderType) << 9) | // 6 values  -> 3 bits
               (static_cast<int>(fTileMode)   << 7) | // 4 values  -> 2 bits
               (static_cast<int>(fBlendMode)  << 2);  // 29 values -> 5 bits
    }

    ShaderCombo::ShaderType fShaderType = ShaderCombo::ShaderType::kNone;
    // Tile mode and blend mode are ignored if shader type is kNone; tile mode is ignored if
    // shader type is kSolidColor.
    SkTileMode fTileMode = SkTileMode::kClamp;
    SkBlendMode fBlendMode = SkBlendMode::kSrc;
};

class UniformData : public SkRefCnt {
public:
    static constexpr uint32_t kInvalidUniformID = 0;

    // TODO: should we require a name (e.g., "gradient_uniforms") for each uniform block so
    // we can better name the Metal FS uniform struct?
    static sk_sp<UniformData> Make(int count,
                                   const Uniform* uniforms,
                                   size_t dataSize);

    ~UniformData() override {
        // TODO: fOffsets and fData should just be allocated right after UniformData in an arena
        delete [] fOffsets;
        delete [] fData;
    }

    void setID(uint32_t id) {   // TODO: maybe make privileged for only UniformCache
        SkASSERT(fID == kInvalidUniformID);
        fID = id;
    }
    uint32_t id() const { return fID; }
    int count() const { return fCount; }
    const Uniform* uniforms() const { return fUniforms; }
    uint32_t* offsets() { return fOffsets; }
    uint32_t offset(int index) {
        SkASSERT(index >= 0 && index < fCount);
        return fOffsets[index];
    }
    char* data() { return fData; }
    size_t dataSize() const { return fDataSize; }

private:
    UniformData(int count,
                const Uniform* uniforms,
                uint32_t* offsets,
                char* data,
                size_t dataSize)
            : fCount(count)
            , fUniforms(uniforms)
            , fOffsets(offsets)
            , fData(data)
            , fDataSize(dataSize) {
    }

    uint32_t fID = kInvalidUniformID;
    const int fCount;
    const Uniform* fUniforms;
    uint32_t* fOffsets; // offset of each uniform in 'fData'
    char* fData;
    const size_t fDataSize;
};

std::tuple<Combination, sk_sp<UniformData>> ExtractCombo(UniformCache*, const PaintParams&);
std::string GetMSLUniformStruct(ShaderCombo::ShaderType);

} // namespace skgpu

#endif // skgpu_ContextUtils_DEFINED
