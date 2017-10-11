# Description:
#  Action listener that generates the LLVM compile command database.
#  http://clang.llvm.org/docs/HowToSetupToolingForLLVM.html

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

py_binary(
    name = "generate_compile_command",
    srcs = ["generate_compile_command.py"],
    visibility = ["//visibility:private"],
    deps = ["//third_party/bazel:extra_actions_proto_py"],
)

extra_action(
    name = "generate_compile_commands_action",
    cmd = " ".join([
        "PYTHONPATH=$$PYTHONPATH:$$PWD",
        "python $(location :generate_compile_command)",
        "$(EXTRA_ACTION_FILE)",
        "$(output $(ACTION_ID)_compile_command)",
    ]),
    out_templates = ["$(ACTION_ID)_compile_command"],
    tools = [":generate_compile_command"],
    visibility = ["//visibility:private"],
)

action_listener(
    name = "generate_compile_commands_listener",
    extra_actions = [":generate_compile_commands_action"],
    mnemonics = [
        # TODO(benvanik): objc?
        "CppCompile",
    ],
    visibility = ["//visibility:public"],
)

py_binary(
    name = "generate_link_target",
    srcs = ["generate_link_target.py"],
    visibility = ["//visibility:private"],
    deps = ["//third_party/bazel:extra_actions_proto_py"],
)

extra_action(
    name = "generate_link_targets_action",
    cmd = " ".join([
        "PYTHONPATH=$$PYTHONPATH:$$PWD",
        "python $(location :generate_link_target)",
        "$(EXTRA_ACTION_FILE)",
        "$(output $(ACTION_ID)_link_target)",
    ]),
    out_templates = ["$(ACTION_ID)_link_target"],
    tools = [":generate_link_target"],
    visibility = ["//visibility:private"],
)

action_listener(
    name = "generate_link_targets_listener",
    extra_actions = [":generate_link_targets_action"],
    mnemonics = [
        "CppLink",
    ],
    visibility = ["//visibility:public"],
)
