/**
 * Minimal MongoDB Extension - No libcurl dependencies
 * Uses raw sockets for HTTP communication to keep size small (~130KB)
 */

#include "extension.h"
#include <map>
#include <string>
#include <sstream>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

// Global variables
std::map<Handle_t, std::string> g_connectionUrls;
std::map<Handle_t, std::string> g_connections;
std::map<Handle_t, std::pair<Handle_t, std::string>> g_collections;
Handle_t g_nextHandle = 1;

// Simple HTTP client using only system calls (no libcurl)
class MinimalHTTP {
public:
    static bool Post(const std::string& url, const std::string& data, std::string& response) {
        std::string host, path;
        int port;
        
        if (!ParseURL(url, host, port, path)) {
            return false;
        }
        
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return false;
        
        // Set socket timeout
        struct timeval timeout;
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        
        struct hostent* server = gethostbyname(host.c_str());
        if (!server) {
            close(sock);
            return false;
        }
        
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        memcpy(&addr.sin_addr.s_addr, server->h_addr, server->h_length);
        
        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(sock);
            return false;
        }
        
        // Build HTTP request
        std::ostringstream request;
        request << "POST " << path << " HTTP/1.1\r\n";
        request << "Host: " << host << "\r\n";
        request << "Content-Type: application/json\r\n";
        request << "Content-Length: " << data.length() << "\r\n";
        request << "Connection: close\r\n";
        request << "\r\n";
        request << data;
        
        std::string requestStr = request.str();
        
        // Send request
        if (send(sock, requestStr.c_str(), requestStr.length(), 0) < 0) {
            close(sock);
            return false;
        }
        
        // Read response
        char buffer[4096];
        response.clear();
        while (true) {
            int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytes <= 0) break;
            buffer[bytes] = '\0';
            response += buffer;
        }
        
        close(sock);
        
        // Extract body (after \r\n\r\n)
        size_t bodyStart = response.find("\r\n\r\n");
        if (bodyStart != std::string::npos) {
            response = response.substr(bodyStart + 4);
        }
        
        return !response.empty();
    }
    
private:
    static bool ParseURL(const std::string& url, std::string& host, int& port, std::string& path) {
        // Parse http://host:port/path
        size_t start = url.find("://");
        if (start == std::string::npos) return false;
        start += 3;
        
        size_t pathPos = url.find('/', start);
        if (pathPos == std::string::npos) {
            pathPos = url.length();
            path = "/";
        } else {
            path = url.substr(pathPos);
        }
        
        std::string hostPort = url.substr(start, pathPos - start);
        size_t colonPos = hostPort.find(':');
        if (colonPos != std::string::npos) {
            host = hostPort.substr(0, colonPos);
            port = atoi(hostPort.substr(colonPos + 1).c_str());
        } else {
            host = hostPort;
            port = 80;
        }
        
        return true;
    }
};

// JSON utility functions
std::string EscapeJsonString(const std::string& str) {
    std::string escaped;
    for (size_t i = 0; i < str.length(); ++i) {
        char c = str[i];
        switch (c) {
            case '"': escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default: escaped += c; break;
        }
    }
    return escaped;
}

// MongoDB_Connect
cell_t MongoDB_Connect(IPluginContext *pContext, const cell_t *params) {
    char *apiUrl;
    pContext->LocalToString(params[1], &apiUrl);
    
    std::string mongoUri = "mongodb://admin:83C.!gotJK%40Z8VJmbDZMxbCk%40kyHJA.R@37.114.54.74:27017/?authSource=admin";
    std::string baseUrl = std::string(apiUrl);
    
    g_pSM->LogMessage(myself, "MongoDB_Connect: Connecting to %s", baseUrl.c_str());
    
    // Create connection
    std::string postData = "{\"uri\":\"" + EscapeJsonString(mongoUri) + "\"}";
    std::string response;
    std::string url = baseUrl + "/api/v1/connections";
    
    if (!MinimalHTTP::Post(url, postData, response)) {
        g_pSM->LogMessage(myself, "MongoDB_Connect: HTTP request failed");
        return 0;
    }
    
    // Extract connection ID
    size_t idStart = response.find("\"connectionId\":\"");
    if (idStart == std::string::npos) {
        g_pSM->LogMessage(myself, "MongoDB_Connect: No connection ID in response");
        return 0;
    }
    
    idStart += 16;
    size_t idEnd = response.find("\"", idStart);
    if (idEnd == std::string::npos) {
        g_pSM->LogMessage(myself, "MongoDB_Connect: Invalid connection ID format");
        return 0;
    }
    
    std::string connectionId = response.substr(idStart, idEnd - idStart);
    
    Handle_t handle = g_nextHandle++;
    g_connectionUrls[handle] = baseUrl;
    g_connections[handle] = connectionId;
    
    g_pSM->LogMessage(myself, "MongoDB_Connect: Success, handle=%d, id=%s", handle, connectionId.c_str());
    return handle;
}

// MongoDB_GetCollection
cell_t MongoDB_GetCollection(IPluginContext *pContext, const cell_t *params) {
    Handle_t connection = params[1];
    char *database, *collection;
    pContext->LocalToString(params[2], &database);
    pContext->LocalToString(params[3], &collection);
    
    if (g_connections.find(connection) == g_connections.end()) {
        return 0;
    }
    
    Handle_t collectionHandle = g_nextHandle++;
    std::string dbColl = std::string(database) + "/" + std::string(collection);
    g_collections[collectionHandle] = std::make_pair(connection, dbColl);
    
    g_pSM->LogMessage(myself, "MongoDB_GetCollection: handle=%d for %s", collectionHandle, dbColl.c_str());
    return collectionHandle;
}

// MongoDB_InsertOneJSON
cell_t MongoDB_InsertOneJSON(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    char *jsonDocument;
    pContext->LocalToString(params[2], &jsonDocument);
    char *insertedId;
    pContext->LocalToString(params[3], &insertedId);
    int maxlen = params[4];
    
    if (g_collections.find(collection) == g_collections.end()) {
        return 0;
    }
    
    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];
    
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);
    
    std::string url = baseUrl + "/api/v1/connections/" + connectionId + 
                     "/databases/" + database + "/collections/" + collectionName + "/documents";
    
    std::string postData = "{\"document\":" + std::string(jsonDocument) + "}";
    std::string response;
    
    if (!MinimalHTTP::Post(url, postData, response)) {
        return 0;
    }
    
    // Extract inserted ID
    size_t idStart = response.find("\"insertedId\":\"");
    if (idStart != std::string::npos) {
        idStart += 14;
        size_t idEnd = response.find("\"", idStart);
        if (idEnd != std::string::npos && (idEnd - idStart) < (size_t)maxlen) {
            std::string actualId = response.substr(idStart, idEnd - idStart);
            strncpy(insertedId, actualId.c_str(), maxlen - 1);
            insertedId[maxlen - 1] = '\0';
            return 1;
        }
    }
    
    strncpy(insertedId, "unknown", maxlen - 1);
    insertedId[maxlen - 1] = '\0';
    return response.find("\"success\":true") != std::string::npos ? 1 : 0;
}

// MongoDB_IsConnected
cell_t MongoDB_IsConnected(IPluginContext *pContext, const cell_t *params) {
    Handle_t connection = params[1];

    // For minimal version, just return true if handle is valid
    // In real implementation, this would check actual connection status
    return (connection != BAD_HANDLE && g_connections.find(connection) != g_connections.end()) ? 1 : 0;
}

// MongoDB_CountDocuments
cell_t MongoDB_CountDocuments(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = params[1];
    
    if (g_collections.find(collection) == g_collections.end()) {
        return 0;
    }
    
    auto& collInfo = g_collections[collection];
    auto& connectionId = g_connections[collInfo.first];
    auto& baseUrl = g_connectionUrls[collInfo.first];
    
    std::string dbColl = collInfo.second;
    size_t slashPos = dbColl.find('/');
    std::string database = dbColl.substr(0, slashPos);
    std::string collectionName = dbColl.substr(slashPos + 1);
    
    std::string url = baseUrl + "/api/v1/connections/" + connectionId + 
                     "/databases/" + database + "/collections/" + collectionName + "/documents/count";
    
    std::string postData = "{\"filter\":{}}";
    std::string response;
    
    if (!MinimalHTTP::Post(url, postData, response)) {
        return 0;
    }
    
    size_t countStart = response.find("\"count\":");
    if (countStart != std::string::npos) {
        countStart += 8;
        size_t countEnd = response.find_first_of(",}", countStart);
        if (countEnd != std::string::npos) {
            std::string countStr = response.substr(countStart, countEnd - countStart);
            return atoi(countStr.c_str());
        }
    }
    
    return 0;
}

// Native exports
const sp_nativeinfo_t g_MongoDBNatives[] = {
    {"MongoDB_Connect",         MongoDB_Connect},
    {"MongoDB_GetCollection",   MongoDB_GetCollection},
    {"MongoDB_IsConnected",     MongoDB_IsConnected},
    {"MongoDB_InsertOneJSON",   MongoDB_InsertOneJSON},
    {"MongoDB_CountDocuments",  MongoDB_CountDocuments},
    {nullptr,                   nullptr}
};

// Extension class
class MinimalMongoDBExtension : public SDKExtension {
public:
    virtual bool SDK_OnLoad(char *error, size_t maxlen, bool late) {
        sharesys->AddNatives(myself, g_MongoDBNatives);
        g_pSM->LogMessage(myself, "Minimal MongoDB Extension loaded (no libcurl)");
        return true;
    }
    
    virtual void SDK_OnUnload() {}
    virtual void SDK_OnAllLoaded() {}
    virtual bool QueryRunning(char *error, size_t maxlen) { return true; }
    virtual const char *GetExtensionName() { return "Minimal MongoDB Extension"; }
    virtual const char *GetExtensionURL() { return "http://www.sourcemod.net/"; }
    virtual const char *GetExtensionTag() { return "mongodb"; }
    virtual const char *GetExtensionAuthor() { return "SourceMod Team"; }
    virtual const char *GetExtensionVerString() { return "1.0.0-minimal"; }
    virtual const char *GetExtensionDescription() { return "Minimal MongoDB Extension without libcurl"; }
};

MinimalMongoDBExtension g_MinimalMongoDBExtension;
SMEXT_LINK(&g_MinimalMongoDBExtension);
