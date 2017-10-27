def generate_vulkan_sources(name=None, outs=None, **kwargs):
  targets = []
  for out in outs:
    out_target = "gen_{}".format(out.replace("-", "_").replace(".", "_"))
    native.genrule(
        name = out_target,
        srcs = [
            "scripts/vk.xml",
        ],
        outs = [out],
        cmd = ("cp $(location @com_github_khronosgroup_vulkan//:layers/vk_validation_error_messages.h) . && " +
               "$(location @com_github_khronosgroup_vulkan//:lvl_genvk) " +
               "-registry $(location @com_github_khronosgroup_vulkan//:scripts/vk.xml) " +
               "-o $(@D) $$(basename $@)"),
        tools = [
            "@com_github_khronosgroup_vulkan//:lvl_genvk",
            "@com_github_khronosgroup_vulkan//:layers/vk_validation_error_messages.h",
        ],
        visibility = ["//visibility:private"],
    )
    targets.append(":{}".format(out_target))

  native.cc_library(
      name = name,
      hdrs = targets,
      includes = ["."],
      visibility = ["//visibility:private"],
  )

def _map_file_paths(srcs, out):
  outs = {}
  for src in srcs:
    src_name = src.name
    if "/" in src_name:
      basename = src_name[src_name.rfind("/") + 1:]
    else:
      basename = src_name
    outs[basename] = out + "/" + basename
  return outs

def _symlink_files_impl(ctx):
  out_files = []
  for src_file in ctx.files.srcs:
    out_file = getattr(ctx.outputs, src_file.basename)
    ctx.actions.run_shell(
        outputs = [out_file],
        inputs = [src_file],
        command = "cp %s %s" % (src_file.path, out_file.path),
    )
    out_files.append(out_file)
  return [DefaultInfo(
      runfiles = ctx.runfiles(files = out_files),
  )]

symlink_files = rule(
    attrs = {
        "srcs": attr.label_list(allow_files = True),
        "out": attr.string(mandatory = True),
    },
    outputs = _map_file_paths,
    implementation = _symlink_files_impl,
)
