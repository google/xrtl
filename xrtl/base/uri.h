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

#ifndef XRTL_BASE_URI_H_
#define XRTL_BASE_URI_H_

#include <string>

#include "xrtl/base/string_view.h"

namespace xrtl {
namespace uri {

// Returns true if the given endpoint is schemeless.
bool IsSchemeless(StringView uri);

// Gets the scheme part of the given URI.
// If the URI is schemeless the empty string is returned.
//
// Examples:
//   scheme://host        -> "scheme"
//   //host               -> ""
StringView GetScheme(StringView uri);

// Gets the host part of the given URI.
// If no host exists (as the URI is path-only) the empty string is returned.
// May contain port information, if present.
//
// Examples:
//   scheme://host        -> "host"
//   scheme://host:123    -> "host:123"
//   scheme://host/path   -> "host"
//   //host/path          -> "host"
StringView GetHost(StringView uri);

// Gets the fully-qualified origin of a URI.
//
// Examples:
//   scheme://host        -> "scheme://host"
//   scheme://host:123    -> "scheme://host:123"
//   scheme://host/path   -> "scheme://host"
//   //host/path          -> "//host"
StringView GetOrigin(StringView uri);

// Gets the path part of the given URI.
// If no path part exists the empty string is returned. Trailing slashes are
// preserved if present.
//
// Examples:
//   scheme://host        -> ""
//   scheme://host/       -> "/"
//   scheme://host/path   -> "path"
//   scheme://host/path/  -> "path/"
//   path                 -> "path"
//   /path                -> "/path"
StringView GetPath(StringView uri);

// Returns true if the given path fragment is absolute.
//
// Examples:
//   ""                   -> false
//   path                 -> false
//   /                    -> true
//   /path                -> true
bool IsPathAbsolute(StringView path);

// Gets the full base path of a URL, removing the last path component.
//
// Examples:
//   http://foo/bar/deep/woo.html = http://foo/bar/deep/
//   http://foo/bar/deep/         = http://foo/bar/
//   http://foo/                  = http://foo/
StringView GetBasePath(StringView url);

// Joins two URI parts together and canonicalizes the resulting URI.
//
// Examples:
//   http://foo/bar/deep/woo.html + ../boo.txt  = http://foo/deep/boo.txt
//   http://foo/bar/woo.html + /boo.txt         = http://foo/boo.txt
std::string JoinParts(StringView left, StringView right);

}  // namespace uri
}  // namespace xrtl

#endif  // XRTL_BASE_URI_H_
