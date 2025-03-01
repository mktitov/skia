/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkStream.h"
#include "src/sksl/codegen/SkVMDebugInfo.h"
#include "tests/Test.h"

DEF_TEST(SkVMDebugInfoSetSource, r) {
    SkSL::SkVMDebugInfo i;
    i.setSource("SkVMDebugInfo::setSource unit test\n"
                "\t// first line\n"
                "\t// second line\n"
                "\t// third line");

    REPORTER_ASSERT(r, i.fSource.size() == 4);
    REPORTER_ASSERT(r, i.fSource[0] == "SkVMDebugInfo::setSource unit test");
    REPORTER_ASSERT(r, i.fSource[1] == "\t// first line");
    REPORTER_ASSERT(r, i.fSource[2] == "\t// second line");
    REPORTER_ASSERT(r, i.fSource[3] == "\t// third line");
}

DEF_TEST(SkVMDebugInfoWriteTrace, r) {
    SkSL::SkVMDebugInfo i;
    i.fSource = {
        "\t// first line",
        "// \"second line\"",
        "//\\\\//\\\\ third line",
    };
    i.fSlotInfo = {
        {"SkVM_Debug_Info", 1, 2, 3, (SkSL::Type::NumberKind)4, 5},
        {"Unit_Test",       6, 7, 8, (SkSL::Type::NumberKind)9, 10},
    };
    i.fFuncInfo = {
        {"void testFunc();"},
    };
    SkDynamicMemoryWStream wstream;
    i.writeTrace(&wstream);
    sk_sp<SkData> trace = wstream.detachAsData();

    static constexpr char kExpected[] =
            R"({"source":["\t// first line","// \"second line\"","//\\\\//\\\\ third line"],"s)"
            R"(lots":[{"slot":0,"name":"SkVM_Debug_Info","columns":1,"rows":2,"index":3,"kind")"
            R"(:4,"line":5},{"slot":1,"name":"Unit_Test","columns":6,"rows":7,"index":8,"kind")"
            R"(:9,"line":10}],"functions":[{"slot":0,"name":"void testFunc();"}]})";

    skstd::string_view actual{reinterpret_cast<const char*>(trace->bytes()), trace->size()};

    REPORTER_ASSERT(r, actual == kExpected,
                    "Expected:\n    %s\n\n  Actual:\n    %.*s\n",
                    kExpected, (int)actual.size(), actual.data());
}

DEF_TEST(SkVMDebugInfoReadTrace, r) {
    const skstd::string_view kJSONTrace =
            R"({"source":["\t// first line","// \"second line\"","//\\\\//\\\\ third line"],"s)"
            R"(lots":[{"slot":0,"name":"SkVM_Debug_Info","columns":1,"rows":2,"index":3,"kind")"
            R"(:4,"line":5},{"slot":1,"name":"Unit_Test","columns":6,"rows":7,"index":8,"kind")"
            R"(:9,"line":10}],"functions":[{"slot":0,"name":"void testFunc();"}]})";

    SkMemoryStream stream(kJSONTrace.data(), kJSONTrace.size(), /*copyData=*/false);
    SkSL::SkVMDebugInfo i;
    REPORTER_ASSERT(r, i.readTrace(&stream));

    REPORTER_ASSERT(r, i.fSource.size() == 3);
    REPORTER_ASSERT(r, i.fSlotInfo.size() == 2);
    REPORTER_ASSERT(r, i.fFuncInfo.size() == 1);

    REPORTER_ASSERT(r, i.fSource[0] == "\t// first line");
    REPORTER_ASSERT(r, i.fSource[1] == "// \"second line\"");
    REPORTER_ASSERT(r, i.fSource[2] == "//\\\\//\\\\ third line");

    REPORTER_ASSERT(r, i.fSlotInfo[0].name == "SkVM_Debug_Info");
    REPORTER_ASSERT(r, i.fSlotInfo[0].columns == 1);
    REPORTER_ASSERT(r, i.fSlotInfo[0].rows == 2);
    REPORTER_ASSERT(r, i.fSlotInfo[0].componentIndex == 3);
    REPORTER_ASSERT(r, i.fSlotInfo[0].numberKind == (SkSL::Type::NumberKind)4);
    REPORTER_ASSERT(r, i.fSlotInfo[0].line == 5);

    REPORTER_ASSERT(r, i.fSlotInfo[1].name == "Unit_Test");
    REPORTER_ASSERT(r, i.fSlotInfo[1].columns == 6);
    REPORTER_ASSERT(r, i.fSlotInfo[1].rows == 7);
    REPORTER_ASSERT(r, i.fSlotInfo[1].componentIndex == 8);
    REPORTER_ASSERT(r, i.fSlotInfo[1].numberKind == (SkSL::Type::NumberKind)9);
    REPORTER_ASSERT(r, i.fSlotInfo[1].line == 10);

    REPORTER_ASSERT(r, i.fFuncInfo[0].name == "void testFunc();");
}
