/**
 * MongoDB Connection Manager
 * Handles connection pooling, lifecycle, and cleanup
 */

import { MongoClient } from 'mongodb';
import { v4 as uuidv4 } from 'uuid';
import { Connection, ConnectionConfig } from '../types';
import { logger } from '../utils/logger';

export class ConnectionManager {
  private connections: Map<string, Connection> = new Map();
  private readonly maxConnections: number;
  private readonly connectionTTL: number; // Time to live in milliseconds
  private cleanupInterval: NodeJS.Timeout | null = null;

  constructor(maxConnections: number = 10, connectionTTL: number = 30 * 60 * 1000) {
    this.maxConnections = maxConnections;
    this.connectionTTL = connectionTTL;
    this.startCleanupTimer();
  }

  /**
   * Create a new MongoDB connection
   */
  async createConnection(config: ConnectionConfig): Promise<string> {
    // Check connection limit
    if (this.connections.size >= this.maxConnections) {
      await this.cleanupOldConnections();
      
      if (this.connections.size >= this.maxConnections) {
        throw new Error(`Maximum connections limit reached (${this.maxConnections})`);
      }
    }

    const connectionId = uuidv4();
    
    try {
      logger.info(`Creating MongoDB connection: ${connectionId}`);
      
      const client = new MongoClient(config.uri, {
        maxPoolSize: config.options?.maxPoolSize || 10,
        serverSelectionTimeoutMS: config.options?.serverSelectionTimeoutMS || 5000,
        socketTimeoutMS: config.options?.socketTimeoutMS || 30000,
        connectTimeoutMS: config.options?.connectTimeoutMS || 10000,
        maxIdleTimeMS: config.options?.maxIdleTimeMS || 30000,
      });

      // Test the connection
      await client.connect();
      await client.db('admin').command({ ping: 1 });

      const connection: Connection = {
        id: connectionId,
        uri: config.uri,
        client,
        createdAt: new Date(),
        lastUsed: new Date(),
        isActive: true,
      };

      this.connections.set(connectionId, connection);
      
      logger.info(`MongoDB connection created successfully: ${connectionId}`);
      return connectionId;
      
    } catch (error) {
      logger.error(`Failed to create MongoDB connection: ${connectionId}`, error);
      throw new Error(`Failed to connect to MongoDB: ${error instanceof Error ? error.message : 'Unknown error'}`);
    }
  }

  /**
   * Get a connection by ID
   */
  getConnection(connectionId: string): Connection | null {
    const connection = this.connections.get(connectionId);
    
    if (connection && connection.isActive) {
      connection.lastUsed = new Date();
      return connection;
    }
    
    return null;
  }

  /**
   * Close a specific connection
   */
  async closeConnection(connectionId: string): Promise<boolean> {
    const connection = this.connections.get(connectionId);
    
    if (!connection) {
      return false;
    }

    try {
      logger.info(`Closing MongoDB connection: ${connectionId}`);
      
      connection.isActive = false;
      await connection.client.close();
      this.connections.delete(connectionId);
      
      logger.info(`MongoDB connection closed: ${connectionId}`);
      return true;
      
    } catch (error) {
      logger.error(`Error closing MongoDB connection: ${connectionId}`, error);
      this.connections.delete(connectionId); // Remove anyway
      return false;
    }
  }

  /**
   * Check if a connection is active
   */
  isConnectionActive(connectionId: string): boolean {
    const connection = this.connections.get(connectionId);
    return connection ? connection.isActive : false;
  }

  /**
   * Get connection statistics
   */
  getConnectionStats() {
    const activeConnections = Array.from(this.connections.values()).filter(conn => conn.isActive);
    
    return {
      total: this.connections.size,
      active: activeConnections.length,
      maxConnections: this.maxConnections,
      connections: activeConnections.map(conn => ({
        id: conn.id,
        createdAt: conn.createdAt,
        lastUsed: conn.lastUsed,
        uri: this.maskUri(conn.uri),
      })),
    };
  }

  /**
   * Close all connections
   */
  async closeAllConnections(): Promise<void> {
    logger.info('Closing all MongoDB connections');
    
    const closePromises = Array.from(this.connections.keys()).map(id => 
      this.closeConnection(id)
    );
    
    await Promise.allSettled(closePromises);
    
    if (this.cleanupInterval) {
      clearInterval(this.cleanupInterval);
      this.cleanupInterval = null;
    }
    
    logger.info('All MongoDB connections closed');
  }

  /**
   * Start cleanup timer for old connections
   */
  private startCleanupTimer(): void {
    this.cleanupInterval = setInterval(() => {
      this.cleanupOldConnections().catch(error => {
        logger.error('Error during connection cleanup', error);
      });
    }, 5 * 60 * 1000); // Run every 5 minutes
  }

  /**
   * Clean up old or inactive connections
   */
  private async cleanupOldConnections(): Promise<void> {
    const now = new Date();
    const connectionsToClose: string[] = [];

    for (const [id, connection] of this.connections) {
      const age = now.getTime() - connection.lastUsed.getTime();
      
      if (age > this.connectionTTL || !connection.isActive) {
        connectionsToClose.push(id);
      }
    }

    if (connectionsToClose.length > 0) {
      logger.info(`Cleaning up ${connectionsToClose.length} old connections`);
      
      const closePromises = connectionsToClose.map(id => this.closeConnection(id));
      await Promise.allSettled(closePromises);
    }
  }

  /**
   * Mask sensitive information in URI
   */
  private maskUri(uri: string): string {
    try {
      const url = new URL(uri);
      if (url.password) {
        url.password = '***';
      }
      return url.toString();
    } catch {
      return uri.replace(/\/\/[^:]+:[^@]+@/, '//***:***@');
    }
  }
}
