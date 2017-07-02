# Description:
#  Khronos reference front-end for GLSL and ESSL, and sample SPIR-V generator.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Mixed: BSD, MIT, Khronos, Apache 2.0

exports_files(["LICENSE"])

COMMON_COPTS = [
    "$(STACK_FRAME_UNLIMITED)",
    "-DGLSLANG_OSINCLUDE_UNIX",
    "-DAMD_EXTENSIONS",
    "-DENABLE_HLSL",
    "-DNV_EXTENSIONS",
]

cc_library(
    name = "glslang_headers",
    hdrs = [
        "OGLCompilersDLL/InitializeDll.h",
        "glslang/Include/InitializeGlobals.h",
        "glslang/Include/ResourceLimits.h",
        "glslang/MachineIndependent/Versions.h",
        "glslang/Public/ShaderLang.h",
    ],
    visibility = ["//visibility:private"],
)

cc_library(
    name = "glslang_internal_headers",
    hdrs = [
        "glslang/Include/BaseTypes.h",
        "glslang/Include/Common.h",
        "glslang/Include/ConstantUnion.h",
        "glslang/Include/InfoSink.h",
        "glslang/Include/PoolAlloc.h",
        "glslang/Include/ShHandle.h",
        "glslang/Include/Types.h",
        "glslang/Include/arrays.h",
        "glslang/Include/intermediate.h",
        "glslang/MachineIndependent/Initialize.h",
        "glslang/MachineIndependent/ParseHelper.h",
        "glslang/MachineIndependent/Scan.h",
        "glslang/MachineIndependent/SymbolTable.h",
        "glslang/MachineIndependent/localintermediate.h",
        "glslang/MachineIndependent/parseVersions.h",
        "glslang/MachineIndependent/preprocessor/PpContext.h",
        "glslang/MachineIndependent/preprocessor/PpTokens.h",
    ],
    deps = ["glslang_headers"],
    copts = COMMON_COPTS,
    visibility = ["//visibility:private"],
)

config_setting(
    name = "windows",
    values = {
        "cpu": "x64_windows_msvc",
    },
)

cc_library(
    name = "OSDependent",
    srcs = select({
        ":windows": [
            "glslang/OSDependent/Windows/ossource.cpp",
        ],
        "//conditions:default": [
            "glslang/OSDependent/Unix/ossource.cpp",
        ],
    }),
    hdrs = ["glslang/OSDependent/osinclude.h"],
    deps = [":glslang_headers"],
    copts = COMMON_COPTS,
    linkopts = select({
        ":windows": [],
        "//conditions:default": [
            "-lpthread",
        ],
    }),
    visibility = ["//visibility:private"],
)

cc_library(
    name = "OGLCompiler",
    srcs = ["OGLCompilersDLL/InitializeDll.cpp"],
    deps = [
        ":OSDependent",
        ":glslang_headers",
    ],
    copts = COMMON_COPTS,
    visibility = ["//visibility:private"],
)

cc_library(
    name = "glslang",
    srcs = [
        "glslang/GenericCodeGen/CodeGen.cpp",
        "glslang/GenericCodeGen/Link.cpp",
        "glslang/MachineIndependent/Constant.cpp",
        "glslang/MachineIndependent/InfoSink.cpp",
        "glslang/MachineIndependent/Initialize.cpp",
        "glslang/MachineIndependent/IntermTraverse.cpp",
        "glslang/MachineIndependent/Intermediate.cpp",
        "glslang/MachineIndependent/ParseContextBase.cpp",
        "glslang/MachineIndependent/ParseHelper.cpp",
        "glslang/MachineIndependent/PoolAlloc.cpp",
        "glslang/MachineIndependent/RemoveTree.cpp",
        "glslang/MachineIndependent/Scan.cpp",
        "glslang/MachineIndependent/ShaderLang.cpp",
        "glslang/MachineIndependent/SymbolTable.cpp",
        "glslang/MachineIndependent/Versions.cpp",
        "glslang/MachineIndependent/glslang_tab.cpp",
        "glslang/MachineIndependent/glslang_tab.cpp.h",
        "glslang/MachineIndependent/intermOut.cpp",
        "glslang/MachineIndependent/iomapper.cpp",
        "glslang/MachineIndependent/limits.cpp",
        "glslang/MachineIndependent/linkValidate.cpp",
        "glslang/MachineIndependent/parseConst.cpp",
        "glslang/MachineIndependent/preprocessor/Pp.cpp",
        "glslang/MachineIndependent/preprocessor/PpAtom.cpp",
        "glslang/MachineIndependent/preprocessor/PpContext.cpp",
        "glslang/MachineIndependent/preprocessor/PpScanner.cpp",
        "glslang/MachineIndependent/preprocessor/PpTokens.cpp",
        "glslang/MachineIndependent/propagateNoContraction.cpp",
        "glslang/MachineIndependent/reflection.cpp",
    ],
    hdrs = [
        "glslang/Include/revision.h",
        "glslang/MachineIndependent/LiveTraverser.h",
        "glslang/MachineIndependent/RemoveTree.h",
        "glslang/MachineIndependent/ScanContext.h",
        "glslang/MachineIndependent/gl_types.h",
        "glslang/MachineIndependent/iomapper.h",
        "glslang/MachineIndependent/propagateNoContraction.h",
        "glslang/MachineIndependent/reflection.h",
    ],
    copts = COMMON_COPTS + [
        # for glslang_tab.cc
        "-Iexternal/com_github_khronosgroup_glslang/glslang/MachineIndependent",
    ],
    deps = [
        ":HLSL",
        ":OGLCompiler",
        ":OSDependent",
        ":glslang_internal_headers",
    ],
)

cc_library(
    name = "SPIRV",
    srcs = [
        "SPIRV/GlslangToSpv.cpp",
        "SPIRV/InReadableOrder.cpp",
        "SPIRV/Logger.cpp",
        "SPIRV/SpvBuilder.cpp",
        "SPIRV/disassemble.cpp",
        "SPIRV/doc.cpp",
    ],
    hdrs = [
        "SPIRV/GLSL.ext.AMD.h",
        "SPIRV/GLSL.ext.KHR.h",
        "SPIRV/GLSL.ext.NV.h",
        "SPIRV/GLSL.std.450.h",
        "SPIRV/GlslangToSpv.h",
        "SPIRV/Logger.h",
        "SPIRV/SpvBuilder.h",
        "SPIRV/bitutils.h",
        "SPIRV/disassemble.h",
        "SPIRV/doc.h",
        "SPIRV/hex_float.h",
        "SPIRV/spirv.hpp",
        "SPIRV/spvIR.h",
    ],
    deps = [":glslang"],
    copts = COMMON_COPTS + select({
        ":windows": [],
        "//conditions:default": ["-Wno-gnu-redeclared-enum"],
    }),
)

cc_library(
    name = "SPVRemapper",
    srcs = ["SPIRV/SPVRemapper.cpp"],
    hdrs = ["SPIRV/SPVRemapper.h"],
    deps = [
        ":SPIRV",
        ":glslang",
    ],
)

cc_library(
    name = "HLSL",
    srcs = [
        "hlsl/hlslAttributes.cpp",
        "hlsl/hlslGrammar.cpp",
        "hlsl/hlslOpMap.cpp",
        "hlsl/hlslParseHelper.cpp",
        "hlsl/hlslParseables.cpp",
        "hlsl/hlslScanContext.cpp",
        "hlsl/hlslTokenStream.cpp",
    ],
    hdrs = [
        "hlsl/hlslAttributes.h",
        "hlsl/hlslGrammar.h",
        "hlsl/hlslOpMap.h",
        "hlsl/hlslParseHelper.h",
        "hlsl/hlslParseables.h",
        "hlsl/hlslScanContext.h",
        "hlsl/hlslTokenStream.h",
        "hlsl/hlslTokens.h",
    ],
    deps = [
        ":OSDependent",
        ":glslang_internal_headers",
    ],
    copts = COMMON_COPTS,
    visibility = ["//visibility:private"],
)

cc_library(
    name = "glslang-default-resource-limits",
    srcs = ["StandAlone/ResourceLimits.cpp"],
    hdrs = ["StandAlone/ResourceLimits.h"],
    deps = [
        ":glslang",
        ":glslang_headers",
    ],
    copts = COMMON_COPTS + [
        "-Ithird_party/glslang",  # for glslang/Include/ResourceLimits.h
    ],
    visibility = ["//visibility:private"],
)

cc_binary(
    name = "glslangValidator",
    srcs = [
        "StandAlone/StandAlone.cpp",
        "StandAlone/Worklist.h",
    ],
    deps = [
        ":HLSL",
        ":OGLCompiler",
        ":OSDependent",
        ":SPIRV",
        ":SPVRemapper",
        ":glslang",
        ":glslang-default-resource-limits",
        ":glslang_headers",
    ],
    copts = COMMON_COPTS + [
        "-Ithird_party/glslang",  # for glslang/Include/ResourceLimits.h
    ],
)

cc_binary(
    name = "spirv-remap",
    srcs = [
        "StandAlone/spirv-remap.cpp",
    ],
    deps = [
        ":HLSL",
        ":OGLCompiler",
        ":OSDependent",
        ":SPIRV",
        ":SPVRemapper",
        ":glslang",
        ":glslang-default-resource-limits",
        ":glslang_headers",
    ],
    copts = COMMON_COPTS,
)
