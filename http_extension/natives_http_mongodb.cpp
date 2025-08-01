/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod HTTP MongoDB Extension - Native Functions Implementation
 * Copyright (C) 2024 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 */

#include "natives.h"
#include "extension.h"

// MongoDB_Connect
cell_t MongoDB_Connect(IPluginContext *pContext, const cell_t *params) {
    char *apiUrl;
    pContext->LocalToString(params[1], &apiUrl);
    
    Handle_t connection = g_HTTPMongoDBExtension.GetMongoAPI()->CreateConnection(apiUrl);
    return static_cast<cell_t>(connection);
}

// MongoDB_Close
cell_t MongoDB_Close(IPluginContext *pContext, const cell_t *params) {
    Handle_t connection = static_cast<Handle_t>(params[1]);
    
    bool success = g_HTTPMongoDBExtension.GetMongoAPI()->CloseConnection(connection);
    return success ? 1 : 0;
}

// MongoDB_IsConnected
cell_t MongoDB_IsConnected(IPluginContext *pContext, const cell_t *params) {
    Handle_t connection = static_cast<Handle_t>(params[1]);
    
    bool isConnected = g_HTTPMongoDBExtension.GetMongoAPI()->IsConnectionActive(connection);
    return isConnected ? 1 : 0;
}

// MongoDB_GetCollection
cell_t MongoDB_GetCollection(IPluginContext *pContext, const cell_t *params) {
    Handle_t connection = static_cast<Handle_t>(params[1]);
    
    char *database, *collection;
    pContext->LocalToString(params[2], &database);
    pContext->LocalToString(params[3], &collection);
    
    Handle_t collectionHandle = g_HTTPMongoDBExtension.GetMongoAPI()->GetCollection(
        connection, database, collection);
    
    return static_cast<cell_t>(collectionHandle);
}

// MongoDB_InsertOne
cell_t MongoDB_InsertOne(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = static_cast<Handle_t>(params[1]);
    Handle_t document = static_cast<Handle_t>(params[2]);
    
    // Get StringMap from handle
    IStringMap* documentMap = nullptr;
    HandleError err = handlesys->ReadHandle(document, 0, nullptr, (void**)&documentMap);
    if (err != HandleError_None) {
        return 0;
    }
    
    std::string insertedId;
    bool success = g_HTTPMongoDBExtension.GetMongoAPI()->InsertOne(collection, documentMap, insertedId);
    
    if (success && params[0] >= 4) {
        pContext->StringToLocal(params[3], params[4], insertedId.c_str());
    }
    
    return success ? 1 : 0;
}

// MongoDB_FindOne
cell_t MongoDB_FindOne(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = static_cast<Handle_t>(params[1]);
    Handle_t filter = (params[0] >= 2) ? static_cast<Handle_t>(params[2]) : BAD_HANDLE;
    
    IStringMap* filterMap = nullptr;
    if (filter != BAD_HANDLE) {
        HandleError err = handlesys->ReadHandle(filter, 0, nullptr, (void**)&filterMap);
        if (err != HandleError_None) {
            return static_cast<cell_t>(BAD_HANDLE);
        }
    }
    
    IStringMap* result = g_HTTPMongoDBExtension.GetMongoAPI()->FindOne(collection, filterMap);
    
    if (result) {
        // Create handle for the result StringMap
        Handle_t resultHandle = handlesys->CreateHandle(0, result, nullptr, nullptr, nullptr);
        return static_cast<cell_t>(resultHandle);
    }
    
    return static_cast<cell_t>(BAD_HANDLE);
}

// MongoDB_UpdateOne
cell_t MongoDB_UpdateOne(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = static_cast<Handle_t>(params[1]);
    Handle_t filter = static_cast<Handle_t>(params[2]);
    Handle_t update = static_cast<Handle_t>(params[3]);
    
    // Get StringMaps from handles
    IStringMap* filterMap = nullptr;
    IStringMap* updateMap = nullptr;
    
    HandleError err1 = handlesys->ReadHandle(filter, 0, nullptr, (void**)&filterMap);
    HandleError err2 = handlesys->ReadHandle(update, 0, nullptr, (void**)&updateMap);
    
    if (err1 != HandleError_None || err2 != HandleError_None) {
        return 0;
    }
    
    bool success = g_HTTPMongoDBExtension.GetMongoAPI()->UpdateOne(collection, filterMap, updateMap);
    return success ? 1 : 0;
}

// MongoDB_DeleteOne
cell_t MongoDB_DeleteOne(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = static_cast<Handle_t>(params[1]);
    Handle_t filter = static_cast<Handle_t>(params[2]);
    
    // Get StringMap from handle
    IStringMap* filterMap = nullptr;
    HandleError err = handlesys->ReadHandle(filter, 0, nullptr, (void**)&filterMap);
    if (err != HandleError_None) {
        return 0;
    }
    
    bool success = g_HTTPMongoDBExtension.GetMongoAPI()->DeleteOne(collection, filterMap);
    return success ? 1 : 0;
}

// MongoDB_CountDocuments
cell_t MongoDB_CountDocuments(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = static_cast<Handle_t>(params[1]);
    Handle_t filter = (params[0] >= 2) ? static_cast<Handle_t>(params[2]) : BAD_HANDLE;
    
    IStringMap* filterMap = nullptr;
    if (filter != BAD_HANDLE) {
        HandleError err = handlesys->ReadHandle(filter, 0, nullptr, (void**)&filterMap);
        if (err != HandleError_None) {
            return -1;
        }
    }
    
    int count = g_HTTPMongoDBExtension.GetMongoAPI()->CountDocuments(collection, filterMap);
    return count;
}

// MongoDB_GetLastError
cell_t MongoDB_GetLastError(IPluginContext *pContext, const cell_t *params) {
    const std::string& error = g_HTTPMongoDBExtension.GetMongoAPI()->GetLastError();
    
    if (params[0] >= 2) {
        pContext->StringToLocal(params[1], params[2], error.c_str());
        return error.length();
    }
    
    return 0;
}

// JSON_StringMapToString
cell_t JSON_StringMapToString(IPluginContext *pContext, const cell_t *params) {
    Handle_t mapHandle = static_cast<Handle_t>(params[1]);
    
    // Get StringMap from handle
    IStringMap* map = nullptr;
    HandleError err = handlesys->ReadHandle(mapHandle, 0, nullptr, (void**)&map);
    if (err != HandleError_None) {
        return 0;
    }
    
    std::string jsonStr;
    bool success = g_HTTPMongoDBExtension.GetJSONManager()->StringMapToJSON(map, jsonStr);
    
    if (success && params[0] >= 3) {
        pContext->StringToLocal(params[2], params[3], jsonStr.c_str());
        return 1;
    }
    
    return 0;
}

// JSON_StringFromString
cell_t JSON_StringFromString(IPluginContext *pContext, const cell_t *params) {
    Handle_t mapHandle = static_cast<Handle_t>(params[1]);
    char *jsonStr;
    pContext->LocalToString(params[2], &jsonStr);
    
    // Get StringMap from handle
    IStringMap* map = nullptr;
    HandleError err = handlesys->ReadHandle(mapHandle, 0, nullptr, (void**)&map);
    if (err != HandleError_None) {
        return 0;
    }
    
    bool success = g_HTTPMongoDBExtension.GetJSONManager()->JSONToStringMap(jsonStr, map);
    return success ? 1 : 0;
}

// Native registration
const sp_nativeinfo_t g_MongoDBNatives[] = {
    {"MongoDB_Connect",         MongoDB_Connect},
    {"MongoDB_Close",           MongoDB_Close},
    {"MongoDB_IsConnected",     MongoDB_IsConnected},
    {"MongoDB_GetCollection",   MongoDB_GetCollection},
    {"MongoDB_InsertOne",       MongoDB_InsertOne},
    {"MongoDB_FindOne",         MongoDB_FindOne},
    {"MongoDB_UpdateOne",       MongoDB_UpdateOne},
    {"MongoDB_DeleteOne",       MongoDB_DeleteOne},
    {"MongoDB_CountDocuments",  MongoDB_CountDocuments},
    {"MongoDB_GetLastError",    MongoDB_GetLastError},
    {NULL,                      NULL}
};

// MongoDB_UpdateMany
cell_t MongoDB_UpdateMany(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = static_cast<Handle_t>(params[1]);
    Handle_t filter = static_cast<Handle_t>(params[2]);
    Handle_t update = static_cast<Handle_t>(params[3]);

    // Get StringMaps from handles
    IStringMap* filterMap = nullptr;
    IStringMap* updateMap = nullptr;

    HandleError err1 = handlesys->ReadHandle(filter, 0, nullptr, (void**)&filterMap);
    HandleError err2 = handlesys->ReadHandle(update, 0, nullptr, (void**)&updateMap);

    if (err1 != HandleError_None || err2 != HandleError_None) {
        return 0;
    }

    bool success = g_HTTPMongoDBExtension.GetMongoAPI()->UpdateMany(collection, filterMap, updateMap);
    return success ? 1 : 0;
}

// MongoDB_DeleteMany
cell_t MongoDB_DeleteMany(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = static_cast<Handle_t>(params[1]);
    Handle_t filter = static_cast<Handle_t>(params[2]);

    // Get StringMap from handle
    IStringMap* filterMap = nullptr;
    HandleError err = handlesys->ReadHandle(filter, 0, nullptr, (void**)&filterMap);
    if (err != HandleError_None) {
        return 0;
    }

    bool success = g_HTTPMongoDBExtension.GetMongoAPI()->DeleteMany(collection, filterMap);
    return success ? 1 : 0;
}

// MongoDB_Find
cell_t MongoDB_Find(IPluginContext *pContext, const cell_t *params) {
    Handle_t collection = static_cast<Handle_t>(params[1]);
    Handle_t filter = (params[0] >= 2) ? static_cast<Handle_t>(params[2]) : BAD_HANDLE;
    Handle_t options = (params[0] >= 3) ? static_cast<Handle_t>(params[3]) : BAD_HANDLE;

    IStringMap* filterMap = nullptr;
    IStringMap* optionsMap = nullptr;

    if (filter != BAD_HANDLE) {
        HandleError err = handlesys->ReadHandle(filter, 0, nullptr, (void**)&filterMap);
        if (err != HandleError_None) {
            return static_cast<cell_t>(BAD_HANDLE);
        }
    }

    if (options != BAD_HANDLE) {
        HandleError err = handlesys->ReadHandle(options, 0, nullptr, (void**)&optionsMap);
        if (err != HandleError_None) {
            return static_cast<cell_t>(BAD_HANDLE);
        }
    }

    IArrayList* result = g_HTTPMongoDBExtension.GetMongoAPI()->Find(collection, filterMap, optionsMap);

    if (result) {
        // Create handle for the result ArrayList
        Handle_t resultHandle = handlesys->CreateHandle(0, result, nullptr, nullptr, nullptr);
        return static_cast<cell_t>(resultHandle);
    }

    return static_cast<cell_t>(BAD_HANDLE);
}

// JSON_ArrayListToString
cell_t JSON_ArrayListToString(IPluginContext *pContext, const cell_t *params) {
    Handle_t arrayHandle = static_cast<Handle_t>(params[1]);

    // Get ArrayList from handle
    IArrayList* array = nullptr;
    HandleError err = handlesys->ReadHandle(arrayHandle, 0, nullptr, (void**)&array);
    if (err != HandleError_None) {
        return 0;
    }

    std::string jsonStr;
    bool success = g_HTTPMongoDBExtension.GetJSONManager()->ArrayListToJSON(array, jsonStr);

    if (success && params[0] >= 3) {
        pContext->StringToLocal(params[2], params[3], jsonStr.c_str());
        return 1;
    }

    return 0;
}

// JSON_ArrayFromString
cell_t JSON_ArrayFromString(IPluginContext *pContext, const cell_t *params) {
    Handle_t arrayHandle = static_cast<Handle_t>(params[1]);
    char *jsonStr;
    pContext->LocalToString(params[2], &jsonStr);

    // Get ArrayList from handle
    IArrayList* array = nullptr;
    HandleError err = handlesys->ReadHandle(arrayHandle, 0, nullptr, (void**)&array);
    if (err != HandleError_None) {
        return 0;
    }

    bool success = g_HTTPMongoDBExtension.GetJSONManager()->JSONToArrayList(jsonStr, array);
    return success ? 1 : 0;
}

const sp_nativeinfo_t g_MongoDBNatives[] = {
    {"MongoDB_Connect",         MongoDB_Connect},
    {"MongoDB_Close",           MongoDB_Close},
    {"MongoDB_IsConnected",     MongoDB_IsConnected},
    {"MongoDB_GetCollection",   MongoDB_GetCollection},
    {"MongoDB_InsertOne",       MongoDB_InsertOne},
    {"MongoDB_FindOne",         MongoDB_FindOne},
    {"MongoDB_Find",            MongoDB_Find},
    {"MongoDB_UpdateOne",       MongoDB_UpdateOne},
    {"MongoDB_UpdateMany",      MongoDB_UpdateMany},
    {"MongoDB_DeleteOne",       MongoDB_DeleteOne},
    {"MongoDB_DeleteMany",      MongoDB_DeleteMany},
    {"MongoDB_CountDocuments",  MongoDB_CountDocuments},
    {"MongoDB_GetLastError",    MongoDB_GetLastError},
    {NULL,                      NULL}
};

const sp_nativeinfo_t g_JSONNatives[] = {
    {"JSON_StringMapToString",  JSON_StringMapToString},
    {"JSON_StringFromString",   JSON_StringFromString},
    {"JSON_ArrayListToString",  JSON_ArrayListToString},
    {"JSON_ArrayFromString",    JSON_ArrayFromString},
    {NULL,                      NULL}
};
