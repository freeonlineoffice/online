/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

// HostUtil is only used in non-mobile apps.
#if !MOBILEAPP

#include "HostUtil.hpp"

#include <common/CommandControl.hpp>
#include <common/ConfigUtil.hpp>
#include <common/Log.hpp>
#include <common/RegexUtil.hpp>

#include <map>
#include <set>
#include <string>

RegexUtil::RegexListMatcher HostUtil::WopiHosts;
std::map<std::string, std::string> HostUtil::AliasHosts;
std::set<std::string> HostUtil::hostList;
std::string HostUtil::FirstHost;
bool HostUtil::WopiEnabled;
std::set<std::string> HostUtil::AllowedWSOriginList;

void HostUtil::parseWopiHost(const Poco::Util::LayeredConfiguration& conf)
{
    // Parse the WOPI settings.
    WopiHosts.clear();
    WopiEnabled = conf.getBool("storage.wopi[@allow]", false);
    if (WopiEnabled)
    {
        for (size_t i = 0;; ++i)
        {
            const std::string path = "storage.wopi.host[" + std::to_string(i) + ']';
            if (!conf.has(path))
            {
                break;
            }

            HostUtil::addWopiHost(conf.getString(path, ""),
                                      conf.getBool(path + "[@allow]", false));
        }
    }
}

void HostUtil::addWopiHost(const std::string& host, bool allow)
{
    if (!host.empty())
    {
        if (allow)
        {
            LOG_INF("Adding trusted WOPI host: [" << host << "].");
            WopiHosts.allow(host);
        }
        else
        {
            LOG_INF("Adding blocked WOPI host: [" << host << "].");
            WopiHosts.deny(host);
        }
    }
}

bool HostUtil::allowedWopiHost(const std::string& host)
{
    return WopiEnabled && WopiHosts.match(host);
}

void HostUtil::parseAliases(Poco::Util::LayeredConfiguration& conf)
{
    WopiEnabled = conf.getBool("storage.wopi[@allow]", false);

    //set alias_groups mode to compat
    if (!conf.has("storage.wopi.alias_groups"))
    {
        conf.setString("storage.wopi.alias_groups[@mode]", "compat");
        return;
    }

    if (conf.has("storage.wopi.alias_groups.group[0]"))
    {
        // group defined in alias_groups
        if (Util::iequal(ConfigUtil::getString("storage.wopi.alias_groups[@mode]", "first"),
                         "first"))
        {
            LOG_ERR("Admins did not set the alias_groups mode to 'groups'");
            AliasHosts.clear();
            return;
        }
    }

    AliasHosts.clear();
    WopiHosts.clear();
    hostList.clear();
#if ENABLE_FEATURE_LOCK
    CommandControl::LockManager::unlockLinkMap.clear();
#endif

    for (size_t i = 0;; i++)
    {
        const std::string path = "storage.wopi.alias_groups.group[" + std::to_string(i) + ']';
        if (!conf.has(path + ".host"))
        {
            break;
        }

        const std::string uri = conf.getString(path + ".host", "");
        if (uri.empty())
        {
            continue;
        }

        const bool allow = conf.getBool(path + ".host[@allow]", false);
        const Poco::URI realUri(uri);

        try
        {
#if ENABLE_FEATURE_LOCK
            CommandControl::LockManager::mapUnlockLink(realUri.getHost(), path);
#endif
            HostUtil::hostList.insert(realUri.getHost());
            HostUtil::addWopiHost(realUri.getHost(), allow);
        }
        catch (const Poco::Exception& exc)
        {
            LOG_WRN("parseAliases: " << exc.displayText());
        }

        for (size_t j = 0;; j++)
        {
            const std::string aliasPath = path + ".alias[" + std::to_string(j) + ']';
            if (!conf.has(aliasPath))
            {
                break;
            }

            try
            {
                const Poco::URI aliasUri(conf.getString(aliasPath, ""));
                if (aliasUri.empty())
                {
                    continue;
                }

                for (const std::string& x : Util::splitStringToVector(aliasUri.getHost(), '|'))
                {
                    const Poco::URI aUri(aliasUri.getScheme() + "://" + x + ':' +
                                         std::to_string(aliasUri.getPort()));
                    LOG_DBG("Mapped URI alias [" << aUri.getAuthority() << "] to canonical URI ["
                                                 << realUri.getAuthority() << ']');
                    AliasHosts.emplace(aUri.getAuthority(), realUri.getAuthority());
#if ENABLE_FEATURE_LOCK
                    CommandControl::LockManager::mapUnlockLink(aUri.getHost(), path);
#endif
                    HostUtil::addWopiHost(aUri.getHost(), allow);
                }
            }
            catch (const Poco::Exception& exc)
            {
                LOG_WRN("parseAliases: " << exc.displayText());
            }
        }
    }
}

std::string HostUtil::getNewUri(const Poco::URI& uri)
{
    if (Util::iequal(ConfigUtil::getString("storage.wopi.alias_groups[@mode]", "first"), "compat"))
    {
        return uri.getPath();
    }

    Poco::URI newUri(uri);
    const std::string value = RegexUtil::getValue(AliasHosts, newUri.getAuthority());
    if (!value.empty())
    {
        newUri.setAuthority(value);
    }
    else
    {
        // It is allowed for the host to be a regex.
        // In that case, the first who connects is treated as the 'host', and stored to the AliasHosts here
        const std::string val = RegexUtil::getValue(hostList, newUri.getHost());
        // compare incoming request's host with existing hostList , if they are not equal it is regex and we store
        // the pair in AliasHosts
        if (val.compare(newUri.getHost()) != 0)
        {
            LOG_DBG("Mapped URI alias [" << val << "] to canonical URI [" << newUri.getHost()
                                         << ']');
            AliasHosts.emplace(val, newUri.getHost());
        }
    }

    if (newUri.getAuthority().empty())
    {
        return newUri.getPath();
    }

    return newUri.getScheme() + "://" + newUri.getHost() + ':' + std::to_string(newUri.getPort()) +
           newUri.getPath();
}

const Poco::URI HostUtil::getNewLockedUri(const Poco::URI& uri)
{
    Poco::URI newUri(uri);
    const std::string value = RegexUtil::getValue(AliasHosts, newUri.getAuthority());
    if (!value.empty())
    {
        newUri.setAuthority(value);
        LOG_WRN("The locked_host: " << uri.getAuthority() << " is alias of "
                                    << newUri.getAuthority() << ", Applying "
                                    << newUri.getAuthority() << " locked_host settings.");
    }

    return newUri;
}

void HostUtil::setFirstHost(const Poco::URI& uri)
{
    if (Util::iequal(ConfigUtil::getString("storage.wopi.alias_groups[@mode]", "first"), "compat"))
    {
        return;
    }

    if (WopiHosts.empty())
    {
        if (FirstHost.empty())
        {
            FirstHost = uri.getAuthority();
            addWopiHost(uri.getHost(), true);
        }
    }
    else if(!FirstHost.empty() && FirstHost != uri.getAuthority())
    {
        LOG_ERR("Only allowed host is: "
                << FirstHost
                << ", To use multiple host/aliases check alias_groups tag in configuration");
    }
}

void HostUtil::parseAllowedWSOrigins(Poco::Util::LayeredConfiguration& conf)
{
    for (size_t i = 0;; i++)
    {
        const std::string path =
            "indirection_endpoint.geolocation_setup.allowed_websocket_origins.origin[" +
            std::to_string(i) + ']';
        if (!conf.has(path))
        {
            break;
        }
        std::string origin = conf.getString(path, "");
        if (!origin.empty())
        {
            LOG_INF("Adding Origin[" << origin << "] to allowed websocket origin list");
            HostUtil::AllowedWSOriginList.insert(std::move(origin));
        }
    }
}

bool HostUtil::allowedWSOrigin(const std::string& origin)
{
    return AllowedWSOriginList.find(origin) != AllowedWSOriginList.end();
}

bool HostUtil::isWopiHostsEmpty()
{
    return WopiHosts.empty();
}

#endif // !MOBILEAPP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
