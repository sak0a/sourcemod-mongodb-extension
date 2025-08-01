/**
 * Type definitions for MongoDB API Service
 */

// MongoDB types are imported where needed

// Connection types
export interface ConnectionConfig {
  uri: string;
  options?: {
    maxPoolSize?: number;
    serverSelectionTimeoutMS?: number;
    socketTimeoutMS?: number;
    connectTimeoutMS?: number;
    maxIdleTimeMS?: number;
  };
}

export interface Connection {
  id: string;
  uri: string;
  client: any; // MongoClient
  createdAt: Date;
  lastUsed: Date;
  isActive: boolean;
}

// Document types
export interface MongoDocument {
  [key: string]: any;
}

export interface InsertResult {
  insertedId: string;
  acknowledged: boolean;
}

export interface InsertManyResult {
  insertedIds: string[];
  insertedCount: number;
  acknowledged: boolean;
}

export interface UpdateResult {
  matchedCount: number;
  modifiedCount: number;
  acknowledged: boolean;
  upsertedId?: string;
}

export interface DeleteResult {
  deletedCount: number;
  acknowledged: boolean;
}

export interface FindOptions {
  limit?: number;
  skip?: number;
  sort?: MongoDocument;
  projection?: MongoDocument;
}

// Operation types for batch processing
export interface BatchOperation {
  type: 'insertOne' | 'insertMany' | 'updateOne' | 'updateMany' | 'deleteOne' | 'deleteMany' | 'find' | 'findOne';
  connectionId: string;
  database: string;
  collection: string;
  document?: MongoDocument;
  documents?: MongoDocument[];
  filter?: MongoDocument;
  update?: MongoDocument;
  options?: FindOptions;
}

export interface BatchResult {
  success: boolean;
  results: any[];
  errors: string[];
}

// Index types
export interface IndexSpec {
  keys: MongoDocument;
  options?: {
    name?: string;
    unique?: boolean;
    sparse?: boolean;
    background?: boolean;
    expireAfterSeconds?: number;
  };
}

// API Response types
export interface ApiResponse<T = any> {
  success: boolean;
  data?: T;
  error?: string;
  timestamp: string;
}

export interface ErrorResponse {
  success: false;
  error: string;
  code?: string;
  details?: any;
  timestamp: string;
}

// Request types
export interface CreateConnectionRequest {
  uri: string;
  options?: ConnectionConfig['options'];
}

export interface InsertOneRequest {
  document: MongoDocument;
}

export interface InsertManyRequest {
  documents: MongoDocument[];
}

export interface FindRequest {
  filter?: MongoDocument;
  options?: FindOptions;
}

export interface UpdateRequest {
  filter: MongoDocument;
  update: MongoDocument;
  options?: {
    upsert?: boolean;
  };
}

export interface DeleteRequest {
  filter: MongoDocument;
}

export interface BatchRequest {
  operations: BatchOperation[];
}

// Utility types
export type ObjectIdString = string;

export interface CollectionInfo {
  name: string;
  type: string;
  options: any;
  info: {
    readOnly: boolean;
    uuid?: string;
  };
  idIndex: any;
}

export interface DatabaseInfo {
  name: string;
  sizeOnDisk: number;
  empty: boolean;
  collections: CollectionInfo[];
}

// Configuration types
export interface ServerConfig {
  port: number;
  host: string;
  cors: {
    origin: string | string[];
    credentials: boolean;
  };
  rateLimit: {
    windowMs: number;
    max: number;
  };
  mongodb: {
    defaultTimeout: number;
    maxConnections: number;
    connectionTTL: number;
  };
  logging: {
    level: string;
    format: string;
  };
}

// Middleware types
export interface RequestWithConnection extends Request {
  connection?: Connection;
}

// Validation schemas
export interface ValidationSchema {
  [key: string]: {
    type: string;
    required?: boolean;
    min?: number;
    max?: number;
    pattern?: string;
  };
}
