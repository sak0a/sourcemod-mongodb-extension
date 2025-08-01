/**
 * Simple HTTP MongoDB Extension
 * Minimal version for testing HTTP API integration
 */

#include "minimal_extension.h"
#include <curl/curl.h>

HTTPMongoDBExtension g_HTTPMongoDBExtension;
SMEXT_LINK(&g_HTTPMongoDBExtension);

// Simple HTTP response callback
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

// Simple HTTP POST function
bool SimpleHTTPPost(const char* url, const char* data, std::string& response) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}

// Native function: MongoDB_Connect
cell_t MongoDB_Connect(IPluginContext *pContext, const cell_t *params) {
    char *uri;
    pContext->LocalToString(params[1], &uri);

    std::string postData = "{\"uri\":\"";
    postData += uri;
    postData += "\"}";

    std::string response;
    bool success = SimpleHTTPPost("http://127.0.0.1:3300/api/v1/connections", postData.c_str(), response);

    if (success && response.find("\"success\":true") != std::string::npos) {
        // Extract connection ID from response
        size_t start = response.find("\"connectionId\":\"") + 16;
        size_t end = response.find("\"", start);
        if (start != std::string::npos && end != std::string::npos) {
            std::string connectionId = response.substr(start, end - start);
            // Store connection ID in a simple way (for demo purposes)
            pContext->StringToLocal(params[2], 64, connectionId.c_str());
            return 1; // success
        }
    }

    return 0; // failure
}

// Native function: MongoDB_Insert
cell_t MongoDB_Insert(IPluginContext *pContext, const cell_t *params) {
    char *connectionId, *database, *collection, *document;
    pContext->LocalToString(params[1], &connectionId);
    pContext->LocalToString(params[2], &database);
    pContext->LocalToString(params[3], &collection);
    pContext->LocalToString(params[4], &document);

    std::string url = "http://127.0.0.1:3300/api/v1/connections/";
    url += connectionId;
    url += "/databases/";
    url += database;
    url += "/collections/";
    url += collection;
    url += "/documents";

    std::string postData = "{\"document\":";
    postData += document;
    postData += "}";

    std::string response;
    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    return (success && response.find("\"success\":true") != std::string::npos) ? 1 : 0;
}

// Native function: MongoDB_Find
cell_t MongoDB_Find(IPluginContext *pContext, const cell_t *params) {
    char *connectionId, *database, *collection, *filter;
    pContext->LocalToString(params[1], &connectionId);
    pContext->LocalToString(params[2], &database);
    pContext->LocalToString(params[3], &collection);
    pContext->LocalToString(params[4], &filter);

    std::string url = "http://127.0.0.1:3300/api/v1/connections/";
    url += connectionId;
    url += "/databases/";
    url += database;
    url += "/collections/";
    url += collection;
    url += "/documents/find";

    std::string postData = "{\"filter\":";
    postData += filter;
    postData += ",\"options\":{\"limit\":100}}";

    std::string response;
    bool success = SimpleHTTPPost(url.c_str(), postData.c_str(), response);

    if (success && response.find("\"success\":true") != std::string::npos) {
        // Store response in output parameter
        pContext->StringToLocal(params[5], params[6], response.c_str());
        return 1;
    }

    return 0;
}

// Native function: MongoDB_Update
cell_t MongoDB_Update(IPluginContext *pContext, const cell_t *params) {
    char *connectionId, *database, *collection, *filter, *update;
    pContext->LocalToString(params[1], &connectionId);
    pContext->LocalToString(params[2], &database);
    pContext->LocalToString(params[3], &collection);
    pContext->LocalToString(params[4], &filter);
    pContext->LocalToString(params[5], &update);

    std::string url = "http://127.0.0.1:3300/api/v1/connections/";
    url += connectionId;
    url += "/databases/";
    url += database;
    url += "/collections/";
    url += collection;
    url += "/documents/updateOne";

    std::string postData = "{\"filter\":";
    postData += filter;
    postData += ",\"update\":";
    postData += update;
    postData += "}";

    std::string response;
    CURL* curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return (res == CURLE_OK && response.find("\"success\":true") != std::string::npos) ? 1 : 0;
    }

    return 0;
}

// Native function: MongoDB_Delete
cell_t MongoDB_Delete(IPluginContext *pContext, const cell_t *params) {
    char *connectionId, *database, *collection, *filter;
    pContext->LocalToString(params[1], &connectionId);
    pContext->LocalToString(params[2], &database);
    pContext->LocalToString(params[3], &collection);
    pContext->LocalToString(params[4], &filter);

    std::string url = "http://127.0.0.1:3300/api/v1/connections/";
    url += connectionId;
    url += "/databases/";
    url += database;
    url += "/collections/";
    url += collection;
    url += "/documents/deleteOne";

    std::string postData = "{\"filter\":";
    postData += filter;
    postData += "}";

    std::string response;
    CURL* curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return (res == CURLE_OK && response.find("\"success\":true") != std::string::npos) ? 1 : 0;
    }

    return 0;
}

// Native exports
const sp_nativeinfo_t g_ExtensionNatives[] = {
    {"MongoDB_Connect",     MongoDB_Connect},
    {"MongoDB_Insert",      MongoDB_Insert},
    {"MongoDB_Find",        MongoDB_Find},
    {"MongoDB_Update",      MongoDB_Update},
    {"MongoDB_Delete",      MongoDB_Delete},
    {nullptr,               nullptr}
};

bool HTTPMongoDBExtension::SDK_OnLoad(char *error, size_t maxlen, bool late) {
    // Initialize curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    return true;
}

void HTTPMongoDBExtension::SDK_OnAllLoaded() {
    // Register natives after all extensions are loaded
    sharesys->AddNatives(myself, g_ExtensionNatives);
}

void HTTPMongoDBExtension::SDK_OnUnload() {
    curl_global_cleanup();
}

// IExtensionInterface methods
bool HTTPMongoDBExtension::OnExtensionLoad(IExtension *me, IShareSys *sys, char *error, size_t maxlength, bool late) {
    // Store references
    myself = me;
    sharesys = sys;

    // Call the SDK load method
    return SDK_OnLoad(error, maxlength, late);
}

void HTTPMongoDBExtension::OnExtensionUnload() {
    SDK_OnUnload();
}

void HTTPMongoDBExtension::OnExtensionsAllLoaded() {
    SDK_OnAllLoaded();
}
