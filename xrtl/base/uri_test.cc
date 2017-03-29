// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "xrtl/base/uri.h"

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace uri {
namespace {

class UriTest : public ::testing::Test {};

TEST_F(UriTest, IsSchemeless) {
  EXPECT_FALSE(IsSchemeless("scheme://"));
  EXPECT_FALSE(IsSchemeless("scheme://host"));
  EXPECT_FALSE(IsSchemeless("scheme://host/path"));
  EXPECT_FALSE(IsSchemeless("scheme:///"));
  EXPECT_FALSE(IsSchemeless("x-scheme://"));
  EXPECT_TRUE(IsSchemeless(""));
  EXPECT_TRUE(IsSchemeless("/"));
  EXPECT_TRUE(IsSchemeless("path"));
  EXPECT_TRUE(IsSchemeless("/path"));
  EXPECT_TRUE(IsSchemeless("//host"));
}

TEST_F(UriTest, GetScheme) {
  EXPECT_EQ("scheme", GetScheme("scheme://"));
  EXPECT_EQ("scheme", GetScheme("scheme://host"));
  EXPECT_EQ("scheme", GetScheme("scheme://host/path"));
  EXPECT_EQ("scheme", GetScheme("scheme:///"));
  EXPECT_EQ("x-scheme", GetScheme("x-scheme://"));
  EXPECT_EQ("", GetScheme(""));
  EXPECT_EQ("", GetScheme("/"));
  EXPECT_EQ("", GetScheme("path"));
  EXPECT_EQ("", GetScheme("/path"));
  EXPECT_EQ("", GetScheme("//host"));
}

TEST_F(UriTest, GetHost) {
  EXPECT_EQ("", GetHost("scheme://"));
  EXPECT_EQ("host", GetHost("scheme://host"));
  EXPECT_EQ("host", GetHost("scheme://host/path"));
  EXPECT_EQ("host:123", GetHost("scheme://host:123/path"));
  EXPECT_EQ("", GetHost("scheme:///"));
  EXPECT_EQ("", GetHost("x-scheme://"));
  EXPECT_EQ("", GetHost(""));
  EXPECT_EQ("", GetHost("/"));
  EXPECT_EQ("", GetHost("path"));
  EXPECT_EQ("", GetHost("/path"));
  EXPECT_EQ("host", GetHost("//host"));
  EXPECT_EQ("host:123", GetHost("//host:123"));
  EXPECT_EQ("host", GetHost("//host/path"));
  EXPECT_EQ("", GetHost("///path"));
  EXPECT_EQ("", GetHost("//"));
}

TEST_F(UriTest, GetOrigin) {
  EXPECT_EQ("scheme://host", GetOrigin("scheme://host"));
  EXPECT_EQ("scheme://host:123", GetOrigin("scheme://host:123"));
  EXPECT_EQ("scheme://host:123", GetOrigin("scheme://host:123/path"));
  EXPECT_EQ("scheme://host", GetOrigin("scheme://host/path"));
  EXPECT_EQ("//host", GetOrigin("//host/path"));
  EXPECT_EQ("//", GetOrigin("//"));
  EXPECT_EQ("//", GetOrigin("/"));
  EXPECT_EQ("//", GetOrigin(""));
}

TEST_F(UriTest, GetPath) {
  EXPECT_EQ("", GetPath("scheme://"));
  EXPECT_EQ("", GetPath("scheme://host"));
  EXPECT_EQ("/", GetPath("scheme://host/"));
  EXPECT_EQ("/path", GetPath("scheme://host/path"));
  EXPECT_EQ("/path", GetPath("scheme://host:123/path"));
  EXPECT_EQ("/", GetPath("scheme:///"));
  EXPECT_EQ("", GetPath("x-scheme://"));
  EXPECT_EQ("", GetPath(""));
  EXPECT_EQ("/", GetPath("/"));
  EXPECT_EQ("path", GetPath("path"));
  EXPECT_EQ("/path", GetPath("/path"));
  EXPECT_EQ("path/", GetPath("path/"));
  EXPECT_EQ("/path/", GetPath("/path/"));
  EXPECT_EQ("", GetPath("//host"));
  EXPECT_EQ("", GetPath("//host:123"));
  EXPECT_EQ("/path", GetPath("//host/path"));
}

TEST_F(UriTest, IsPathAbsolute) {
  EXPECT_FALSE(IsPathAbsolute(""));
  EXPECT_FALSE(IsPathAbsolute("path"));
  EXPECT_FALSE(IsPathAbsolute("path/"));
  EXPECT_TRUE(IsPathAbsolute("/"));
  EXPECT_TRUE(IsPathAbsolute("/path"));
  EXPECT_TRUE(IsPathAbsolute("/path/"));
}

TEST_F(UriTest, GetBasePath) {
  EXPECT_EQ("http://foo/bar/deep/",
            GetBasePath("http://foo/bar/deep/woo.html"));
  EXPECT_EQ("http://foo/bar/", GetBasePath("http://foo/bar/deep/"));
  EXPECT_EQ("http://foo/bar/", GetBasePath("http://foo/bar/baz.html"));
  EXPECT_EQ("http://foo/", GetBasePath("http://foo/bar.html"));
  EXPECT_EQ("http://foo/", GetBasePath("http://foo/"));
  EXPECT_EQ("http://foo", GetBasePath("http://foo"));
  EXPECT_EQ("http://", GetBasePath("http://"));
  EXPECT_EQ("http", GetBasePath("http"));
  EXPECT_EQ("/asd/", GetBasePath("/asd/"));
  EXPECT_EQ("/asd", GetBasePath("/asd"));
  EXPECT_EQ("asd/", GetBasePath("asd/"));
  EXPECT_EQ("//", GetBasePath("//"));
  EXPECT_EQ("/", GetBasePath("/"));
  EXPECT_EQ("", GetBasePath(""));
}

TEST_F(UriTest, JoinParts) {
  EXPECT_EQ("http://foo/bar/deep/boo.txt",
            JoinParts("http://foo/bar/deep/woo.html", "boo.txt"));
  EXPECT_EQ("http://foo/boo.txt",
            JoinParts("http://foo/bar/woo.html", "/boo.txt"));
  EXPECT_EQ("http://foo/bar", JoinParts("http://foo/", "/bar"));
  EXPECT_EQ("http://foo/", JoinParts("http://foo/", "/"));
  EXPECT_EQ("http://foo/", JoinParts("http://foo", "/"));
  EXPECT_EQ("http://foo/", JoinParts("http://foo/", ""));
  EXPECT_EQ("", JoinParts("", ""));
}

}  // namespace
}  // namespace uri
}  // namespace xrtl
