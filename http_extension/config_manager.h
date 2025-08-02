/**
 * MongoDB Extension Configuration Manager
 * Handles loading and managing configuration from files
 */

#ifndef _CONFIG_MANAGER_H_
#define _CONFIG_MANAGER_H_

#include <string>
#include <map>

// Only include SourceMod headers when building as extension
#ifdef SOURCEMOD_BUILD
#include "smsdk_ext.h"
#endif

class ConfigManager
{
public:
    ConfigManager();
    ~ConfigManager();
    
    // Load configuration from file
    bool LoadConfig(const char* configPath);
    
    // Get configuration values
    std::string GetAPIServiceURL() const { return m_apiServiceURL; }
    std::string GetAPIKey() const { return m_apiKey; }
    int GetTimeout() const { return m_timeout; }
    int GetRetries() const { return m_retries; }
    bool IsDebugEnabled() const { return m_debug; }
    
    std::string GetDefaultDatabase() const { return m_defaultDatabase; }
    
    int GetMaxConnections() const { return m_maxConnections; }
    int GetIdleTimeout() const { return m_idleTimeout; }
    
    // Set configuration values (for runtime changes)
    void SetAPIServiceURL(const std::string& url) { m_apiServiceURL = url; }
    void SetAPIKey(const std::string& key) { m_apiKey = key; }
    void SetTimeout(int timeout) { m_timeout = timeout; }
    void SetRetries(int retries) { m_retries = retries; }
    void SetDebug(bool debug) { m_debug = debug; }
    
    // Get last error
    std::string GetLastError() const { return m_lastError; }
    
private:
    // Configuration values
    std::string m_apiServiceURL;
    std::string m_apiKey;
    int m_timeout;
    int m_retries;
    bool m_debug;
    
    std::string m_defaultDatabase;
    
    int m_maxConnections;
    int m_idleTimeout;
    
    std::string m_lastError;

    // Helper methods for JSON parsing
    bool ParseJSON(const std::string& jsonContent);
    std::string ExtractJSONSection(const std::string& json, const std::string& sectionName);
    std::string ExtractJSONString(const std::string& section, const std::string& key, const std::string& defaultValue);
    int ExtractJSONInt(const std::string& section, const std::string& key, int defaultValue);
    bool ExtractJSONBool(const std::string& section, const std::string& key, bool defaultValue);
};

#endif // _CONFIG_MANAGER_H_
