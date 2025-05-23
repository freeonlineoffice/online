/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>

class StreamSocket;
namespace http
{
class Response;
}

class ProxyRequestHandler
{
public:
    static void handleRequest(const std::string& relPath,
                              const std::shared_ptr<StreamSocket>& socket,
                              const std::string& serverUri);
    static const std::string& getProxyRatingServer()
    {
        static const std::string ProxyRatingServer = "https://rating.collaboraonline.com";
        return ProxyRatingServer;
    }

private:
    static std::chrono::system_clock::time_point MaxAge;
    static std::unordered_map<std::string, std::shared_ptr<http::Response>> CacheFileHash;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
