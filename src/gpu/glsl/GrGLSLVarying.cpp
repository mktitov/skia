/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"

void GrGLSLVaryingHandler::addPassThroughAttribute(const GrShaderVar& vsVar,
                                                   const char* output,
                                                   Interpolation interpolation) {
    SkASSERT(vsVar.getType() != kVoid_GrSLType);
    GrGLSLVarying v(vsVar.getType());
    this->addVarying(vsVar.c_str(), &v, interpolation);
    fProgramBuilder->fVS.codeAppendf("%s = %s;", v.vsOut(), vsVar.c_str());
    fProgramBuilder->fFS.codeAppendf("%s = %s;", output, v.fsIn());
}

static bool use_flat_interpolation(GrGLSLVaryingHandler::Interpolation interpolation,
                                   const GrShaderCaps& shaderCaps) {
    switch (interpolation) {
        using Interpolation = GrGLSLVaryingHandler::Interpolation;
        case Interpolation::kInterpolated:
            return false;
        case Interpolation::kCanBeFlat:
            SkASSERT(!shaderCaps.preferFlatInterpolation() ||
                     shaderCaps.flatInterpolationSupport());
            return shaderCaps.preferFlatInterpolation();
        case Interpolation::kMustBeFlat:
            SkASSERT(shaderCaps.flatInterpolationSupport());
            return true;
    }
    SK_ABORT("Invalid interpolation");
}

void GrGLSLVaryingHandler::addVarying(const char* name, GrGLSLVarying* varying,
                                      Interpolation interpolation) {
    SkASSERT(GrSLTypeIsFloatType(varying->type()) || Interpolation::kMustBeFlat == interpolation);
    VaryingInfo& v = fVaryings.push_back();

    SkASSERT(varying);
    SkASSERT(kVoid_GrSLType != varying->fType);
    v.fType = varying->fType;
    v.fIsFlat = use_flat_interpolation(interpolation, *fProgramBuilder->shaderCaps());
    v.fVsOut = fProgramBuilder->nameVariable('v', name);
    v.fVisibility = kNone_GrShaderFlags;
    if (varying->isInVertexShader()) {
        varying->fVsOut = v.fVsOut.c_str();
        v.fVisibility |= kVertex_GrShaderFlag;
    }
    if (varying->isInFragmentShader()) {
        varying->fFsIn = v.fVsOut.c_str();
        v.fVisibility |= kFragment_GrShaderFlag;
    }
}

void GrGLSLVaryingHandler::emitAttributes(const GrGeometryProcessor& gp) {
    for (const auto& attr : gp.vertexAttributes()) {
        this->addAttribute(attr.asShaderVar());
    }
    for (const auto& attr : gp.instanceAttributes()) {
        this->addAttribute(attr.asShaderVar());
    }
}

void GrGLSLVaryingHandler::addAttribute(const GrShaderVar& var) {
    SkASSERT(GrShaderVar::TypeModifier::In == var.getTypeModifier());
    for (const GrShaderVar& attr : fVertexInputs.items()) {
        // if attribute already added, don't add it again
        if (attr.getName().equals(var.getName())) {
            return;
        }
    }
    fVertexInputs.push_back(var);
}

void GrGLSLVaryingHandler::setNoPerspective() {
    const GrShaderCaps& caps = *fProgramBuilder->shaderCaps();
    if (!caps.noperspectiveInterpolationSupport()) {
        return;
    }
    if (const char* extension = caps.noperspectiveInterpolationExtensionString()) {
        int bit = 1 << GrGLSLShaderBuilder::kNoPerspectiveInterpolation_GLSLPrivateFeature;
        fProgramBuilder->fVS.addFeature(bit, extension);
        fProgramBuilder->fFS.addFeature(bit, extension);
    }
    fDefaultInterpolationModifier = "noperspective";
}

void GrGLSLVaryingHandler::finalize() {
    for (const VaryingInfo& v : fVaryings.items()) {
        const char* modifier = v.fIsFlat ? "flat" : fDefaultInterpolationModifier;
        if (v.fVisibility & kVertex_GrShaderFlag) {
            fVertexOutputs.emplace_back(v.fVsOut, v.fType, GrShaderVar::TypeModifier::Out,
                                        GrShaderVar::kNonArray, SkString(), SkString(modifier));
        }
        if (v.fVisibility & kFragment_GrShaderFlag) {
            const char* fsIn = v.fVsOut.c_str();
            fFragInputs.emplace_back(SkString(fsIn), v.fType, GrShaderVar::TypeModifier::In,
                                     GrShaderVar::kNonArray, SkString(), SkString(modifier));
        }
    }
    this->onFinalize();
}

void GrGLSLVaryingHandler::appendDecls(const VarArray& vars, SkString* out) const {
    for (const GrShaderVar& varying : vars.items()) {
        varying.appendDecl(fProgramBuilder->shaderCaps(), out);
        out->append(";");
    }
}

void GrGLSLVaryingHandler::getVertexDecls(SkString* inputDecls, SkString* outputDecls) const {
    this->appendDecls(fVertexInputs, inputDecls);
    this->appendDecls(fVertexOutputs, outputDecls);
}

void GrGLSLVaryingHandler::getFragDecls(SkString* inputDecls, SkString* outputDecls) const {
    // We should not have any outputs in the fragment shader when using version 1.10
    SkASSERT(SkSL::GLSLGeneration::k110 != fProgramBuilder->shaderCaps()->generation() ||
             fFragOutputs.empty());
    this->appendDecls(fFragInputs, inputDecls);
    this->appendDecls(fFragOutputs, outputDecls);
}
