/**
 * Connection management routes
 */

import { Router, Request, Response } from 'express';
import { body, param, validationResult } from 'express-validator';
import { ConnectionManager } from '../managers/ConnectionManager';
import { CreateConnectionRequest, ApiResponse } from '../types';
import { asyncHandler, createError } from '../middleware/errorHandler';
import { logger } from '../utils/logger';

const router = Router();

// Validation middleware
const validateCreateConnection = [
  body('uri')
    .isString()
    .notEmpty()
    .withMessage('URI is required')
    .matches(/^mongodb(\+srv)?:\/\//)
    .withMessage('Invalid MongoDB URI format'),
  body('options.maxPoolSize')
    .optional()
    .isInt({ min: 1, max: 100 })
    .withMessage('maxPoolSize must be between 1 and 100'),
  body('options.serverSelectionTimeoutMS')
    .optional()
    .isInt({ min: 1000, max: 60000 })
    .withMessage('serverSelectionTimeoutMS must be between 1000 and 60000'),
];

const validateConnectionId = [
  param('connectionId')
    .isUUID()
    .withMessage('Invalid connection ID format'),
];

// Helper function to check validation results
const checkValidation = (req: Request, res: Response) => {
  const errors = validationResult(req);
  if (!errors.isEmpty()) {
    return res.status(400).json({
      success: false,
      error: 'Validation failed',
      details: errors.array(),
      timestamp: new Date().toISOString(),
    });
  }
  return null;
};

/**
 * POST /api/v1/connections
 * Create a new MongoDB connection
 */
router.post('/', validateCreateConnection, asyncHandler(async (req: Request, res: Response) => {
  const validationError = checkValidation(req, res);
  if (validationError) return;

  const connectionManager: ConnectionManager = req.app.locals['connectionManager'];
  const { uri, options }: CreateConnectionRequest = req.body;

  logger.info('Creating new MongoDB connection', { uri: uri.replace(/\/\/[^:]+:[^@]+@/, '//***:***@') });

  try {
    const connectionId = await connectionManager.createConnection({ uri, options: options || {} });

    const response: ApiResponse<{ connectionId: string }> = {
      success: true,
      data: { connectionId },
      timestamp: new Date().toISOString(),
    };

    res.status(201).json(response);
  } catch (error) {
    throw createError(
      error instanceof Error ? error.message : 'Failed to create connection',
      500,
      'CONNECTION_FAILED'
    );
  }
}));

/**
 * GET /api/v1/connections/:connectionId
 * Get connection status
 */
router.get('/:connectionId', validateConnectionId, asyncHandler(async (req: Request, res: Response) => {
  const validationError = checkValidation(req, res);
  if (validationError) return;

  const connectionManager: ConnectionManager = req.app.locals['connectionManager'];
  const { connectionId } = req.params;

  const connection = connectionManager.getConnection(connectionId!);
  
  if (!connection) {
    throw createError('Connection not found', 404, 'CONNECTION_NOT_FOUND');
  }

  const response: ApiResponse<{ 
    id: string; 
    isActive: boolean; 
    createdAt: Date; 
    lastUsed: Date; 
  }> = {
    success: true,
    data: {
      id: connection.id,
      isActive: connection.isActive,
      createdAt: connection.createdAt,
      lastUsed: connection.lastUsed,
    },
    timestamp: new Date().toISOString(),
  };

  res.json(response);
}));

/**
 * DELETE /api/v1/connections/:connectionId
 * Close a MongoDB connection
 */
router.delete('/:connectionId', validateConnectionId, asyncHandler(async (req: Request, res: Response) => {
  const validationError = checkValidation(req, res);
  if (validationError) return;

  const connectionManager: ConnectionManager = req.app.locals['connectionManager'];
  const { connectionId } = req.params;

  logger.info('Closing MongoDB connection', { connectionId });

  const success = await connectionManager.closeConnection(connectionId!);
  
  if (!success) {
    throw createError('Connection not found', 404, 'CONNECTION_NOT_FOUND');
  }

  const response: ApiResponse<{ message: string }> = {
    success: true,
    data: { message: 'Connection closed successfully' },
    timestamp: new Date().toISOString(),
  };

  res.json(response);
}));

/**
 * GET /api/v1/connections
 * Get all connection statistics
 */
router.get('/', asyncHandler(async (req: Request, res: Response) => {
  const connectionManager: ConnectionManager = req.app.locals['connectionManager'];
  
  const stats = connectionManager.getConnectionStats();
  
  const response: ApiResponse<typeof stats> = {
    success: true,
    data: stats,
    timestamp: new Date().toISOString(),
  };

  res.json(response);
}));

/**
 * POST /api/v1/connections/:connectionId/ping
 * Test connection health
 */
router.post('/:connectionId/ping', validateConnectionId, asyncHandler(async (req: Request, res: Response) => {
  const validationError = checkValidation(req, res);
  if (validationError) return;

  const connectionManager: ConnectionManager = req.app.locals['connectionManager'];
  const { connectionId } = req.params;

  const connection = connectionManager.getConnection(connectionId!);
  
  if (!connection) {
    throw createError('Connection not found', 404, 'CONNECTION_NOT_FOUND');
  }

  try {
    // Test the connection with a ping and measure latency
    const startTime = Date.now();
    await connection.client.db('admin').command({ ping: 1 });
    const latency = Date.now() - startTime;

    const response: ApiResponse<{
      message: string;
      latency: number;
      status: string;
      connectionId: string;
    }> = {
      success: true,
      data: {
        message: 'Connection is healthy',
        latency,
        status: 'healthy',
        connectionId: req.params['connectionId']!
      },
      timestamp: new Date().toISOString(),
    };

    res.json(response);
  } catch (error) {
    throw createError('Connection health check failed', 503, 'CONNECTION_UNHEALTHY');
  }
}));

/**
 * GET /:connectionId/health
 * Enhanced health check for a specific connection
 */
router.get('/:connectionId/health', validateConnectionId, asyncHandler(async (req: Request, res: Response) => {
  const connectionManager: ConnectionManager = req.app.locals['connectionManager'];
  const { connectionId } = req.params;

  const connection = connectionManager.getConnection(connectionId!);
  if (!connection) {
    throw createError('Connection not found', 404, 'CONNECTION_NOT_FOUND');
  }

  try {
    // Comprehensive health check
    const startTime = Date.now();

    // Test basic connectivity
    await connection.client.db('admin').command({ ping: 1 });
    const pingLatency = Date.now() - startTime;

    // Try to get server status, but don't fail if we can't (due to auth)
    let serverInfo: any = {
      version: 'unknown',
      host: 'unknown',
      uptime: 0,
      connections: { current: 0, available: 0 }
    };

    try {
      const serverStatus = await connection.client.db('admin').command({ serverStatus: 1 });
      serverInfo = {
        uptime: serverStatus.uptime || 0,
        connections: serverStatus.connections || { current: 0, available: 0 },
        version: serverStatus.version || 'unknown',
        host: serverStatus.host || 'unknown',
      };
    } catch (authError) {
      // If we can't get server status due to auth, that's okay
      // We'll just use basic info
      logger.warn('Could not get server status (likely due to auth restrictions)', {
        connectionId: connectionId!,
        error: authError instanceof Error ? authError.message : 'Unknown error'
      });
    }

    const response: ApiResponse<{
      status: string;
      connectionId: string;
      latency: number;
      serverInfo: any;
      timestamp: string;
    }> = {
      success: true,
      data: {
        status: 'healthy',
        connectionId: connectionId!,
        latency: pingLatency,
        serverInfo: serverInfo,
        timestamp: new Date().toISOString(),
      },
      timestamp: new Date().toISOString(),
    };

    res.json(response);
  } catch (error) {
    const response: ApiResponse<{
      status: string;
      connectionId: string;
      error: string;
    }> = {
      success: false,
      data: {
        status: 'unhealthy',
        connectionId: connectionId!,
        error: error instanceof Error ? error.message : 'Unknown error',
      },
      timestamp: new Date().toISOString(),
    };

    res.status(503).json(response);
  }
}));

export { router as connectionRoutes };
