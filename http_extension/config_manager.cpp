/**
 * MongoDB Extension Configuration Manager Implementation
 */

#include "config_manager.h"
#include <fstream>
#include <sstream>
#include <cstring>
#ifndef SOURCEMOD_BUILD
#include <iostream>
#endif

ConfigManager::ConfigManager()
    : m_apiServiceURL("http://127.0.0.1:3300")
    , m_apiKey("sourcemod-mongodb-extension-2024")
    , m_timeout(30000)
    , m_retries(3)
    , m_debug(false)
    , m_defaultDatabase("sourcemod")
    , m_maxConnections(5)
    , m_idleTimeout(300)
{
}

ConfigManager::~ConfigManager()
{
}

bool ConfigManager::LoadConfig(const char* configPath)
{
    m_lastError = "";

    // Set defaults first
    m_apiServiceURL = "http://127.0.0.1:3300/api/v1";
    m_apiKey = "sourcemod-mongodb-extension-2024";
    m_timeout = 30000;
    m_retries = 3;
    m_debug = false;
    m_defaultDatabase = "sourcemod";
    m_maxConnections = 5;
    m_idleTimeout = 300;

    // Try to open and parse the JSON config file
    std::ifstream file(configPath);
    if (!file.is_open())
    {
        m_lastError = "Failed to open configuration file: ";
        m_lastError += configPath;
        return false;
    }

    // Read entire file into string
    std::string jsonContent((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
    file.close();

    // Parse JSON content
    if (!ParseJSON(jsonContent))
    {
        return false; // Error message set in ParseJSON
    }

    return true;
}

bool ConfigManager::ParseJSON(const std::string& jsonContent)
{
    try
    {
        // Simple JSON parser - look for key-value pairs in specific sections

        // Parse API section
        std::string apiSection = ExtractJSONSection(jsonContent, "api");
        if (!apiSection.empty())
        {
            m_apiServiceURL = ExtractJSONString(apiSection, "url", m_apiServiceURL);
            m_apiKey = ExtractJSONString(apiSection, "api_key", m_apiKey);
            m_timeout = ExtractJSONInt(apiSection, "timeout", 30) * 1000; // Convert to milliseconds
            m_retries = ExtractJSONInt(apiSection, "max_retries", 3);
        }

        // Parse database section
        std::string dbSection = ExtractJSONSection(jsonContent, "database");
        if (!dbSection.empty())
        {
            m_defaultDatabase = ExtractJSONString(dbSection, "default_db", m_defaultDatabase);
        }

        // Parse connections section
        std::string connSection = ExtractJSONSection(jsonContent, "connections");
        if (!connSection.empty())
        {
            m_maxConnections = ExtractJSONInt(connSection, "pool_size", 5);
            m_idleTimeout = ExtractJSONInt(connSection, "keep_alive", 300);
        }

        // Parse development section
        std::string devSection = ExtractJSONSection(jsonContent, "development");
        if (!devSection.empty())
        {
            m_debug = ExtractJSONBool(devSection, "debug_mode", false);
        }

        return true;
    }
    catch (...)
    {
        m_lastError = "Failed to parse JSON configuration";
        return false;
    }
}

std::string ConfigManager::ExtractJSONSection(const std::string& json, const std::string& sectionName)
{
    std::string searchKey = "\"" + sectionName + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos)
        return "";

    // Find the opening brace after the key
    size_t bracePos = json.find('{', keyPos);
    if (bracePos == std::string::npos)
        return "";

    // Find the matching closing brace
    int braceCount = 1;
    size_t pos = bracePos + 1;
    while (pos < json.length() && braceCount > 0)
    {
        if (json[pos] == '{')
            braceCount++;
        else if (json[pos] == '}')
            braceCount--;
        pos++;
    }

    if (braceCount == 0)
        return json.substr(bracePos + 1, pos - bracePos - 2);

    return "";
}

std::string ConfigManager::ExtractJSONString(const std::string& section, const std::string& key, const std::string& defaultValue)
{
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = section.find(searchKey);
    if (keyPos == std::string::npos)
        return defaultValue;

    // Find the colon after the key
    size_t colonPos = section.find(':', keyPos);
    if (colonPos == std::string::npos)
        return defaultValue;

    // Find the opening quote of the value
    size_t quotePos = section.find('"', colonPos);
    if (quotePos == std::string::npos)
        return defaultValue;

    // Find the closing quote
    size_t endQuotePos = section.find('"', quotePos + 1);
    if (endQuotePos == std::string::npos)
        return defaultValue;

    return section.substr(quotePos + 1, endQuotePos - quotePos - 1);
}

int ConfigManager::ExtractJSONInt(const std::string& section, const std::string& key, int defaultValue)
{
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = section.find(searchKey);
    if (keyPos == std::string::npos)
        return defaultValue;

    // Find the colon after the key
    size_t colonPos = section.find(':', keyPos);
    if (colonPos == std::string::npos)
        return defaultValue;

    // Find the number after the colon
    size_t numStart = colonPos + 1;
    while (numStart < section.length() && (section[numStart] == ' ' || section[numStart] == '\t'))
        numStart++;

    if (numStart >= section.length())
        return defaultValue;

    // Extract the number
    size_t numEnd = numStart;
    while (numEnd < section.length() && (std::isdigit(section[numEnd]) || section[numEnd] == '-'))
        numEnd++;

    if (numEnd > numStart)
    {
        try
        {
            return std::stoi(section.substr(numStart, numEnd - numStart));
        }
        catch (...)
        {
            return defaultValue;
        }
    }

    return defaultValue;
}

bool ConfigManager::ExtractJSONBool(const std::string& section, const std::string& key, bool defaultValue)
{
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = section.find(searchKey);
    if (keyPos == std::string::npos)
        return defaultValue;

    // Find the colon after the key
    size_t colonPos = section.find(':', keyPos);
    if (colonPos == std::string::npos)
        return defaultValue;

    // Look for true/false after the colon
    size_t truePos = section.find("true", colonPos);
    size_t falsePos = section.find("false", colonPos);

    if (truePos != std::string::npos && (falsePos == std::string::npos || truePos < falsePos))
        return true;
    else if (falsePos != std::string::npos)
        return false;

    return defaultValue;
}


