/**
 * Health check routes
 */

import { Router, Request, Response } from 'express';
import { ConnectionManager } from '../managers/ConnectionManager';
import { ApiResponse } from '../types';

const router = Router();

/**
 * GET /health
 * Basic health check
 */
router.get('/', (_req: Request, res: Response) => {
  const response: ApiResponse<{
    status: string;
    uptime: number;
    timestamp: string;
    version: string;
  }> = {
    success: true,
    data: {
      status: 'healthy',
      uptime: process.uptime(),
      timestamp: new Date().toISOString(),
      version: '1.0.0',
    },
    timestamp: new Date().toISOString(),
  };

  res.json(response);
});

/**
 * GET /health/detailed
 * Detailed health check including connections
 */
router.get('/detailed', (req: Request, res: Response) => {
  const connectionManager: ConnectionManager = req.app.locals['connectionManager'];
  const stats = connectionManager.getConnectionStats();

  const response: ApiResponse<{
    status: string;
    uptime: number;
    memory: NodeJS.MemoryUsage;
    connections: typeof stats;
    timestamp: string;
  }> = {
    success: true,
    data: {
      status: 'healthy',
      uptime: process.uptime(),
      memory: process.memoryUsage(),
      connections: stats,
      timestamp: new Date().toISOString(),
    },
    timestamp: new Date().toISOString(),
  };

  res.json(response);
});

export { router as healthRoutes };
