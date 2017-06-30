# Description:
#  SwiftShader is a high-performance CPU-based implementation of the OpenGL ES
#  and Direct3D 9 graphics APIs. Its goal is to provide hardware independence
#  for advanced 3D graphics.

package(default_visibility = ["//visibility:private"])

licenses(["notice"])  # Apache 2.0

exports_files(["LICENSE.txt"])

COMMON_COPTS = [
    # non-win:
    "-Wno-sign-compare",
    "-Wno-inconsistent-missing-override",
    "-Wno-unneeded-internal-declaration",
    "-Wno-undefined-var-template",
    # win:
    #"/wd4201",  # nameless struct/union
    #"/wd4065",  # switch statement contains 'default' but no 'case' labels
    #"/wd4324",  # structure was padded due to alignment specifier
    #"/wd5030",  # attribute is not recognized

    "-x", "c++",

    "-Iexternal/com_github_google_swiftshader/include/",
    "-Iexternal/com_github_google_swiftshader/src/",
    "-Iexternal/com_github_google_swiftshader/src/Common/",

    "-Iexternal/com_github_google_swiftshader/third_party/subzero/pnacl-llvm/include/",
    "-Iexternal/com_github_google_swiftshader/third_party/llvm-subzero/include/",

    # android:
    #"-Iexternal/com_github_google_swiftshader/third_party/llvm-subzero/build/Android/include/",
    #"-Iexternal/com_github_google_swiftshader/third_party/llvm-subzero/lib/Support/Unix/",
    # linux:
    "-Iexternal/com_github_google_swiftshader/third_party/llvm-subzero/build/Linux/include/",
    "-Iexternal/com_github_google_swiftshader/third_party/llvm-subzero/lib/Support/Unix/",
    # mac:
    #"-Iexternal/com_github_google_swiftshader/third_party/llvm-subzero/build/MacOS/include/",
    #"-Iexternal/com_github_google_swiftshader/third_party/llvm-subzero/lib/Support/Unix/",
    # windows:
    #"-Iexternal/com_github_google_swiftshader/third_party/llvm-subzero/build/Windows/include/",
    #"-Iexternal/com_github_google_swiftshader/third_party/llvm-subzero/lib/Support/Windows/",

    "-ffunction-sections",
    "-fdata-sections",
    "-fomit-frame-pointer",
    "-fno-operator-names",
    "-fno-exceptions",
    "-fvisibility=protected",

    # Subzero:
    "-DALLOW_DUMP=0",
    "-DALLOW_TIMERS=0",
    "-DALLOW_LLVM_CL=0",
    "-DALLOW_LLVM_IR=0",
    "-DALLOW_LLVM_IR_AS_INPUT=0",
    "-DALLOW_MINIMAL_BUILD=0",
    "-DALLOW_WASM=0",
    "-DICE_THREAD_LOCAL_HACK=0",

    # arm32:
    # "-DSZTARGET=ARM32",
    # x32:
    # "-DSZTARGET=X8632",
    # x64:
    "-DSZTARGET=X8664",

    # win:
    # "-DSUBZERO_USE_MICROSOFT_ABI",
]

COMMON_DEFINES = [
    "ANGLE_DISABLE_TRACE",
]

COMMON_LINKOPTS = [
]

cc_library(
    name = "gl_headers",
    hdrs = [
        "include/EGL/egl.h",
        "include/EGL/eglext.h",
        "include/EGL/eglplatform.h",
        "include/GLES/gl.h",
        "include/GLES/glext.h",
        "include/GLES/glplatform.h",
        "include/GLES2/gl2.h",
        "include/GLES2/gl2ext.h",
        "include/GLES2/gl2platform.h",
        "include/GLES3/gl3.h",
        "include/GLES3/gl3platform.h",
        "include/KHR/khrplatform.h",
    ],
)

cc_library(
    name = "common",
    srcs = [
        "src/Common/CPUID.cpp",
        "src/Common/Configurator.cpp",
        "src/Common/Debug.cpp",
        "src/Common/Half.cpp",
        "src/Common/Math.cpp",
        "src/Common/Memory.cpp",
        "src/Common/Resource.cpp",
        "src/Common/Socket.cpp",
        "src/Common/Thread.cpp",
        "src/Common/Timer.cpp",
    ],
    hdrs = [
        "src/Common/CPUID.hpp",
        "src/Common/Configurator.hpp",
        "src/Common/Debug.hpp",
        "src/Common/Half.hpp",
        "src/Common/Math.hpp",
        "src/Common/Memory.hpp",
        "src/Common/MutexLock.hpp",
        "src/Common/Resource.hpp",
        "src/Common/SharedLibrary.hpp",
        "src/Common/Socket.hpp",
        "src/Common/Thread.hpp",
        "src/Common/Timer.hpp",
        "src/Common/Types.hpp",
        "src/Common/Version.h",
    ],
    copts = COMMON_COPTS + [
        "-DLOG_TAG=\"swiftshader_common\"",
    ],
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS,
)

cc_library(
    name = "main_config",
    srcs = [
        "src/Main/Config.cpp",
    ],
    hdrs = [
        "src/Main/Config.hpp",
    ],
    deps = [
        ":common",
    ],
    copts = COMMON_COPTS + [
        "-DLOG_TAG=\"swiftshader_main\"",

        "-Iexternal/com_github_google_swiftshader/src/Common/",
    ],
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS,
)

cc_library(
    name = "main",
    srcs = [
        "src/Main/FrameBuffer.cpp",
        "src/Main/SwiftConfig.cpp",
        "src/Renderer/Color.hpp",
        "src/Renderer/Surface.hpp",

        # android:
        # "src/Main/FrameBufferAndroid.cpp",

        # win:
        # "src/Main/FrameBufferDD.cpp",
        # "src/Main/FrameBufferGDI.cpp",
        # "src/Main/FrameBufferWin.cpp",

        # osx:
        # "src/Main/FrameBufferOSX.mm",

        # linux:
        "src/Main/FrameBufferX11.cpp",
        "src/Main/libX11.cpp",
    ],
    hdrs = [
        "src/Main/Config.hpp",
        "src/Main/FrameBuffer.hpp",
        "src/Main/SwiftConfig.hpp",

        # android:
        # "src/Main/FrameBufferAndroid.hpp",

        # win:
        # "src/Main/FrameBufferDD.hpp",
        # "src/Main/FrameBufferGDI.hpp",
        # "src/Main/FrameBufferWin.hpp",

        # osx:
        # "src/Main/FrameBufferOSX.hpp",

        # linux:
        "src/Main/FrameBufferX11.hpp",
        "src/Main/libX11.hpp",
    ],
    deps = [
        ":common",
        ":main_config",
        ":reactor",
    ],
    copts = COMMON_COPTS + [
        "-DLOG_TAG=\"swiftshader_main\"",

        "-Iexternal/com_github_google_swiftshader/src/Common/",
    ],
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS + [
        # win:
        # dxguid.lib  # For FrameBufferDD

        # osx:
        # Quartz.framework, Cocoa.framework
    ],
)

cc_library(
    name = "shader",
    srcs = [
        "src/Shader/Constants.cpp",
        "src/Shader/PixelPipeline.cpp",
        "src/Shader/PixelProgram.cpp",
        "src/Shader/PixelRoutine.cpp",
        "src/Shader/PixelShader.cpp",
        "src/Shader/SamplerCore.cpp",
        "src/Shader/SetupRoutine.cpp",
        "src/Shader/Shader.cpp",
        "src/Shader/ShaderCore.cpp",
        "src/Shader/VertexPipeline.cpp",
        "src/Shader/VertexProgram.cpp",
        "src/Shader/VertexRoutine.cpp",
        "src/Shader/VertexShader.cpp",
    ],
    hdrs = [
        "src/Shader/Constants.hpp",
        "src/Shader/PixelPipeline.hpp",
        "src/Shader/PixelProgram.hpp",
        "src/Shader/PixelRoutine.hpp",
        "src/Shader/PixelShader.hpp",
        "src/Shader/SamplerCore.hpp",
        "src/Shader/SetupRoutine.hpp",
        "src/Shader/Shader.hpp",
        "src/Shader/ShaderCore.hpp",
        "src/Shader/VertexPipeline.hpp",
        "src/Shader/VertexProgram.hpp",
        "src/Shader/VertexRoutine.hpp",
        "src/Shader/VertexShader.hpp",
    ],
    deps = [
        ":common",
        ":main",
        ":main_config",
        ":renderer_hdrs",
    ],
    copts = COMMON_COPTS + [
        "-DLOG_TAG=\"swiftshader_shader\"",

        "-Iexternal/com_github_google_swiftshader/src/Main/",
        "-Iexternal/com_github_google_swiftshader/src/Renderer/",
        "-Iexternal/com_github_google_swiftshader/src/Shader/",
    ],
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS,
)

cc_library(
    name = "llvm_subzero",
    srcs = [
        "third_party/llvm-subzero/lib/Demangle/ItaniumDemangle.cpp",
        "third_party/llvm-subzero/lib/Support/APInt.cpp",
        "third_party/llvm-subzero/lib/Support/Atomic.cpp",
        "third_party/llvm-subzero/lib/Support/CommandLine.cpp",
        "third_party/llvm-subzero/lib/Support/ConvertUTF.cpp",
        "third_party/llvm-subzero/lib/Support/ConvertUTFWrapper.cpp",
        "third_party/llvm-subzero/lib/Support/Debug.cpp",
        "third_party/llvm-subzero/lib/Support/Errno.cpp",
        "third_party/llvm-subzero/lib/Support/ErrorHandling.cpp",
        "third_party/llvm-subzero/lib/Support/FoldingSet.cpp",
        "third_party/llvm-subzero/lib/Support/Hashing.cpp",
        "third_party/llvm-subzero/lib/Support/Host.cpp",
        "third_party/llvm-subzero/lib/Support/ManagedStatic.cpp",
        "third_party/llvm-subzero/lib/Support/MemoryBuffer.cpp",
        "third_party/llvm-subzero/lib/Support/Mutex.cpp",
        "third_party/llvm-subzero/lib/Support/NativeFormatting.cpp",
        "third_party/llvm-subzero/lib/Support/Path.cpp",
        "third_party/llvm-subzero/lib/Support/Process.cpp",
        "third_party/llvm-subzero/lib/Support/Program.cpp",
        "third_party/llvm-subzero/lib/Support/Regex.cpp",
        "third_party/llvm-subzero/lib/Support/Signals.cpp",
        "third_party/llvm-subzero/lib/Support/SmallPtrSet.cpp",
        "third_party/llvm-subzero/lib/Support/SmallVector.cpp",
        "third_party/llvm-subzero/lib/Support/StringExtras.cpp",
        "third_party/llvm-subzero/lib/Support/StringMap.cpp",
        "third_party/llvm-subzero/lib/Support/StringRef.cpp",
        "third_party/llvm-subzero/lib/Support/StringSaver.cpp",
        "third_party/llvm-subzero/lib/Support/TargetParser.cpp",
        "third_party/llvm-subzero/lib/Support/Threading.cpp",
        "third_party/llvm-subzero/lib/Support/Timer.cpp",
        "third_party/llvm-subzero/lib/Support/Triple.cpp",
        "third_party/llvm-subzero/lib/Support/Twine.cpp",
        "third_party/llvm-subzero/lib/Support/circular_raw_ostream.cpp",
        "third_party/llvm-subzero/lib/Support/raw_os_ostream.cpp",
        "third_party/llvm-subzero/lib/Support/raw_ostream.cpp",
        "third_party/llvm-subzero/lib/Support/regerror.c",
        "third_party/llvm-subzero/lib/Support/regex_impl.h",
        "third_party/llvm-subzero/lib/Support/regfree.c",
        "third_party/llvm-subzero/lib/Support/regstrlcpy.c",
    ],
    hdrs = [
        "third_party/subzero/pnacl-llvm/include/llvm/Bitcode/NaCl/NaClBitcodeDefs.h",
        "third_party/subzero/pnacl-llvm/include/llvm/Bitcode/NaCl/NaClBitcodeHeader.h",
        "third_party/subzero/pnacl-llvm/include/llvm/Bitcode/NaCl/NaClBitcodeParser.h",
        "third_party/subzero/pnacl-llvm/include/llvm/Bitcode/NaCl/NaClBitCodes.h",
        "third_party/subzero/pnacl-llvm/include/llvm/Bitcode/NaCl/NaClBitstreamReader.h",
        "third_party/subzero/pnacl-llvm/include/llvm/Bitcode/NaCl/NaClLLVMBitCodes.h",
        "third_party/subzero/pnacl-llvm/include/llvm/Support/DataStream.h",
        "third_party/subzero/pnacl-llvm/include/llvm/Support/MemoryObject.h",
        "third_party/subzero/pnacl-llvm/include/llvm/Support/StreamingMemoryObject.h",
        "third_party/llvm-subzero/include/llvm/ADT/APFloat.h",
        "third_party/llvm-subzero/include/llvm/ADT/APInt.h",
        "third_party/llvm-subzero/include/llvm/ADT/ArrayRef.h",
        "third_party/llvm-subzero/include/llvm/ADT/BitVector.h",
        "third_party/llvm-subzero/include/llvm/ADT/DenseMap.h",
        "third_party/llvm-subzero/include/llvm/ADT/DenseMapInfo.h",
        "third_party/llvm-subzero/include/llvm/ADT/EpochTracker.h",
        "third_party/llvm-subzero/include/llvm/ADT/FoldingSet.h",
        "third_party/llvm-subzero/include/llvm/ADT/Hashing.h",
        "third_party/llvm-subzero/include/llvm/ADT/IntrusiveRefCntPtr.h",
        "third_party/llvm-subzero/include/llvm/ADT/None.h",
        "third_party/llvm-subzero/include/llvm/ADT/Optional.h",
        "third_party/llvm-subzero/include/llvm/ADT/PointerIntPair.h",
        "third_party/llvm-subzero/include/llvm/ADT/PointerUnion.h",
        "third_party/llvm-subzero/include/llvm/ADT/SmallPtrSet.h",
        "third_party/llvm-subzero/include/llvm/ADT/SmallSet.h",
        "third_party/llvm-subzero/include/llvm/ADT/SmallString.h",
        "third_party/llvm-subzero/include/llvm/ADT/SmallVector.h",
        "third_party/llvm-subzero/include/llvm/ADT/Statistic.h",
        "third_party/llvm-subzero/include/llvm/ADT/StringExtras.h",
        "third_party/llvm-subzero/include/llvm/ADT/StringMap.h",
        "third_party/llvm-subzero/include/llvm/ADT/StringRef.h",
        "third_party/llvm-subzero/include/llvm/ADT/StringSwitch.h",
        "third_party/llvm-subzero/include/llvm/ADT/STLExtras.h",
        "third_party/llvm-subzero/include/llvm/ADT/Triple.h",
        "third_party/llvm-subzero/include/llvm/ADT/Twine.h",
        "third_party/llvm-subzero/include/llvm/ADT/edit_distance.h",
        "third_party/llvm-subzero/include/llvm/ADT/ilist.h",
        "third_party/llvm-subzero/include/llvm/ADT/ilist_base.h",
        "third_party/llvm-subzero/include/llvm/ADT/ilist_iterator.h",
        "third_party/llvm-subzero/include/llvm/ADT/ilist_node.h",
        "third_party/llvm-subzero/include/llvm/ADT/ilist_node_base.h",
        "third_party/llvm-subzero/include/llvm/ADT/ilist_node_options.h",
        "third_party/llvm-subzero/include/llvm/ADT/iterator.h",
        "third_party/llvm-subzero/include/llvm/ADT/iterator_range.h",
        "third_party/llvm-subzero/include/llvm/ADT/simple_ilist.h",
        "third_party/llvm-subzero/include/llvm/Demangle/Demangle.h",
        "third_party/llvm-subzero/include/llvm/IR/Argument.h",
        "third_party/llvm-subzero/include/llvm/IR/Attributes.h",
        "third_party/llvm-subzero/include/llvm/IR/BasicBlock.h",
        "third_party/llvm-subzero/include/llvm/IR/CallingConv.h",
        "third_party/llvm-subzero/include/llvm/IR/Constant.h",
        "third_party/llvm-subzero/include/llvm/IR/DebugLoc.h",
        "third_party/llvm-subzero/include/llvm/IR/DerivedTypes.h",
        "third_party/llvm-subzero/include/llvm/IR/Function.h",
        "third_party/llvm-subzero/include/llvm/IR/GlobalObject.h",
        "third_party/llvm-subzero/include/llvm/IR/GlobalValue.h",
        "third_party/llvm-subzero/include/llvm/IR/Instruction.h",
        "third_party/llvm-subzero/include/llvm/IR/Intrinsics.h",
        "third_party/llvm-subzero/include/llvm/IR/LLVMContext.h",
        "third_party/llvm-subzero/include/llvm/IR/Metadata.h",
        "third_party/llvm-subzero/include/llvm/IR/OperandTraits.h",
        "third_party/llvm-subzero/include/llvm/IR/SymbolTableListTraits.h",
        "third_party/llvm-subzero/include/llvm/IR/TrackingMDRef.h",
        "third_party/llvm-subzero/include/llvm/IR/Type.h",
        "third_party/llvm-subzero/include/llvm/IR/Use.h",
        "third_party/llvm-subzero/include/llvm/IR/User.h",
        "third_party/llvm-subzero/include/llvm/IR/Value.h",
        "third_party/llvm-subzero/include/llvm/IRReader/IRReader.h",
        "third_party/llvm-subzero/include/llvm/Support/AlignOf.h",
        "third_party/llvm-subzero/include/llvm/Support/Allocator.h",
        "third_party/llvm-subzero/include/llvm/Support/ARMBuildAttributes.h",
        "third_party/llvm-subzero/include/llvm/Support/Atomic.h",
        "third_party/llvm-subzero/include/llvm/Support/Casting.h",
        "third_party/llvm-subzero/include/llvm/Support/CBindingWrapping.h",
        "third_party/llvm-subzero/include/llvm/Support/Chrono.h",
        "third_party/llvm-subzero/include/llvm/Support/CommandLine.h",
        "third_party/llvm-subzero/include/llvm/Support/Compiler.h",
        "third_party/llvm-subzero/include/llvm/Support/ConvertUTF.h",
        "third_party/llvm-subzero/include/llvm/Support/COFF.h",
        "third_party/llvm-subzero/include/llvm/Support/DataStream.h",
        "third_party/llvm-subzero/include/llvm/Support/Debug.h",
        "third_party/llvm-subzero/include/llvm/Support/ELF.h",
        "third_party/llvm-subzero/include/llvm/Support/Endian.h",
        "third_party/llvm-subzero/include/llvm/Support/Errc.h",
        "third_party/llvm-subzero/include/llvm/Support/Errno.h",
        "third_party/llvm-subzero/include/llvm/Support/Error.h",
        "third_party/llvm-subzero/include/llvm/Support/ErrorHandling.h",
        "third_party/llvm-subzero/include/llvm/Support/ErrorOr.h",
        "third_party/llvm-subzero/include/llvm/Support/FileSystem.h",
        "third_party/llvm-subzero/include/llvm/Support/FileUtilities.h",
        "third_party/llvm-subzero/include/llvm/Support/Format.h",
        "third_party/llvm-subzero/include/llvm/Support/FormatCommon.h",
        "third_party/llvm-subzero/include/llvm/Support/FormatProviders.h",
        "third_party/llvm-subzero/include/llvm/Support/FormatVariadic.h",
        "third_party/llvm-subzero/include/llvm/Support/FormatVariadicDetails.h",
        "third_party/llvm-subzero/include/llvm/Support/Host.h",
        "third_party/llvm-subzero/include/llvm/Support/MD5.h",
        "third_party/llvm-subzero/include/llvm/Support/MachO.h",
        "third_party/llvm-subzero/include/llvm/Support/ManagedStatic.h",
        "third_party/llvm-subzero/include/llvm/Support/MathExtras.h",
        "third_party/llvm-subzero/include/llvm/Support/Memory.h",
        "third_party/llvm-subzero/include/llvm/Support/MemoryBuffer.h",
        "third_party/llvm-subzero/include/llvm/Support/MemoryObject.h",
        "third_party/llvm-subzero/include/llvm/Support/Mutex.h",
        "third_party/llvm-subzero/include/llvm/Support/MutexGuard.h",
        "third_party/llvm-subzero/include/llvm/Support/NativeFormatting.h",
        "third_party/llvm-subzero/include/llvm/Support/Options.h",
        "third_party/llvm-subzero/include/llvm/Support/Path.h",
        "third_party/llvm-subzero/include/llvm/Support/PointerLikeTypeTraits.h",
        "third_party/llvm-subzero/include/llvm/Support/Process.h",
        "third_party/llvm-subzero/include/llvm/Support/Program.h",
        "third_party/llvm-subzero/include/llvm/Support/Regex.h",
        "third_party/llvm-subzero/include/llvm/Support/SMLoc.h",
        "third_party/llvm-subzero/include/llvm/Support/Signals.h",
        "third_party/llvm-subzero/include/llvm/Support/SourceMgr.h",
        "third_party/llvm-subzero/include/llvm/Support/StreamingMemoryObject.h",
        "third_party/llvm-subzero/include/llvm/Support/StringSaver.h",
        "third_party/llvm-subzero/include/llvm/Support/SwapByteOrder.h",
        "third_party/llvm-subzero/include/llvm/Support/TargetParser.h",
        "third_party/llvm-subzero/include/llvm/Support/Threading.h",
        "third_party/llvm-subzero/include/llvm/Support/Timer.h",
        "third_party/llvm-subzero/include/llvm/Support/UniqueLock.h",
        "third_party/llvm-subzero/include/llvm/Support/Valgrind.h",
        "third_party/llvm-subzero/include/llvm/Support/WindowsError.h",
        "third_party/llvm-subzero/include/llvm/Support/YAMLParser.h",
        "third_party/llvm-subzero/include/llvm/Support/YAMLTraits.h",
        "third_party/llvm-subzero/include/llvm/Support/circular_raw_ostream.h",
        "third_party/llvm-subzero/include/llvm/Support/raw_os_ostream.h",
        "third_party/llvm-subzero/include/llvm/Support/raw_ostream.h",
        "third_party/llvm-subzero/include/llvm/Support/thread.h",
        "third_party/llvm-subzero/include/llvm/Support/type_traits.h",
        "third_party/llvm-subzero/include/llvm-c/ErrorHandling.h",
        "third_party/llvm-subzero/include/llvm-c/Support.h",
        "third_party/llvm-subzero/include/llvm-c/Types.h",
        "third_party/llvm-subzero/lib/Support/regcclass.h",
        "third_party/llvm-subzero/lib/Support/regcname.h",
        "third_party/llvm-subzero/lib/Support/regex2.h",
        "third_party/llvm-subzero/lib/Support/regutils.h",

        # non-win:
        "third_party/llvm-subzero/lib/Support/Unix/Host.inc",
        "third_party/llvm-subzero/lib/Support/Unix/Path.inc",
        "third_party/llvm-subzero/lib/Support/Unix/Process.inc",
        "third_party/llvm-subzero/lib/Support/Unix/Program.inc",
        "third_party/llvm-subzero/lib/Support/Unix/Signals.inc",
        "third_party/llvm-subzero/lib/Support/Unix/Unix.h",

        # win:
        #"third_party/llvm-subzero/lib/Support/Windows/Path.inc",

        # android:
        #"third_party/llvm-subzero/build/Android/include/llvm/Config/abi-breaking.h",
        #"third_party/llvm-subzero/build/Android/include/llvm/Config/config.h",
        #"third_party/llvm-subzero/build/Android/include/llvm/Config/llvm-config.h",
        #"third_party/llvm-subzero/build/Android/include/llvm/Support/DataTypes.h",
        # linux:
        "third_party/llvm-subzero/build/Linux/include/llvm/Config/abi-breaking.h",
        "third_party/llvm-subzero/build/Linux/include/llvm/Config/config.h",
        "third_party/llvm-subzero/build/Linux/include/llvm/Config/llvm-config.h",
        "third_party/llvm-subzero/build/Linux/include/llvm/Support/DataTypes.h",
        # mac:
        #"third_party/llvm-subzero/build/MacOS/include/llvm/Config/abi-breaking.h",
        #"third_party/llvm-subzero/build/MacOS/include/llvm/Config/config.h",
        #"third_party/llvm-subzero/build/MacOS/include/llvm/Config/llvm-config.h",
        #"third_party/llvm-subzero/build/MacOS/include/llvm/Support/DataTypes.h",
        # windows:
        #"third_party/llvm-subzero/build/Windows/include/llvm/Config/abi-breaking.h",
        #"third_party/llvm-subzero/build/Windows/include/llvm/Config/config.h",
        #"third_party/llvm-subzero/build/Windows/include/llvm/Config/llvm-config.h",
        #"third_party/llvm-subzero/build/Windows/include/llvm/Support/DataTypes.h",
    ],
    textual_hdrs = [
        "third_party/llvm-subzero/include/llvm/IR/Attributes.inc",
        "third_party/llvm-subzero/include/llvm/IR/Instruction.def",
        "third_party/llvm-subzero/include/llvm/IR/Metadata.def",
        "third_party/llvm-subzero/include/llvm/IR/Value.def",
        "third_party/llvm-subzero/include/llvm/Support/AArch64TargetParser.def",
        "third_party/llvm-subzero/include/llvm/Support/ARMTargetParser.def",
        "third_party/llvm-subzero/include/llvm/Support/MachO.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/AArch64.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/AMDGPU.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/ARM.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/AVR.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/BPF.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/Hexagon.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/Lanai.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/Mips.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/PowerPC.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/PowerPC64.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/RISCV.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/Sparc.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/SystemZ.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/WebAssembly.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/i386.def",
        "third_party/llvm-subzero/include/llvm/Support/ELFRelocs/x86_64.def",
        "third_party/llvm-subzero/build/Linux/include/llvm/IR/Attributes.gen",
        "third_party/llvm-subzero/build/Linux/include/llvm/IR/Intrinsics.gen",
        "third_party/llvm-subzero/lib/Support/regengine.inc",
    ],
    copts = COMMON_COPTS,
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS,
    linkstatic = 1,
)

cc_library(
    name = "subzero",
    srcs = [
        "third_party/subzero/src/IceAssembler.cpp",
        "third_party/subzero/src/IceCfg.cpp",
        "third_party/subzero/src/IceCfgNode.cpp",
        "third_party/subzero/src/IceClFlags.cpp",
        "third_party/subzero/src/IceELFObjectWriter.cpp",
        "third_party/subzero/src/IceELFSection.cpp",
        "third_party/subzero/src/IceFixups.cpp",
        "third_party/subzero/src/IceGlobalContext.cpp",
        "third_party/subzero/src/IceGlobalInits.cpp",
        "third_party/subzero/src/IceInst.cpp",
        "third_party/subzero/src/IceInstrumentation.cpp",
        "third_party/subzero/src/IceIntrinsics.cpp",
        "third_party/subzero/src/IceLiveness.cpp",
        "third_party/subzero/src/IceLoopAnalyzer.cpp",
        "third_party/subzero/src/IceMangling.cpp",
        "third_party/subzero/src/IceMemory.cpp",
        "third_party/subzero/src/IceOperand.cpp",
        "third_party/subzero/src/IceRNG.cpp",
        "third_party/subzero/src/IceRangeSpec.cpp",
        "third_party/subzero/src/IceRegAlloc.cpp",
        "third_party/subzero/src/IceRevision.cpp",
        "third_party/subzero/src/IceSwitchLowering.cpp",
        "third_party/subzero/src/IceTargetLowering.cpp",
        "third_party/subzero/src/IceTargetLoweringX86.cpp",
        "third_party/subzero/src/IceThreading.cpp",
        "third_party/subzero/src/IceTimerTree.cpp",
        "third_party/subzero/src/IceTypes.cpp",
        "third_party/subzero/src/IceVariableSplitting.cpp",

        # arm:
        #"third_party/subzero/src/IceInstARM32.cpp",
        #"third_party/subzero/src/IceTargetLoweringARM32.cpp",
        # x32:
        #"third_party/subzero/src/IceInstX8632.cpp",
        #"third_party/subzero/src/IceTargetLoweringX8632.cpp",
        # x64:
        "third_party/subzero/src/IceInstX8664.cpp",
        "third_party/subzero/src/IceTargetLoweringX8664.cpp",
    ],
    hdrs = [
        "third_party/subzero/src/IceAssembler.h",
        "third_party/subzero/src/IceBitVector.h",
        "third_party/subzero/src/IceBuildDefs.h",
        "third_party/subzero/src/IceCfg.h",
        "third_party/subzero/src/IceCfgNode.h",
        "third_party/subzero/src/IceClFlags.h",
        "third_party/subzero/src/IceDefs.h",
        "third_party/subzero/src/IceELFObjectWriter.h",
        "third_party/subzero/src/IceELFSection.h",
        "third_party/subzero/src/IceELFStreamer.h",
        "third_party/subzero/src/IceFixups.h",
        "third_party/subzero/src/IceGlobalContext.h",
        "third_party/subzero/src/IceGlobalInits.h",
        "third_party/subzero/src/IceInst.h",
        "third_party/subzero/src/IceInstVarIter.h",
        "third_party/subzero/src/IceInstrumentation.h",
        "third_party/subzero/src/IceIntrinsics.h",
        "third_party/subzero/src/IceLiveness.h",
        "third_party/subzero/src/IceLoopAnalyzer.h",
        "third_party/subzero/src/IceMangling.h",
        "third_party/subzero/src/IceMemory.h",
        "third_party/subzero/src/IceOperand.h",
        "third_party/subzero/src/IcePhiLoweringImpl.h",
        "third_party/subzero/src/IceRNG.h",
        "third_party/subzero/src/IceRangeSpec.h",
        "third_party/subzero/src/IceRegAlloc.h",
        "third_party/subzero/src/IceRegList.h",
        "third_party/subzero/src/IceRegistersARM32.h",
        "third_party/subzero/src/IceRegistersMIPS32.h",
        "third_party/subzero/src/IceRegistersX8632.h",
        "third_party/subzero/src/IceRegistersX8664.h",
        "third_party/subzero/src/IceRevision.h",
        "third_party/subzero/src/IceStringPool.h",
        "third_party/subzero/src/IceSwitchLowering.h",
        "third_party/subzero/src/IceTLS.h",
        "third_party/subzero/src/IceTargetLowering.h",
        "third_party/subzero/src/IceTargetLoweringX86Base.h",
        "third_party/subzero/src/IceTargetLoweringX86BaseImpl.h",
        "third_party/subzero/src/IceTargetLoweringX86RegClass.h",
        "third_party/subzero/src/IceThreading.h",
        "third_party/subzero/src/IceTimerTree.h",
        "third_party/subzero/src/IceTypes.h",
        "third_party/subzero/src/IceUtils.h",
        "third_party/subzero/src/IceVariableSplitting.h",

        # arm:
        #"third_party/subzero/src/IceAssemblerARM32.h",
        #"third_party/subzero/src/IceConditionCodesARM32.h",
        #"third_party/subzero/src/IceInstARM32.h",
        #"third_party/subzero/src/IceTargetLoweringARM32.h",
        # x32:
        #"third_party/subzero/src/IceAssemblerX86Base.h",
        #"third_party/subzero/src/IceAssemblerX86BaseImpl.h",
        #"third_party/subzero/src/IceAssemblerX8632.h",
        #"third_party/subzero/src/IceConditionCodesX8632.h",
        #"third_party/subzero/src/IceInstX8632.h",
        #"third_party/subzero/src/IceInstX86Base.h",
        #"third_party/subzero/src/IceInstX86BaseImpl.h",
        #"third_party/subzero/src/IceTargetLoweringX8632.h",
        #"third_party/subzero/src/IceTargetLoweringX8632Traits.h",
        # x64:
        "third_party/subzero/src/IceAssemblerX86Base.h",
        "third_party/subzero/src/IceAssemblerX86BaseImpl.h",
        "third_party/subzero/src/IceAssemblerX8664.h",
        "third_party/subzero/src/IceConditionCodesX8664.h",
        "third_party/subzero/src/IceInstX86Base.h",
        "third_party/subzero/src/IceInstX86BaseImpl.h",
        "third_party/subzero/src/IceInstX8664.h",
        "third_party/subzero/src/IceTargetLoweringX8664.h",
        "third_party/subzero/src/IceTargetLoweringX8664Traits.h",
    ],
    textual_hdrs = [
        "third_party/subzero/src/IceClFlags.def",
        "third_party/subzero/src/IceInst.def",
        "third_party/subzero/src/IceTargetLowering.def",
        "third_party/subzero/src/IceTimerTree.def",
        "third_party/subzero/src/IceTypes.def",
        "third_party/subzero/src/SZTargets.def",

        # arm:
        #"third_party/subzero/src/IceInstARM32.def",
        #"third_party/subzero/src/IceRegistersARM32.def",
        #"third_party/subzero/src/IceTargetLoweringARM32.def",
        # x32:
        #"third_party/subzero/src/IceInstX8632.def",
        #"third_party/subzero/src/IceTargetLoweringX8632.def",
        # x64:
        "third_party/subzero/src/IceInstX8664.def",
        "third_party/subzero/src/IceTargetLoweringX8664.def",
    ],
    deps = [
        ":llvm_subzero",
    ],
    copts = COMMON_COPTS + [
        "-Iexternal/com_github_google_swiftshader/third_party/subzero/",
    ],
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS,
)

cc_library(
    name = "reactor",
    srcs = [
        "src/Reactor/Routine.cpp",
        "src/Reactor/Optimizer.cpp",
        "src/Reactor/SubzeroReactor.cpp",
    ],
    hdrs = [
        "src/Reactor/Nucleus.hpp",
        "src/Reactor/Reactor.hpp",
        "src/Reactor/Routine.hpp",
        "src/Reactor/Optimizer.hpp",
    ],
    deps = [
        ":common",
        ":subzero",
    ],
    copts = COMMON_COPTS + [
        "-DLOG_TAG=\"swiftshader_reactor\"",
        "-Iexternal/com_github_google_swiftshader/third_party/subzero/",
    ],
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS,
)

cc_library(
    name = "renderer",
    srcs = [
        "src/Main/Config.hpp",
        "src/Renderer/Blitter.cpp",
        "src/Renderer/Clipper.cpp",
        "src/Renderer/Color.cpp",
        "src/Renderer/Context.cpp",
        "src/Renderer/ETC_Decoder.cpp",
        "src/Renderer/Matrix.cpp",
        "src/Renderer/PixelProcessor.cpp",
        "src/Renderer/Plane.cpp",
        "src/Renderer/Point.cpp",
        "src/Renderer/QuadRasterizer.cpp",
        "src/Renderer/Renderer.cpp",
        "src/Renderer/Sampler.cpp",
        "src/Renderer/SetupProcessor.cpp",
        "src/Renderer/Surface.cpp",
        "src/Renderer/TextureStage.cpp",
        "src/Renderer/Vector.cpp",
        "src/Renderer/VertexProcessor.cpp",
    ],
    hdrs = [
        "src/Renderer/Blitter.hpp",
        "src/Renderer/Clipper.hpp",
        "src/Renderer/Color.hpp",
        "src/Renderer/Context.hpp",
        "src/Renderer/ETC_Decoder.hpp",
        "src/Renderer/LRUCache.hpp",
        "src/Renderer/Matrix.hpp",
        "src/Renderer/PixelProcessor.hpp",
        "src/Renderer/Plane.hpp",
        "src/Renderer/Point.hpp",
        "src/Renderer/Polygon.hpp",
        "src/Renderer/Primitive.hpp",
        "src/Renderer/QuadRasterizer.hpp",
        "src/Renderer/Rasterizer.hpp",
        "src/Renderer/Renderer.hpp",
        "src/Renderer/RoutineCache.hpp",
        "src/Renderer/Sampler.hpp",
        "src/Renderer/SetupProcessor.hpp",
        "src/Renderer/Stream.hpp",
        "src/Renderer/Surface.hpp",
        "src/Renderer/TextureStage.hpp",
        "src/Renderer/Vector.hpp",
        "src/Renderer/Vertex.hpp",
        "src/Renderer/VertexProcessor.hpp",
    ],
    deps = [
        ":common",
        ":main_config",
        ":reactor",
        ":renderer_hdrs",
        ":shader",
    ],
    copts = COMMON_COPTS + [
        "-DLOG_TAG=\"swiftshader_renderer\"",

        "-Iexternal/com_github_google_swiftshader/src/Common/",
        "-Iexternal/com_github_google_swiftshader/src/Main/",
        "-Iexternal/com_github_google_swiftshader/src/Renderer",
        "-Iexternal/com_github_google_swiftshader/src/Shader/",
    ],
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS,
)

cc_library(
    name = "renderer_hdrs",
    hdrs = [
        "src/Renderer/Blitter.hpp",
        "src/Renderer/Clipper.hpp",
        "src/Renderer/Color.hpp",
        "src/Renderer/Context.hpp",
        "src/Renderer/ETC_Decoder.hpp",
        "src/Renderer/LRUCache.hpp",
        "src/Renderer/Matrix.hpp",
        "src/Renderer/PixelProcessor.hpp",
        "src/Renderer/Plane.hpp",
        "src/Renderer/Point.hpp",
        "src/Renderer/Polygon.hpp",
        "src/Renderer/Primitive.hpp",
        "src/Renderer/QuadRasterizer.hpp",
        "src/Renderer/Rasterizer.hpp",
        "src/Renderer/Renderer.hpp",
        "src/Renderer/RoutineCache.hpp",
        "src/Renderer/Sampler.hpp",
        "src/Renderer/SetupProcessor.hpp",
        "src/Renderer/Stream.hpp",
        "src/Renderer/Surface.hpp",
        "src/Renderer/TextureStage.hpp",
        "src/Renderer/Vector.hpp",
        "src/Renderer/Vertex.hpp",
        "src/Renderer/VertexProcessor.hpp",
    ],
    deps = [
        ":common",
        ":main_config",
        ":reactor",
    ],
    copts = COMMON_COPTS + [
        "-DLOG_TAG=\"swiftshader_renderer\"",

        "-Iexternal/com_github_google_swiftshader/src/Common/",
        "-Iexternal/com_github_google_swiftshader/src/Main/",
    ],
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS,
)

cc_library(
    name = "gl_common",
    srcs = [
        "src/OpenGL/common/Image.cpp",
        "src/OpenGL/common/MatrixStack.cpp",
        "src/OpenGL/common/Object.cpp",
        "src/OpenGL/common/debug.cpp",
        "src/OpenGL/libEGL/Context.hpp",
        "src/OpenGL/libEGL/Texture.hpp",
    ],
    hdrs = [
        "src/OpenGL/common/Image.hpp",
        "src/OpenGL/common/MatrixStack.hpp",
        "src/OpenGL/common/NameSpace.hpp",
        "src/OpenGL/common/Object.hpp",
        "src/OpenGL/common/Surface.hpp",
        "src/OpenGL/common/debug.h",
    ],
    deps = [
        ":common",
        ":gl_headers",
        ":renderer",
    ],
    copts = COMMON_COPTS + [
        "-DLOG_TAG=\"swiftshader_opengl_common\"",

        "-Iexternal/com_github_google_swiftshader/src/OpenGL/",
    ],
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS,
)

cc_library(
    name = "gl_compiler",
    srcs = [
        "src/OpenGL/compiler/AnalyzeCallDepth.cpp",
        "src/OpenGL/compiler/Compiler.cpp",
        "src/OpenGL/compiler/Diagnostics.cpp",
        "src/OpenGL/compiler/DirectiveHandler.cpp",
        "src/OpenGL/compiler/InfoSink.cpp",
        "src/OpenGL/compiler/Initialize.cpp",
        "src/OpenGL/compiler/InitializeParseContext.cpp",
        "src/OpenGL/compiler/IntermTraverse.cpp",
        "src/OpenGL/compiler/Intermediate.cpp",
        "src/OpenGL/compiler/OutputASM.cpp",
        "src/OpenGL/compiler/ParseHelper.cpp",
        "src/OpenGL/compiler/PoolAlloc.cpp",
        "src/OpenGL/compiler/SymbolTable.cpp",
        "src/OpenGL/compiler/TranslatorASM.cpp",
        "src/OpenGL/compiler/ValidateGlobalInitializer.cpp",
        "src/OpenGL/compiler/ValidateLimitations.cpp",
        "src/OpenGL/compiler/ValidateSwitch.cpp",
        "src/OpenGL/compiler/debug.cpp",
        "src/OpenGL/compiler/glslang_lex.cpp",
        "src/OpenGL/compiler/glslang_tab.cpp",
        "src/OpenGL/compiler/intermOut.cpp",
        "src/OpenGL/compiler/parseConst.cpp",
        "src/OpenGL/compiler/util.cpp",
        "src/OpenGL/compiler/preprocessor/Diagnostics.cpp",
        "src/OpenGL/compiler/preprocessor/DirectiveHandler.cpp",
        "src/OpenGL/compiler/preprocessor/DirectiveParser.cpp",
        "src/OpenGL/compiler/preprocessor/ExpressionParser.cpp",
        "src/OpenGL/compiler/preprocessor/Input.cpp",
        "src/OpenGL/compiler/preprocessor/Lexer.cpp",
        "src/OpenGL/compiler/preprocessor/Macro.cpp",
        "src/OpenGL/compiler/preprocessor/MacroExpander.cpp",
        "src/OpenGL/compiler/preprocessor/Preprocessor.cpp",
        "src/OpenGL/compiler/preprocessor/Token.cpp",
        "src/OpenGL/compiler/preprocessor/Tokenizer.cpp",

        # win:
        #"src/OpenGL/compiler/ossource_win.cpp",
        # non-win:
        "src/OpenGL/compiler/ossource_posix.cpp",

        "src/OpenGL/libGLESv2/ResourceManager.h",
        "src/OpenGL/libGLESv2/Shader.h",
    ],
    hdrs = [
        "src/OpenGL/compiler/AnalyzeCallDepth.h",
        "src/OpenGL/compiler/BaseTypes.h",
        "src/OpenGL/compiler/Common.h",
        "src/OpenGL/compiler/Compiler.h",
        "src/OpenGL/compiler/ConstantUnion.h",
        "src/OpenGL/compiler/Diagnostics.h",
        "src/OpenGL/compiler/DirectiveHandler.h",
        "src/OpenGL/compiler/ExtensionBehavior.h",
        "src/OpenGL/compiler/InfoSink.h",
        "src/OpenGL/compiler/Initialize.h",
        "src/OpenGL/compiler/InitializeGlobals.h",
        "src/OpenGL/compiler/InitializeParseContext.h",
        "src/OpenGL/compiler/MMap.h",
        "src/OpenGL/compiler/OutputASM.h",
        "src/OpenGL/compiler/ParseHelper.h",
        "src/OpenGL/compiler/PoolAlloc.h",
        "src/OpenGL/compiler/Pragma.h",
        "src/OpenGL/compiler/SymbolTable.h",
        "src/OpenGL/compiler/TranslatorASM.h",
        "src/OpenGL/compiler/Types.h",
        "src/OpenGL/compiler/ValidateGlobalInitializer.h",
        "src/OpenGL/compiler/ValidateLimitations.h",
        "src/OpenGL/compiler/ValidateSwitch.h",
        "src/OpenGL/compiler/debug.h",
        "src/OpenGL/compiler/glslang.h",
        "src/OpenGL/compiler/glslang_tab.h",
        "src/OpenGL/compiler/intermediate.h",
        "src/OpenGL/compiler/localintermediate.h",
        "src/OpenGL/compiler/osinclude.h",
        "src/OpenGL/compiler/util.h",
        "src/OpenGL/compiler/preprocessor/Diagnostics.h",
        "src/OpenGL/compiler/preprocessor/DirectiveHandler.h",
        "src/OpenGL/compiler/preprocessor/DirectiveParser.h",
        "src/OpenGL/compiler/preprocessor/ExpressionParser.h",
        "src/OpenGL/compiler/preprocessor/Input.h",
        "src/OpenGL/compiler/preprocessor/Lexer.h",
        "src/OpenGL/compiler/preprocessor/Macro.h",
        "src/OpenGL/compiler/preprocessor/MacroExpander.h",
        "src/OpenGL/compiler/preprocessor/Preprocessor.h",
        "src/OpenGL/compiler/preprocessor/SourceLocation.h",
        "src/OpenGL/compiler/preprocessor/Token.h",
        "src/OpenGL/compiler/preprocessor/Tokenizer.h",
        "src/OpenGL/compiler/preprocessor/length_limits.h",
        "src/OpenGL/compiler/preprocessor/numeric_lex.h",
        "src/OpenGL/compiler/preprocessor/pp_utils.h",
    ],
    deps = [
        ":common",
        ":gl_common",
        ":gl_headers",
        "@//xrtl/base:debugging_settings",
    ],
    copts = COMMON_COPTS + [
        "-DLOG_TAG=\"swiftshader_opengl_compiler\"",

        "-Iexternal/com_github_google_swiftshader/src/OpenGL/",
    ],
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS,
)

genrule(
    name = "libglesv2_exports_lds",
    srcs = ["src/OpenGL/libGLESv2/exports.map"],
    outs = ["libglesv2_exports.lds"],
    cmd  = "\n".join([
        "cp $< $@",
    ]),
)

# TODO(benvanik): figure out how to plumb this into DLL on win:
# "src/OpenGL/libGLESv2/libGLESv2.def",

LIBGLESV2_SRCS = [
    "src/OpenGL/libGLESv2/Buffer.cpp",
    "src/OpenGL/libGLESv2/Buffer.h",
    "src/OpenGL/libGLESv2/Context.cpp",
    "src/OpenGL/libGLESv2/Context.h",
    "src/OpenGL/libGLESv2/Device.cpp",
    "src/OpenGL/libGLESv2/Device.hpp",
    "src/OpenGL/libGLESv2/Fence.cpp",
    "src/OpenGL/libGLESv2/Fence.h",
    "src/OpenGL/libGLESv2/Framebuffer.cpp",
    "src/OpenGL/libGLESv2/Framebuffer.h",
    "src/OpenGL/libGLESv2/IndexDataManager.cpp",
    "src/OpenGL/libGLESv2/IndexDataManager.h",
    "src/OpenGL/libGLESv2/Program.cpp",
    "src/OpenGL/libGLESv2/Program.h",
    "src/OpenGL/libGLESv2/Query.cpp",
    "src/OpenGL/libGLESv2/Query.h",
    "src/OpenGL/libGLESv2/Renderbuffer.cpp",
    "src/OpenGL/libGLESv2/Renderbuffer.h",
    "src/OpenGL/libGLESv2/ResourceManager.cpp",
    "src/OpenGL/libGLESv2/ResourceManager.h",
    "src/OpenGL/libGLESv2/Sampler.h",
    "src/OpenGL/libGLESv2/Shader.cpp",
    "src/OpenGL/libGLESv2/Shader.h",
    "src/OpenGL/libGLESv2/Texture.cpp",
    "src/OpenGL/libGLESv2/Texture.h",
    "src/OpenGL/libGLESv2/TransformFeedback.cpp",
    "src/OpenGL/libGLESv2/TransformFeedback.h",
    "src/OpenGL/libGLESv2/VertexArray.cpp",
    "src/OpenGL/libGLESv2/VertexArray.h",
    "src/OpenGL/libGLESv2/VertexDataManager.cpp",
    "src/OpenGL/libGLESv2/VertexDataManager.h",
    "src/OpenGL/libGLESv2/libGLESv2.cpp",
    "src/OpenGL/libGLESv2/libGLESv2.hpp",
    "src/OpenGL/libGLESv2/libGLESv3.cpp",
    "src/OpenGL/libGLESv2/main.cpp",
    "src/OpenGL/libGLESv2/main.h",
    "src/OpenGL/libGLESv2/mathutil.h",
    "src/OpenGL/libGLESv2/resource.h",
    "src/OpenGL/libGLESv2/utilities.cpp",
    "src/OpenGL/libGLESv2/utilities.h",

    "src/OpenGL/libGLES_CM/libGLES_CM.hpp",
    "src/OpenGL/libEGL/Config.h",
    "src/OpenGL/libEGL/Context.hpp",
    "src/OpenGL/libEGL/Display.h",
    "src/OpenGL/libEGL/Surface.hpp",
    "src/OpenGL/libEGL/Sync.hpp",
    "src/OpenGL/libEGL/Texture.hpp",
    "src/OpenGL/libEGL/libEGL.hpp",
    "src/OpenGL/libEGL/main.h",
]

cc_binary(
    name = "libGLESv2.dll",
    srcs = LIBGLESV2_SRCS,
    deps = [
        ":gl_common",
        ":gl_compiler",
        ":gl_headers",
        ":reactor",
        ":renderer",
        "@//xrtl/base:debugging_settings",
    ],
    copts = COMMON_COPTS + [
        "-DLOG_TAG=\"swiftshader_libGLESv2\"",
        "-DGL_API=",
        "-DGL_GLEXT_PROTOTYPES",
        "-DGL_APICALL=",
        "-DLIBGLESV2_EXPORTS",
        "-Iexternal/com_github_google_swiftshader/src/OpenGL/",
    ],
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS + [
        "/DEF:libGLESv2.def",
    ],
    linkshared = 1,
    visibility = ["//visibility:public"],
)

# libGLESv2 for Linux:
cc_binary(
    name = "libGLESv2.so",
    srcs = LIBGLESV2_SRCS,
    deps = [
        ":gl_common",
        ":gl_compiler",
        ":gl_headers",
        ":libglesv2_exports.lds",
        ":reactor",
        ":renderer",
        "@//xrtl/base:debugging_settings",
    ],
    copts = COMMON_COPTS + [
        "-DLOG_TAG=\"swiftshader_libGLESv2\"",
        "-DGL_API=",
        "-DGL_GLEXT_PROTOTYPES",
        "-DGL_APICALL='__attribute__((visibility(\"default\")))'",
        "-Iexternal/com_github_google_swiftshader/src/OpenGL/",
    ],
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS + [
        "-fvisibility=hidden",
        "-Wl,--gc-sections",
        "-Wl,--version-script",
        ":libglesv2_exports.lds",
    ],
    linkshared = 1,
    visibility = ["//visibility:public"],
)

genrule(
    name = "libegl_exports_lds",
    srcs = ["src/OpenGL/libEGL/exports.map"],
    outs = ["libegl_exports.lds"],
    cmd  = "\n".join([
        "cp $< $@",
    ]),
)

# TODO(benvanik): figure out how to plumb this into DLL on win:
# "src/OpenGL/libEGL/libEGL.def",

LIBEGL_SRCS = [
    "src/OpenGL/libEGL/Config.cpp",
    "src/OpenGL/libEGL/Config.h",
    "src/OpenGL/libEGL/Context.hpp",
    "src/OpenGL/libEGL/Display.cpp",
    "src/OpenGL/libEGL/Display.h",
    "src/OpenGL/libEGL/Surface.cpp",
    "src/OpenGL/libEGL/Surface.hpp",
    "src/OpenGL/libEGL/Sync.hpp",
    "src/OpenGL/libEGL/Texture.hpp",
    "src/OpenGL/libEGL/libEGL.cpp",
    "src/OpenGL/libEGL/libEGL.hpp",
    "src/OpenGL/libEGL/main.cpp",
    "src/OpenGL/libEGL/main.h",
    "src/OpenGL/libEGL/resource.h",

    "src/OpenGL/libGLES_CM/libGLES_CM.hpp",
    "src/OpenGL/libGLESv2/libGLESv2.hpp",
]

# libEGL for Windows:
cc_binary(
    name = "libEGL.dll",
    srcs = LIBEGL_SRCS,
    deps = [
        ":gl_common",
        ":gl_headers",
        "@//xrtl/base:debugging_settings",
    ],
    copts = COMMON_COPTS + [
        "-DLOG_TAG=\"swiftshader_libEGL\"",
        "-DEGL_EGLEXT_PROTOTYPES",
        "-DEGLAPI=",
        "-DLIBEGL_EXPORTS",
        "-Iexternal/com_github_google_swiftshader/src/OpenGL/",
    ],
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS + [
        "/DEF:libEGL.def",
    ],
    linkshared = 1,
    visibility = ["//visibility:public"],
)

# libEGL for Linux:
cc_binary(
    name = "libEGL.so",
    srcs = LIBEGL_SRCS + [
        # osx:
        # OSXUtils.hpp
        # OSXUtils.mm
    ],
    deps = [
        ":gl_common",
        ":gl_headers",
        ":libegl_exports.lds",
        "@//xrtl/base:debugging_settings",
    ],
    copts = COMMON_COPTS + [
        "-DLOG_TAG=\"swiftshader_libEGL\"",
        "-DEGL_EGLEXT_PROTOTYPES",
        "-DEGLAPI='__attribute__((visibility(\"default\")))'",
        "-Iexternal/com_github_google_swiftshader/src/OpenGL/",
    ],
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS + [
        # osx:
        # Quartz.framework, Cocoa.framework
        "-fvisibility=hidden",
        "-Wl,--gc-sections",
        "-Wl,--version-script",
        ":libegl_exports.lds",
    ],
    linkshared = 1,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "swiftshader",
    srcs = select({
        "@//xrtl/tools/target_platform:linux": [
            ":libGLESv2.so",
            ":libEGL.so",
        ],
        "@//xrtl/tools/target_platform:macos": [],
        "@//xrtl/tools/target_platform:windows": [
            ":libGLESv2.dll",
            ":libEGL.dll",
        ],
    }),
    copts = COMMON_COPTS,
    defines = COMMON_DEFINES,
    linkopts = COMMON_LINKOPTS,
    visibility = ["//visibility:public"],
)
