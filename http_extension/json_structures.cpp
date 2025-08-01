/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod HTTP MongoDB Extension - JSON Structure Manager Implementation
 * Copyright (C) 2024 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 */

#include "json_structures.h"
#include <regex>
#include <sstream>
#include <iomanip>

// Static constants
const char* JSONStructureManager::MONGO_TYPE_SUFFIX = "_type";
const char* JSONStructureManager::OBJECT_ID_TYPE = "ObjectId";
const char* JSONStructureManager::DATE_TYPE = "Date";
const char* JSONStructureManager::DOCUMENT_TYPE = "Document";
const char* JSONStructureManager::ARRAY_TYPE = "Array";

JSONStructureManager::JSONStructureManager() {
}

JSONStructureManager::~JSONStructureManager() {
}

bool JSONStructureManager::StringMapToJSON(IStringMap* map, std::string& jsonStr) {
    if (!map) {
        m_lastError = "StringMap is null";
        return false;
    }
    
    try {
        json jsonObj = json::object();
        
        // Iterate through StringMap
        IStringMapIterator* iter = map->GetIterator();
        if (!iter) {
            m_lastError = "Failed to get StringMap iterator";
            return false;
        }
        
        char key[256];
        while (iter->GetNext(key, sizeof(key))) {
            if (!ProcessStringMapEntry(map, key, jsonObj)) {
                iter->Destroy();
                return false;
            }
        }
        
        iter->Destroy();
        
        // Convert to string
        jsonStr = jsonObj.dump();
        return true;
        
    } catch (const json::exception& e) {
        m_lastError = "JSON conversion error: ";
        m_lastError += e.what();
        return false;
    }
}

bool JSONStructureManager::JSONToStringMap(const std::string& jsonStr, IStringMap* map) {
    if (!map) {
        m_lastError = "StringMap is null";
        return false;
    }
    
    if (!IsValidJSON(jsonStr)) {
        m_lastError = "Invalid JSON string";
        return false;
    }
    
    try {
        json jsonObj = json::parse(jsonStr);
        
        if (!jsonObj.is_object()) {
            m_lastError = "JSON must be an object for StringMap conversion";
            return false;
        }
        
        // Clear existing map
        map->Clear();
        
        // Process each JSON entry
        for (auto& [key, value] : jsonObj.items()) {
            if (!ProcessJSONEntry(key, value, map)) {
                return false;
            }
        }
        
        return true;
        
    } catch (const json::exception& e) {
        m_lastError = "JSON parsing error: ";
        m_lastError += e.what();
        return false;
    }
}

bool JSONStructureManager::ArrayListToJSON(IArrayList* array, std::string& jsonStr) {
    if (!array) {
        m_lastError = "ArrayList is null";
        return false;
    }
    
    try {
        json jsonArray = json::array();
        
        size_t length = array->Length();
        for (size_t i = 0; i < length; i++) {
            if (!ProcessArrayListEntry(array, i, jsonArray)) {
                return false;
            }
        }
        
        jsonStr = jsonArray.dump();
        return true;
        
    } catch (const json::exception& e) {
        m_lastError = "JSON array conversion error: ";
        m_lastError += e.what();
        return false;
    }
}

bool JSONStructureManager::JSONToArrayList(const std::string& jsonStr, IArrayList* array) {
    if (!array) {
        m_lastError = "ArrayList is null";
        return false;
    }
    
    if (!IsValidJSON(jsonStr)) {
        m_lastError = "Invalid JSON string";
        return false;
    }
    
    try {
        json jsonArray = json::parse(jsonStr);
        
        if (!jsonArray.is_array()) {
            m_lastError = "JSON must be an array for ArrayList conversion";
            return false;
        }
        
        // Clear existing array
        array->Clear();
        
        // Process each JSON array element
        for (const auto& value : jsonArray) {
            if (!ProcessJSONArrayEntry(value, array)) {
                return false;
            }
        }
        
        return true;
        
    } catch (const json::exception& e) {
        m_lastError = "JSON array parsing error: ";
        m_lastError += e.what();
        return false;
    }
}

bool JSONStructureManager::ProcessStringMapEntry(IStringMap* map, const std::string& key, json& jsonObj) {
    // Skip type metadata keys
    if (key.find(MONGO_TYPE_SUFFIX) != std::string::npos) {
        return true;
    }
    
    // Check if this key has a MongoDB type
    std::string mongoType;
    if (HasMongoType(map, key, mongoType)) {
        // Handle MongoDB-specific types
        if (mongoType == OBJECT_ID_TYPE) {
            char value[256];
            if (map->GetString(key.c_str(), value, sizeof(value))) {
                jsonObj[key] = value; // ObjectId as string in JSON
            }
        } else if (mongoType == DATE_TYPE) {
            int timestamp;
            if (map->GetValue(key.c_str(), &timestamp)) {
                jsonObj[key] = timestamp; // Date as timestamp
            }
        } else if (mongoType == DOCUMENT_TYPE) {
            char nestedJson[4096];
            if (map->GetString(key.c_str(), nestedJson, sizeof(nestedJson))) {
                try {
                    jsonObj[key] = json::parse(nestedJson);
                } catch (const json::exception&) {
                    jsonObj[key] = nestedJson; // Fallback to string
                }
            }
        } else if (mongoType == ARRAY_TYPE) {
            char arrayJson[4096];
            if (map->GetString(key.c_str(), arrayJson, sizeof(arrayJson))) {
                try {
                    jsonObj[key] = json::parse(arrayJson);
                } catch (const json::exception&) {
                    jsonObj[key] = arrayJson; // Fallback to string
                }
            }
        }
    } else {
        // Handle regular types
        char stringValue[4096];
        if (map->GetString(key.c_str(), stringValue, sizeof(stringValue))) {
            // Try to detect the actual type
            json::value_t detectedType = DetectJSONType(stringValue);
            
            switch (detectedType) {
                case json::value_t::boolean:
                    jsonObj[key] = (strcmp(stringValue, "true") == 0);
                    break;
                case json::value_t::number_integer:
                    jsonObj[key] = std::stoll(stringValue);
                    break;
                case json::value_t::number_float:
                    jsonObj[key] = std::stod(stringValue);
                    break;
                default:
                    jsonObj[key] = stringValue;
                    break;
            }
        } else {
            // Try as integer
            int intValue;
            if (map->GetValue(key.c_str(), &intValue)) {
                jsonObj[key] = intValue;
            }
        }
    }
    
    return true;
}

bool JSONStructureManager::ProcessJSONEntry(const std::string& key, const json& value, IStringMap* map) {
    try {
        if (value.is_string()) {
            map->SetString(key.c_str(), value.get<std::string>().c_str());
        } else if (value.is_boolean()) {
            map->SetString(key.c_str(), value.get<bool>() ? "true" : "false");
        } else if (value.is_number_integer()) {
            map->SetValue(key.c_str(), value.get<int>());
        } else if (value.is_number_float()) {
            std::string floatStr = std::to_string(value.get<double>());
            map->SetString(key.c_str(), floatStr.c_str());
        } else if (value.is_object()) {
            // Store nested object as JSON string
            std::string nestedJson = value.dump();
            map->SetString(key.c_str(), nestedJson.c_str());
            SetMongoType(map, key, DOCUMENT_TYPE);
        } else if (value.is_array()) {
            // Store array as JSON string
            std::string arrayJson = value.dump();
            map->SetString(key.c_str(), arrayJson.c_str());
            SetMongoType(map, key, ARRAY_TYPE);
        } else if (value.is_null()) {
            map->SetString(key.c_str(), "");
        }
        
        return true;
        
    } catch (const json::exception& e) {
        m_lastError = "Error processing JSON entry: ";
        m_lastError += e.what();
        return false;
    }
}

bool JSONStructureManager::ProcessArrayListEntry(IArrayList* array, int index, json& jsonArray) {
    // For simplicity, we'll assume ArrayList contains strings
    // In a full implementation, we'd need to handle different types
    char value[4096];
    if (array->GetString(index, value, sizeof(value))) {
        // Try to detect type
        json::value_t detectedType = DetectJSONType(value);
        
        switch (detectedType) {
            case json::value_t::boolean:
                jsonArray.push_back(strcmp(value, "true") == 0);
                break;
            case json::value_t::number_integer:
                jsonArray.push_back(std::stoll(value));
                break;
            case json::value_t::number_float:
                jsonArray.push_back(std::stod(value));
                break;
            default:
                jsonArray.push_back(value);
                break;
        }
    }
    
    return true;
}

bool JSONStructureManager::ProcessJSONArrayEntry(const json& value, IArrayList* array) {
    try {
        if (value.is_string()) {
            array->PushString(value.get<std::string>().c_str());
        } else if (value.is_boolean()) {
            array->PushString(value.get<bool>() ? "true" : "false");
        } else if (value.is_number()) {
            std::string numStr = std::to_string(value.get<double>());
            array->PushString(numStr.c_str());
        } else {
            // Convert complex types to JSON string
            std::string jsonStr = value.dump();
            array->PushString(jsonStr.c_str());
        }
        
        return true;
        
    } catch (const json::exception& e) {
        m_lastError = "Error processing JSON array entry: ";
        m_lastError += e.what();
        return false;
    }
}

json::value_t JSONStructureManager::DetectJSONType(const std::string& value) {
    if (value.empty()) {
        return json::value_t::string;
    }
    
    if (IsBoolean(value)) {
        return json::value_t::boolean;
    }
    
    if (IsNumeric(value)) {
        if (value.find('.') != std::string::npos) {
            return json::value_t::number_float;
        } else {
            return json::value_t::number_integer;
        }
    }
    
    return json::value_t::string;
}

bool JSONStructureManager::IsBoolean(const std::string& value) {
    return value == "true" || value == "false";
}

bool JSONStructureManager::IsNumeric(const std::string& value) {
    if (value.empty()) return false;
    
    size_t start = 0;
    if (value[0] == '-' || value[0] == '+') {
        start = 1;
        if (value.length() == 1) return false;
    }
    
    bool hasDecimal = false;
    for (size_t i = start; i < value.length(); i++) {
        if (value[i] == '.') {
            if (hasDecimal) return false; // Multiple decimals
            hasDecimal = true;
        } else if (!std::isdigit(value[i])) {
            return false;
        }
    }
    
    return true;
}

bool JSONStructureManager::HasMongoType(IStringMap* map, const std::string& key, std::string& mongoType) {
    std::string typeKey = key + MONGO_TYPE_SUFFIX;
    char typeValue[64];
    
    if (map->GetString(typeKey.c_str(), typeValue, sizeof(typeValue))) {
        mongoType = typeValue;
        return true;
    }
    
    return false;
}

void JSONStructureManager::SetMongoType(IStringMap* map, const std::string& key, const std::string& mongoType) {
    std::string typeKey = key + MONGO_TYPE_SUFFIX;
    map->SetString(typeKey.c_str(), mongoType.c_str());
}

bool JSONStructureManager::IsValidJSON(const std::string& jsonStr) {
    try {
        json::parse(jsonStr);
        return true;
    } catch (const json::exception&) {
        return false;
    }
}
