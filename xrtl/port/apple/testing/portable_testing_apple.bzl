"""cc_test rules for MacOS."""

load("@build_bazel_rules_apple//apple:ios.bzl", "ios_unit_test")
load("@build_bazel_rules_apple//apple:macos.bzl", "macos_unit_test")

is_bazel = not hasattr(native, "genmpm")

def _merge_dicts(a, b):
  result = {}
  for (key, value) in a.items():
    result[key] = value
  for (key, value) in b.items():
    if not key in result:
      result[key] = value
  return result

def _dirname(p):
  (prefix, sep, _) = p.rpartition("/")
  if not prefix:
    return sep
  else:
    return prefix.rstrip("/")

def _objc_merged_zip_impl(ctx):
  """Implementation of the objc_merged_zip rule.

  Args:
    ctx: The Skylark context.
  Returns:
    A struct with an objc provider that lists the zip files to be merged into
    the final bundle.
  """
  objc_provider = apple_common.new_objc_provider(merge_zip=depset(ctx.files.srcs))
  return struct(objc=objc_provider)

objc_merged_zip = rule(
    _objc_merged_zip_impl,
    attrs = {
        "srcs": attr.label_list(
            allow_files = True,
            mandatory = True,
        ),
    },
    fragments = ["apple"],
)

"""Merges .zip file inputs into an ios_* bundle.

Add this to the deps of an ios_unit_test or ios_application to have the contents
of the input .zip files extracted with full paths into the resulting ipa.

Args:
  srcs: a list of .zip files to merge into the bundle.

Outs:
  A synthetic rule that can be put into the deps of an ios_unit_test rule.
"""

def _filegroup_zip_impl(ctx):
  """Implementation of the filegroup_zip rule.

  Args:
    ctx: The Skylark context.
  Returns:
    A struct with the output zip file.
  """
  output_zip = ctx.new_file(ctx.label.name + ".zip")
  commands = []
  for f in ctx.files.srcs:
    if f.root == ctx.bin_dir or f.root == ctx.genfiles_dir:
      commands.append("mkdir -p " + _dirname(f.short_path))
      commands.append("cp -f " + f.path + " " + f.short_path)
  commands.append("zip -q %s %s" % (
      output_zip.path,
      " ".join([f.short_path for f in ctx.files.srcs])))
  # TODO(benvanik): figure out why this fails on exoblaze.
  if not is_bazel:
    for f in ctx.files.srcs:
      commands.append("rm " + f.short_path)
  ctx.action(
      inputs = ctx.files.srcs,
      outputs = [output_zip],
      command = " && ".join(commands),
  )

  return struct(files=depset([output_zip]))

filegroup_zip = rule(
    _filegroup_zip_impl,
    attrs = {
        "srcs": attr.label_list(
            allow_files = True,
            mandatory = True,
        ),
    },
)

"""Zips source files with their absolute paths into a single .zip file.

Args:
  srcs: source files/labels/etc.

Outs:
  A .zip file.

Example:
  filegroup(name="A", srcs=["a/A.txt"])
  filegroup(name="B", srcs=["b/B.txt"])
  filegroup_zip(name="z", srcs=[":A", ":B"])
  $ blaze build :z
  -> z.zip:
     - a/A.txt
     - b/B.txt
"""

def apple_cc_test(name, platform, **kwargs):
  # Primary test library, similar to a cc_test.
  native.objc_library(
      name = "%s_lib" % (name),
      srcs = kwargs["srcs"],
      deps = kwargs["deps"],
      testonly = 1,
      visibility = ["//visibility:private"],
      alwayslink = 1,
  )
  kwargs.pop("srcs")
  kwargs.pop("deps")

  # Create a .zip of all the source data files.
  # We'll then let the objc bundler unpack it when putting it into the IPA.
  # We need to do this as otherwise the resource rules will strip paths and
  # we'll end up with just leaf file names.
  if "data" in kwargs:
    filegroup_zip(
        name = "%s_data" % (name),
        srcs = kwargs["data"],
        visibility = ["//visibility:private"],
    )
    objc_merged_zip(
        name = "%s_merged_zip" % (name),
        srcs = [":%s_data" % (name)],
        visibility = ["//visibility:private"],
    )
    kwargs.pop("data")
  else:
    native.cc_library(
        name = "%s_merged_zip" % (name),
        srcs = [],
        visibility = ["//visibility:private"],
    )

  test_deps = [
      ":%s_lib" % (name),
      ":%s_merged_zip" % (name),
      "//xrtl/port/apple/testing:apple_test_main",
  ]

  if platform == "macos":
    macos_unit_test(
        name = name,
        deps = test_deps,
        infoplists = ["//xrtl/port/apple/testing:apple_test.plist"],
        bundle_id = "com.google.xrtl.testing",
        **_merge_dicts({
          "minimum_os_version": "10.8",
        }, kwargs)
    )
  else:
    ios_unit_test(
        name = name,
        deps = test_deps,
        infoplists = ["//xrtl/port/apple/testing:apple_test.plist"],
        bundle_id = "com.google.xrtl.testing",
        **_merge_dicts({
          "minimum_os_version": "10.0",
        }, kwargs)
    )
