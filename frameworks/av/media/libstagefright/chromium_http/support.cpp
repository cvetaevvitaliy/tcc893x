/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "ChromiumHTTPDataSourceSupport"
#include <utils/Log.h>

#include <media/stagefright/foundation/AString.h>

#include "support.h"

#include "android/net/android_network_library_impl.h"
#include "base/logging.h"
#include "base/threading/thread.h"
#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
#include "net/base/net_errors.h"
#endif
#include "net/base/cert_verifier.h"
#include "net/base/cookie_monster.h"
#include "net/base/host_resolver.h"
#include "net/base/ssl_config_service.h"
#include "net/http/http_auth_handler_factory.h"
#include "net/http/http_cache.h"
#include "net/proxy/proxy_config_service_android.h"

#include "include/ChromiumHTTPDataSource.h"

#include <cutils/log.h>
#include <cutils/properties.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/Utils.h>
#include <string>

namespace android {

static Mutex gNetworkThreadLock;
static base::Thread *gNetworkThread = NULL;
static scoped_refptr<SfRequestContext> gReqContext;
static scoped_ptr<net::NetworkChangeNotifier> gNetworkChangeNotifier;

bool logMessageHandler(
        int severity,
        const char* file,
        int line,
        size_t message_start,
        const std::string& str) {
    int androidSeverity = ANDROID_LOG_VERBOSE;
    switch(severity) {
    case logging::LOG_FATAL:
        androidSeverity = ANDROID_LOG_FATAL;
        break;
    case logging::LOG_ERROR_REPORT:
    case logging::LOG_ERROR:
        androidSeverity = ANDROID_LOG_ERROR;
        break;
    case logging::LOG_WARNING:
        androidSeverity = ANDROID_LOG_WARN;
        break;
    default:
        androidSeverity = ANDROID_LOG_VERBOSE;
        break;
    }
    android_printLog(androidSeverity, "chromium-libstagefright",
                    "%s:%d: %s", file, line, str.c_str());
    return false;
}

struct AutoPrioritySaver {
    AutoPrioritySaver()
        : mTID(androidGetTid()),
          mPrevPriority(androidGetThreadPriority(mTID)) {
        androidSetThreadPriority(mTID, ANDROID_PRIORITY_NORMAL);
    }

    ~AutoPrioritySaver() {
        androidSetThreadPriority(mTID, mPrevPriority);
    }

private:
    pid_t mTID;
    int mPrevPriority;

    DISALLOW_EVIL_CONSTRUCTORS(AutoPrioritySaver);
};

static void InitializeNetworkThreadIfNecessary() {
    Mutex::Autolock autoLock(gNetworkThreadLock);

    if (gNetworkThread == NULL) {
        // Make sure any threads spawned by the chromium framework are
        // running at normal priority instead of inheriting this thread's.
        AutoPrioritySaver saver;

        gNetworkThread = new base::Thread("network");
        base::Thread::Options options;
        options.message_loop_type = MessageLoop::TYPE_IO;
        CHECK(gNetworkThread->StartWithOptions(options));

        gReqContext = new SfRequestContext;

        gNetworkChangeNotifier.reset(net::NetworkChangeNotifier::Create());

        net::AndroidNetworkLibrary::RegisterSharedInstance(
                new SfNetworkLibrary);
        logging::SetLogMessageHandler(logMessageHandler);
    }
}

static void MY_LOGE(const char *s) {
    LOG_PRI(ANDROID_LOG_ERROR, LOG_TAG, "%s", s);
}

static void MY_LOGI(const char *s) {
    LOG_PRI(ANDROID_LOG_INFO, LOG_TAG, "%s", s);
}

static void MY_LOGV(const char *s) {
#if !defined(LOG_NDEBUG) || LOG_NDEBUG == 0
    LOG_PRI(ANDROID_LOG_VERBOSE, LOG_TAG, "%s", s);
#endif
}

SfNetLog::SfNetLog()
    : mNextID(1) {
}

void SfNetLog::AddEntry(
        EventType type,
        const base::TimeTicks &time,
        const Source &source,
        EventPhase phase,
        EventParameters *params) {
#if 0
    MY_LOGI(StringPrintf(
                "AddEntry time=%s type=%s source=%s phase=%s\n",
                TickCountToString(time).c_str(),
                EventTypeToString(type),
                SourceTypeToString(source.type),
                EventPhaseToString(phase)).c_str());
#endif
}

uint32 SfNetLog::NextID() {
    return mNextID++;
}

net::NetLog::LogLevel SfNetLog::GetLogLevel() const {
    return LOG_BASIC;
}

////////////////////////////////////////////////////////////////////////////////

SfRequestContext::SfRequestContext() {
    mUserAgent = MakeUserAgent().c_str();

    set_net_log(new SfNetLog());

    set_host_resolver(
        net::CreateSystemHostResolver(
                net::HostResolver::kDefaultParallelism,
                NULL /* resolver_proc */,
                net_log()));

    set_ssl_config_service(
        net::SSLConfigService::CreateSystemSSLConfigService());

    mProxyConfigService = new net::ProxyConfigServiceAndroid;

    set_proxy_service(net::ProxyService::CreateWithoutProxyResolver(
        mProxyConfigService, net_log()));

    set_http_transaction_factory(new net::HttpCache(
            host_resolver(),
            new net::CertVerifier(),
            dnsrr_resolver(),
            dns_cert_checker(),
            proxy_service(),
            ssl_config_service(),
            net::HttpAuthHandlerFactory::CreateDefault(host_resolver()),
            network_delegate(),
            net_log(),
            NULL));  // backend_factory

    set_cookie_store(new net::CookieMonster(NULL, NULL));
}

const std::string &SfRequestContext::GetUserAgent(const GURL &url) const {
    return mUserAgent;
}

status_t SfRequestContext::updateProxyConfig(
        const char *host, int32_t port, const char *exclusionList) {
    Mutex::Autolock autoLock(mProxyConfigLock);

    if (host == NULL || *host == '\0') {
        MY_LOGV("updateProxyConfig NULL");

        std::string proxy;
        std::string exList;
        mProxyConfigService->UpdateProxySettings(proxy, exList);
    } else {
#if !defined(LOG_NDEBUG) || LOG_NDEBUG == 0
        LOG_PRI(ANDROID_LOG_VERBOSE, LOG_TAG,
                "updateProxyConfig %s:%d, exclude '%s'",
                host, port, exclusionList);
#endif

        std::string proxy = StringPrintf("%s:%d", host, port).c_str();
        std::string exList = exclusionList;
        mProxyConfigService->UpdateProxySettings(proxy, exList);
    }

    return OK;
}

////////////////////////////////////////////////////////////////////////////////

SfNetworkLibrary::SfNetworkLibrary() {}

SfNetworkLibrary::VerifyResult SfNetworkLibrary::VerifyX509CertChain(
        const std::vector<std::string>& cert_chain,
        const std::string& hostname,
        const std::string& auth_type) {
    return VERIFY_OK;
}

////////////////////////////////////////////////////////////////////////////////

SfDelegate::SfDelegate()
    : mOwner(NULL),
      mURLRequest(NULL),
      mReadBuffer(new net::IOBufferWithSize(8192)),
      mNumBytesRead(0),
      mNumBytesTotal(0),
      mDataDestination(NULL),
#if _USE_TCC_AWESOMEPLAYER_
      mDummyRangeRequired(false),
#endif
      mAtEOS(false) {
#if _USE_TCC_AWESOMEPLAYER_
    char value[PROPERTY_VALUE_MAX];
    property_get("tcc.http.msg.log", value, "0");
    mEnableHttpMsgLog = (!atoi(value)) ? false : true;
#endif

    InitializeNetworkThreadIfNecessary();
}

SfDelegate::~SfDelegate() {
    CHECK(mURLRequest == NULL);
}

// static
status_t SfDelegate::UpdateProxyConfig(
        const char *host, int32_t port, const char *exclusionList) {
    InitializeNetworkThreadIfNecessary();

    return gReqContext->updateProxyConfig(host, port, exclusionList);
}

void SfDelegate::setOwner(ChromiumHTTPDataSource *owner) {
    mOwner = owner;
}

void SfDelegate::setUID(uid_t uid) {
    gReqContext->setUID(uid);
}

bool SfDelegate::getUID(uid_t *uid) const {
    return gReqContext->getUID(uid);
}

void SfDelegate::OnReceivedRedirect(
            net::URLRequest *request, const GURL &new_url, bool *defer_redirect) {
#if _USE_TCC_AWESOMEPLAYER_
	MY_LOGI(StringPrintf("OnReceivedRedirect: host %s", new_url.host().c_str()).c_str());
	MY_LOGI(StringPrintf("OnReceivedRedirect: port %s", new_url.port().c_str()).c_str());
	MY_LOGI(StringPrintf("OnReceivedRedirect: path %s", new_url.path().c_str()).c_str());
#endif
    MY_LOGV("OnReceivedRedirect");
}

void SfDelegate::OnAuthRequired(
            net::URLRequest *request, net::AuthChallengeInfo *auth_info) {
    MY_LOGV("OnAuthRequired");

    inherited::OnAuthRequired(request, auth_info);
}

void SfDelegate::OnCertificateRequested(
            net::URLRequest *request, net::SSLCertRequestInfo *cert_request_info) {
    MY_LOGV("OnCertificateRequested");

    inherited::OnCertificateRequested(request, cert_request_info);
}

void SfDelegate::OnSSLCertificateError(
            net::URLRequest *request, int cert_error, net::X509Certificate *cert) {
    fprintf(stderr, "OnSSLCertificateError cert_error=%d\n", cert_error);

    inherited::OnSSLCertificateError(request, cert_error, cert);
}

void SfDelegate::OnGetCookies(net::URLRequest *request, bool blocked_by_policy) {
    MY_LOGV("OnGetCookies");
}

void SfDelegate::OnSetCookie(
        net::URLRequest *request,
        const std::string &cookie_line,
        const net::CookieOptions &options,
        bool blocked_by_policy) {
    MY_LOGV("OnSetCookie");
}

void SfDelegate::OnResponseStarted(net::URLRequest *request) {
    if (request->status().status() != net::URLRequestStatus::SUCCESS) {
        MY_LOGE(StringPrintf(
                    "Request failed with status %d and os_error %d",
                    request->status().status(),
                    request->status().os_error()).c_str());

        delete mURLRequest;
        mURLRequest = NULL;

    #if _USE_TCC_AWESOMEPLAYER_
        if (request->status().status() == 4 &&
                (request->status().os_error() == net::ERR_ADDRESS_UNREACHABLE ||
                request->status().os_error() == net::ERR_CONNECTION_TIMED_OUT ||
                request->status().os_error() == net::ERR_NAME_NOT_RESOLVED)) { 
                mOwner->onConnectionFailed(ERROR_CONNECTION_LOST);
            return;
        }
    #endif
            mOwner->onConnectionFailed(ERROR_IO);
        return;
    } else if (mRangeRequested && request->GetResponseCode() != 206) {
        MY_LOGI(StringPrintf(
                    "We requested a content range, but server didn't "
                    "support that. (responded with %d)",
                    request->GetResponseCode()).c_str());

    #if _USE_TCC_AWESOMEPLAYER_
        int32_t httpStatus = request->GetResponseCode();
    #endif
        delete mURLRequest;
        mURLRequest = NULL;

        mOwner->onConnectionFailed(-EPIPE
            #if _USE_TCC_AWESOMEPLAYER_
                , httpStatus
            #endif
                );
        return;
    } else if ((request->GetResponseCode() / 100) != 2) {
        MY_LOGI(StringPrintf(
                    "Server responded with http status %d",
                    request->GetResponseCode()).c_str());

    #if _USE_TCC_AWESOMEPLAYER_
        int32_t httpStatus = request->GetResponseCode();
    #endif
        delete mURLRequest;
        mURLRequest = NULL;

        mOwner->onConnectionFailed(ERROR_IO
            #if _USE_TCC_AWESOMEPLAYER_
                , httpStatus
            #endif
                );
        return;
    }

    MY_LOGV("OnResponseStarted");

    std::string headers;
    request->GetAllResponseHeaders(&headers);

#if _USE_TCC_AWESOMEPLAYER_
    if (mEnableHttpMsgLog) { 
        MY_LOGI(StringPrintf("response headers: %s", headers.c_str()).c_str());
    }
#else
    MY_LOGV(StringPrintf("response headers: %s", headers.c_str()).c_str());
#endif

    std::string contentType;
    request->GetResponseHeaderByName("Content-Type", &contentType);

#ifdef _TCC_PROPRIETARY_DEPENDENCIES_
    std::string transferEncoding;
    request->GetResponseHeaderByName("Transfer-Encoding", &transferEncoding);
    if(transferEncoding.size() == 0) {
        request->GetResponseHeaderByName("Transfer-encoding", &transferEncoding);
    }
    if(transferEncoding.size() == 0) {
        request->GetResponseHeaderByName("TRANSFER-ENCODING", &transferEncoding);
    }
#if 0
    std::string modH264Streaming;
    request->GetResponseHeaderByName("X-Mod-H264-Streaming", &modH264Streaming);
    if (modH264Streaming.size() > 0) {
        mDummyRangeRequired = true; 

        delete mURLRequest;
        mURLRequest = NULL;

        mOwner->onConnectionFailed(ERROR_UNSUPPORTED);
        return;
    }
#endif
#endif

    mOwner->onConnectionEstablished(
            request->GetExpectedContentSize(), contentType.c_str()
        #ifdef _TCC_PROPRIETARY_DEPENDENCIES_
			, transferEncoding.c_str()
        #endif
			);
}

void SfDelegate::OnReadCompleted(net::URLRequest *request, int bytes_read) {
    if (bytes_read == -1) {
        MY_LOGI(StringPrintf(
                    "OnReadCompleted, read failed, status %d",
                    request->status().status()).c_str());

        mOwner->onReadCompleted(ERROR_IO);
        return;
    }

    MY_LOGV(StringPrintf("OnReadCompleted, read %d bytes", bytes_read).c_str());

    if (bytes_read < 0) {
        MY_LOGI(StringPrintf(
                    "Read failed w/ status %d\n",
                    request->status().status()).c_str());

        mOwner->onReadCompleted(ERROR_IO);
        return;
    } else if (bytes_read == 0) {
        mAtEOS = true;
        mOwner->onReadCompleted(mNumBytesRead);
        return;
    }

    CHECK_GT(bytes_read, 0);
    CHECK_LE(mNumBytesRead + bytes_read, mNumBytesTotal);

    memcpy((uint8_t *)mDataDestination + mNumBytesRead,
           mReadBuffer->data(),
           bytes_read);

    mNumBytesRead += bytes_read;

    readMore(request);
}

void SfDelegate::readMore(net::URLRequest *request) {
    while (mNumBytesRead < mNumBytesTotal) {
        size_t copy = mNumBytesTotal - mNumBytesRead;
        if (copy > mReadBuffer->size()) {
            copy = mReadBuffer->size();
        }

        int n;
        if (request->Read(mReadBuffer, copy, &n)) {
            MY_LOGV(StringPrintf("Read %d bytes directly.", n).c_str());

            CHECK_LE((size_t)n, copy);

            memcpy((uint8_t *)mDataDestination + mNumBytesRead,
                   mReadBuffer->data(),
                   n);

            mNumBytesRead += n;

            if (n == 0) {
                mAtEOS = true;
                break;
            }
        } else {
            MY_LOGV("readMore pending read");

            if (request->status().status() != net::URLRequestStatus::IO_PENDING) {
                MY_LOGI(StringPrintf(
                            "Direct read failed w/ status %d\n",
                            request->status().status()).c_str());

                mOwner->onReadCompleted(ERROR_IO);
                return;
            }

            return;
        }
    }

    mOwner->onReadCompleted(mNumBytesRead);
}

void SfDelegate::initiateConnection(
        const char *uri,
        const KeyedVector<String8, String8> *headers,
        off64_t offset) {
    GURL url(uri);

    MessageLoop *loop = gNetworkThread->message_loop();
    loop->PostTask(
            FROM_HERE,
            NewRunnableFunction(
                &SfDelegate::OnInitiateConnectionWrapper,
                this,
                url,
                headers,
                offset));

}

// static
void SfDelegate::OnInitiateConnectionWrapper(
        SfDelegate *me, GURL url,
        const KeyedVector<String8, String8> *headers,
        off64_t offset) {
    me->onInitiateConnection(url, headers, offset);
}

void SfDelegate::onInitiateConnection(
        const GURL &url,
        const KeyedVector<String8, String8> *extra,
        off64_t offset) {
    CHECK(mURLRequest == NULL);

    mURLRequest = new net::URLRequest(url, this);
    mAtEOS = false;

    mRangeRequested = false;

    if (offset != 0 || extra != NULL
        #if _USE_TCC_AWESOMEPLAYER_
	        || mDummyRangeRequired
        #endif
		) {
        net::HttpRequestHeaders headers =
            mURLRequest->extra_request_headers();

        if (offset != 0
        #if _USE_TCC_AWESOMEPLAYER_
	        || mDummyRangeRequired
        #endif
			) {
            headers.AddHeaderFromString(
                    StringPrintf("Range: bytes=%lld-", offset).c_str());

            mRangeRequested = true;
        }

    #if _USE_TCC_AWESOMEPLAYER_
        // Replace User-Agent if requested.
        int32_t ua_index = -1;

        if (extra != NULL) {
            ua_index = (*extra).indexOfKey(String8("x-requested-user-agent"));

            if (ua_index >= 0) {
                std::string user_agent((*extra).valueAt(ua_index).string());
                headers.SetHeader("User-Agent", user_agent);
            }
        } 

        if (ua_index < 0) {
            std::string user_agent("AppleCoreMedia/1.0.0.7B367 (iPad; U; CPU OS 4_3_3 like Mac OS X; en-us)");
            headers.SetHeader("User-Agent", user_agent);
        }
    #else
        std::string user_agent("Mozilla/5.0 (iPad; U; CPU OS 4_3_3 like Mac OS X; en-us) "
                               " AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 "
                               "Mobile/8J2 Safari/6533.18.5");
        headers.SetHeader("User-Agent", user_agent);
    #endif

        if (extra != NULL) {
            for (size_t i = 0; i < extra->size(); ++i) {
            #if _USE_TCC_AWESOMEPLAYER_
                if (i == (size_t)ua_index) {
                    continue;
                }
            #endif
                AString s;
                s.append(extra->keyAt(i).string());
                s.append(": ");
                s.append(extra->valueAt(i).string());

                headers.AddHeaderFromString(s.c_str());
            }
        }

        mURLRequest->SetExtraRequestHeaders(headers);
    }

    mURLRequest->set_context(gReqContext);

    mURLRequest->Start();
}

void SfDelegate::initiateDisconnect() {
    MessageLoop *loop = gNetworkThread->message_loop();
    loop->PostTask(
            FROM_HERE,
            NewRunnableFunction(
                &SfDelegate::OnInitiateDisconnectWrapper, this));
}

// static
void SfDelegate::OnInitiateDisconnectWrapper(SfDelegate *me) {
    me->onInitiateDisconnect();
}

void SfDelegate::onInitiateDisconnect() {
    if (mURLRequest == NULL) {
        return;
    }

    mURLRequest->Cancel();

    delete mURLRequest;
    mURLRequest = NULL;

    mOwner->onDisconnectComplete();
}

void SfDelegate::initiateRead(void *data, size_t size) {
    MessageLoop *loop = gNetworkThread->message_loop();
    loop->PostTask(
            FROM_HERE,
            NewRunnableFunction(
                &SfDelegate::OnInitiateReadWrapper, this, data, size));
}

// static
void SfDelegate::OnInitiateReadWrapper(
        SfDelegate *me, void *data, size_t size) {
    me->onInitiateRead(data, size);
}

void SfDelegate::onInitiateRead(void *data, size_t size) {
    CHECK(mURLRequest != NULL);

    mNumBytesRead = 0;
    mNumBytesTotal = size;
    mDataDestination = data;

    if (mAtEOS) {
        mOwner->onReadCompleted(0);
        return;
    }

    readMore(mURLRequest);
}

}  // namespace android

