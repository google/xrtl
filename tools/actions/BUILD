# Description:
#  Action listener that generates the LLVM compile command database.
#  http://clang.llvm.org/docs/HowToSetupToolingForLLVM.html

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

py_binary(
    name = "generate_compile_command",
    srcs = ["generate_compile_command.py"],
    deps = ["//third_party/bazel:extra_actions_proto_py"],
    visibility = ["//visibility:private"],
)

extra_action(
    name = "generate_compile_commands_action",
    tools = [":generate_compile_command"],
    out_templates = ["$(ACTION_ID)_compile_command"],
    cmd = " ".join([
        "$(location :generate_compile_command)",
        "$(EXTRA_ACTION_FILE)",
        "$(output $(ACTION_ID)_compile_command)",
    ]),
    visibility = ["//visibility:private"],
)

action_listener(
    name = "generate_compile_commands_listener",
    mnemonics = [
        # TODO(benvanik): objc?
        "CppCompile",
    ],
    extra_actions = [":generate_compile_commands_action"],
    visibility = ["//visibility:public"],
)

py_binary(
    name = "generate_link_target",
    srcs = ["generate_link_target.py"],
    deps = ["//third_party/bazel:extra_actions_proto_py"],
    visibility = ["//visibility:private"],
)

extra_action(
    name = "generate_link_targets_action",
    tools = [":generate_link_target"],
    out_templates = ["$(ACTION_ID)_link_target"],
    cmd = " ".join([
        "$(location :generate_link_target)",
        "$(EXTRA_ACTION_FILE)",
        "$(output $(ACTION_ID)_link_target)",
    ]),
    visibility = ["//visibility:private"],
)

action_listener(
    name = "generate_link_targets_listener",
    mnemonics = [
        "CppLink",
    ],
    extra_actions = [":generate_link_targets_action"],
    visibility = ["//visibility:public"],
)
