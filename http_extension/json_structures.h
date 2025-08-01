/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod HTTP MongoDB Extension - JSON Structure Manager
 * Copyright (C) 2024 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 */

#ifndef _JSON_STRUCTURES_H_
#define _JSON_STRUCTURES_H_

#include <IADTFactory.h>
#include <ICellArray.h>
#include <string>
#include <memory>
#include "third_party/json.hpp"

using json = nlohmann::json;
using namespace SourceMod;

/**
 * @brief JSON Structure Manager for SourceMod modern structures
 * 
 * Handles conversion between SourceMod's StringMap/ArrayList and JSON,
 * with support for MongoDB-specific data types like ObjectId and Date.
 */
class JSONStructureManager {
public:
    JSONStructureManager();
    ~JSONStructureManager();
    
    // StringMap <-> JSON conversion (using IBasicTrie)
    bool StringMapToJSON(IBasicTrie* map, std::string& jsonStr);
    bool JSONToStringMap(const std::string& jsonStr, IBasicTrie* map);

    // ArrayList <-> JSON array conversion (using ICellArray)
    bool ArrayListToJSON(ICellArray* array, std::string& jsonStr);
    bool JSONToArrayList(const std::string& jsonStr, ICellArray* array);

    // Type-aware conversion (handles MongoDB types)
    bool SetTypedValue(IBasicTrie* map, const std::string& key, const json& value);
    json GetTypedValue(IBasicTrie* map, const std::string& key);
    
    // MongoDB-specific conversions
    bool ConvertObjectId(const std::string& objectIdStr, json& value);
    bool ConvertDate(int timestamp, json& value);
    bool ParseObjectId(const json& value, std::string& objectIdStr);
    bool ParseDate(const json& value, int& timestamp);
    
    // Utility functions
    bool IsValidJSON(const std::string& jsonStr);
    std::string EscapeString(const std::string& str);
    std::string UnescapeString(const std::string& str);
    
    // Error handling
    const std::string& GetLastError() const { return m_lastError; }
    void ClearLastError() { m_lastError.clear(); }

private:
    std::string m_lastError;
    
    // Helper methods for type detection and conversion
    json::value_t DetectJSONType(const std::string& value);
    bool IsObjectId(const std::string& value);
    bool IsNumeric(const std::string& value);
    bool IsBoolean(const std::string& value);
    
    // StringMap iteration helpers
    bool ProcessStringMapEntry(IBasicTrie* map, const std::string& key, json& jsonObj);
    bool ProcessJSONEntry(const std::string& key, const json& value, IBasicTrie* map);

    // ArrayList conversion helpers
    bool ProcessArrayListEntry(ICellArray* array, int index, json& jsonArray);
    bool ProcessJSONArrayEntry(const json& value, ICellArray* array);

    // Type conversion helpers
    bool ConvertStringMapValue(IBasicTrie* map, const std::string& key, json& value);
    bool ConvertJSONValue(const json& value, IBasicTrie* map, const std::string& key);

    // MongoDB type detection
    bool HasMongoType(IBasicTrie* map, const std::string& key, std::string& mongoType);
    void SetMongoType(IBasicTrie* map, const std::string& key, const std::string& mongoType);
    
    // Constants for MongoDB types
    static const std::string MONGO_TYPE_SUFFIX;
    static const std::string OBJECT_ID_TYPE;
    static const std::string DATE_TYPE;
    static const std::string DOCUMENT_TYPE;
    static const std::string ARRAY_TYPE;
};

    // Constants for MongoDB types
    static const char* MONGO_TYPE_SUFFIX;
    static const char* OBJECT_ID_TYPE;
    static const char* DATE_TYPE;
    static const char* DOCUMENT_TYPE;
    static const char* ARRAY_TYPE;

#endif // _JSON_STRUCTURES_H_
