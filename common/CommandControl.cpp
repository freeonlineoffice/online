/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "CommandControl.hpp"

#include <common/ConfigUtil.hpp>
#include <common/RegexUtil.hpp>
#include <common/Util.hpp>

#include <string>
#include <unordered_set>

namespace CommandControl
{
bool LockManager::_isLockedUser = false;
bool LockManager::_isHostReadOnly = false;
std::unordered_set<std::string> LockManager::LockedCommandList;
std::string LockManager::LockedCommandListString;
RegexUtil::RegexListMatcher LockManager::readOnlyWopiHosts;
RegexUtil::RegexListMatcher LockManager::disabledCommandWopiHosts;
std::map<std::string, std::string> LockManager::unlockLinkMap;
bool LockManager::lockHostEnabled = false;
std::string LockManager::translationPath = std::string();
std::string LockManager::unlockLink = std::string();

} // namespace CommandControl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
