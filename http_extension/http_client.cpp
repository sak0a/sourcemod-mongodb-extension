/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod HTTP MongoDB Extension - HTTP Client Implementation
 * Copyright (C) 2024 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 */

#include "http_client.h"
#include <chrono>
#include <thread>
#include <cstring>

HTTPClient::HTTPClient(const std::string& baseUrl) 
    : m_baseUrl(baseUrl), m_timeout(30000), m_retryCount(3), 
      m_userAgent("SourceMod-MongoDB-Extension/1.0"), m_curl(nullptr), 
      m_initialized(false), m_pThreader(nullptr) {
    
    // Initialize stats
    memset(&m_stats, 0, sizeof(m_stats));
    
    // Set default headers
    m_headers["Content-Type"] = "application/json";
    m_headers["Accept"] = "application/json";
}

HTTPClient::~HTTPClient() {
    Shutdown();
}

bool HTTPClient::Initialize() {
    if (m_initialized) {
        return true;
    }
    
    // Initialize libcurl
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK) {
        m_lastError = "Failed to initialize libcurl: ";
        m_lastError += curl_easy_strerror(res);
        return false;
    }
    
    // Create curl handle
    m_curl = curl_easy_init();
    if (!m_curl) {
        m_lastError = "Failed to create CURL handle";
        curl_global_cleanup();
        return false;
    }
    
    // Get threader interface
    m_pThreader = threader;
    if (!m_pThreader) {
        m_lastError = "Failed to get threader interface";
        curl_easy_cleanup(m_curl);
        curl_global_cleanup();
        return false;
    }
    
    m_initialized = true;
    return true;
}

void HTTPClient::Shutdown() {
    if (!m_initialized) {
        return;
    }
    
    if (m_curl) {
        curl_easy_cleanup(m_curl);
        m_curl = nullptr;
    }
    
    curl_global_cleanup();
    m_initialized = false;
}

bool HTTPClient::SendRequest(const std::string& endpoint, const std::string& method,
                           const std::string& data, std::string& response) {
    if (!m_initialized) {
        m_lastError = "HTTP client not initialized";
        return false;
    }
    
    std::string url = m_baseUrl + endpoint;
    response.clear();
    
    auto startTime = std::chrono::steady_clock::now();
    bool success = false;
    int httpCode = 0;
    
    // Retry logic
    for (int attempt = 0; attempt <= m_retryCount; ++attempt) {
        if (PerformRequest(m_curl, url, method, data, response, httpCode)) {
            success = true;
            break;
        }
        
        if (attempt < m_retryCount && ShouldRetry(httpCode, m_lastError, attempt)) {
            // Exponential backoff: 100ms, 200ms, 400ms, etc.
            int delayMs = 100 * (1 << attempt);
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
            m_stats.retryCount++;
        } else {
            break;
        }
    }
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    UpdateStats(success, duration.count());
    
    return success;
}

bool HTTPClient::SendRequestAsync(const std::string& endpoint, const std::string& method,
                                 const std::string& data, IPluginFunction* callback, 
                                 cell_t userData) {
    if (!m_initialized) {
        m_lastError = "HTTP client not initialized";
        return false;
    }
    
    if (!callback) {
        m_lastError = "Callback function is required for async requests";
        return false;
    }
    
    // Create request data
    RequestData* requestData = new RequestData();
    requestData->url = m_baseUrl + endpoint;
    requestData->method = method;
    requestData->data = data;
    requestData->callback = callback;
    requestData->userData = userData;
    requestData->success = false;
    requestData->httpCode = 0;
    requestData->startTime = std::chrono::steady_clock::now();
    
    // Queue async request
    if (!m_pThreader->MakeThread(AsyncRequestThread, requestData)) {
        m_lastError = "Failed to create async request thread";
        delete requestData;
        return false;
    }
    
    return true;
}

void HTTPClient::AsyncRequestThread(void* data) {
    RequestData* requestData = static_cast<RequestData*>(data);
    
    // Create a new CURL handle for this thread
    CURL* curl = curl_easy_init();
    if (!curl) {
        requestData->success = false;
        requestData->error = "Failed to create CURL handle for async request";
        
        // Call callback with error
        if (requestData->callback) {
            requestData->callback->PushCell(0); // success = false
            requestData->callback->PushString(""); // empty response
            requestData->callback->PushString(requestData->error.c_str());
            requestData->callback->PushCell(requestData->userData);
            requestData->callback->Execute(nullptr);
        }
        
        delete requestData;
        return;
    }
    
    // Perform the request
    // Note: We need to access the HTTPClient instance to get configuration
    // For now, we'll use default settings
    // TODO: Pass HTTPClient instance or configuration to thread
    
    curl_easy_cleanup(curl);
    delete requestData;
}

bool HTTPClient::PerformRequest(CURL* curl, const std::string& url, const std::string& method,
                               const std::string& data, std::string& response, int& httpCode) {
    response.clear();
    
    // Setup CURL options
    SetupCurlOptions(curl, url, method, data, response);
    
    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        m_lastError = "CURL request failed: ";
        m_lastError += curl_easy_strerror(res);
        return false;
    }
    
    // Get HTTP response code
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    
    // Check if HTTP status indicates success (200-299)
    if (httpCode < 200 || httpCode >= 300) {
        m_lastError = "HTTP request failed with status code: " + std::to_string(httpCode);
        return false;
    }
    
    return true;
}

void HTTPClient::SetupCurlOptions(CURL* curl, const std::string& url, const std::string& method,
                                 const std::string& data, std::string& response) {
    // Basic options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, m_timeout);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, m_userAgent.c_str());
    
    // SSL options (for HTTPS)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    // Set HTTP method and data
    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
    } else if (method == "PUT") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
    } else if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        if (!data.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
        }
    } else {
        // Default to GET
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    }
    
    // Set headers
    struct curl_slist* headers = nullptr;
    for (const auto& header : m_headers) {
        std::string headerStr = header.first + ": " + header.second;
        headers = curl_slist_append(headers, headerStr.c_str());
    }
    
    if (headers) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        // Note: headers will be freed after curl_easy_perform
        // In a production implementation, we should manage this properly
    }
}

size_t HTTPClient::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

size_t HTTPClient::HeaderCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    // Header processing can be implemented here if needed
    return totalSize;
}

bool HTTPClient::ShouldRetry(int httpCode, const std::string& error, int attempt) {
    // Retry on network errors or server errors (5xx)
    if (httpCode == 0 || (httpCode >= 500 && httpCode < 600)) {
        return true;
    }
    
    // Retry on specific CURL errors
    if (error.find("timeout") != std::string::npos ||
        error.find("connection") != std::string::npos) {
        return true;
    }
    
    return false;
}

void HTTPClient::UpdateStats(bool success, double responseTime) {
    m_stats.totalRequests++;
    
    if (success) {
        m_stats.successfulRequests++;
    } else {
        m_stats.failedRequests++;
    }
    
    // Update average response time
    if (m_stats.totalRequests == 1) {
        m_stats.averageResponseTime = responseTime;
    } else {
        m_stats.averageResponseTime = 
            (m_stats.averageResponseTime * (m_stats.totalRequests - 1) + responseTime) / 
            m_stats.totalRequests;
    }
}

void HTTPClient::SetTimeout(int timeoutMs) {
    m_timeout = timeoutMs;
}

void HTTPClient::SetRetryCount(int retries) {
    m_retryCount = retries;
}

void HTTPClient::AddHeader(const std::string& key, const std::string& value) {
    m_headers[key] = value;
}

void HTTPClient::SetUserAgent(const std::string& userAgent) {
    m_userAgent = userAgent;
}

void HTTPClient::ResetStats() {
    memset(&m_stats, 0, sizeof(m_stats));
}
