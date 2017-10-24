"""Portable test rules that enable cc_test targets to run everywhere."""

# Platform name and package path aliases.
# Define new platforms there.
load(
    "//xrtl/tools/target_platform:platforms.bzl",
    "PLATFORMS",
    "PLATFORM_NAMES",
)

# Platform-specific test rule definitions. All platforms not in this set will
# use normal cc_test rules.
load(
    "//xrtl/port/android/testing:portable_testing_android.bzl",
    "android_cc_test",
)
load(
    "//xrtl/port/apple/testing:portable_testing_apple.bzl",
    "apple_cc_test",
)

is_bazel = not hasattr(native, "genmpm")

def _add_tag(kwargs, tag):
  """Adds a tag to a (possibly extant) kwargs tags list."""
  if "tags" in kwargs:
    if not tag in kwargs["tags"]:
      kwargs["tags"].append(tag)
  else:
    kwargs["tags"] = [tag]

def _filter_platform_args(platform_name, **kwargs):
  """Filters kwargs by prefix, including all those that match or have no other
  platform prefix.

  Args:
    platform_name: platform prefix to match (like 'ios').
    **kwargs: arguments to filter.

  Returns:
    A kwargs dictionary of all unprefixed args and those that have the requested
    platform prefix. The platform prefix will be stripped and lists will be
    merged.
  """
  include_prefix = "%s_" % (platform_name)
  exclude_prefixes = ["%s_" % (name) for name in PLATFORM_NAMES]
  exclude_prefixes.remove(include_prefix)
  result_kwargs = {}
  for (key, value) in kwargs.items():
    skipped = False
    for exclude_prefix in exclude_prefixes:
      if key.startswith(exclude_prefix):
        skipped = True
        break
    if skipped: continue
    if key.startswith(include_prefix):
      # This is an arg targeting our platform; strip prefix from key.
      key = key.replace(include_prefix, "")
    else:
      # Generic arg for all platforms.
      pass
    # Merge into existing kwargs.
    if key in result_kwargs:
      result_kwargs[key].extend(value)
    else:
      result_kwargs[key] = value
  return result_kwargs

def _find_default_package_platforms():
  """Finds a package_platforms in the same package and returns its values.
  If no package_platforms is found the default is all platforms.

  Returns:
    A list of default platforms for the package.
  """
  this_package_platforms = native.existing_rule("package_platforms_placeholder")
  if this_package_platforms:
    return list(this_package_platforms["srcs"])
  return PLATFORMS

def package_platforms(default_platforms = PLATFORMS):
  """Sets the default platforms used by portable_cc_test rules in the file.
  By default all platforms will be used for portable_cc_test targets unless
  specific platforms are specified per-target.

  This is most often used under port/ paths to avoid needing to define the
  platform for each target.

  Usage:
    my_package/BUILD:
      load("//xrtl/testing:portable_testing.bzl", "package_platforms")
      package_platforms(
          default_platforms = ["//xrtl/tools/target_platform:android"],
      )
  """
  native.filegroup(
      name = "package_platforms_placeholder",
      srcs = default_platforms,
  )

def portable_cc_test(
    name,
    srcs,
    deps = [],
    platforms = None,
    **kwargs):
  """Macro that creates zero or more platform-specific rules.

  By default this will create rules for all available platforms or those
  specified by a package_platforms rule in the same package. Passing a list
  of platforms will only create rules for each platform specified.

  Each generated platform rule will have the base name with a platform suffix:
    portable_cc_test(name = "my_test", platforms = [
        "//xrtl/tools/target_platform:android",
        "//xrtl/tools/target_platform:ios",
    ])
    ->
    :my_test          - alias to the current config target below
    :my_test_android  - android test rule
    :my_test_ios      - ios test rule

  Additional arguments may be provided and they will be passed to each platform
  test rule created. If arguments are prefixed with a platform name they will be
  passed only to that platform; for example:
    portable_cc_test(name = "my_test", platforms = [
        "//xrtl/tools/target_platform:android",
        "//xrtl/tools/target_platform:ios",
    ], android_deps = [...], ios_deps = [...])

  Usage:
    my_package/BUILD:
      cc_library(
          name = "my_library",
          ...,
      )
      portable_cc_test(
          name = "my_library_test",
          srcs = ["my_library_test.cc"],
          deps = [":my_library"],
          # Android only:
          platforms = ["//xrtl/tools/target_platform:android"],
      )
    $ bazel test :my_library_test  # no-op
    $ bazel test --config=android_x86_64 :my_library_test
    $ bazel test --config=ios_x86_64 :my_library_test

  Args:
    name: master rule name.
    srcs: test sources.
    deps: dependencies for all platforms.
    platforms: list of platforms that the test can run on. Defaults to all
               supported platforms or those in a package_platforms rule.
    kwargs: passed to all test rules.
  """
  # Hidden rule for build cleaner to track deps.
  # This works in conjunction with the register_extension_info macro below.
  native.cc_library(
      name = "%s_deps" % (name),
      srcs = srcs,
      deps = deps + [
          # TODO(benvanik): host deps; probably just linux.
      ],
      tags = ["manual", "nobuilder"],
      testonly = 1,
      visibility = ["//visibility:private"],
  )

  # Determine full set of platforms (based on package defaults/etc).
  target_platforms = []
  if platforms:
    # Specific platforms set on rule.
    target_platforms = platforms[:]
  else:
    # Default platforms for the package.
    target_platforms = _find_default_package_platforms()[:]

  # Generate all platform rules.
  platform_test_targets = []
  for target_platform in target_platforms:
    # Filter kwargs to include all generic args plus platform-specific args
    # (those with a platform name prefix, like ios_).
    platform_name = target_platform.replace("//xrtl/tools/target_platform:", "")
    platform_kwargs = _filter_platform_args(platform_name,
                                            srcs = srcs, deps = deps, **kwargs)

    platform_test_name = "%s_%s" % (name, platform_name)
    platform_test_targets.append(platform_test_name)

    # Add a test-for=platform tag so that our --test_tag_filters can select
    # tests by platform.
    _add_tag(platform_kwargs, "test-for=%s" % (platform_name))

    # Generate actual test rule.
    # Some platforms can use the native cc_test, while others require special
    # handling. Ideally we'd get everything down to cc_test.
    if target_platform == "//xrtl/tools/target_platform:android":
      android_cc_test(
          name = platform_test_name, **platform_kwargs)
    elif (target_platform == "//xrtl/tools/target_platform:ios" or
          target_platform == "//xrtl/tools/target_platform:macos"):
      apple_cc_test(
          name = platform_test_name,
          platform = platform_name,
          **platform_kwargs
      )
    else:
      native.cc_test(
          name = platform_test_name, **platform_kwargs)

def portable_test_suite(
    name = None,
    platforms = None,
    tests = None,
    tags = None):
  """Macro that acts like a native test_suite but is platform aware.

  Each generated platform test_suite will have the base name with a platform
  suffix:
    portable_test_suite(platforms = [
        "//xrtl/tools/target_platform:android",
        "//xrtl/tools/target_platform:ios",
    ])
    ->
    :all_tests_android  - android test rule
    :all_tests_ios      - ios test rule

  Args:
    name: test suite name. If omitted the name will be 'all_tests'.
    platforms: list of platforms that the test can run on. Defaults to all
               supported platforms or those in a package_platforms rule.
    tests: optional list of portable_cc_test or portable_test_suite rules.
           If omitted all tests in the current file will be included.
    tags: tags associated with the test suite.
  """
  package_name = native.package_name()
  if package_name.rfind('/'):
    package_name = package_name[package_name.rfind('/') + 1:]
  test_suite_name = name if name else "all_tests"

  # Determine full set of platforms (based on package defaults/etc).
  target_platforms = []
  if platforms:
    # Specific platforms set on rule.
    target_platforms = platforms
  else:
    # Default platforms for the package.
    target_platforms = _find_default_package_platforms()

  platform_test_target_lists = {}
  if tests == None:
    # No tests were specified so scan for all portable_cc_test targets in the
    # current package and use those.
    test_rules = []
    for rule in native.existing_rules().values():
      rule_label = "%s//%s:%s" % (native.repository_name(),
                                  native.package_name(),
                                  rule["name"])
      rule_platform = None
      if rule["kind"] == "apple_unit_test":
        rule_platform = "//xrtl/tools/target_platform:%s" % (
            rule["platform_type"])
      elif rule["kind"] == "cc_test":
        # TODO(benvanik): real rule so we don't have to do this.
        for tag in rule["tags"]:
          if tag.startswith("test-for="):
            test_for = tag[tag.find("=") + 1:]
            if test_for in PLATFORM_NAMES:
              rule_platform = "//xrtl/tools/target_platform:%s" % (test_for)
      if rule_platform:
        platform_list = platform_test_target_lists.get(rule_platform, None)
        if not platform_list:
          platform_list = []
          platform_test_target_lists[rule_platform] = platform_list
        platform_list.append(rule_label)
  else:
    # TODO(benvanik): may need custom rule impl to expose data.
    test_rules = []
    fail("NOT YET IMPLEMENTED")
    return

  # Split passed tests based on platforms supported and recursively go into
  # test suites.
  for test_rule in test_rules:
    # TODO(benvanik): process list, somewhat like this:
    # platform_test_target_lists["//xrtl/tools/target_platform:macos"] = [
    #    Label(native.repository_name() + "//" + native.package_name() +
    #          test + "_macos")]
    pass

  # Generate all platform test suites.
  platform_test_suites = []
  for target_platform in target_platforms:
    platform_name = target_platform.replace("//xrtl/tools/target_platform:", "")
    platform_tags = tags[:] if tags else []
    test_for_tag = "test-for=%s" % (platform_name)
    if not test_for_tag in platform_tags:
      platform_tags.append(test_for_tag)

    # Add dummy targets when otherwise we have no tests. We don't want
    # native.test_suite trying to infer targets and ignoring all our hard work.
    platform_test_targets = platform_test_target_lists.get(target_platform,
                                                           None)
    if platform_test_targets == None:
      platform_test_targets = ["//xrtl/testing:empty_test"]

    # Emit platform-specific native test suite.
    platform_test_suite_name = "%s_%s" % (test_suite_name, platform_name)
    native.test_suite(
        name = platform_test_suite_name,
        tests = platform_test_targets,
        tags = platform_tags,
    )
    platform_test_suites.append(platform_test_suite_name)

  # Generate a master test_suite that includes all the ones we generated.
  # TODO(benvanik): figure out a way to make this work with filtering:
  # native.test_suite(
  #     name = test_suite_name,
  #     tests = [":%s" % (name) for name in platform_test_suites],
  #     tags = tags,
  # )

  # Emit specialized test suites for certain platforms.
  # TODO(benvanik): XCTestSuite to make running in xcode less painful.
  pass
