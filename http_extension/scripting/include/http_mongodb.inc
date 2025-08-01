/**
 * HTTP MongoDB Extension Include File
 * Provides MongoDB operations via HTTP API
 */

#if defined _http_mongodb_included
 #endinput
#endif
#define _http_mongodb_included

// Configuration management
native bool MongoDB_LoadConfig(const char[] configPath);
native bool MongoDB_SetAPIURL(const char[] url);
native bool MongoDB_GetAPIURL(char[] buffer, int maxlen);
native bool MongoDB_SetTimeout(int timeout);
native int MongoDB_GetTimeout();

// Connection management
native Handle MongoDB_Connect(const char[] url = "");  // Empty string uses config URL
native Handle MongoDB_GetCollection(Handle connection, const char[] database, const char[] collection);
native bool MongoDB_IsConnected(Handle connection);
native void MongoDB_Close(Handle connection);

// Document operations
native bool MongoDB_InsertOne(Handle collection, StringMap document, char[] insertedId, int maxlen);
native bool MongoDB_InsertOneJSON(Handle collection, const char[] jsonDocument, char[] insertedId, int maxlen);
native bool MongoDB_InsertMany(Handle collection, ArrayList documents, ArrayList insertedIds);
native StringMap MongoDB_FindOne(Handle collection, StringMap filter);
native StringMap MongoDB_FindOneJSON(Handle collection, const char[] jsonFilter);
native ArrayList MongoDB_Find(Handle collection, StringMap filter, StringMap options);
native bool MongoDB_UpdateOne(Handle collection, StringMap filter, StringMap update);
native bool MongoDB_UpdateMany(Handle collection, StringMap filter, StringMap update);
native bool MongoDB_DeleteOne(Handle collection, StringMap filter);
native bool MongoDB_DeleteMany(Handle collection, StringMap filter);
native int MongoDB_CountDocuments(Handle collection, StringMap filter);

// Index operations
native bool MongoDB_CreateIndex(Handle collection, StringMap keys, StringMap options);
native bool MongoDB_DropIndex(Handle collection, const char[] indexName);

// Error handling
native bool MongoDB_GetLastError(char[] buffer, int maxlen);

// JSON utility functions (basic implementations)
native bool JSON_StringMapToString(StringMap map, char[] buffer, int maxlen);
native bool JSON_StringFromString(StringMap map, const char[] jsonStr);
native bool JSON_ArrayListToString(ArrayList array, char[] buffer, int maxlen);
native bool JSON_ArrayFromString(ArrayList array, const char[] jsonStr);

/**
 * MongoDB Connection - Handle wrapper for connection management
 */
methodmap MongoConnection < Handle {
    // Connect using explicit URL
    public MongoConnection(const char[] url) {
        return view_as<MongoConnection>(MongoDB_Connect(url));
    }

    // Connect using configured URL (from config file)
    public static MongoConnection FromConfig() {
        return view_as<MongoConnection>(MongoDB_Connect(""));
    }
    
    public MongoCollection GetCollection(const char[] database, const char[] collection) {
        return view_as<MongoCollection>(MongoDB_GetCollection(this, database, collection));
    }
    
    public bool IsConnected() {
        return MongoDB_IsConnected(this);
    }
    
    public void Close() {
        MongoDB_Close(this);
    }
}

/**
 * MongoDB Collection - Handle wrapper for collection operations
 */
methodmap MongoCollection < Handle {
    // Insert a single document
    public bool InsertOne(StringMap document, char[] insertedId, int maxlen) {
        return MongoDB_InsertOne(this, document, insertedId, maxlen);
    }

    // Insert a single document with JSON string
    public bool InsertOneJSON(const char[] jsonDocument, char[] insertedId, int maxlen) {
        return MongoDB_InsertOneJSON(this, jsonDocument, insertedId, maxlen);
    }

    // Insert multiple documents
    public bool InsertMany(ArrayList documents, ArrayList insertedIds) {
        return MongoDB_InsertMany(this, documents, insertedIds);
    }
    
    // Find a single document
    public StringMap FindOne(StringMap filter = null) {
        return view_as<StringMap>(MongoDB_FindOne(this, filter));
    }

    // Find a single document with JSON filter
    public StringMap FindOneJSON(const char[] jsonFilter) {
        return view_as<StringMap>(MongoDB_FindOneJSON(this, jsonFilter));
    }

    // Find multiple documents
    public ArrayList Find(StringMap filter = null, StringMap options = null) {
        return view_as<ArrayList>(MongoDB_Find(this, filter, options));
    }
    
    // Update a single document
    public bool UpdateOne(StringMap filter, StringMap update) {
        return MongoDB_UpdateOne(this, filter, update);
    }
    
    // Update multiple documents
    public bool UpdateMany(StringMap filter, StringMap update) {
        return MongoDB_UpdateMany(this, filter, update);
    }
    
    // Delete a single document
    public bool DeleteOne(StringMap filter) {
        return MongoDB_DeleteOne(this, filter);
    }
    
    // Delete multiple documents
    public bool DeleteMany(StringMap filter) {
        return MongoDB_DeleteMany(this, filter);
    }
    
    // Count documents
    public int CountDocuments(StringMap filter = null) {
        return MongoDB_CountDocuments(this, filter);
    }
    
    // Create an index
    public bool CreateIndex(StringMap keys, StringMap options = null) {
        return MongoDB_CreateIndex(this, keys, options);
    }
    
    // Drop an index
    public bool DropIndex(const char[] indexName) {
        return MongoDB_DropIndex(this, indexName);
    }
}

/**
 * MongoDB Document - Enhanced StringMap with MongoDB-specific functionality
 */
methodmap MongoDocument < StringMap {
    public MongoDocument() {
        return view_as<MongoDocument>(new StringMap());
    }
    
    // Set ObjectId
    public void SetObjectId(const char[] key, const char[] objectId) {
        this.SetString(key, objectId);
    }
    
    // Get ObjectId
    public bool GetObjectId(const char[] key, char[] buffer, int maxlen) {
        return this.GetString(key, buffer, maxlen);
    }
    
    // Set Date (timestamp)
    public void SetDate(const char[] key, int timestamp) {
        this.SetValue(key, timestamp);
    }
    
    // Get Date
    public int GetDate(const char[] key) {
        int value;
        this.GetValue(key, value);
        return value;
    }
    
    // Set nested document (stores as JSON string)
    public void SetDocument(const char[] key, MongoDocument document) {
        char jsonStr[4096];
        document.ToString(jsonStr, sizeof(jsonStr));
        this.SetString(key, jsonStr);
    }
    
    // Get nested document (parses from JSON string)
    public MongoDocument GetDocument(const char[] key) {
        char jsonStr[4096];
        if (this.GetString(key, jsonStr, sizeof(jsonStr))) {
            return MongoDocument.FromString(jsonStr);
        }
        return null;
    }
    
    // Set array (stores as JSON string)
    public void SetArray(const char[] key, MongoDocumentArray array) {
        char jsonStr[4096];
        array.ToString(jsonStr, sizeof(jsonStr));
        this.SetString(key, jsonStr);
    }
    
    // Convert to JSON string
    public bool ToString(char[] buffer, int maxlen) {
        return JSON_StringMapToString(this, buffer, maxlen);
    }
    
    // Create from JSON string
    public static MongoDocument FromString(const char[] jsonStr) {
        MongoDocument doc = new MongoDocument();
        JSON_StringFromString(doc, jsonStr);
        return doc;
    }
}

/**
 * MongoDB Document Array - Enhanced ArrayList for MongoDB documents
 */
methodmap MongoDocumentArray < ArrayList {
    public MongoDocumentArray() {
        return view_as<MongoDocumentArray>(new ArrayList());
    }
    
    // Push a document (as JSON string)
    public void PushDocument(MongoDocument document) {
        char jsonStr[4096];
        document.ToString(jsonStr, sizeof(jsonStr));
        this.PushString(jsonStr);
    }
    
    // Get a document by index
    public MongoDocument GetDocument(int index) {
        char jsonStr[4096];
        this.GetString(index, jsonStr, sizeof(jsonStr));
        return MongoDocument.FromString(jsonStr);
    }
    
    // Set a document at index
    public void SetDocument(int index, MongoDocument document) {
        char jsonStr[4096];
        document.ToString(jsonStr, sizeof(jsonStr));
        this.SetString(index, jsonStr);
    }
    
    // Insert document at specific position (simplified)
    public void InsertDocument(int index, MongoDocument document) {
        char jsonStr[4096];
        document.ToString(jsonStr, sizeof(jsonStr));
        this.PushString(jsonStr);
    }
    
    // Remove document at index
    public void RemoveDocument(int index) {
        this.Erase(index);
    }
    
    // Find document by field value
    public int FindDocument(const char[] field, const char[] value) {
        for (int i = 0; i < this.Length; i++) {
            MongoDocument doc = this.GetDocument(i);
            char docValue[256];
            if (doc.GetString(field, docValue, sizeof(docValue))) {
                if (StrEqual(docValue, value)) {
                    delete doc;
                    return i;
                }
            }
            delete doc;
        }
        return -1;
    }
    
    // Convert to JSON string
    public void ToString(char[] buffer, int maxlen) {
        JSON_ArrayListToString(this, buffer, maxlen);
    }
    
    // Create from JSON string
    public static MongoDocumentArray FromString(const char[] jsonStr) {
        MongoDocumentArray arr = new MongoDocumentArray();
        JSON_ArrayFromString(arr, jsonStr);
        return arr;
    }
}

/**
 * MongoDB Result Set - Simple result container
 */
methodmap MongoResultSet < ArrayList {
    public MongoResultSet() {
        return view_as<MongoResultSet>(new ArrayList());
    }
}

/**
 * MongoDB Find Options - Simple options container
 */
methodmap MongoFindOptions < StringMap {
    public MongoFindOptions() {
        return view_as<MongoFindOptions>(new StringMap());
    }

    public void SetLimit(int limit) {
        this.SetValue("limit", limit);
    }

    public void SetSkip(int skip) {
        this.SetValue("skip", skip);
    }
    
    // Simplified sort methods
    public void SortAscending(const char[] field) {
        char sortStr[256];
        Format(sortStr, sizeof(sortStr), "{\"%s\":1}", field);
        this.SetString("sort", sortStr);
    }

    public void SortDescending(const char[] field) {
        char sortStr[256];
        Format(sortStr, sizeof(sortStr), "{\"%s\":-1}", field);
        this.SetString("sort", sortStr);
    }
}

/**
 * MongoDB Query Builder - Enhanced query construction
 */
methodmap MongoQuery < MongoDocument {
    public MongoQuery() {
        return view_as<MongoQuery>(new MongoDocument());
    }

    // Basic comparison methods
    public MongoQuery Where(const char[] field, const char[] op, any value) {
        if (StrEqual(op, "=") || StrEqual(op, "==")) {
            this.SetValue(field, value);
        } else {
            char opKey[32];
            Format(opKey, sizeof(opKey), "$%s", op);
            char fieldKey[128];
            Format(fieldKey, sizeof(fieldKey), "%s_%s", field, opKey);
            this.SetValue(fieldKey, value);
        }
        return this;
    }

    // Convenience methods for common operations
    public MongoQuery Equals(const char[] field, any value) {
        this.SetValue(field, value);
        return this;
    }

    public MongoQuery NotEquals(const char[] field, any value) {
        return this.Where(field, "ne", value);
    }

    public MongoQuery GreaterThan(const char[] field, any value) {
        return this.Where(field, "gt", value);
    }

    public MongoQuery GreaterThanOrEqual(const char[] field, any value) {
        return this.Where(field, "gte", value);
    }

    public MongoQuery LessThan(const char[] field, any value) {
        return this.Where(field, "lt", value);
    }

    public MongoQuery LessThanOrEqual(const char[] field, any value) {
        return this.Where(field, "lte", value);
    }

    // Array operations (simplified)
    public MongoQuery In(const char[] field, MongoDocumentArray values) {
        char fieldName[128];
        Format(fieldName, sizeof(fieldName), "%s_$in", field);
        char jsonStr[4096];
        values.ToString(jsonStr, sizeof(jsonStr));
        this.SetString(fieldName, jsonStr);
        return this;
    }

    public MongoQuery NotIn(const char[] field, MongoDocumentArray values) {
        char fieldName[128];
        Format(fieldName, sizeof(fieldName), "%s_$nin", field);
        char jsonStr[4096];
        values.ToString(jsonStr, sizeof(jsonStr));
        this.SetString(fieldName, jsonStr);
        return this;
    }

    // Text search
    public MongoQuery TextSearch(const char[] searchText) {
        char textQuery[512];
        Format(textQuery, sizeof(textQuery), "{\"$search\":\"%s\"}", searchText);
        this.SetString("$text", textQuery);
        return this;
    }

    // Logical operations (basic implementations)
    public MongoQuery And(MongoQuery other) {
        // Basic implementation - in a full version this would merge queries
        #pragma unused other
        return this;
    }

    public MongoQuery Or(MongoQuery other) {
        // Basic implementation - in a full version this would merge queries
        #pragma unused other
        return this;
    }

    public MongoQuery Not(MongoQuery condition) {
        // Basic implementation - in a full version this would wrap with $not
        #pragma unused condition
        return this;
    }
}

/**
 * Simple callback types
 */
typedef MongoCallback = function void (bool success, any data);
