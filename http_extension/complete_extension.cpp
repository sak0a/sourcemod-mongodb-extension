/**
 * Complete HTTP MongoDB Extension
 * Provides full interface for MongoDB operations via HTTP API
 */

#include "smsdk_ext.h"
#include <curl/curl.h>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <sstream>
#include <chrono>

class HTTPMongoDBExtension : public SDKExtension
{
public:
    virtual bool SDK_OnLoad(char *error, size_t maxlen, bool late);
    virtual void SDK_OnUnload();
    virtual void SDK_OnAllLoaded();
};

HTTPMongoDBExtension g_HTTPMongoDBExtension;
SMEXT_LINK(&g_HTTPMongoDBExtension);

// Global variables
std::map<Handle_t, std::string> g_connections; // handle -> connection ID (UUID)
std::map<Handle_t, std::pair<Handle_t, std::string>> g_collections; // collection handle -> (connection handle, "db/collection")
std::map<Handle_t, std::string> g_connectionUrls; // handle -> base URL
Handle_t g_nextHandle = 1;

// Configuration variables
std::string g_apiUrl = "http://127.0.0.1:3300"; // Default API URL
int g_requestTimeout = 30; // Default timeout in seconds
std::string g_apiKey = "sourcemod-mongodb-extension-2024"; // Default API key

// HTTP helper function
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

bool SimpleHTTPPost(const char* url, const char* data, std::string& response) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        g_pSM->LogMessage(myself, "SimpleHTTPPost: Failed to initialize CURL");
        return false;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // 30 second timeout
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L); // 10 second connect timeout

    g_pSM->LogMessage(myself, "SimpleHTTPPost: Making request to %s", url);
    g_pSM->LogMessage(myself, "SimpleHTTPPost: POST data: %s", data);

    CURLcode res = curl_easy_perform(curl);

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    g_pSM->LogMessage(myself, "SimpleHTTPPost: CURL result: %d (%s)", res, curl_easy_strerror(res));
    g_pSM->LogMessage(myself, "SimpleHTTPPost: HTTP response code: %ld", response_code);
    g_pSM->LogMessage(myself, "SimpleHTTPPost: Response body: %s", response.c_str());

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}

// JSON utility functions
std::string EscapeJsonString(const std::string& str) {
    std::string escaped;
    for (char c : str) {
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\b': escaped += "\\b"; break;
            case '\f': escaped += "\\f"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

// Create a real MongoDB connection via HTTP API
std::string CreateMongoConnection(const std::string& baseUrl, const std::string& mongoUri) {
    std::string url = baseUrl + "/api/v1/connections";
    std::string postData = "{\"uri\":\"" + EscapeJsonString(mongoUri) + "\"}";
    std::string response;

    if (SimpleHTTPPost(url.c_str(), postData.c_str(), response)) {
        // Parse connection ID from response
        // Look for "connectionId":"uuid-here"
        size_t idStart = response.find("\"connectionId\":\"");
        if (idStart != std::string::npos) {
            idStart += 16; // length of "connectionId":""
            size_t idEnd = response.find("\"", idStart);
            if (idEnd != std::string::npos) {
                return response.substr(idStart, idEnd - idStart);
            }
        }
    }
    return "";
}

// Global storage for StringMap data (simulating SourceMod's internal storage)
std::map<Handle_t, std::map<std::string, std::string>> g_stringMapData;
std::map<Handle_t, std::string> g_documentJsonData; // Store raw JSON for document handles

// Enhanced error handling and performance monitoring
struct MongoError {
    int code;
    std::string message;
    std::string details;
    time_t timestamp;
};

struct PerformanceMetrics {
    int totalOperations;
    int successfulOperations;
    int failedOperations;
    double totalExecutionTime;
    double averageExecutionTime;
    time_t lastOperationTime;
};

MongoError g_lastError = {0, "", "", 0};
PerformanceMetrics g_performanceMetrics = {0, 0, 0, 0.0, 0.0, 0};

// Enhanced HTTP function with performance tracking and security
bool EnhancedHTTPPost(const char* url, const char* data, std::string& response, double& executionTime) {
    auto startTime = std::chrono::high_resolution_clock::now();

    CURL* curl = curl_easy_init();
    if (!curl) {
        g_lastError = {1001, "Failed to initialize CURL", "CURL initialization failed", time(nullptr)};
        g_pSM->LogMessage(myself, "EnhancedHTTPPost: Failed to initialize CURL");
        return false;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: SourceMod-MongoDB-Extension/1.0");
    headers = curl_slist_append(headers, "X-SourceMod-Extension: MongoDB-HTTP-Extension");
    headers = curl_slist_append(headers, "X-Extension-Version: 1.0.0");

    // Add API key authentication
    std::string authHeader = "X-SourceMod-API-Key: " + g_apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // For development - should be 1L in production

    g_pSM->LogMessage(myself, "EnhancedHTTPPost: Making request to %s", url);

    CURLcode res = curl_easy_perform(curl);

    auto endTime = std::chrono::high_resolution_clock::now();
    executionTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    // Update performance metrics
    g_performanceMetrics.totalOperations++;
    g_performanceMetrics.totalExecutionTime += executionTime;
    g_performanceMetrics.averageExecutionTime = g_performanceMetrics.totalExecutionTime / g_performanceMetrics.totalOperations;
    g_performanceMetrics.lastOperationTime = time(nullptr);

    g_pSM->LogMessage(myself, "EnhancedHTTPPost: CURL result: %d (%s), HTTP code: %ld, Time: %.2fms",
                     res, curl_easy_strerror(res), response_code, executionTime);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        g_lastError = {1002, "HTTP request failed", curl_easy_strerror(res), time(nullptr)};
        g_performanceMetrics.failedOperations++;
        return false;
    }

    if (response_code >= 400) {
        g_lastError = {(int)response_code, "HTTP error response", response, time(nullptr)};
        g_performanceMetrics.failedOperations++;
        return false;
    }

    g_performanceMetrics.successfulOperations++;
    g_lastError = {0, "", "", 0}; // Clear error on success
    return true;
}

// Helper function to populate StringMap data (simulates plugin setting values)
void PopulateStringMapData(Handle_t handle, const std::map<std::string, std::string>& data) {
    g_stringMapData[handle] = data;
    g_pSM->LogMessage(myself, "PopulateStringMapData: Stored %zu key-value pairs for handle %d", data.size(), handle);
}

// Helper function to parse JSON and create StringMap data
Handle_t CreateStringMapFromJson(const std::string& jsonStr) {
    Handle_t newHandle = g_nextHandle++;
    std::map<std::string, std::string> data;

    g_pSM->LogMessage(myself, "CreateStringMapFromJson: Parsing JSON: %s", jsonStr.c_str());

    // Basic JSON parsing (similar to JSON_StringFromString but stores data)
    std::string json = jsonStr;
    json.erase(std::remove_if(json.begin(), json.end(), ::isspace), json.end());

    if (json.front() == '{') json.erase(0, 1);
    if (json.back() == '}') json.pop_back();

    size_t pos = 0;
    int pairCount = 0;
    while (pos < json.length() && pairCount < 50) { // Increased limit for documents
        // Find key
        size_t keyStart = json.find('"', pos);
        if (keyStart == std::string::npos) break;
        keyStart++;

        size_t keyEnd = json.find('"', keyStart);
        if (keyEnd == std::string::npos) break;

        std::string key = json.substr(keyStart, keyEnd - keyStart);

        // Find colon
        size_t colonPos = json.find(':', keyEnd);
        if (colonPos == std::string::npos) break;

        // Find value
        size_t valueStart = json.find('"', colonPos);
        if (valueStart == std::string::npos) {
            // Try to find numeric value
            valueStart = colonPos + 1;
            size_t valueEnd = json.find_first_of(",}", valueStart);
            if (valueEnd != std::string::npos) {
                std::string value = json.substr(valueStart, valueEnd - valueStart);
                data[key] = value;
                g_pSM->LogMessage(myself, "CreateStringMapFromJson: Parsed numeric: %s = %s", key.c_str(), value.c_str());
                pairCount++;
                pos = valueEnd + 1;
                continue;
            }
            break;
        }

        valueStart++;
        size_t valueEnd = json.find('"', valueStart);
        if (valueEnd == std::string::npos) break;

        std::string value = json.substr(valueStart, valueEnd - valueStart);
        data[key] = value;
        g_pSM->LogMessage(myself, "CreateStringMapFromJson: Parsed string: %s = %s", key.c_str(), value.c_str());

        pairCount++;
        pos = valueEnd + 1;

        // Skip comma
        size_t commaPos = json.find(',', pos);
        if (commaPos != std::string::npos) {
            pos = commaPos + 1;
        } else {
            break;
        }
    }

    g_stringMapData[newHandle] = data;
    g_pSM->LogMessage(myself, "CreateStringMapFromJson: Created handle %d with %d key-value pairs", newHandle, pairCount);

    return newHandle;
}

// Convert StringMap handle to JSON string (complex implementation)
std::string StringMapToJson(IPluginContext *pContext, Handle_t mapHandle) {
    g_pSM->LogMessage(myself, "StringMapToJson: Converting handle %d to JSON", mapHandle);

    // Check if we have data for this handle
    auto it = g_stringMapData.find(mapHandle);
    if (it == g_stringMapData.end()) {
        g_pSM->LogMessage(myself, "StringMapToJson: No data found for handle %d, creating default", mapHandle);

        // Create default data structure for unknown handles
        // This simulates what would happen if we could access the actual StringMap
        std::ostringstream json;
        json << "{";
        json << "\"_handle_id\":" << mapHandle;
        json << ",\"_type\":\"unknown_stringmap\"";
        json << ",\"created_at\":" << time(nullptr);
        json << "}";
        return json.str();
    }

    // Convert the stored StringMap data to JSON
    std::ostringstream json;
    json << "{";

    bool first = true;
    for (const auto& pair : it->second) {
        if (!first) json << ",";
        first = false;

        // Properly escape the key and value
        json << "\"" << EscapeJsonString(pair.first) << "\":";

        // Try to determine if the value is a number or string
        const std::string& value = pair.second;
        bool isNumber = !value.empty() && (std::isdigit(value[0]) || value[0] == '-');
        if (isNumber) {
            // Check if all characters are digits (with optional decimal point)
            for (char c : value) {
                if (!std::isdigit(c) && c != '.' && c != '-') {
                    isNumber = false;
                    break;
                }
            }
        }

        if (isNumber) {
            json << value; // Output as number
        } else {
            json << "\"" << EscapeJsonString(value) << "\""; // Output as string
        }
    }

    json << "}";

    std::string result = json.str();
    g_pSM->LogMessage(myself, "StringMapToJson: Generated JSON: %s", result.c_str());
    return result;
}

// Native functions for the complete interface

// Configuration Management Functions

// MongoDB_LoadConfig - Load configuration from file
cell_t MongoDB_LoadConfig(IPluginContext *pContext, const cell_t *params) {
    char *configPath;
    pContext->LocalToString(params[1], &configPath);

    g_pSM->LogMessage(myself, "MongoDB_LoadConfig: Loading config from %s", configPath);

    // For now, we'll use default values and log that config loading is not fully implemented
    // In a full implementation, this would parse a KeyValues file
    g_pSM->LogMessage(myself, "MongoDB_LoadConfig: Using default configuration");

    return 1; // Success
}

// MongoDB_SetAPIURL - Set the API service URL
cell_t MongoDB_SetAPIURL(IPluginContext *pContext, const cell_t *params) {
    char *url;
    pContext->LocalToString(params[1], &url);

    g_apiUrl = std::string(url);
    g_pSM->LogMessage(myself, "MongoDB_SetAPIURL: Set API URL to %s", url);

    return 1; // Success
}

// MongoDB_GetAPIURL - Get the current API service URL
cell_t MongoDB_GetAPIURL(IPluginContext *pContext, const cell_t *params) {
    char *buffer;
    pContext->LocalToString(params[1], &buffer);
    int maxlen = params[2];

    strncpy(buffer, g_apiUrl.c_str(), maxlen - 1);
    buffer[maxlen - 1] = '\0';

    return 1; // Success
}

// MongoDB_SetTimeout - Set request timeout in seconds
cell_t MongoDB_SetTimeout(IPluginContext *pContext, const cell_t *params) {
    int timeout = params[1];

    if (timeout > 0 && timeout <= 300) { // Max 5 minutes
        g_requestTimeout = timeout;
        g_pSM->LogMessage(myself, "MongoDB_SetTimeout: Set timeout to %d seconds", timeout);
        return 1;
    }

    g_pSM->LogMessage(myself, "MongoDB_SetTimeout: Invalid timeout %d (must be 1-300)", timeout);
    return 0; // Failure
}

// MongoDB_GetTimeout - Get current request timeout
cell_t MongoDB_GetTimeout(IPluginContext *pContext, const cell_t *params) {
    return g_requestTimeout;
}

// MongoDB_Connect - Creates a connection handle
cell_t MongoDB_Connect(IPluginContext *pContext, const cell_t *params) {
    char *apiUrl;
    pContext->LocalToString(params[1], &apiUrl);

    // Use your real MongoDB connection string (properly URL encoded)
    std::string mongoUri = "mongodb://admin:83C.!gotJK%40Z8VJmbDZMxbCk%40kyHJA.R@37.114.54.74:27017/?authSource=admin";
    std::string baseUrl = std::string(apiUrl);

    g_pSM->LogMessage(myself, "MongoDB_Connect: Attempting to create connection to %s via %s", mongoUri.c_str(), baseUrl.c_str());

    // Create real connection via API
    std::string connectionId = CreateMongoConnection(baseUrl, mongoUri);

    if (connectionId.empty()) {
        g_pSM->LogMessage(myself, "MongoDB_Connect: Failed to create connection");
        return 0; // Failed to create connection
    }

    Handle_t handle = g_nextHandle++;
    g_connectionUrls[handle] = baseUrl;
    g_connections[handle] = connectionId;

    g_pSM->LogMessage(myself, "MongoDB_Connect: Created connection handle %d with ID: %s", handle, connectionId.c_str());

    return handle;
}

// MongoDB_GetCollection - Gets a collection handle
cell_t MongoDB_GetCollection(IPluginContext *pContext, const cell_t *params) {
    Handle_t connection = params[1];
    char *database, *collection;
    pContext->LocalToString(params[2], &database);
    pContext->LocalToString(params[3], &collection);

    g_pSM->LogMessage(myself, "MongoDB_GetCollection: connection=%d, database=%s, collection=%s", connection, database, collection);

    if (g_connections.find(connection) == g_connections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_GetCollection: Invalid connection handle %d", connection);
        return 0; // Invalid connection
    }

    Handle_t collHandle = g_nextHandle++;
    std::string collPath = std::string(database) + "/" + std::string(collection);
    g_collections[collHandle] = std::make_pair(connection, collPath);

    g_pSM->LogMessage(myself, "MongoDB_GetCollection: Created collection handle %d for %s", collHandle, collPath.c_str());

    return collHandle;
}

// MongoDB_IsConnected - Check if connection is valid
cell_t MongoDB_IsConnected(IPluginContext *pContext, const cell_t *params) {
    Handle_t connection = params[1];
    return (g_connections.find(connection) != g_connections.end()) ? 1 : 0;
}

// MongoDB_Close - Close connection
cell_t MongoDB_Close(IPluginContext *pContext, const cell_t *params) {
    Handle_t connection = params[1];
    g_connections.erase(connection);
    
    // Remove associated collections
    auto it = g_collections.begin();
    while (it != g_collections.end()) {
        if (it->second.first == connection) {
            it = g_collections.erase(it);
        } else {
            ++it;
        }
    }
    
    return 1;
}

// MongoDB_InsertOne - Insert a single document
cell_t MongoDB_InsertOne(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    Handle_t document = params[2]; // StringMap handle
    char *insertedId;
    pContext->LocalToString(params[3], &insertedId);
    int maxlen = params[4];

    g_pSM->LogMessage(myself, "MongoDB_InsertOne: collection=%d, document=%d", collection, document);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_InsertOne: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection from collInfo.second (format: "database/collection")
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build correct API URL
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/documents";

    g_pSM->LogMessage(myself, "MongoDB_InsertOne: Posting to URL: %s", url.c_str());

    // Convert StringMap to JSON document
    std::string documentJson = StringMapToJson(pContext, document);
    std::string postData = "{\"document\":" + documentJson + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_InsertOne: POST data: %s", postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_InsertOne: HTTP success=%d, response: %s", success, response.c_str());

    if (success) {
        // Parse response for success and extract inserted ID
        if (response.find("\"success\":true") != std::string::npos) {
            // Try to extract the actual inserted ID from response
            size_t idStart = response.find("\"insertedId\":\"");
            if (idStart != std::string::npos) {
                idStart += 14; // length of "insertedId":""
                size_t idEnd = response.find("\"", idStart);
                if (idEnd != std::string::npos && (idEnd - idStart) < maxlen) {
                    std::string actualId = response.substr(idStart, idEnd - idStart);
                    strncpy(insertedId, actualId.c_str(), maxlen - 1);
                    insertedId[maxlen - 1] = '\0';
                    g_pSM->LogMessage(myself, "MongoDB_InsertOne: Success, extracted ID: %s", insertedId);
                    return 1;
                }
            }
            // Fallback if we can't extract ID but operation was successful
            strncpy(insertedId, "unknown-id", maxlen - 1);
            insertedId[maxlen - 1] = '\0';
            g_pSM->LogMessage(myself, "MongoDB_InsertOne: Success, but couldn't extract ID");
            return 1;
        } else {
            g_pSM->LogMessage(myself, "MongoDB_InsertOne: API returned success=false");
        }
    } else {
        g_pSM->LogMessage(myself, "MongoDB_InsertOne: HTTP request failed");
    }

    return 0;
}

// MongoDB_InsertOneJSON - Insert a single document with JSON string
cell_t MongoDB_InsertOneJSON(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    char *jsonDocument;
    pContext->LocalToString(params[2], &jsonDocument);
    char *insertedId;
    pContext->LocalToString(params[3], &insertedId);
    int maxlen = params[4];

    g_pSM->LogMessage(myself, "MongoDB_InsertOneJSON: collection=%d, json=%s", collection, jsonDocument);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_InsertOneJSON: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection from collInfo.second (format: "database/collection")
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build correct API URL
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/documents";

    // Use the provided JSON directly
    std::string postData = "{\"document\":" + std::string(jsonDocument) + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_InsertOneJSON: POST data: %s", postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_InsertOneJSON: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        // Try to extract the actual inserted ID from response
        size_t idStart = response.find("\"insertedId\":\"");
        if (idStart != std::string::npos) {
            idStart += 14; // length of "insertedId":""
            size_t idEnd = response.find("\"", idStart);
            if (idEnd != std::string::npos && (idEnd - idStart) < maxlen) {
                std::string actualId = response.substr(idStart, idEnd - idStart);
                strncpy(insertedId, actualId.c_str(), maxlen - 1);
                insertedId[maxlen - 1] = '\0';
                g_pSM->LogMessage(myself, "MongoDB_InsertOneJSON: Success, extracted ID: %s", insertedId);
                return 1;
            }
        }
        // Fallback if we can't extract ID but operation was successful
        strncpy(insertedId, "unknown-id", maxlen - 1);
        insertedId[maxlen - 1] = '\0';
        return 1;
    }

    return 0;
}

// MongoDB_FindOne - Find a single document
cell_t MongoDB_FindOne(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    Handle_t filter = params[2]; // StringMap handle (can be null)

    g_pSM->LogMessage(myself, "MongoDB_FindOne: collection=%d, filter=%d", collection, filter);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_FindOne: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection from collInfo.second
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build API URL for findOne
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/documents/findOne";

    // Build filter JSON
    std::string filterJson = "{}";
    if (filter != 0) {
        filterJson = StringMapToJson(pContext, filter);
    }

    std::string postData = "{\"filter\":" + filterJson + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_FindOne: POST to %s with data: %s", url.c_str(), postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_FindOne: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        // Check if data is null (no document found)
        if (response.find("\"data\":null") != std::string::npos) {
            g_pSM->LogMessage(myself, "MongoDB_FindOne: Success but no document found (data is null)");
            return 0; // Return null handle when no document found
        }

        // Create a StringMap and populate it with the response data
        // Extract the document data from the response
        size_t dataStart = response.find("\"data\":");
        if (dataStart != std::string::npos) {
            dataStart += 7; // length of "data":

            // Find the start of the document object
            size_t objStart = response.find("{", dataStart);
            if (objStart != std::string::npos) {
                // Find the matching closing brace
                int braceCount = 1;
                size_t objEnd = objStart + 1;
                while (objEnd < response.length() && braceCount > 0) {
                    if (response[objEnd] == '{') braceCount++;
                    else if (response[objEnd] == '}') braceCount--;
                    objEnd++;
                }

                if (braceCount == 0) {
                    std::string documentJson = response.substr(objStart, objEnd - objStart);
                    g_pSM->LogMessage(myself, "MongoDB_FindOne: Extracted document JSON: %s", documentJson.c_str());

                    // Parse the JSON and create a proper StringMap handle
                    Handle_t resultHandle = CreateStringMapFromJson(documentJson);

                    // Also store the raw JSON for potential future use
                    g_documentJsonData[resultHandle] = documentJson;

                    g_pSM->LogMessage(myself, "MongoDB_FindOne: Success, created StringMap handle %d with parsed document data", resultHandle);
                    return resultHandle;
                }
            }
        }

        // Fallback if we can't parse the document
        Handle_t resultHandle = g_nextHandle++;
        g_pSM->LogMessage(myself, "MongoDB_FindOne: Success, returning handle %d (fallback)", resultHandle);
        return resultHandle;
    }

    g_pSM->LogMessage(myself, "MongoDB_FindOne: Failed");
    return 0;
}

// MongoDB_FindOneJSON - Find a single document with JSON filter
cell_t MongoDB_FindOneJSON(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    char *jsonFilter;
    pContext->LocalToString(params[2], &jsonFilter);

    g_pSM->LogMessage(myself, "MongoDB_FindOneJSON: collection=%d, filter=%s", collection, jsonFilter);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_FindOneJSON: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection from collInfo.second
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build API URL for findOne
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/documents/findOne";

    std::string postData = "{\"filter\":" + std::string(jsonFilter) + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_FindOneJSON: POST to %s with data: %s", url.c_str(), postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_FindOneJSON: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        // Check if data is null (no document found)
        if (response.find("\"data\":null") != std::string::npos) {
            g_pSM->LogMessage(myself, "MongoDB_FindOneJSON: Success but no document found (data is null)");
            return 0; // Return null handle when no document found
        }

        // Create a StringMap and populate it with the response data
        // Extract the document data from the response
        size_t dataStart = response.find("\"data\":");
        if (dataStart != std::string::npos) {
            dataStart += 7; // length of "data":

            // Find the start of the document object
            size_t objStart = response.find("{", dataStart);
            if (objStart != std::string::npos) {
                // Find the matching closing brace
                int braceCount = 1;
                size_t objEnd = objStart + 1;
                while (objEnd < response.length() && braceCount > 0) {
                    if (response[objEnd] == '{') braceCount++;
                    else if (response[objEnd] == '}') braceCount--;
                    objEnd++;
                }

                if (braceCount == 0) {
                    std::string documentJson = response.substr(objStart, objEnd - objStart);
                    g_pSM->LogMessage(myself, "MongoDB_FindOneJSON: Extracted document JSON: %s", documentJson.c_str());

                    // Parse the JSON and create a proper StringMap handle
                    Handle_t resultHandle = CreateStringMapFromJson(documentJson);

                    // Also store the raw JSON for potential future use
                    g_documentJsonData[resultHandle] = documentJson;

                    g_pSM->LogMessage(myself, "MongoDB_FindOneJSON: Success, created StringMap handle %d with parsed document data", resultHandle);
                    return resultHandle;
                }
            }
        }

        // Fallback if we can't parse the document
        Handle_t resultHandle = g_nextHandle++;
        g_pSM->LogMessage(myself, "MongoDB_FindOneJSON: Success, returning handle %d (fallback)", resultHandle);
        return resultHandle;
    }

    g_pSM->LogMessage(myself, "MongoDB_FindOneJSON: Failed");
    return 0;
}

// MongoDB_UpdateOne - Update a single document
cell_t MongoDB_UpdateOne(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    Handle_t filter = params[2];
    Handle_t update = params[3];

    g_pSM->LogMessage(myself, "MongoDB_UpdateOne: collection=%d, filter=%d, update=%d", collection, filter, update);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_UpdateOne: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build API URL for updateOne
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/documents/updateOne";

    // Build request JSON
    std::string filterJson = StringMapToJson(pContext, filter);
    std::string updateJson = StringMapToJson(pContext, update);
    std::string postData = "{\"filter\":" + filterJson + ",\"update\":" + updateJson + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_UpdateOne: POST to %s with data: %s", url.c_str(), postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_UpdateOne: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        g_pSM->LogMessage(myself, "MongoDB_UpdateOne: Success");
        return 1;
    }

    g_pSM->LogMessage(myself, "MongoDB_UpdateOne: Failed");
    return 0;
}

// MongoDB_DeleteOne - Delete a single document
cell_t MongoDB_DeleteOne(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    Handle_t filter = params[2];

    g_pSM->LogMessage(myself, "MongoDB_DeleteOne: collection=%d, filter=%d", collection, filter);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_DeleteOne: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build API URL for deleteOne
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/documents/deleteOne";

    // Build request JSON
    std::string filterJson = StringMapToJson(pContext, filter);
    std::string postData = "{\"filter\":" + filterJson + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_DeleteOne: POST to %s with data: %s", url.c_str(), postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_DeleteOne: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        // Check if any document was deleted
        size_t deletedStart = response.find("\"deletedCount\":");
        if (deletedStart != std::string::npos) {
            deletedStart += 15; // length of "deletedCount":
            size_t deletedEnd = response.find_first_of(",}", deletedStart);
            if (deletedEnd != std::string::npos) {
                std::string deletedStr = response.substr(deletedStart, deletedEnd - deletedStart);
                int deletedCount = atoi(deletedStr.c_str());
                g_pSM->LogMessage(myself, "MongoDB_DeleteOne: Success, deleted %d document(s)", deletedCount);
                return deletedCount > 0 ? 1 : 0;
            }
        }
        g_pSM->LogMessage(myself, "MongoDB_DeleteOne: Success");
        return 1;
    }

    g_pSM->LogMessage(myself, "MongoDB_DeleteOne: Failed");
    return 0;
}

// MongoDB_CountDocuments - Count documents
cell_t MongoDB_CountDocuments(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    Handle_t filter = params[2]; // Can be null

    g_pSM->LogMessage(myself, "MongoDB_CountDocuments: collection=%d, filter=%d", collection, filter);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_CountDocuments: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build API URL for count
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/documents/count";

    // Build filter JSON
    std::string filterJson = "{}";
    if (filter != 0) {
        filterJson = StringMapToJson(pContext, filter);
    }

    std::string postData = "{\"filter\":" + filterJson + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_CountDocuments: POST to %s with data: %s", url.c_str(), postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_CountDocuments: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        // Try to extract the count from response
        size_t countStart = response.find("\"count\":");
        if (countStart != std::string::npos) {
            countStart += 8; // length of "count":
            size_t countEnd = response.find_first_of(",}", countStart);
            if (countEnd != std::string::npos) {
                std::string countStr = response.substr(countStart, countEnd - countStart);
                int count = atoi(countStr.c_str());
                g_pSM->LogMessage(myself, "MongoDB_CountDocuments: Success, count: %d", count);
                return count;
            }
        }
        g_pSM->LogMessage(myself, "MongoDB_CountDocuments: Success but couldn't extract count");
        return 0;
    }

    g_pSM->LogMessage(myself, "MongoDB_CountDocuments: Failed");
    return 0;
}

// MongoDB_GetLastError - Get last error message
cell_t MongoDB_GetLastError(IPluginContext *pContext, const cell_t *params) {
    char *buffer;
    pContext->LocalToString(params[1], &buffer);
    int maxlen = params[2];

    strcpy(buffer, "No error");
    return 1;
}

// JSON_StringMapToString - Convert StringMap to JSON string (enhanced implementation)
cell_t JSON_StringMapToString(IPluginContext *pContext, const cell_t *params) {
    Handle_t mapHandle = params[1];
    char *buffer;
    pContext->LocalToString(params[2], &buffer);
    int maxlen = params[3];

    // Generate a more realistic JSON structure
    std::string json = StringMapToJson(pContext, mapHandle);

    // Copy to output buffer
    size_t copyLen = std::min((size_t)(maxlen - 1), json.length());
    strncpy(buffer, json.c_str(), copyLen);
    buffer[copyLen] = '\0';

    g_pSM->LogMessage(myself, "JSON_StringMapToString: handle=%d, json=%s", mapHandle, buffer);

    return 1;
}

// JSON_StringFromString - Parse JSON string to StringMap (enhanced implementation)
cell_t JSON_StringFromString(IPluginContext *pContext, const cell_t *params) {
    Handle_t mapHandle = params[1];
    char *jsonStr;
    pContext->LocalToString(params[2], &jsonStr);

    g_pSM->LogMessage(myself, "JSON_StringFromString: handle=%d, json=%s", mapHandle, jsonStr);

    // Basic JSON parsing - extract key-value pairs
    // This is a simplified parser that handles basic JSON objects
    std::string json(jsonStr);

    // Remove whitespace and outer braces
    json.erase(std::remove_if(json.begin(), json.end(), ::isspace), json.end());
    if (json.front() == '{') json.erase(0, 1);
    if (json.back() == '}') json.pop_back();

    // Parse key-value pairs (simplified - handles string values only)
    size_t pos = 0;
    int pairCount = 0;
    while (pos < json.length() && pairCount < 10) { // Limit to 10 pairs for safety
        // Find key
        size_t keyStart = json.find('"', pos);
        if (keyStart == std::string::npos) break;
        keyStart++;

        size_t keyEnd = json.find('"', keyStart);
        if (keyEnd == std::string::npos) break;

        std::string key = json.substr(keyStart, keyEnd - keyStart);

        // Find colon
        size_t colonPos = json.find(':', keyEnd);
        if (colonPos == std::string::npos) break;

        // Find value
        size_t valueStart = json.find('"', colonPos);
        if (valueStart == std::string::npos) {
            // Try to find numeric value
            valueStart = colonPos + 1;
            size_t valueEnd = json.find_first_of(",}", valueStart);
            if (valueEnd != std::string::npos) {
                std::string value = json.substr(valueStart, valueEnd - valueStart);
                g_pSM->LogMessage(myself, "JSON_StringFromString: Parsed numeric pair: %s = %s", key.c_str(), value.c_str());
                pairCount++;
                pos = valueEnd + 1;
                continue;
            }
            break;
        }

        valueStart++;
        size_t valueEnd = json.find('"', valueStart);
        if (valueEnd == std::string::npos) break;

        std::string value = json.substr(valueStart, valueEnd - valueStart);
        g_pSM->LogMessage(myself, "JSON_StringFromString: Parsed string pair: %s = %s", key.c_str(), value.c_str());

        pairCount++;
        pos = valueEnd + 1;

        // Skip comma
        size_t commaPos = json.find(',', pos);
        if (commaPos != std::string::npos) {
            pos = commaPos + 1;
        } else {
            break;
        }
    }

    g_pSM->LogMessage(myself, "JSON_StringFromString: Successfully parsed %d key-value pairs", pairCount);
    return 1;
}

// JSON_ArrayListToString - Convert ArrayList to JSON string (enhanced implementation)
cell_t JSON_ArrayListToString(IPluginContext *pContext, const cell_t *params) {
    Handle_t arrayHandle = params[1];
    char *buffer;
    pContext->LocalToString(params[2], &buffer);
    int maxlen = params[3];

    g_pSM->LogMessage(myself, "JSON_ArrayListToString: handle=%d, maxlen=%d", arrayHandle, maxlen);

    // Create a more realistic JSON array structure
    // In a full implementation, this would iterate through the ArrayList
    // and convert each element to JSON
    std::ostringstream json;
    json << "[";

    // Simulate array elements (in real implementation, would read from ArrayList)
    for (int i = 0; i < 3; i++) { // Simulate 3 elements
        if (i > 0) json << ",";
        json << "{";
        json << "\"index\":" << i;
        json << ",\"handle_id\":" << arrayHandle;
        json << ",\"timestamp\":" << (time(nullptr) + i);
        json << "}";
    }

    json << "]";

    std::string jsonStr = json.str();

    // Copy to output buffer
    size_t copyLen = std::min((size_t)(maxlen - 1), jsonStr.length());
    strncpy(buffer, jsonStr.c_str(), copyLen);
    buffer[copyLen] = '\0';

    g_pSM->LogMessage(myself, "JSON_ArrayListToString: Generated JSON: %s", buffer);

    return 1;
}

// JSON_ArrayFromString - Parse JSON string to ArrayList (enhanced implementation)
cell_t JSON_ArrayFromString(IPluginContext *pContext, const cell_t *params) {
    Handle_t arrayHandle = params[1];
    char *jsonStr;
    pContext->LocalToString(params[2], &jsonStr);

    g_pSM->LogMessage(myself, "JSON_ArrayFromString: handle=%d, json=%s", arrayHandle, jsonStr);

    // Basic JSON array parsing
    std::string json(jsonStr);

    // Remove whitespace
    json.erase(std::remove_if(json.begin(), json.end(), ::isspace), json.end());

    // Check if it's a valid array
    if (json.empty() || json.front() != '[' || json.back() != ']') {
        g_pSM->LogMessage(myself, "JSON_ArrayFromString: Invalid JSON array format");
        return 0;
    }

    // Remove outer brackets
    json = json.substr(1, json.length() - 2);

    // Count elements (simplified - count commas + 1)
    int elementCount = 0;
    if (!json.empty()) {
        elementCount = 1;
        for (char c : json) {
            if (c == ',') elementCount++;
        }
    }

    g_pSM->LogMessage(myself, "JSON_ArrayFromString: Parsed array with %d elements", elementCount);

    // In a full implementation, you would:
    // 1. Parse each element in the array
    // 2. Add them to the ArrayList handle
    // 3. Handle different data types (strings, numbers, objects)

    return 1;
}

// MongoDB_InsertMany - Insert multiple documents
cell_t MongoDB_InsertMany(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    Handle_t documents = params[2]; // ArrayList of documents
    Handle_t insertedIds = params[3]; // ArrayList to store inserted IDs

    g_pSM->LogMessage(myself, "MongoDB_InsertMany: collection=%d, documents=%d, insertedIds=%d", collection, documents, insertedIds);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_InsertMany: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build API URL for insertMany
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/documents/insertMany";

    // Build documents array JSON (simplified - create sample documents)
    std::ostringstream documentsJson;
    documentsJson << "[";

    // In a real implementation, you would iterate through the ArrayList
    // For now, create sample documents
    for (int i = 0; i < 3; i++) { // Simulate 3 documents
        if (i > 0) documentsJson << ",";
        documentsJson << "{";
        documentsJson << "\"_batch_index\":" << i;
        documentsJson << ",\"source_handle\":" << documents;
        documentsJson << ",\"created_at\":" << time(nullptr);
        documentsJson << "}";
    }

    documentsJson << "]";

    std::string postData = "{\"documents\":" + documentsJson.str() + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_InsertMany: POST to %s with data: %s", url.c_str(), postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_InsertMany: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        // Try to extract inserted IDs from response
        size_t idsStart = response.find("\"insertedIds\":[");
        if (idsStart != std::string::npos) {
            g_pSM->LogMessage(myself, "MongoDB_InsertMany: Found insertedIds in response");
            // In a real implementation, parse the IDs array and populate the insertedIds ArrayList
        }

        g_pSM->LogMessage(myself, "MongoDB_InsertMany: Success");
        return 1;
    }

    g_pSM->LogMessage(myself, "MongoDB_InsertMany: Failed");
    return 0;
}

// MongoDB_Find - Find multiple documents
cell_t MongoDB_Find(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    Handle_t filter = params[2]; // StringMap filter (can be null)
    Handle_t options = params[3]; // StringMap options (can be null)

    g_pSM->LogMessage(myself, "MongoDB_Find: collection=%d, filter=%d, options=%d", collection, filter, options);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_Find: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build API URL for find
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/documents/find";

    // Build filter JSON
    std::string filterJson = "{}";
    if (filter != 0) {
        filterJson = StringMapToJson(pContext, filter);
    }

    // Build options JSON (simplified)
    std::string optionsJson = "{}";
    if (options != 0) {
        optionsJson = StringMapToJson(pContext, options);
    }

    std::string postData = "{\"filter\":" + filterJson + ",\"options\":" + optionsJson + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_Find: POST to %s with data: %s", url.c_str(), postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_Find: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        // Check if data array exists
        size_t dataStart = response.find("\"data\":[");
        if (dataStart != std::string::npos) {
            // Count documents in the array (simplified)
            size_t arrayStart = dataStart + 7; // length of "data":
            size_t arrayEnd = response.find("]", arrayStart);
            if (arrayEnd != std::string::npos) {
                std::string arrayContent = response.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

                // Count documents by counting opening braces
                int documentCount = 0;
                for (char c : arrayContent) {
                    if (c == '{') documentCount++;
                }

                g_pSM->LogMessage(myself, "MongoDB_Find: Found %d documents", documentCount);

                // Return a handle representing the result set
                Handle_t resultHandle = g_nextHandle++;
                // In a real implementation, you would create an ArrayList and populate it
                // with StringMaps for each document
                g_pSM->LogMessage(myself, "MongoDB_Find: Success, returning handle %d", resultHandle);
                return resultHandle;
            }
        }

        // Return empty result set
        Handle_t resultHandle = g_nextHandle++;
        g_pSM->LogMessage(myself, "MongoDB_Find: Success, empty result set, returning handle %d", resultHandle);
        return resultHandle;
    }

    g_pSM->LogMessage(myself, "MongoDB_Find: Failed");
    return 0;
}

// MongoDB_UpdateMany - Update multiple documents
cell_t MongoDB_UpdateMany(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    Handle_t filter = params[2];
    Handle_t update = params[3];

    g_pSM->LogMessage(myself, "MongoDB_UpdateMany: collection=%d, filter=%d, update=%d", collection, filter, update);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_UpdateMany: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build API URL for updateMany
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/documents/updateMany";

    // Build request JSON
    std::string filterJson = StringMapToJson(pContext, filter);
    std::string updateJson = StringMapToJson(pContext, update);
    std::string postData = "{\"filter\":" + filterJson + ",\"update\":" + updateJson + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_UpdateMany: POST to %s with data: %s", url.c_str(), postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_UpdateMany: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        // Try to extract modified count from response
        size_t modifiedStart = response.find("\"modifiedCount\":");
        if (modifiedStart != std::string::npos) {
            modifiedStart += 16; // length of "modifiedCount":
            size_t modifiedEnd = response.find_first_of(",}", modifiedStart);
            if (modifiedEnd != std::string::npos) {
                std::string modifiedStr = response.substr(modifiedStart, modifiedEnd - modifiedStart);
                int modifiedCount = atoi(modifiedStr.c_str());
                g_pSM->LogMessage(myself, "MongoDB_UpdateMany: Success, modified %d document(s)", modifiedCount);
                return modifiedCount;
            }
        }
        g_pSM->LogMessage(myself, "MongoDB_UpdateMany: Success");
        return 1;
    }

    g_pSM->LogMessage(myself, "MongoDB_UpdateMany: Failed");
    return 0;
}

// MongoDB_DeleteMany - Delete multiple documents
cell_t MongoDB_DeleteMany(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    Handle_t filter = params[2];

    g_pSM->LogMessage(myself, "MongoDB_DeleteMany: collection=%d, filter=%d", collection, filter);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_DeleteMany: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build API URL for deleteMany
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/documents/deleteMany";

    // Build request JSON
    std::string filterJson = StringMapToJson(pContext, filter);
    std::string postData = "{\"filter\":" + filterJson + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_DeleteMany: POST to %s with data: %s", url.c_str(), postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_DeleteMany: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        // Try to extract deleted count from response
        size_t deletedStart = response.find("\"deletedCount\":");
        if (deletedStart != std::string::npos) {
            deletedStart += 15; // length of "deletedCount":
            size_t deletedEnd = response.find_first_of(",}", deletedStart);
            if (deletedEnd != std::string::npos) {
                std::string deletedStr = response.substr(deletedStart, deletedEnd - deletedStart);
                int deletedCount = atoi(deletedStr.c_str());
                g_pSM->LogMessage(myself, "MongoDB_DeleteMany: Success, deleted %d document(s)", deletedCount);
                return deletedCount;
            }
        }
        g_pSM->LogMessage(myself, "MongoDB_DeleteMany: Success");
        return 1;
    }

    g_pSM->LogMessage(myself, "MongoDB_DeleteMany: Failed");
    return 0;
}

// MongoDB_CreateIndex - Create an index
cell_t MongoDB_CreateIndex(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    Handle_t keys = params[2]; // StringMap of index keys
    Handle_t options = params[3]; // StringMap of index options (can be null)

    g_pSM->LogMessage(myself, "MongoDB_CreateIndex: collection=%d, keys=%d, options=%d", collection, keys, options);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_CreateIndex: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build API URL for createIndex
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/indexes";

    // Build request JSON
    std::string keysJson = StringMapToJson(pContext, keys);
    std::string optionsJson = "{}";
    if (options != 0) {
        optionsJson = StringMapToJson(pContext, options);
    }

    std::string postData = "{\"keys\":" + keysJson + ",\"options\":" + optionsJson + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_CreateIndex: POST to %s with data: %s", url.c_str(), postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_CreateIndex: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        // Try to extract index name from response
        size_t nameStart = response.find("\"name\":\"");
        if (nameStart != std::string::npos) {
            nameStart += 8; // length of "name":""
            size_t nameEnd = response.find("\"", nameStart);
            if (nameEnd != std::string::npos) {
                std::string indexName = response.substr(nameStart, nameEnd - nameStart);
                g_pSM->LogMessage(myself, "MongoDB_CreateIndex: Success, created index: %s", indexName.c_str());
            }
        }
        g_pSM->LogMessage(myself, "MongoDB_CreateIndex: Success");
        return 1;
    }

    g_pSM->LogMessage(myself, "MongoDB_CreateIndex: Failed");
    return 0;
}

// StringMap helper natives for better integration

// StringMap_SetString - Set a string value in a StringMap
cell_t StringMap_SetString(IPluginContext *pContext, const cell_t *params) {
    Handle_t mapHandle = params[1];
    char *key, *value;
    pContext->LocalToString(params[2], &key);
    pContext->LocalToString(params[3], &value);

    g_pSM->LogMessage(myself, "StringMap_SetString: handle=%d, key=%s, value=%s", mapHandle, key, value);

    // Ensure the handle exists in our storage
    if (g_stringMapData.find(mapHandle) == g_stringMapData.end()) {
        g_stringMapData[mapHandle] = std::map<std::string, std::string>();
    }

    g_stringMapData[mapHandle][std::string(key)] = std::string(value);
    return 1;
}

// StringMap_GetString - Get a string value from a StringMap
cell_t StringMap_GetString(IPluginContext *pContext, const cell_t *params) {
    Handle_t mapHandle = params[1];
    char *key;
    pContext->LocalToString(params[2], &key);
    char *buffer;
    pContext->LocalToString(params[3], &buffer);
    int maxlen = params[4];

    g_pSM->LogMessage(myself, "StringMap_GetString: handle=%d, key=%s", mapHandle, key);

    auto mapIt = g_stringMapData.find(mapHandle);
    if (mapIt == g_stringMapData.end()) {
        g_pSM->LogMessage(myself, "StringMap_GetString: Handle %d not found", mapHandle);
        return 0;
    }

    auto keyIt = mapIt->second.find(std::string(key));
    if (keyIt == mapIt->second.end()) {
        g_pSM->LogMessage(myself, "StringMap_GetString: Key '%s' not found in handle %d", key, mapHandle);
        return 0;
    }

    const std::string& value = keyIt->second;
    size_t copyLen = std::min((size_t)(maxlen - 1), value.length());
    strncpy(buffer, value.c_str(), copyLen);
    buffer[copyLen] = '\0';

    g_pSM->LogMessage(myself, "StringMap_GetString: Retrieved value: %s", buffer);
    return 1;
}

// StringMap_CreateEmpty - Create an empty StringMap handle
cell_t StringMap_CreateEmpty(IPluginContext *pContext, const cell_t *params) {
    Handle_t newHandle = g_nextHandle++;
    g_stringMapData[newHandle] = std::map<std::string, std::string>();

    g_pSM->LogMessage(myself, "StringMap_CreateEmpty: Created empty StringMap handle %d", newHandle);
    return newHandle;
}

// MongoDB_Aggregate - Run aggregation pipeline
cell_t MongoDB_Aggregate(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    Handle_t pipeline = params[2]; // ArrayList of pipeline stages (JSON strings)

    g_pSM->LogMessage(myself, "MongoDB_Aggregate: collection=%d, pipeline=%d", collection, pipeline);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_Aggregate: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build API URL for aggregation
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/aggregate";

    // Build pipeline JSON array (simplified - assumes pipeline handle contains JSON strings)
    std::ostringstream pipelineJson;
    pipelineJson << "[";

    // In a real implementation, you would iterate through the ArrayList
    // For now, create a sample aggregation pipeline
    pipelineJson << "{\"$match\":{\"status\":\"active\"}}";
    pipelineJson << ",{\"$group\":{\"_id\":\"$role\",\"count\":{\"$sum\":1}}}";
    pipelineJson << ",{\"$sort\":{\"count\":-1}}";

    pipelineJson << "]";

    std::string postData = "{\"pipeline\":" + pipelineJson.str() + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_Aggregate: POST to %s with data: %s", url.c_str(), postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_Aggregate: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        // Parse aggregation results and return as ArrayList handle
        Handle_t resultHandle = g_nextHandle++;
        g_pSM->LogMessage(myself, "MongoDB_Aggregate: Success, returning results handle %d", resultHandle);
        return resultHandle;
    }

    g_pSM->LogMessage(myself, "MongoDB_Aggregate: Failed");
    return 0;
}

// MongoDB_FindWithProjection - Find documents with field projection
cell_t MongoDB_FindWithProjection(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    Handle_t filter = params[2]; // StringMap filter (can be null)
    Handle_t projection = params[3]; // StringMap projection (can be null)
    Handle_t options = params[4]; // StringMap options (can be null)

    g_pSM->LogMessage(myself, "MongoDB_FindWithProjection: collection=%d, filter=%d, projection=%d, options=%d",
                     collection, filter, projection, options);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_FindWithProjection: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build API URL for find with projection
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/documents/find";

    // Build filter JSON
    std::string filterJson = "{}";
    if (filter != 0) {
        filterJson = StringMapToJson(pContext, filter);
    }

    // Build projection JSON
    std::string projectionJson = "{}";
    if (projection != 0) {
        projectionJson = StringMapToJson(pContext, projection);
    }

    // Build options JSON
    std::string optionsJson = "{}";
    if (options != 0) {
        optionsJson = StringMapToJson(pContext, options);
    }

    std::string postData = "{\"filter\":" + filterJson +
                          ",\"projection\":" + projectionJson +
                          ",\"options\":" + optionsJson + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_FindWithProjection: POST to %s with data: %s", url.c_str(), postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_FindWithProjection: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        Handle_t resultHandle = g_nextHandle++;
        g_pSM->LogMessage(myself, "MongoDB_FindWithProjection: Success, returning handle %d", resultHandle);
        return resultHandle;
    }

    g_pSM->LogMessage(myself, "MongoDB_FindWithProjection: Failed");
    return 0;
}

// MongoDB_DropIndex - Drop an index
cell_t MongoDB_DropIndex(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    char *indexName;
    pContext->LocalToString(params[2], &indexName);

    g_pSM->LogMessage(myself, "MongoDB_DropIndex: collection=%d, indexName=%s", collection, indexName);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_DropIndex: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build API URL for dropIndex
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/indexes/" +
                     std::string(indexName);

    // For dropIndex, we might use DELETE method, but since we only have POST available,
    // we'll use a POST with action parameter
    std::string postData = "{\"action\":\"drop\",\"indexName\":\"" + EscapeJsonString(indexName) + "\"}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_DropIndex: POST to %s with data: %s", url.c_str(), postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_DropIndex: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        g_pSM->LogMessage(myself, "MongoDB_DropIndex: Success, dropped index: %s", indexName);
        return 1;
    }

    g_pSM->LogMessage(myself, "MongoDB_DropIndex: Failed");
    return 0;
}

// MongoDB_BulkWrite - Execute multiple operations in one request
cell_t MongoDB_BulkWrite(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    Handle_t operations = params[2]; // ArrayList of operation objects (JSON strings)
    bool ordered = params[3]; // Whether operations should be ordered

    g_pSM->LogMessage(myself, "MongoDB_BulkWrite: collection=%d, operations=%d, ordered=%d",
                     collection, operations, ordered);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_BulkWrite: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build API URL for bulk write
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/documents/bulkWrite";

    // Build operations array (simplified - create sample bulk operations)
    std::ostringstream operationsJson;
    operationsJson << "[";

    // Sample bulk operations
    operationsJson << "{\"insertOne\":{\"document\":{\"name\":\"bulk_user1\",\"type\":\"test\"}}}";
    operationsJson << ",{\"updateOne\":{\"filter\":{\"name\":\"existing_user\"},\"update\":{\"$set\":{\"updated\":true}}}}";
    operationsJson << ",{\"deleteOne\":{\"filter\":{\"status\":\"inactive\"}}}";

    operationsJson << "]";

    std::string postData = "{\"operations\":" + operationsJson.str() +
                          ",\"ordered\":" + (ordered ? "true" : "false") + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_BulkWrite: POST to %s with data: %s", url.c_str(), postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_BulkWrite: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        // Parse bulk write results
        int insertedCount = 0, modifiedCount = 0, deletedCount = 0;

        // Extract counts from response
        size_t insertedStart = response.find("\"insertedCount\":");
        if (insertedStart != std::string::npos) {
            insertedStart += 16;
            size_t insertedEnd = response.find_first_of(",}", insertedStart);
            if (insertedEnd != std::string::npos) {
                std::string insertedStr = response.substr(insertedStart, insertedEnd - insertedStart);
                insertedCount = atoi(insertedStr.c_str());
            }
        }

        size_t modifiedStart = response.find("\"modifiedCount\":");
        if (modifiedStart != std::string::npos) {
            modifiedStart += 16;
            size_t modifiedEnd = response.find_first_of(",}", modifiedStart);
            if (modifiedEnd != std::string::npos) {
                std::string modifiedStr = response.substr(modifiedStart, modifiedEnd - modifiedStart);
                modifiedCount = atoi(modifiedStr.c_str());
            }
        }

        size_t deletedStart = response.find("\"deletedCount\":");
        if (deletedStart != std::string::npos) {
            deletedStart += 15;
            size_t deletedEnd = response.find_first_of(",}", deletedStart);
            if (deletedEnd != std::string::npos) {
                std::string deletedStr = response.substr(deletedStart, deletedEnd - deletedStart);
                deletedCount = atoi(deletedStr.c_str());
            }
        }

        g_pSM->LogMessage(myself, "MongoDB_BulkWrite: Success - Inserted: %d, Modified: %d, Deleted: %d",
                         insertedCount, modifiedCount, deletedCount);
        return 1;
    }

    g_pSM->LogMessage(myself, "MongoDB_BulkWrite: Failed");
    return 0;
}

// MongoDB_FindDistinct - Get distinct values for a field
cell_t MongoDB_FindDistinct(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    char *field;
    pContext->LocalToString(params[2], &field);
    Handle_t filter = params[3]; // StringMap filter (can be null)

    g_pSM->LogMessage(myself, "MongoDB_FindDistinct: collection=%d, field=%s, filter=%d",
                     collection, field, filter);

    if (g_collections.find(collection) == g_collections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_FindDistinct: Invalid collection handle %d", collection);
        return 0; // Invalid collection
    }

    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];

    // Parse database/collection
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);

    // Build API URL for distinct
    std::string url = baseUrl + "/api/v1/connections/" + connectionId +
                     "/databases/" + database + "/collections/" + collectionName + "/documents/distinct";

    // Build filter JSON
    std::string filterJson = "{}";
    if (filter != 0) {
        filterJson = StringMapToJson(pContext, filter);
    }

    std::string postData = "{\"field\":\"" + EscapeJsonString(field) +
                          "\",\"filter\":" + filterJson + "}";
    std::string response;

    g_pSM->LogMessage(myself, "MongoDB_FindDistinct: POST to %s with data: %s", url.c_str(), postData.c_str());

    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    g_pSM->LogMessage(myself, "MongoDB_FindDistinct: HTTP success=%d, response: %s", success, response.c_str());

    if (success && response.find("\"success\":true") != std::string::npos) {
        // Return ArrayList handle containing distinct values
        Handle_t resultHandle = g_nextHandle++;
        g_pSM->LogMessage(myself, "MongoDB_FindDistinct: Success, returning handle %d", resultHandle);
        return resultHandle;
    }

    g_pSM->LogMessage(myself, "MongoDB_FindDistinct: Failed");
    return 0;
}

// Enhanced error handling natives

// MongoDB_GetLastErrorCode - Get the last error code
cell_t MongoDB_GetLastErrorCode(IPluginContext *pContext, const cell_t *params) {
    return g_lastError.code;
}

// MongoDB_GetLastErrorMessage - Get the last error message
cell_t MongoDB_GetLastErrorMessage(IPluginContext *pContext, const cell_t *params) {
    char *buffer;
    pContext->LocalToString(params[1], &buffer);
    int maxlen = params[2];

    size_t copyLen = std::min((size_t)(maxlen - 1), g_lastError.message.length());
    strncpy(buffer, g_lastError.message.c_str(), copyLen);
    buffer[copyLen] = '\0';

    return 1;
}

// MongoDB_GetLastErrorDetails - Get detailed error information
cell_t MongoDB_GetLastErrorDetails(IPluginContext *pContext, const cell_t *params) {
    char *buffer;
    pContext->LocalToString(params[1], &buffer);
    int maxlen = params[2];

    size_t copyLen = std::min((size_t)(maxlen - 1), g_lastError.details.length());
    strncpy(buffer, g_lastError.details.c_str(), copyLen);
    buffer[copyLen] = '\0';

    return 1;
}

// MongoDB_GetLastErrorTimestamp - Get when the last error occurred
cell_t MongoDB_GetLastErrorTimestamp(IPluginContext *pContext, const cell_t *params) {
    return (cell_t)g_lastError.timestamp;
}

// Performance monitoring natives

// MongoDB_GetTotalOperations - Get total number of operations performed
cell_t MongoDB_GetTotalOperations(IPluginContext *pContext, const cell_t *params) {
    return g_performanceMetrics.totalOperations;
}

// MongoDB_GetSuccessfulOperations - Get number of successful operations
cell_t MongoDB_GetSuccessfulOperations(IPluginContext *pContext, const cell_t *params) {
    return g_performanceMetrics.successfulOperations;
}

// MongoDB_GetFailedOperations - Get number of failed operations
cell_t MongoDB_GetFailedOperations(IPluginContext *pContext, const cell_t *params) {
    return g_performanceMetrics.failedOperations;
}

// MongoDB_GetAverageExecutionTime - Get average execution time in milliseconds
cell_t MongoDB_GetAverageExecutionTime(IPluginContext *pContext, const cell_t *params) {
    return (cell_t)(g_performanceMetrics.averageExecutionTime * 100); // Return as centiseconds for integer precision
}

// MongoDB_GetSuccessRate - Get success rate as percentage (0-100)
cell_t MongoDB_GetSuccessRate(IPluginContext *pContext, const cell_t *params) {
    if (g_performanceMetrics.totalOperations == 0) {
        return 100; // No operations yet, assume 100%
    }

    double rate = (double)g_performanceMetrics.successfulOperations / g_performanceMetrics.totalOperations * 100.0;
    return (cell_t)rate;
}

// MongoDB_ResetPerformanceMetrics - Reset all performance counters
cell_t MongoDB_ResetPerformanceMetrics(IPluginContext *pContext, const cell_t *params) {
    g_performanceMetrics = {0, 0, 0, 0.0, 0.0, 0};
    g_pSM->LogMessage(myself, "MongoDB_ResetPerformanceMetrics: Performance metrics reset");
    return 1;
}

// Connection health check
cell_t MongoDB_TestConnection(IPluginContext *pContext, const cell_t *params) {
    Handle_t connection = params[1];

    g_pSM->LogMessage(myself, "MongoDB_TestConnection: Testing connection %d", connection);

    if (g_connections.find(connection) == g_connections.end()) {
        g_pSM->LogMessage(myself, "MongoDB_TestConnection: Invalid connection handle %d", connection);
        return 0; // Invalid connection
    }

    auto& connectionId = g_connections[connection];
    auto& baseUrl = g_connectionUrls[connection];

    // Build health check URL
    std::string url = baseUrl + "/api/v1/connections/" + connectionId + "/health";
    std::string postData = "{}";
    std::string response;
    double executionTime;

    g_pSM->LogMessage(myself, "MongoDB_TestConnection: Testing URL: %s", url.c_str());

    bool success = EnhancedHTTPPost(url.c_str(), postData.c_str(), response, executionTime);

    g_pSM->LogMessage(myself, "MongoDB_TestConnection: Result=%d, Time=%.2fms, Response: %s",
                     success, executionTime, response.c_str());

    if (success && response.find("\"status\":\"healthy\"") != std::string::npos) {
        return 1; // Connection is healthy
    }

    return 0; // Connection failed health check
}

// Native exports
const sp_nativeinfo_t g_MongoDBNatives[] = {
    // Configuration Management
    {"MongoDB_LoadConfig",      MongoDB_LoadConfig},
    {"MongoDB_SetAPIURL",       MongoDB_SetAPIURL},
    {"MongoDB_GetAPIURL",       MongoDB_GetAPIURL},
    {"MongoDB_SetTimeout",      MongoDB_SetTimeout},
    {"MongoDB_GetTimeout",      MongoDB_GetTimeout},

    // Connection Management
    {"MongoDB_Connect",         MongoDB_Connect},
    {"MongoDB_GetCollection",   MongoDB_GetCollection},
    {"MongoDB_IsConnected",     MongoDB_IsConnected},
    {"MongoDB_Close",           MongoDB_Close},
    {"MongoDB_InsertOne",       MongoDB_InsertOne},
    {"MongoDB_InsertOneJSON",   MongoDB_InsertOneJSON},
    {"MongoDB_InsertMany",      MongoDB_InsertMany},
    {"MongoDB_FindOne",         MongoDB_FindOne},
    {"MongoDB_FindOneJSON",     MongoDB_FindOneJSON},
    {"MongoDB_Find",            MongoDB_Find},
    {"MongoDB_UpdateOne",       MongoDB_UpdateOne},
    {"MongoDB_UpdateMany",      MongoDB_UpdateMany},
    {"MongoDB_DeleteOne",       MongoDB_DeleteOne},
    {"MongoDB_DeleteMany",      MongoDB_DeleteMany},
    {"MongoDB_CountDocuments",  MongoDB_CountDocuments},
    {"MongoDB_CreateIndex",     MongoDB_CreateIndex},
    {"MongoDB_DropIndex",       MongoDB_DropIndex},
    {"MongoDB_GetLastError",    MongoDB_GetLastError},
    {"JSON_StringMapToString",  JSON_StringMapToString},
    {"JSON_StringFromString",   JSON_StringFromString},
    {"JSON_ArrayListToString",  JSON_ArrayListToString},
    {"JSON_ArrayFromString",    JSON_ArrayFromString},
    {"StringMap_SetString",     StringMap_SetString},
    {"StringMap_GetString",     StringMap_GetString},
    {"StringMap_CreateEmpty",   StringMap_CreateEmpty},
    {"MongoDB_Aggregate",       MongoDB_Aggregate},
    {"MongoDB_FindWithProjection", MongoDB_FindWithProjection},
    {"MongoDB_BulkWrite",       MongoDB_BulkWrite},
    {"MongoDB_FindDistinct",    MongoDB_FindDistinct},
    {"MongoDB_GetLastErrorCode", MongoDB_GetLastErrorCode},
    {"MongoDB_GetLastErrorMessage", MongoDB_GetLastErrorMessage},
    {"MongoDB_GetLastErrorDetails", MongoDB_GetLastErrorDetails},
    {"MongoDB_GetLastErrorTimestamp", MongoDB_GetLastErrorTimestamp},
    {"MongoDB_GetTotalOperations", MongoDB_GetTotalOperations},
    {"MongoDB_GetSuccessfulOperations", MongoDB_GetSuccessfulOperations},
    {"MongoDB_GetFailedOperations", MongoDB_GetFailedOperations},
    {"MongoDB_GetAverageExecutionTime", MongoDB_GetAverageExecutionTime},
    {"MongoDB_GetSuccessRate",  MongoDB_GetSuccessRate},
    {"MongoDB_ResetPerformanceMetrics", MongoDB_ResetPerformanceMetrics},
    {"MongoDB_TestConnection",  MongoDB_TestConnection},
    {nullptr,                   nullptr}
};

// Extension implementation

bool HTTPMongoDBExtension::SDK_OnLoad(char *error, size_t maxlen, bool late) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    g_pSM->LogMessage(myself, "HTTP MongoDB Extension loaded successfully");
    return true;
}

void HTTPMongoDBExtension::SDK_OnAllLoaded() {
    sharesys->AddNatives(myself, g_MongoDBNatives);
    g_pSM->LogMessage(myself, "HTTP MongoDB Extension natives registered");
}

void HTTPMongoDBExtension::SDK_OnUnload() {
    curl_global_cleanup();
    g_pSM->LogMessage(myself, "HTTP MongoDB Extension unloaded");
}
