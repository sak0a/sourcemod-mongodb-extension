/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod HTTP MongoDB Extension - MongoDB API Layer Implementation
 * Copyright (C) 2024 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 */

#include "mongodb_api.h"
#include "http_client.h"
#include "json_structures.h"
#include "third_party/json.hpp"
#include <chrono>

using json = nlohmann::json;

// Static constants
const char* MongoDBAPILayer::API_VERSION = "v1";
const int MongoDBAPILayer::DEFAULT_TIMEOUT = 30000;
const int MongoDBAPILayer::MAX_RETRY_COUNT = 3;

MongoDBAPILayer::MongoDBAPILayer(HTTPClient* httpClient, JSONStructureManager* jsonManager)
    : m_httpClient(httpClient), m_jsonManager(jsonManager), m_handleSys(nullptr),
      m_connectionHandleType(0), m_collectionHandleType(0) {
    
    // Initialize stats
    memset(&m_stats, 0, sizeof(m_stats));
}

MongoDBAPILayer::~MongoDBAPILayer() {
    Shutdown();
}

bool MongoDBAPILayer::Initialize() {
    // Get handle system
    m_handleSys = handlesys;
    if (!m_handleSys) {
        m_lastError = "Failed to get handle system";
        return false;
    }
    
    // Create handle types (simplified - in production, we'd create proper types)
    m_connectionHandleType = 0; // Use default type for now
    m_collectionHandleType = 0; // Use default type for now
    
    return true;
}

void MongoDBAPILayer::Shutdown() {
    // Close all connections
    for (auto& [handle, connInfo] : m_connections) {
        // Send close request to API
        std::string endpoint = "/api/" + std::string(API_VERSION) + "/connections/" + connInfo->connectionId;
        std::string response;
        m_httpClient->SendRequest(endpoint, "DELETE", "", response);
    }
    
    m_connections.clear();
    m_collections.clear();
}

Handle_t MongoDBAPILayer::CreateConnection(const std::string& apiUrl) {
    try {
        // Build connection request
        json requestJson = {
            {"uri", apiUrl}
        };
        
        std::string requestData = requestJson.dump();
        std::string responseData;
        
        // Send connection request
        std::string endpoint = "/api/" + std::string(API_VERSION) + "/connections";
        if (!SendAPIRequest(endpoint, "POST", requestData, responseData)) {
            return BAD_HANDLE;
        }
        
        // Parse response
        json responseJson = json::parse(responseData);
        if (!responseJson["success"].get<bool>()) {
            m_lastError = responseJson.value("error", "Unknown error");
            return BAD_HANDLE;
        }
        
        std::string connectionId = responseJson["data"]["connectionId"].get<std::string>();
        
        // Create connection info
        auto connInfo = std::make_unique<ConnectionInfo>();
        connInfo->connectionId = connectionId;
        connInfo->apiUrl = apiUrl;
        connInfo->isActive = true;
        connInfo->createdAt = std::chrono::steady_clock::now();
        connInfo->lastUsed = std::chrono::steady_clock::now();
        
        // Create handle
        Handle_t handle = CreateConnectionHandle(std::move(connInfo));
        
        UpdateStats(true);
        m_stats.totalConnections++;
        m_stats.activeConnections++;
        
        return handle;
        
    } catch (const json::exception& e) {
        m_lastError = "JSON error in CreateConnection: ";
        m_lastError += e.what();
        UpdateStats(false);
        return BAD_HANDLE;
    }
}

bool MongoDBAPILayer::CloseConnection(Handle_t connection) {
    ConnectionInfo* connInfo = GetConnectionInfo(connection);
    if (!connInfo) {
        m_lastError = "Invalid connection handle";
        return false;
    }
    
    try {
        // Send close request to API
        std::string endpoint = "/api/" + std::string(API_VERSION) + "/connections/" + connInfo->connectionId;
        std::string responseData;
        
        if (SendAPIRequest(endpoint, "DELETE", "", responseData)) {
            connInfo->isActive = false;
            m_stats.activeConnections--;
            
            // Remove from map
            m_connections.erase(connection);
            
            return true;
        }
        
        return false;
        
    } catch (const std::exception& e) {
        m_lastError = "Error closing connection: ";
        m_lastError += e.what();
        return false;
    }
}

bool MongoDBAPILayer::IsConnectionActive(Handle_t connection) {
    ConnectionInfo* connInfo = GetConnectionInfo(connection);
    return connInfo && connInfo->isActive;
}

Handle_t MongoDBAPILayer::GetCollection(Handle_t connection, const std::string& database, 
                                       const std::string& collection) {
    ConnectionInfo* connInfo = GetConnectionInfo(connection);
    if (!connInfo) {
        m_lastError = "Invalid connection handle";
        return BAD_HANDLE;
    }
    
    // Create collection info
    auto collInfo = std::make_unique<CollectionInfo>();
    collInfo->connectionId = connInfo->connectionId;
    collInfo->database = database;
    collInfo->collection = collection;
    collInfo->connectionHandle = connection;
    
    // Create handle
    Handle_t handle = CreateCollectionHandle(std::move(collInfo));
    return handle;
}

bool MongoDBAPILayer::InsertOne(Handle_t collection, IStringMap* document, std::string& insertedId) {
    CollectionInfo* collInfo = GetCollectionInfo(collection);
    if (!collInfo) {
        m_lastError = "Invalid collection handle";
        return false;
    }
    
    if (!ValidateStringMap(document, "document")) {
        return false;
    }
    
    try {
        // Convert document to JSON
        std::string documentJson;
        if (!m_jsonManager->StringMapToJSON(document, documentJson)) {
            m_lastError = "Failed to convert document to JSON: " + m_jsonManager->GetLastError();
            return false;
        }
        
        // Build request
        json requestJson = {
            {"document", json::parse(documentJson)}
        };
        
        std::string requestData = requestJson.dump();
        std::string responseData;
        
        // Send request
        std::string endpoint = BuildURL(*collInfo, "documents");
        if (!SendAPIRequest(endpoint, "POST", requestData, responseData)) {
            return false;
        }
        
        // Parse response
        if (!ParseInsertResponse(responseData, insertedId)) {
            return false;
        }
        
        UpdateStats(true);
        return true;
        
    } catch (const json::exception& e) {
        m_lastError = "JSON error in InsertOne: ";
        m_lastError += e.what();
        UpdateStats(false);
        return false;
    }
}

IStringMap* MongoDBAPILayer::FindOne(Handle_t collection, IStringMap* filter) {
    CollectionInfo* collInfo = GetCollectionInfo(collection);
    if (!collInfo) {
        m_lastError = "Invalid collection handle";
        return nullptr;
    }
    
    try {
        // Build request
        json requestJson = {{"filter", json::object()}};
        
        if (filter) {
            std::string filterJson;
            if (m_jsonManager->StringMapToJSON(filter, filterJson)) {
                requestJson["filter"] = json::parse(filterJson);
            }
        }
        
        std::string requestData = requestJson.dump();
        std::string responseData;
        
        // Send request
        std::string endpoint = BuildURL(*collInfo, "documents/findOne");
        if (!SendAPIRequest(endpoint, "POST", requestData, responseData)) {
            return nullptr;
        }
        
        // Parse response
        IStringMap* result = ParseFindOneResponse(responseData);
        if (result) {
            UpdateStats(true);
        } else {
            UpdateStats(false);
        }
        
        return result;
        
    } catch (const json::exception& e) {
        m_lastError = "JSON error in FindOne: ";
        m_lastError += e.what();
        UpdateStats(false);
        return nullptr;
    }
}

int MongoDBAPILayer::CountDocuments(Handle_t collection, IStringMap* filter) {
    CollectionInfo* collInfo = GetCollectionInfo(collection);
    if (!collInfo) {
        m_lastError = "Invalid collection handle";
        return -1;
    }
    
    try {
        // Build request
        json requestJson = {{"filter", json::object()}};
        
        if (filter) {
            std::string filterJson;
            if (m_jsonManager->StringMapToJSON(filter, filterJson)) {
                requestJson["filter"] = json::parse(filterJson);
            }
        }
        
        std::string requestData = requestJson.dump();
        std::string responseData;
        
        // Send request
        std::string endpoint = BuildURL(*collInfo, "documents/count");
        if (!SendAPIRequest(endpoint, "POST", requestData, responseData)) {
            return -1;
        }
        
        // Parse response
        int count;
        if (ParseCountResponse(responseData, count)) {
            UpdateStats(true);
            return count;
        } else {
            UpdateStats(false);
            return -1;
        }
        
    } catch (const json::exception& e) {
        m_lastError = "JSON error in CountDocuments: ";
        m_lastError += e.what();
        UpdateStats(false);
        return -1;
    }
}

// Helper methods implementation

std::string MongoDBAPILayer::BuildURL(const CollectionInfo& collInfo, const std::string& operation) {
    return "/api/" + std::string(API_VERSION) + "/connections/" + collInfo.connectionId + 
           "/databases/" + collInfo.database + "/collections/" + collInfo.collection + "/" + operation;
}

bool MongoDBAPILayer::SendAPIRequest(const std::string& endpoint, const std::string& method,
                                   const std::string& requestData, std::string& responseData) {
    if (!m_httpClient->SendRequest(endpoint, method, requestData, responseData)) {
        m_lastError = "HTTP request failed: " + m_httpClient->GetLastError();
        return false;
    }
    
    return true;
}

ConnectionInfo* MongoDBAPILayer::GetConnectionInfo(Handle_t connection) {
    auto it = m_connections.find(connection);
    return (it != m_connections.end()) ? it->second.get() : nullptr;
}

CollectionInfo* MongoDBAPILayer::GetCollectionInfo(Handle_t collection) {
    auto it = m_collections.find(collection);
    return (it != m_collections.end()) ? it->second.get() : nullptr;
}

Handle_t MongoDBAPILayer::CreateConnectionHandle(std::unique_ptr<ConnectionInfo> connInfo) {
    // Simplified handle creation - in production, use proper handle types
    Handle_t handle = static_cast<Handle_t>(reinterpret_cast<uintptr_t>(connInfo.get()));
    m_connections[handle] = std::move(connInfo);
    return handle;
}

Handle_t MongoDBAPILayer::CreateCollectionHandle(std::unique_ptr<CollectionInfo> collInfo) {
    // Simplified handle creation - in production, use proper handle types
    Handle_t handle = static_cast<Handle_t>(reinterpret_cast<uintptr_t>(collInfo.get()));
    m_collections[handle] = std::move(collInfo);
    return handle;
}

bool MongoDBAPILayer::ValidateStringMap(IStringMap* map, const std::string& context) {
    if (!map) {
        m_lastError = "StringMap is null in " + context;
        return false;
    }
    return true;
}

bool MongoDBAPILayer::ParseInsertResponse(const std::string& response, std::string& insertedId) {
    try {
        json responseJson = json::parse(response);
        if (!responseJson["success"].get<bool>()) {
            m_lastError = responseJson.value("error", "Insert operation failed");
            return false;
        }
        
        insertedId = responseJson["data"]["insertedId"].get<std::string>();
        return true;
        
    } catch (const json::exception& e) {
        m_lastError = "Failed to parse insert response: ";
        m_lastError += e.what();
        return false;
    }
}

IStringMap* MongoDBAPILayer::ParseFindOneResponse(const std::string& response) {
    try {
        json responseJson = json::parse(response);
        if (!responseJson["success"].get<bool>()) {
            m_lastError = responseJson.value("error", "Find operation failed");
            return nullptr;
        }
        
        if (responseJson["data"].is_null()) {
            return nullptr; // No document found
        }
        
        // Create new StringMap
        IStringMap* result = adtfactory->CreateBasicStringMap();
        if (!result) {
            m_lastError = "Failed to create StringMap for result";
            return nullptr;
        }
        
        // Convert JSON to StringMap
        std::string documentJson = responseJson["data"].dump();
        if (!m_jsonManager->JSONToStringMap(documentJson, result)) {
            result->Destroy();
            m_lastError = "Failed to convert response to StringMap: " + m_jsonManager->GetLastError();
            return nullptr;
        }
        
        return result;
        
    } catch (const json::exception& e) {
        m_lastError = "Failed to parse find response: ";
        m_lastError += e.what();
        return nullptr;
    }
}

bool MongoDBAPILayer::ParseCountResponse(const std::string& response, int& count) {
    try {
        json responseJson = json::parse(response);
        if (!responseJson["success"].get<bool>()) {
            m_lastError = responseJson.value("error", "Count operation failed");
            return false;
        }
        
        count = responseJson["data"]["count"].get<int>();
        return true;
        
    } catch (const json::exception& e) {
        m_lastError = "Failed to parse count response: ";
        m_lastError += e.what();
        return false;
    }
}

bool MongoDBAPILayer::UpdateOne(Handle_t collection, IStringMap* filter, IStringMap* update) {
    CollectionInfo* collInfo = GetCollectionInfo(collection);
    if (!collInfo) {
        m_lastError = "Invalid collection handle";
        return false;
    }

    if (!ValidateStringMap(filter, "filter") || !ValidateStringMap(update, "update")) {
        return false;
    }

    try {
        // Convert filter and update to JSON
        std::string filterJson, updateJson;
        if (!m_jsonManager->StringMapToJSON(filter, filterJson) ||
            !m_jsonManager->StringMapToJSON(update, updateJson)) {
            m_lastError = "Failed to convert parameters to JSON";
            return false;
        }

        // Build request
        json requestJson = {
            {"filter", json::parse(filterJson)},
            {"update", json::parse(updateJson)}
        };

        std::string requestData = requestJson.dump();
        std::string responseData;

        // Send request
        std::string endpoint = BuildURL(*collInfo, "documents/updateOne");
        if (!SendAPIRequest(endpoint, "PUT", requestData, responseData)) {
            return false;
        }

        // Parse response
        int matchedCount, modifiedCount;
        if (ParseUpdateResponse(responseData, matchedCount, modifiedCount)) {
            UpdateStats(true);
            return true;
        } else {
            UpdateStats(false);
            return false;
        }

    } catch (const json::exception& e) {
        m_lastError = "JSON error in UpdateOne: ";
        m_lastError += e.what();
        UpdateStats(false);
        return false;
    }
}

bool MongoDBAPILayer::DeleteOne(Handle_t collection, IStringMap* filter) {
    CollectionInfo* collInfo = GetCollectionInfo(collection);
    if (!collInfo) {
        m_lastError = "Invalid collection handle";
        return false;
    }

    if (!ValidateStringMap(filter, "filter")) {
        return false;
    }

    try {
        // Convert filter to JSON
        std::string filterJson;
        if (!m_jsonManager->StringMapToJSON(filter, filterJson)) {
            m_lastError = "Failed to convert filter to JSON";
            return false;
        }

        // Build request
        json requestJson = {
            {"filter", json::parse(filterJson)}
        };

        std::string requestData = requestJson.dump();
        std::string responseData;

        // Send request
        std::string endpoint = BuildURL(*collInfo, "documents/deleteOne");
        if (!SendAPIRequest(endpoint, "DELETE", requestData, responseData)) {
            return false;
        }

        // Parse response
        int deletedCount;
        if (ParseDeleteResponse(responseData, deletedCount)) {
            UpdateStats(true);
            return deletedCount > 0;
        } else {
            UpdateStats(false);
            return false;
        }

    } catch (const json::exception& e) {
        m_lastError = "JSON error in DeleteOne: ";
        m_lastError += e.what();
        UpdateStats(false);
        return false;
    }
}

bool MongoDBAPILayer::ParseUpdateResponse(const std::string& response, int& matchedCount, int& modifiedCount) {
    try {
        json responseJson = json::parse(response);
        if (!responseJson["success"].get<bool>()) {
            m_lastError = responseJson.value("error", "Update operation failed");
            return false;
        }

        matchedCount = responseJson["data"]["matchedCount"].get<int>();
        modifiedCount = responseJson["data"]["modifiedCount"].get<int>();
        return true;

    } catch (const json::exception& e) {
        m_lastError = "Failed to parse update response: ";
        m_lastError += e.what();
        return false;
    }
}

bool MongoDBAPILayer::ParseDeleteResponse(const std::string& response, int& deletedCount) {
    try {
        json responseJson = json::parse(response);
        if (!responseJson["success"].get<bool>()) {
            m_lastError = responseJson.value("error", "Delete operation failed");
            return false;
        }

        deletedCount = responseJson["data"]["deletedCount"].get<int>();
        return true;

    } catch (const json::exception& e) {
        m_lastError = "Failed to parse delete response: ";
        m_lastError += e.what();
        return false;
    }
}

void MongoDBAPILayer::UpdateStats(bool success) {
    m_stats.totalOperations++;

    if (success) {
        m_stats.successfulOperations++;
    } else {
        m_stats.failedOperations++;
    }
}
