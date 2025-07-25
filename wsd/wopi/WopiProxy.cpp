/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "WopiProxy.hpp"

#include <common/Anonymizer.hpp>
#include "FileUtil.hpp"
#include "HttpHelper.hpp"
#include "HttpRequest.hpp"
#include "Protocol.hpp"
#include <LOOLWSD.hpp>
#include <Exceptions.hpp>
#include <Log.hpp>
#include <Util.hpp>
#include <common/JsonUtil.hpp>
#include <wopi/StorageConnectionManager.hpp>
#include <wopi/WopiStorage.hpp>

void WopiProxy::handleRequest([[maybe_unused]] const std::shared_ptr<TerminatingPoll>& poll,
                              SocketDisposition& disposition)
{
    std::string url = _requestDetails.getDocumentURI();
    if (url.starts_with("/wasm/"))
    {
        url = url.substr(6);
    }

    LOG_INF("URL [" << url << "] for WS Request.");

    std::shared_ptr<StreamSocket> socket = _socket.lock();
    if (!socket)
    {
        LOG_ERR("Invalid socket while handling wopi proxy request for [" << LOOLWSD::anonymizeUrl(url) << ']');
        return;
    }

    const auto uriPublic = RequestDetails::sanitizeURI(url);
    std::string docKey = RequestDetails::getDocKey(uriPublic);
    const std::string fileId = Uri::getFilenameFromURL(Uri::decode(docKey));
    Anonymizer::mapAnonymized(fileId,
                              fileId); // Identity mapping, since fileId is already obfuscated

    LOG_INF("Starting GET request handler for session [" << _id << "] on url ["
                                                         << LOOLWSD::anonymizeUrl(url) << "].");

    LOG_INF("Sanitized URI [" << LOOLWSD::anonymizeUrl(url) << "] to ["
                              << LOOLWSD::anonymizeUrl(uriPublic.toString())
                              << "] and mapped to docKey [" << docKey << "] for session [" << _id
                              << "].");

    // Before we create DocBroker with a SocketPoll thread, a ClientSession, and a Kit process,
    // we need to vet this request by invoking CheckFileInfo.
    // For that, we need the storage settings to create a connection.
    const StorageBase::StorageType storageType =
        StorageBase::validate(uriPublic, /*takeOwnership=*/false);
    switch (storageType)
    {
        case StorageBase::StorageType::Unsupported:
            LOG_ERR("Unsupported URI [" << LOOLWSD::anonymizeUrl(uriPublic.toString())
                                        << "] or no storage configured");
            throw BadRequestException("No Storage configured or invalid URI [" +
                                      LOOLWSD::anonymizeUrl(uriPublic.toString()) + ']');
            break;

        case StorageBase::StorageType::Unauthorized:
            LOG_ERR("No authorized hosts found matching the target host [" << uriPublic.getHost()
                                                                           << "] in config");
            HttpHelper::sendErrorAndShutdown(http::StatusCode::Unauthorized, socket);
            break;

        case StorageBase::StorageType::Conversion:
            // We don't expect conversion requests.
            LOG_ERR("Unsupported URI [" << LOOLWSD::anonymizeUrl(uriPublic.toString())
                                        << "] for conversion");
            throw BadRequestException("Invalid URI for conversion [" +
                                      LOOLWSD::anonymizeUrl(uriPublic.toString()) + ']');
            break;

#if ENABLE_LOCAL_FILESYSTEM
        case StorageBase::StorageType::FileSystem:
        {
            LOG_INF("URI [" << LOOLWSD::anonymizeUrl(uriPublic.toString()) << "] on docKey ["
                            << docKey << "] is for a FileSystem document");

            // Send the file contents.
            std::unique_ptr<std::vector<char>> data = FileUtil::readFile(uriPublic.getPath());
            if (data)
            {
                http::Response response(http::StatusCode::OK);
                response.setBody(std::string(data->data(), data->size()),
                                 "application/octet-stream");
                socket->sendAndShutdown(response);
            }
            else
            {
                HttpHelper::sendErrorAndShutdown(http::StatusCode::NotFound, socket);
            }
            break;
        }
#endif // ENABLE_LOCAL_FILESYSTEM

#if !MOBILEAPP
        case StorageBase::StorageType::Wopi:
            LOG_INF("URI [" << LOOLWSD::anonymizeUrl(uriPublic.toString()) << "] on docKey ["
                            << docKey << "] is for a WOPI document");
            // Remove from the current poll and transfer.
            disposition.setTransfer(*poll,
                [this, &poll, docKey = std::move(docKey), url = std::move(url),
                 uriPublic](const std::shared_ptr<Socket>& moveSocket)
                {
                    LOG_TRC_S('#' << moveSocket->getFD()
                                  << ": Dissociating client socket from "
                                     "ClientRequestDispatcher and invoking CheckFileInfo for ["
                                  << docKey << ']');

                    // CheckFileInfo and only when it's good create DocBroker.
                    checkFileInfo(poll, uriPublic, HTTP_REDIRECTION_LIMIT);
                });
            break;
#endif //!MOBILEAPP
    }
}

#if !MOBILEAPP
void WopiProxy::checkFileInfo(const std::shared_ptr<TerminatingPoll>& poll, const Poco::URI& uri,
                              int redirectLimit)
{
    auto cfiContinuation = [this, poll, uri]([[maybe_unused]] CheckFileInfo& checkFileInfo)
    {
        const std::string uriAnonym = LOOLWSD::anonymizeUrl(uri.toString());

        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (!socket)
        {
            LOG_ERR("Invalid socket while handling wopi CheckFileInfo for [" << uriAnonym << ']');
            return;
        }

        assert(&checkFileInfo == _checkFileInfo.get() && "Unknown CheckFileInfo instance");
        if (_checkFileInfo && _checkFileInfo->state() == CheckFileInfo::State::Pass &&
            _checkFileInfo->wopiInfo())
        {
            Poco::JSON::Object::Ptr object = _checkFileInfo->wopiInfo();

            std::size_t size = 0;
            std::string filename, ownerId, lastModifiedTime;
            JsonUtil::findJSONValue(object, "Size", size);
            JsonUtil::findJSONValue(object, "OwnerId", ownerId);
            JsonUtil::findJSONValue(object, "BaseFileName", filename);
            JsonUtil::findJSONValue(object, "LastModifiedTime", lastModifiedTime);

            const StorageBase::FileInfo fileInfo(size, std::move(filename), std::move(ownerId),
                                                 std::move(lastModifiedTime));

            // if (LOOLWSD::AnonymizeUserData)
            //     Anonymizer::mapAnonymized(Uri::getFilenameFromURL(filename),
            //                         Uri::getFilenameFromURL(getUri().toString()));

            auto wopiInfo = std::make_unique<WopiStorage::WOPIFileInfo>(fileInfo, object, uri);
            // if (wopiInfo->getSupportsLocks())
            //     lockCtx.initSupportsLocks();

            std::string url = checkFileInfo.url().toString();

            // If FileUrl is set, we use it for GetFile.
            const std::string fileUrl = wopiInfo->getFileUrl();

            // First try the FileUrl, if provided.
            if (!fileUrl.empty())
            {
                const std::string fileUrlAnonym = LOOLWSD::anonymizeUrl(fileUrl);
                const auto uriPublic = RequestDetails::sanitizeURI(url);
                try
                {
                    LOG_INF("WOPI::GetFile using FileUrl: " << fileUrlAnonym);
                    return download(poll, url, Poco::URI(fileUrl), HTTP_REDIRECTION_LIMIT);
                }
                catch (const std::exception& ex)
                {
                    LOG_ERR("Could not download document from WOPI FileUrl [" + fileUrlAnonym +
                                "]. Will use default URL. Error: "
                            << ex.what());
                    // Fall-through.
                }
            }

            // Try the default URL, we either don't have FileUrl, or it failed.
            // WOPI URI to download files ends in '/contents'.
            // Add it here to get the payload instead of file info.
            Poco::URI uriObject(uri);
            uriObject.setPath(uriObject.getPath() + "/contents");
            url = uriObject.toString();

            try
            {
                LOG_INF("WOPI::GetFile using default URI: " << uriAnonym);
                return download(poll, url, uriObject, HTTP_REDIRECTION_LIMIT);
            }
            catch (const std::exception& ex)
            {
                LOG_ERR(
                    "Cannot download document from WOPI storage uri [" + uriAnonym + "]. Error: "
                    << ex.what());
                // Fall-through.
            }
        }

        LOG_ERR("Invalid URI or access denied to [" << uriAnonym << ']');
        HttpHelper::sendErrorAndShutdown(http::StatusCode::Unauthorized, socket);
    };

    // CheckFileInfo asynchronously.
    _checkFileInfo = std::make_shared<CheckFileInfo>(poll, uri, std::move(cfiContinuation));
    _checkFileInfo->checkFileInfo(redirectLimit);
}

void WopiProxy::download(const std::shared_ptr<TerminatingPoll>& poll, const std::string& url,
                         const Poco::URI& uriPublic, int redirectLimit)
{
    std::string uriAnonym = LOOLWSD::anonymizeUrl(uriPublic.toString());

    LOG_DBG("Getting info for wopi uri [" << uriAnonym << ']');
    _httpSession = StorageConnectionManager::getHttpSession(uriPublic);
    Authorization auth = Authorization::create(uriPublic);
    const http::Request httpRequest = StorageConnectionManager::createHttpRequest(uriPublic, auth);

    const auto startTime = std::chrono::steady_clock::now();

    LOG_TRC("WOPI::GetFile request header for URI [" << uriAnonym << "]:\n"
                                                     << httpRequest.header());

    http::Session::FinishedCallback finishedCallback =
        [this, &poll, startTime, url, uriAnonym=std::move(uriAnonym),
         redirectLimit](const std::shared_ptr<http::Session>& session)
    {
        if (SigUtil::getShutdownRequestFlag())
        {
            LOG_DBG("Shutdown flagged, giving up on in-flight requests");
            return;
        }

        std::shared_ptr<StreamSocket> socket = _socket.lock();
        if (!socket)
        {
            LOG_ERR("Invalid socket while downloading [" << uriAnonym << ']');
            return;
        }

        const std::shared_ptr<const http::Response> httpResponse = session->response();
        LOG_TRC("WOPI::GetFile returned " << httpResponse->statusLine().statusCode());

        const http::StatusCode statusCode = httpResponse->statusLine().statusCode();
        if (statusCode == http::StatusCode::MovedPermanently ||
            statusCode == http::StatusCode::Found ||
            statusCode == http::StatusCode::TemporaryRedirect ||
            statusCode == http::StatusCode::PermanentRedirect)
        {
            if (redirectLimit)
            {
                const std::string& location = httpResponse->get("Location");
                LOG_TRC("WOPI::GetFile redirect to URI [" << LOOLWSD::anonymizeUrl(location)
                                                          << "]");

                download(poll, location, Poco::URI(location), redirectLimit - 1);
                return;
            }
            else
            {
                LOG_WRN("WOPI::GetFile redirected too many times. Giving up on URI [" << uriAnonym
                                                                                      << ']');
            }
        }

        std::chrono::milliseconds callDurationMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() -
                                                                  startTime);
        (void)callDurationMs;

        // Note: we don't log the response if obfuscation is enabled, except for failures.
        const bool failed = (httpResponse->statusLine().statusCode() != http::StatusCode::OK);
        if (Log::isEnabled(failed ? Log::Level::ERR : Log::Level::TRC))
        {
            const std::string& wopiResponse = httpResponse->getBody();

            std::ostringstream oss;
            oss << "WOPI::GetFile " << (failed ? "failed" : "returned") << " for URI [" << uriAnonym
                << "]: " << httpResponse->statusLine().statusCode() << ' '
                << httpResponse->statusLine().reasonPhrase()
                << ". Headers: " << httpResponse->header()
                << (failed ? "\tBody: [" + LOOLProtocol::getAbbreviatedMessage(wopiResponse) + ']'
                           : std::string());

            if (failed)
            {
                LOG_ERR(oss.str());
            }
            else
            {
                LOG_TRC(oss.str());
            }
        }

        if (failed)
        {
            if (httpResponse->statusLine().statusCode() == http::StatusCode::Forbidden)
            {
                LOG_ERR("Access denied to [" << uriAnonym << ']');
                HttpHelper::sendErrorAndShutdown(http::StatusCode::Forbidden, socket);
                return;
            }

            LOG_ERR("Invalid URI or access denied to [" << uriAnonym << ']');
            HttpHelper::sendErrorAndShutdown(http::StatusCode::Unauthorized, socket);
            return;
        }

        http::Response response(http::StatusCode::OK);
        response.setBody(httpResponse->getBody(), "application/octet-stream");
        socket->sendAndShutdown(response);
    };

    _httpSession->setFinishedHandler(std::move(finishedCallback));

    // Run the GET request on the WebServer Poll.
    _httpSession->asyncRequest(httpRequest, poll);
}
#endif //!MOBILEAPP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
