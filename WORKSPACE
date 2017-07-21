workspace(name = "xrtl")

load("//xrtl:workspace.bzl", "xrtl_workspace")
xrtl_workspace()

# Android SDK and NDK.
# We use the ANDROID_HOME and ANDROID_NDK_HOME environment variables to
# allow non-local installs (though xtool always sets them).
#
# If you get errors about the Android tools you'll need to run xtool setup:
# $ xtool setup --android-sdk
#
# If you don't want to build any Android targets you can comment out these
# lines.
# android_sdk_repository(name = "androidsdk")
# android_ndk_repository(name = "androidndk")
