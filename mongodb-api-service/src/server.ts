/**
 * MongoDB API Service - Main Server
 * HTTP API for SourceMod MongoDB Extension
 */

import express from 'express';
import cors from 'cors';
import helmet from 'helmet';
import compression from 'compression';

import dotenv from 'dotenv';

import { ConnectionManager } from './managers/ConnectionManager';
import { connectionRoutes } from './routes/connectionRoutes';
import { databaseRoutes } from './routes/databaseRoutes';
import { batchRoutes } from './routes/batchRoutes';
import { errorHandler } from './middleware/errorHandler';
import { requestLogger } from './middleware/requestLogger';
import { authenticateApiKey, verifySourceModExtension } from './middleware/auth';
import {
  createRateLimit,
  createSlowDown,
  validateRequestSize,
  sanitizeInput,
  enforceHttps,
  securityHeaders,
  securityLogger
} from './middleware/security';
import { logger } from './utils/logger';

// Load environment variables
dotenv.config();

class MongoDBAPIServer {
  private app: express.Application;
  private connectionManager: ConnectionManager;
  private server: any;

  constructor() {
    this.app = express();
    this.connectionManager = new ConnectionManager(
      parseInt(process.env['MAX_CONNECTIONS'] || '10'),
      parseInt(process.env['CONNECTION_TTL'] || '1800000') // 30 minutes
    );
    
    this.setupMiddleware();
    this.setupRoutes();
    this.setupErrorHandling();
  }

  private setupMiddleware(): void {
    // Trust proxy for accurate IP addresses
    this.app.set('trust proxy', 1);

    // HTTPS enforcement (production only)
    this.app.use(enforceHttps());

    // Security headers
    this.app.use(securityHeaders());

    // Enhanced security middleware
    this.app.use(helmet({
      contentSecurityPolicy: false, // Disable for API
      crossOriginEmbedderPolicy: false,
    }));

    // CORS configuration with enhanced security
    this.app.use(cors({
      origin: process.env['CORS_ORIGIN']?.split(',') || [
        'http://localhost:3000',
        'http://127.0.0.1:3000',
        'http://localhost:3300',
        'http://127.0.0.1:3300'
      ],
      credentials: true,
      methods: ['GET', 'POST', 'PUT', 'DELETE', 'OPTIONS'],
      allowedHeaders: [
        'Content-Type',
        'Authorization',
        'X-Requested-With',
        'X-API-Key',
        'X-SourceMod-API-Key',
        'X-SourceMod-Extension',
        'X-Extension-Version'
      ],
    }));

    // Enhanced rate limiting and slow down
    this.app.use('/api/', createRateLimit());
    this.app.use('/api/', createSlowDown());

    // Request size validation
    this.app.use(validateRequestSize());

    // Security logging
    this.app.use(securityLogger());

    // Body parsing and compression
    this.app.use(compression());
    this.app.use(express.json({ limit: '10mb' }));
    this.app.use(express.urlencoded({ extended: true, limit: '10mb' }));

    // Request logging
    this.app.use(requestLogger);

    // Make connection manager available to routes
    this.app.locals['connectionManager'] = this.connectionManager;
  }

  private setupRoutes(): void {
    // Basic health check endpoint (simple)
    this.app.get('/health', (_req, res) => {
      res.json({
        success: true,
        data: {
          status: 'healthy',
          uptime: process.uptime(),
          timestamp: new Date().toISOString(),
          version: '1.0.0',
        },
        timestamp: new Date().toISOString(),
      });
    });

    // Input sanitization for all API routes
    this.app.use('/api/', sanitizeInput());

    // API routes with authentication and SourceMod verification
    // Connection routes (require API key authentication + SourceMod verification)
    this.app.use('/api/v1/connections',
      verifySourceModExtension(),
      authenticateApiKey({ required: true, permissions: ['read', 'write'] }),
      connectionRoutes
    );

    // Database routes (require API key authentication + SourceMod verification)
    this.app.use('/api/v1/connections',
      verifySourceModExtension(),
      authenticateApiKey({ required: true, permissions: ['read', 'write'] }),
      databaseRoutes
    );

    // Batch routes (require API key authentication + SourceMod verification with write permissions)
    this.app.use('/api/v1/batch',
      verifySourceModExtension(),
      authenticateApiKey({ required: true, permissions: ['write'] }),
      batchRoutes
    );

    // Root endpoint
    this.app.get('/', (_req, res) => {
      res.json({
        success: true,
        message: 'MongoDB API Service for SourceMod',
        version: '1.0.0',
        endpoints: {
          health: '/health',
          connections: '/api/v1/connections',
          batch: '/api/v1/batch',
        },
        timestamp: new Date().toISOString(),
      });
    });

    // 404 handler
    this.app.use('*', (req, res) => {
      res.status(404).json({
        success: false,
        error: 'Endpoint not found',
        path: req.originalUrl,
        timestamp: new Date().toISOString(),
      });
    });
  }

  private setupErrorHandling(): void {
    this.app.use(errorHandler);
  }

  public async start(): Promise<void> {
    const port = parseInt(process.env['PORT'] || '3000');
    const host = process.env['HOST'] || '0.0.0.0';

    return new Promise((resolve, reject) => {
      this.server = this.app.listen(port, host, () => {
        logger.info(`MongoDB API Service started on ${host}:${port}`);
        logger.info(`Environment: ${process.env['NODE_ENV'] || 'development'}`);
        logger.info(`Max connections: ${process.env['MAX_CONNECTIONS'] || '10'}`);
        resolve();
      });

      this.server.on('error', (error: Error) => {
        logger.error('Server startup error:', error);
        reject(error);
      });
    });
  }

  public async stop(): Promise<void> {
    logger.info('Shutting down MongoDB API Service...');

    // Close all MongoDB connections
    await this.connectionManager.closeAllConnections();

    // Close HTTP server
    if (this.server) {
      return new Promise((resolve) => {
        this.server.close(() => {
          logger.info('MongoDB API Service stopped');
          resolve();
        });
      });
    }
  }

  public getApp(): express.Application {
    return this.app;
  }
}

// Create and start server
const server = new MongoDBAPIServer();

// Graceful shutdown handling
process.on('SIGTERM', async () => {
  logger.info('SIGTERM received, shutting down gracefully');
  await server.stop();
  process.exit(0);
});

process.on('SIGINT', async () => {
  logger.info('SIGINT received, shutting down gracefully');
  await server.stop();
  process.exit(0);
});

process.on('unhandledRejection', (reason, promise) => {
  logger.error('Unhandled Rejection at:', promise, 'reason:', reason);
});

process.on('uncaughtException', (error) => {
  logger.error('Uncaught Exception:', error);
  process.exit(1);
});

// Start the server
if (require.main === module) {
  server.start().catch((error) => {
    logger.error('Failed to start server:', error);
    process.exit(1);
  });
}

export default server;
