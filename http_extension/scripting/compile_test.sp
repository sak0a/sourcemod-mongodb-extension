/**
 * Compilation test for MongoDB include file
 */

#include <sourcemod>
#include <http_mongodb>

#pragma semicolon 1
#pragma newdecls required

public Plugin myinfo = {
    name = "MongoDB Compile Test",
    author = "Test",
    description = "Test compilation of MongoDB include",
    version = "1.0.0",
    url = ""
};

public void OnPluginStart() {
    // Test that all methodmaps compile correctly
    
    // Test MongoConnection
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    
    // Test MongoCollection
    MongoCollection coll = conn.GetCollection("test", "test");
    
    // Test MongoDocument
    MongoDocument doc = new MongoDocument();
    doc.SetString("name", "test");
    doc.SetValue("score", 100);
    doc.SetDate("timestamp", GetTime());
    doc.SetObjectId("_id", "507f1f77bcf86cd799439011");
    
    // Test nested document
    MongoDocument nested = new MongoDocument();
    nested.SetString("field", "value");
    doc.SetDocument("nested", nested);
    
    // Test MongoDocumentArray
    MongoDocumentArray arr = new MongoDocumentArray();
    arr.PushDocument(doc);
    
    // Test MongoQuery
    MongoQuery query = new MongoQuery();
    query.Equals("name", "test");
    query.GreaterThan("score", 50);
    query.Where("status", "eq", "active");
    
    // Test MongoFindOptions
    MongoFindOptions options = new MongoFindOptions();
    options.SetLimit(10);
    options.SetSkip(5);
    
    // Test operations
    char insertedId[64];
    coll.InsertOne(doc, insertedId, sizeof(insertedId));

    // Test bulk operations
    ArrayList docs = new ArrayList();
    ArrayList ids = new ArrayList();
    coll.InsertMany(docs, ids);

    StringMap result = coll.FindOne(query);
    ArrayList results = coll.Find(query, options);

    coll.UpdateOne(query, doc);
    coll.UpdateMany(query, doc);

    coll.DeleteOne(query);
    coll.DeleteMany(query);

    int count = coll.CountDocuments(query);

    // Test index operations
    StringMap indexKeys = new StringMap();
    indexKeys.SetValue("name", 1);
    StringMap indexOptions = new StringMap();
    indexOptions.SetString("name", "name_index");

    coll.CreateIndex(indexKeys, indexOptions);
    coll.DropIndex("name_index");
    
    // Cleanup
    delete doc;
    delete nested;
    delete arr;
    delete query;
    delete options;
    delete docs;
    delete ids;
    delete indexKeys;
    delete indexOptions;
    if (result != null) delete result;
    if (results != null) delete results;
    conn.Close();
    
    PrintToServer("MongoDB include compilation test completed successfully!");
}
