/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod HTTP MongoDB Extension - Native Functions
 * Copyright (C) 2024 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 */

#ifndef _NATIVES_H_
#define _NATIVES_H_

#include <IPluginSys.h>

using namespace SourceMod;

// MongoDB Connection Natives
cell_t MongoDB_Connect(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_ConnectWithConfig(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_ConnectFromConfigFile(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_GetConnectionConfig(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_Close(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_IsConnected(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_Ping(IPluginContext *pContext, const cell_t *params);

// Collection Natives
cell_t MongoDB_GetCollection(IPluginContext *pContext, const cell_t *params);

// Document Operations
cell_t MongoDB_InsertOne(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_InsertMany(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_FindOne(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_Find(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_UpdateOne(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_UpdateMany(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_DeleteOne(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_DeleteMany(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_CountDocuments(IPluginContext *pContext, const cell_t *params);

// Index Operations
cell_t MongoDB_CreateIndex(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_DropIndex(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_ListIndexes(IPluginContext *pContext, const cell_t *params);

// Async Operations
cell_t MongoDB_InsertOneAsync(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_FindOneAsync(IPluginContext *pContext, const cell_t *params);

// Error Handling
cell_t MongoDB_GetLastError(IPluginContext *pContext, const cell_t *params);

// JSON Structure Natives
cell_t JSON_StringMapToString(IPluginContext *pContext, const cell_t *params);
cell_t JSON_StringFromString(IPluginContext *pContext, const cell_t *params);
cell_t JSON_ArrayListToString(IPluginContext *pContext, const cell_t *params);
cell_t JSON_ArrayFromString(IPluginContext *pContext, const cell_t *params);

// Statistics
cell_t MongoDB_GetStats(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_ResetStats(IPluginContext *pContext, const cell_t *params);

// Native registration arrays
extern const sp_nativeinfo_t g_MongoDBNatives[];
extern const sp_nativeinfo_t g_JSONNatives[];

#endif // _NATIVES_H_
