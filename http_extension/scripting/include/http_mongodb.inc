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

// Advanced operations
native ArrayList MongoDB_Aggregate(Handle collection, ArrayList pipeline);
native ArrayList MongoDB_FindWithProjection(Handle collection, StringMap filter, StringMap projection, StringMap options);
native bool MongoDB_BulkWrite(Handle collection, ArrayList operations, bool ordered);
native ArrayList MongoDB_FindDistinct(Handle collection, const char[] field, StringMap filter);

// Enhanced error handling
native int MongoDB_GetLastErrorCode();
native bool MongoDB_GetLastErrorMessage(char[] buffer, int maxlen);
native bool MongoDB_GetLastErrorDetails(char[] buffer, int maxlen);
native int MongoDB_GetLastErrorTimestamp();

// Performance monitoring
native int MongoDB_GetTotalOperations();
native int MongoDB_GetSuccessfulOperations();
native int MongoDB_GetFailedOperations();
native int MongoDB_GetAverageExecutionTime();  // Returns centiseconds (1/100th of a second)
native int MongoDB_GetSuccessRate();           // Returns percentage (0-100)
native bool MongoDB_ResetPerformanceMetrics();

// Connection management
native bool MongoDB_TestConnection(Handle connection);

// Legacy error handling (deprecated - use enhanced error functions)
native bool MongoDB_GetLastError(char[] buffer, int maxlen);

// JSON utility functions (enhanced implementations)
native bool JSON_StringMapToString(StringMap map, char[] buffer, int maxlen);
native bool JSON_StringFromString(StringMap map, const char[] jsonStr);
native bool JSON_ArrayListToString(ArrayList array, char[] buffer, int maxlen);
native bool JSON_ArrayFromString(ArrayList array, const char[] jsonStr);

// StringMap helper functions for enhanced integration
native bool StringMap_SetString(Handle map, const char[] key, const char[] value);
native bool StringMap_GetString(Handle map, const char[] key, char[] buffer, int maxlen);
native Handle StringMap_CreateEmpty();

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

    // Advanced operations

    // Run aggregation pipeline
    public ArrayList Aggregate(ArrayList pipeline) {
        return view_as<ArrayList>(MongoDB_Aggregate(this, pipeline));
    }

    // Find with field projection
    public ArrayList FindWithProjection(StringMap filter = null, StringMap projection = null, StringMap options = null) {
        return view_as<ArrayList>(MongoDB_FindWithProjection(this, filter, projection, options));
    }

    // Bulk write operations
    public bool BulkWrite(ArrayList operations, bool ordered = true) {
        return MongoDB_BulkWrite(this, operations, ordered);
    }

    // Find distinct values
    public ArrayList FindDistinct(const char[] field, StringMap filter = null) {
        return view_as<ArrayList>(MongoDB_FindDistinct(this, field, filter));
    }
}

/**
 * MongoDB Document - Enhanced StringMap with MongoDB-specific functionality
 */
methodmap MongoDocument < StringMap {
    public MongoDocument() {
        return view_as<MongoDocument>(StringMap_CreateEmpty());
    }

    // Create from existing StringMap handle (useful for FindOne results)
    public static MongoDocument FromHandle(Handle handle) {
        return view_as<MongoDocument>(handle);
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

    // Enhanced string operations using helper natives
    public bool SetStringValue(const char[] key, const char[] value) {
        return StringMap_SetString(this, key, value);
    }

    public bool GetStringValue(const char[] key, char[] buffer, int maxlen) {
        return StringMap_GetString(this, key, buffer, maxlen);
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

    // Basic comparison methods (enhanced)
    public MongoQuery Where(const char[] field, const char[] op, any value) {
        if (StrEqual(op, "=") || StrEqual(op, "==") || StrEqual(op, "eq")) {
            this.SetValue(field, value);
        } else {
            // Create proper MongoDB operator structure: {"field": {"$op": value}}
            char fieldValue[256];
            char opKey[32];
            Format(opKey, sizeof(opKey), "$%s", op);
            Format(fieldValue, sizeof(fieldValue), "{\"%s\":%d}", opKey, value);
            this.SetString(field, fieldValue);
        }
        return this;
    }

    // String-based where clause for complex values
    public MongoQuery WhereString(const char[] field, const char[] op, const char[] value) {
        if (StrEqual(op, "=") || StrEqual(op, "==") || StrEqual(op, "eq")) {
            this.SetString(field, value);
        } else {
            // Create proper MongoDB operator structure: {"field": {"$op": "value"}}
            char fieldValue[512];
            char opKey[32];
            Format(opKey, sizeof(opKey), "$%s", op);
            Format(fieldValue, sizeof(fieldValue), "{\"%s\":\"%s\"}", opKey, value);
            this.SetString(field, fieldValue);
        }
        return this;
    }

    // Convenience methods for common operations (enhanced)
    public MongoQuery Equals(const char[] field, any value) {
        this.SetValue(field, value);
        return this;
    }

    public MongoQuery EqualsString(const char[] field, const char[] value) {
        this.SetString(field, value);
        return this;
    }

    public MongoQuery NotEquals(const char[] field, any value) {
        return this.Where(field, "ne", value);
    }

    public MongoQuery NotEqualsString(const char[] field, const char[] value) {
        return this.WhereString(field, "ne", value);
    }

    public MongoQuery GreaterThan(const char[] field, any value) {
        return this.Where(field, "gt", value);
    }

    public MongoQuery GreaterThanString(const char[] field, const char[] value) {
        return this.WhereString(field, "gt", value);
    }

    public MongoQuery GreaterThanOrEqual(const char[] field, any value) {
        return this.Where(field, "gte", value);
    }

    public MongoQuery GreaterThanOrEqualString(const char[] field, const char[] value) {
        return this.WhereString(field, "gte", value);
    }

    public MongoQuery LessThan(const char[] field, any value) {
        return this.Where(field, "lt", value);
    }

    public MongoQuery LessThanString(const char[] field, const char[] value) {
        return this.WhereString(field, "lt", value);
    }

    public MongoQuery LessThanOrEqual(const char[] field, any value) {
        return this.Where(field, "lte", value);
    }

    public MongoQuery LessThanOrEqualString(const char[] field, const char[] value) {
        return this.WhereString(field, "lte", value);
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

    // Logical operations (full implementations)
    public MongoQuery And(MongoQuery other) {
        // Create $and array with both queries
        char thisJson[2048], otherJson[2048];
        this.ToString(thisJson, sizeof(thisJson));
        other.ToString(otherJson, sizeof(otherJson));

        // Create new query with $and operator
        MongoQuery andQuery = new MongoQuery();
        char andValue[4096];
        Format(andValue, sizeof(andValue), "[%s,%s]", thisJson, otherJson);
        andQuery.SetString("$and", andValue);

        return andQuery;
    }

    public MongoQuery Or(MongoQuery other) {
        // Create $or array with both queries
        char thisJson[2048], otherJson[2048];
        this.ToString(thisJson, sizeof(thisJson));
        other.ToString(otherJson, sizeof(otherJson));

        // Create new query with $or operator
        MongoQuery orQuery = new MongoQuery();
        char orValue[4096];
        Format(orValue, sizeof(orValue), "[%s,%s]", thisJson, otherJson);
        orQuery.SetString("$or", orValue);

        return orQuery;
    }

    public MongoQuery Not(MongoQuery condition) {
        // Wrap the condition with $not operator
        char conditionJson[2048];
        condition.ToString(conditionJson, sizeof(conditionJson));

        // Create new query with $not operator
        MongoQuery notQuery = new MongoQuery();
        notQuery.SetString("$not", conditionJson);

        return notQuery;
    }

    // Additional logical operations
    public MongoQuery Nor(MongoQuery other) {
        // Create $nor array (neither this nor other)
        char thisJson[2048], otherJson[2048];
        this.ToString(thisJson, sizeof(thisJson));
        other.ToString(otherJson, sizeof(otherJson));

        MongoQuery norQuery = new MongoQuery();
        char norValue[4096];
        Format(norValue, sizeof(norValue), "[%s,%s]", thisJson, otherJson);
        norQuery.SetString("$nor", norValue);

        return norQuery;
    }

    // Exists check
    public MongoQuery Exists(const char[] field, bool exists = true) {
        char fieldKey[128];
        Format(fieldKey, sizeof(fieldKey), "%s.$exists", field);
        this.SetValue(fieldKey, exists ? 1 : 0);
        return this;
    }

    // Type check
    public MongoQuery Type(const char[] field, int bsonType) {
        char fieldKey[128];
        Format(fieldKey, sizeof(fieldKey), "%s.$type", field);
        this.SetValue(fieldKey, bsonType);
        return this;
    }

    // Regular expression
    public MongoQuery Regex(const char[] field, const char[] pattern, const char[] options = "") {
        char fieldKey[128];
        Format(fieldKey, sizeof(fieldKey), "%s.$regex", field);
        this.SetString(fieldKey, pattern);

        if (strlen(options) > 0) {
            char optionsKey[128];
            Format(optionsKey, sizeof(optionsKey), "%s.$options", field);
            this.SetString(optionsKey, options);
        }
        return this;
    }
}

/**
 * MongoDB Aggregation Pipeline Builder
 */
methodmap MongoAggregation < ArrayList {
    public MongoAggregation() {
        return view_as<MongoAggregation>(new ArrayList(ByteCountToCells(512)));
    }

    // Add a $match stage
    public MongoAggregation Match(MongoQuery query) {
        char queryJson[1024];
        query.ToString(queryJson, sizeof(queryJson));

        char stageJson[1024];
        Format(stageJson, sizeof(stageJson), "{\"$match\":%s}", queryJson);
        this.PushString(stageJson);
        return this;
    }

    // Add a $group stage
    public MongoAggregation Group(const char[] idField, MongoDocument groupSpec) {
        char groupJson[1024];
        groupSpec.ToString(groupJson, sizeof(groupJson));

        char stageJson[1024];
        Format(stageJson, sizeof(stageJson), "{\"$group\":{\"_id\":\"%s\",%s}}", idField, groupJson);
        this.PushString(stageJson);
        return this;
    }

    // Add a $sort stage
    public MongoAggregation Sort(MongoDocument sortSpec) {
        char sortJson[512];
        sortSpec.ToString(sortJson, sizeof(sortJson));

        char stageJson[512];
        Format(stageJson, sizeof(stageJson), "{\"$sort\":%s}", sortJson);
        this.PushString(stageJson);
        return this;
    }

    // Add a $limit stage
    public MongoAggregation Limit(int count) {
        char stageJson[128];
        Format(stageJson, sizeof(stageJson), "{\"$limit\":%d}", count);
        this.PushString(stageJson);
        return this;
    }

    // Add a $skip stage
    public MongoAggregation Skip(int count) {
        char stageJson[128];
        Format(stageJson, sizeof(stageJson), "{\"$skip\":%d}", count);
        this.PushString(stageJson);
        return this;
    }

    // Add a $project stage
    public MongoAggregation Project(MongoDocument projection) {
        char projJson[1024];
        projection.ToString(projJson, sizeof(projJson));

        char stageJson[1024];
        Format(stageJson, sizeof(stageJson), "{\"$project\":%s}", projJson);
        this.PushString(stageJson);
        return this;
    }

    // Add a custom stage
    public MongoAggregation AddStage(const char[] stageJson) {
        this.PushString(stageJson);
        return this;
    }
}

/**
 * MongoDB Bulk Operations Builder
 */
methodmap MongoBulkOps < ArrayList {
    public MongoBulkOps() {
        return view_as<MongoBulkOps>(new ArrayList(ByteCountToCells(1024)));
    }

    // Add an insertOne operation
    public MongoBulkOps InsertOne(MongoDocument document) {
        char docJson[1024];
        document.ToString(docJson, sizeof(docJson));

        char opJson[1024];
        Format(opJson, sizeof(opJson), "{\"insertOne\":{\"document\":%s}}", docJson);
        this.PushString(opJson);
        return this;
    }

    // Add an updateOne operation
    public MongoBulkOps UpdateOne(MongoQuery filter, MongoDocument update, bool upsert = false) {
        char filterJson[512], updateJson[512];
        filter.ToString(filterJson, sizeof(filterJson));
        update.ToString(updateJson, sizeof(updateJson));

        char opJson[1024];
        Format(opJson, sizeof(opJson), "{\"updateOne\":{\"filter\":%s,\"update\":%s,\"upsert\":%s}}",
               filterJson, updateJson, upsert ? "true" : "false");
        this.PushString(opJson);
        return this;
    }

    // Add an updateMany operation
    public MongoBulkOps UpdateMany(MongoQuery filter, MongoDocument update, bool upsert = false) {
        char filterJson[512], updateJson[512];
        filter.ToString(filterJson, sizeof(filterJson));
        update.ToString(updateJson, sizeof(updateJson));

        char opJson[1024];
        Format(opJson, sizeof(opJson), "{\"updateMany\":{\"filter\":%s,\"update\":%s,\"upsert\":%s}}",
               filterJson, updateJson, upsert ? "true" : "false");
        this.PushString(opJson);
        return this;
    }

    // Add a deleteOne operation
    public MongoBulkOps DeleteOne(MongoQuery filter) {
        char filterJson[512];
        filter.ToString(filterJson, sizeof(filterJson));

        char opJson[512];
        Format(opJson, sizeof(opJson), "{\"deleteOne\":{\"filter\":%s}}", filterJson);
        this.PushString(opJson);
        return this;
    }

    // Add a deleteMany operation
    public MongoBulkOps DeleteMany(MongoQuery filter) {
        char filterJson[512];
        filter.ToString(filterJson, sizeof(filterJson));

        char opJson[512];
        Format(opJson, sizeof(opJson), "{\"deleteMany\":{\"filter\":%s}}", filterJson);
        this.PushString(opJson);
        return this;
    }

    // Add a replaceOne operation
    public MongoBulkOps ReplaceOne(MongoQuery filter, MongoDocument replacement, bool upsert = false) {
        char filterJson[512], replaceJson[512];
        filter.ToString(filterJson, sizeof(filterJson));
        replacement.ToString(replaceJson, sizeof(replaceJson));

        char opJson[1024];
        Format(opJson, sizeof(opJson), "{\"replaceOne\":{\"filter\":%s,\"replacement\":%s,\"upsert\":%s}}",
               filterJson, replaceJson, upsert ? "true" : "false");
        this.PushString(opJson);
        return this;
    }
}

/**
 * MongoDB Performance Monitor
 */
methodmap MongoPerformance {
    // Get total operations count
    public static int GetTotalOperations() {
        return MongoDB_GetTotalOperations();
    }

    // Get successful operations count
    public static int GetSuccessfulOperations() {
        return MongoDB_GetSuccessfulOperations();
    }

    // Get failed operations count
    public static int GetFailedOperations() {
        return MongoDB_GetFailedOperations();
    }

    // Get average execution time in centiseconds
    public static int GetAverageExecutionTime() {
        return MongoDB_GetAverageExecutionTime();
    }

    // Get success rate as percentage (0-100)
    public static int GetSuccessRate() {
        return MongoDB_GetSuccessRate();
    }

    // Reset all performance metrics
    public static void Reset() {
        MongoDB_ResetPerformanceMetrics();
    }

    // Get performance summary
    public static void GetSummary(char[] buffer, int maxlen) {
        int total = MongoDB_GetTotalOperations();
        int successful = MongoDB_GetSuccessfulOperations();
        int failed = MongoDB_GetFailedOperations();
        int avgTime = MongoDB_GetAverageExecutionTime();
        int successRate = MongoDB_GetSuccessRate();

        Format(buffer, maxlen, "Operations: %d (Success: %d, Failed: %d), Success Rate: %d%%, Avg Time: %d.%02ds",
               total, successful, failed, successRate, avgTime / 100, avgTime % 100);
    }
}

/**
 * MongoDB Error Handler
 */
methodmap MongoError {
    // Get last error code
    public static int GetCode() {
        return MongoDB_GetLastErrorCode();
    }

    // Get last error message
    public static void GetMessage(char[] buffer, int maxlen) {
        MongoDB_GetLastErrorMessage(buffer, maxlen);
    }

    // Get last error details
    public static void GetDetails(char[] buffer, int maxlen) {
        MongoDB_GetLastErrorDetails(buffer, maxlen);
    }

    // Get last error timestamp
    public static int GetTimestamp() {
        return MongoDB_GetLastErrorTimestamp();
    }

    // Check if there's an active error
    public static bool HasError() {
        return MongoDB_GetLastErrorCode() != 0;
    }

    // Get formatted error string
    public static void GetFormatted(char[] buffer, int maxlen) {
        int code = MongoDB_GetLastErrorCode();
        if (code == 0) {
            strcopy(buffer, maxlen, "No error");
            return;
        }

        char message[256], details[512];
        MongoDB_GetLastErrorMessage(message, sizeof(message));
        MongoDB_GetLastErrorDetails(details, sizeof(details));

        if (strlen(details) > 0) {
            Format(buffer, maxlen, "Error %d: %s - %s", code, message, details);
        } else {
            Format(buffer, maxlen, "Error %d: %s", code, message);
        }
    }
}

/**
 * Simple callback types
 */
typedef MongoCallback = function void (bool success, any data);
