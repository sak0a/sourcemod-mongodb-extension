/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod HTTP MongoDB Extension - HTTP Client
 * Copyright (C) 2024 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 */

#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_

#include <string>
#include <map>
#include <memory>
#include <chrono>
#include <curl/curl.h>
#include <IThreader.h>
#include <IPluginSys.h>

using namespace SourceMod;

/**
 * @brief HTTP Client for MongoDB API communication
 * 
 * Handles HTTP requests to the MongoDB API service with support for
 * synchronous and asynchronous operations, connection pooling, and retry logic.
 */
class HTTPClient {
public:
    HTTPClient(const std::string& baseUrl);
    ~HTTPClient();
    
    bool Initialize();
    void Shutdown();
    
    // Synchronous requests
    bool SendRequest(const std::string& endpoint, const std::string& method,
                    const std::string& data, std::string& response);
    
    // Asynchronous requests
    bool SendRequestAsync(const std::string& endpoint, const std::string& method,
                         const std::string& data, IPluginFunction* callback, 
                         cell_t userData);
    
    // Configuration
    void SetTimeout(int timeoutMs);
    void SetRetryCount(int retries);
    void AddHeader(const std::string& key, const std::string& value);
    void SetUserAgent(const std::string& userAgent);
    
    // Error handling
    const std::string& GetLastError() const { return m_lastError; }
    void ClearLastError() { m_lastError.clear(); }
    
    // Statistics
    struct Stats {
        size_t totalRequests;
        size_t successfulRequests;
        size_t failedRequests;
        size_t retryCount;
        double averageResponseTime;
    };
    
    const Stats& GetStats() const { return m_stats; }
    void ResetStats();

private:
    struct RequestData {
        std::string url;
        std::string method;
        std::string data;
        std::string response;
        IPluginFunction* callback;
        cell_t userData;
        bool success;
        int httpCode;
        std::string error;
        
        // Timing
        std::chrono::steady_clock::time_point startTime;
        std::chrono::steady_clock::time_point endTime;
    };
    
    // Static callbacks for libcurl
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response);
    static size_t HeaderCallback(void* contents, size_t size, size_t nmemb, void* userp);
    
    // Threading
    static void AsyncRequestThread(void* data);
    void ProcessAsyncRequest(RequestData* requestData);
    
    // Internal methods
    bool PerformRequest(CURL* curl, const std::string& url, const std::string& method,
                       const std::string& data, std::string& response, int& httpCode);
    void SetupCurlOptions(CURL* curl, const std::string& url, const std::string& method,
                         const std::string& data, std::string& response);
    bool ShouldRetry(int httpCode, const std::string& error, int attempt);
    void UpdateStats(bool success, double responseTime);
    
    // Member variables
    std::string m_baseUrl;
    std::map<std::string, std::string> m_headers;
    int m_timeout;
    int m_retryCount;
    std::string m_userAgent;
    std::string m_lastError;
    
    // Statistics
    Stats m_stats;
    
    // Thread safety
    IThreader* m_pThreader;
    
    // Connection reuse
    CURL* m_curl;
    bool m_initialized;
};

#endif // _HTTP_CLIENT_H_
