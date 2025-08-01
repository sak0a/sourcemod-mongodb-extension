/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod HTTP MongoDB Extension - MongoDB API Layer
 * Copyright (C) 2024 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 */

#ifndef _MONGODB_API_H_
#define _MONGODB_API_H_

#include <IHandleSys.h>
#include <IADTFactory.h>
#include <ICellArray.h>
#include <string>
#include <map>
#include <memory>

using namespace SourceMod;

// Forward declarations
class HTTPClient;
class JSONStructureManager;

/**
 * @brief Connection information structure
 */
struct ConnectionInfo {
    std::string connectionId;
    std::string apiUrl;
    bool isActive;
    std::chrono::steady_clock::time_point createdAt;
    std::chrono::steady_clock::time_point lastUsed;
};

/**
 * @brief Collection information structure
 */
struct CollectionInfo {
    std::string connectionId;
    std::string database;
    std::string collection;
    Handle_t connectionHandle;
};

/**
 * @brief MongoDB API Layer for SourceMod
 * 
 * Provides MongoDB functionality through HTTP API calls while maintaining
 * the same interface as the original MongoDB extension.
 */
class MongoDBAPILayer {
public:
    MongoDBAPILayer(HTTPClient* httpClient, JSONStructureManager* jsonManager);
    ~MongoDBAPILayer();
    
    bool Initialize();
    void Shutdown();
    
    // Connection management
    Handle_t CreateConnection(const std::string& apiUrl);
    bool CloseConnection(Handle_t connection);
    bool IsConnectionActive(Handle_t connection);
    bool PingConnection(Handle_t connection);
    
    // Collection operations
    Handle_t GetCollection(Handle_t connection, const std::string& database, 
                          const std::string& collection);
    
    // Document operations
    bool InsertOne(Handle_t collection, IBasicTrie* document, std::string& insertedId);
    bool InsertMany(Handle_t collection, ICellArray* documents, ICellArray* insertedIds);
    IBasicTrie* FindOne(Handle_t collection, IBasicTrie* filter);
    ICellArray* Find(Handle_t collection, IBasicTrie* filter, IBasicTrie* options);
    bool UpdateOne(Handle_t collection, IBasicTrie* filter, IBasicTrie* update);
    bool UpdateMany(Handle_t collection, IBasicTrie* filter, IBasicTrie* update);
    bool DeleteOne(Handle_t collection, IBasicTrie* filter);
    bool DeleteMany(Handle_t collection, IBasicTrie* filter);
    int CountDocuments(Handle_t collection, IBasicTrie* filter);

    // Index operations
    bool CreateIndex(Handle_t collection, IBasicTrie* keys, IBasicTrie* options);
    bool DropIndex(Handle_t collection, const std::string& indexName);
    ICellArray* ListIndexes(Handle_t collection);
    
    // Async operations
    bool InsertOneAsync(Handle_t collection, IBasicTrie* document,
                       IPluginFunction* callback, cell_t userData);
    bool FindOneAsync(Handle_t collection, IBasicTrie* filter,
                     IPluginFunction* callback, cell_t userData);

    // Batch operations
    bool ExecuteBatch(ICellArray* operations);
    
    // Error handling
    const std::string& GetLastError() const { return m_lastError; }
    void ClearLastError() { m_lastError.clear(); }
    
    // Statistics
    struct Stats {
        size_t totalConnections;
        size_t activeConnections;
        size_t totalOperations;
        size_t successfulOperations;
        size_t failedOperations;
    };
    
    const Stats& GetStats() const { return m_stats; }
    void ResetStats();

private:
    HTTPClient* m_httpClient;
    JSONStructureManager* m_jsonManager;
    IHandleSys* m_handleSys;
    
    std::map<Handle_t, std::unique_ptr<ConnectionInfo>> m_connections;
    std::map<Handle_t, std::unique_ptr<CollectionInfo>> m_collections;
    
    std::string m_lastError;
    Stats m_stats;
    
    // Handle types
    HandleType_t m_connectionHandleType;
    HandleType_t m_collectionHandleType;
    
    // Helper methods
    std::string BuildURL(const CollectionInfo& collInfo, const std::string& operation);
    bool ParseResponse(const std::string& response, std::string& data, std::string& error);
    bool SendAPIRequest(const std::string& endpoint, const std::string& method,
                       const std::string& requestData, std::string& responseData);
    
    ConnectionInfo* GetConnectionInfo(Handle_t connection);
    CollectionInfo* GetCollectionInfo(Handle_t collection);
    
    // Request builders
    std::string BuildInsertRequest(IBasicTrie* document);
    std::string BuildInsertManyRequest(ICellArray* documents);
    std::string BuildFindRequest(IBasicTrie* filter, IBasicTrie* options);
    std::string BuildUpdateRequest(IBasicTrie* filter, IBasicTrie* update);
    std::string BuildDeleteRequest(IBasicTrie* filter);
    std::string BuildCountRequest(IBasicTrie* filter);
    std::string BuildIndexRequest(IBasicTrie* keys, IBasicTrie* options);

    // Response parsers
    bool ParseInsertResponse(const std::string& response, std::string& insertedId);
    bool ParseInsertManyResponse(const std::string& response, ICellArray* insertedIds);
    IBasicTrie* ParseFindOneResponse(const std::string& response);
    ICellArray* ParseFindResponse(const std::string& response);
    bool ParseUpdateResponse(const std::string& response, int& matchedCount, int& modifiedCount);
    bool ParseDeleteResponse(const std::string& response, int& deletedCount);
    bool ParseCountResponse(const std::string& response, int& count);
    
    // Handle management
    Handle_t CreateConnectionHandle(std::unique_ptr<ConnectionInfo> connInfo);
    Handle_t CreateCollectionHandle(std::unique_ptr<CollectionInfo> collInfo);
    void DestroyConnectionHandle(Handle_t handle);
    void DestroyCollectionHandle(Handle_t handle);
    
    // Validation
    bool ValidateStringMap(IBasicTrie* map, const std::string& context);
    bool ValidateArrayList(ICellArray* array, const std::string& context);
    
    // Statistics
    void UpdateStats(bool success);
    
    // Constants
    static const char* API_VERSION;
    static const int DEFAULT_TIMEOUT;
    static const int MAX_RETRY_COUNT;
};

#endif // _MONGODB_API_H_
