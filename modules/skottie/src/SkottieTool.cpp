/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "include/private/SkTPin.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/utils/SkottieUtils.h"
#include "modules/skresources/include/SkResources.h"
#include "src/core/SkOSFile.h"
#include "src/core/SkTaskGroup.h"
#include "src/utils/SkOSPath.h"
#include "tools/flags/CommandLineFlags.h"

#include "gzip/decompress.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <future>
#include <numeric>
#include <vector>

static DEFINE_string2(input,  i, nullptr, "Input .tgs file.");
static DEFINE_string2(out,    w, nullptr, "Output file.");

static DEFINE_double(t0,       0, "Timeline start [0..1].");
static DEFINE_double(quality, 92, "Webp quality rate");

static DEFINE_int(width , 128, "Render width.");
static DEFINE_int(height, 128, "Render height.");
static DEFINE_int(threads,  0, "Number of worker threads (0 -> cores count).");

static DEFINE_bool(compression, false, "Webp compression");

namespace {

static constexpr SkColor kClearColor = SK_ColorTRANSPARENT;

class Sink {
public:
    Sink() = default;
    virtual ~Sink() = default;
    Sink(const Sink&) = delete;
    Sink& operator=(const Sink&) = delete;

    virtual SkCanvas* beginFrame() = 0;
    virtual bool endFrame() = 0;
};

class WEBPSink final : public Sink {
public:
    static std::unique_ptr<Sink> Make(const SkMatrix& scale_matrix, const SkWebpEncoder::Options& opt) {
        auto surface = SkSurface::MakeRasterN32Premul(FLAGS_width, FLAGS_height);
        if (!surface) {
            SkDebugf("Could not allocate a %d x %d surface.\n", FLAGS_width, FLAGS_height);
            return nullptr;
        }

        return std::unique_ptr<Sink>(new WEBPSink(std::move(surface), scale_matrix, opt));
    }

private:
    WEBPSink(sk_sp<SkSurface> surface, const SkMatrix& scale_matrix, const SkWebpEncoder::Options& webp_options)
        : fSurface(std::move(surface))
        , options(std::move(webp_options)) {
        fSurface->getCanvas()->concat(scale_matrix);
    }

    SkCanvas* beginFrame() override {
        auto* canvas = fSurface->getCanvas();
        canvas->clear(kClearColor);
        return canvas;
    }

    bool endFrame() override {
        const auto stream = new SkFILEWStream(FLAGS_out[0]);
        if (!stream->isValid()) {
            return false;
        }

        sk_sp<SkImage> img = fSurface->makeImageSnapshot();
        SkPixmap pixmap;
        return img->peekPixels(&pixmap) && SkWebpEncoder::Encode(stream, pixmap, options);
    }

    const sk_sp<SkSurface> fSurface;
    const SkWebpEncoder::Options options;
};

static std::vector<std::promise<sk_sp<SkImage>>> gMP4Frames;

class Logger final : public skottie::Logger {
public:
    struct LogEntry {
        SkString fMessage,
                 fJSON;
    };

    void log(skottie::Logger::Level lvl, const char message[], const char json[]) override {
        auto& log = lvl == skottie::Logger::Level::kError ? fErrors : fWarnings;
        log.push_back({ SkString(message), json ? SkString(json) : SkString() });
    }

    void report() const {
        SkDebugf("Animation loaded with %zu error%s, %zu warning%s.\n",
                 fErrors.size(), fErrors.size() == 1 ? "" : "s",
                 fWarnings.size(), fWarnings.size() == 1 ? "" : "s");

        const auto& show = [](const LogEntry& log, const char prefix[]) {
            SkDebugf("%s%s", prefix, log.fMessage.c_str());
            if (!log.fJSON.isEmpty())
                SkDebugf(" : %s", log.fJSON.c_str());
            SkDebugf("\n");
        };

        for (const auto& err : fErrors)   show(err, "  !! ");
        for (const auto& wrn : fWarnings) show(wrn, "  ?? ");
    }

private:
    std::vector<LogEntry> fErrors,
                          fWarnings;
};

} // namespace

extern bool gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental;

int main(int argc, char** argv) {

    CommandLineFlags::Parse(argc, argv);

    gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental = true;
    SkAutoGraphics ag;

    if (FLAGS_input.isEmpty() || FLAGS_out.isEmpty()) {
        SkDebugf("Missing required 'input' and 'out' args.\n");
        return 1;
    }

    std::ifstream stream(FLAGS_input[0], std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open()) {
        SkDebugf("could not open: %s.\n'", FLAGS_input[0]);
        return 1;
    }
    std::string buffer_compressed((std::istreambuf_iterator<char>(stream.rdbuf())),
                                   std::istreambuf_iterator<char>());
    stream.close();

    const auto lottieBytes = gzip::decompress(buffer_compressed.c_str(), buffer_compressed.size());
    const auto byteStream = SkMemoryStream::MakeDirect(lottieBytes.data(), lottieBytes.size());
    const auto data = byteStream->asData();

    if (!data) {
        SkDebugf("Could not load %s.\n", FLAGS_input[0]);
        return 1;
    }

    // auto logger = sk_make_sp<Logger>();

    // Instantiate an animation on the main thread for two reasons:
    //   - we need to know its duration upfront
    //   - we want to only report parsing errors once
    const auto anim = skottie::Animation::Builder()
                //   .setLogger(logger)
                  .make(static_cast<const char*>(data->data()), data->size());

    if (!anim) {
        SkDebugf("Could not parse animation: '%s'.\n", FLAGS_input[0]);
        return 1;
    }

    const auto scale_matrix = SkMatrix::RectToRect(SkRect::MakeSize(anim->size()),
                                                   SkRect::MakeIWH(FLAGS_width, FLAGS_height),
                                                   SkMatrix::kCenter_ScaleToFit);
    // logger->report();

    const auto t0 = SkTPin(FLAGS_t0, 0.0, 1.0),
       native_fps = anim->fps(),
           frame0 = anim->duration() * t0 * native_fps;

    SkWebpEncoder::Options options {
        .fCompression = FLAGS_compression ? SkWebpEncoder::Compression::kLossy : SkWebpEncoder::Compression::kLossless,
        .fQuality = static_cast<float>(FLAGS_quality),
    };
    static auto sink = WEBPSink::Make(scale_matrix, options);

    if (sink && anim) {
        anim->seekFrame(frame0);
        anim->render(sink->beginFrame());
        sink->endFrame();
    }

    return 0;
}
