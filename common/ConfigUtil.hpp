/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Configuration related utilities.
// Placed here to reduce avoid polluting
// Util.hpp with the config headers.
// This is designed to be used from both wsd and kit.

#pragma once

#include <string>

namespace Poco
{
namespace Util
{
class AbstractConfiguration;
}
} // namespace Poco

namespace config
{
/// Initialize the config from an XML string.
void initialize(const std::string& xml);

/// Initialize the config given a pointer to a long-lived pointer.
void initialize(const Poco::Util::AbstractConfiguration* config);

/// Check if the config has been initialized
bool isInitialized();

/// Returns the value of an entry as string or @def if it is not found.
std::string getString(const std::string& key, const std::string& def);

/// Returns true if and only if the property with the given key exists.
bool has(const std::string& key);

/// Returns the value of an entry as string or @def if it is not found.
bool getBool(const std::string& key, const bool def);

/// Returns the value of an entry as int or @def if it is not found.
int getInt(const std::string& key, const int def);

/// Return true if SSL is enabled in the config and no fuzzing is enabled.
bool isSslEnabled();

/// Return true if build is support key enabled (ENABLE_SUPPORT_KEY is defined)
bool isSupportKeyEnabled();

} // namespace config
