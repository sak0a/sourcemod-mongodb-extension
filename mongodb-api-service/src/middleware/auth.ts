import { Request, Response, NextFunction } from 'express';
import { securityManager } from '../config/security';
import { createError } from './errorHandler';
import { logger } from '../utils/logger';

// Extend Request interface to include auth info
declare global {
  namespace Express {
    interface Request {
      auth?: {
        keyName: string;
        permissions: string[];
        authenticated: boolean;
      };
    }
  }
}

export interface AuthOptions {
  required?: boolean;
  permissions?: string[];
  allowAnonymous?: boolean;
}

/**
 * API Key Authentication Middleware
 */
export const authenticateApiKey = (options: AuthOptions = {}) => {
  return async (req: Request, _res: Response, next: NextFunction) => {
    const { required = true, permissions = [], allowAnonymous = false } = options;

    try {
      // Extract API key from various sources
      let apiKey: string | undefined;

      // 1. Authorization header (Bearer token)
      const authHeader = req.headers.authorization;
      if (authHeader && authHeader.startsWith('Bearer ')) {
        apiKey = authHeader.substring(7);
      }

      // 2. X-API-Key header
      if (!apiKey) {
        apiKey = req.headers['x-api-key'] as string;
      }

      // 3. Query parameter (less secure, for development only)
      if (!apiKey && process.env['NODE_ENV'] !== 'production') {
        apiKey = req.query['apiKey'] as string;
      }

      // 4. Custom SourceMod header
      if (!apiKey) {
        apiKey = req.headers['x-sourcemod-api-key'] as string;
      }

      if (!apiKey) {
        if (allowAnonymous) {
          req.auth = {
            keyName: 'anonymous',
            permissions: ['read'],
            authenticated: false,
          };
          return next();
        }

        if (required) {
          logger.warn('API key authentication failed: No API key provided', {
            ip: req.ip,
            userAgent: req.get('User-Agent'),
            url: req.url,
          });

          throw createError('API key required', 401, 'MISSING_API_KEY');
        }

        return next();
      }

      // Validate API key
      const validation = await securityManager.validateApiKey(apiKey);

      if (!validation.valid) {
        logger.warn('API key authentication failed: Invalid API key', {
          ip: req.ip,
          userAgent: req.get('User-Agent'),
          url: req.url,
          keyPrefix: apiKey.substring(0, 8) + '...',
        });

        throw createError('Invalid API key', 401, 'INVALID_API_KEY');
      }

      // Check permissions
      if (permissions.length > 0) {
        const hasPermission = permissions.some(permission => 
          validation.permissions?.includes(permission) || validation.permissions?.includes('admin')
        );

        if (!hasPermission) {
          logger.warn('API key authorization failed: Insufficient permissions', {
            ip: req.ip,
            keyName: validation.keyName,
            requiredPermissions: permissions,
            userPermissions: validation.permissions,
          });

          throw createError('Insufficient permissions', 403, 'INSUFFICIENT_PERMISSIONS');
        }
      }

      // Set auth info on request
      req.auth = {
        keyName: validation.keyName!,
        permissions: validation.permissions!,
        authenticated: true,
      };

      logger.info('API key authentication successful', {
        keyName: validation.keyName,
        permissions: validation.permissions,
        ip: req.ip,
      });

      next();
    } catch (error) {
      next(error);
    }
  };
};

/**
 * JWT Authentication Middleware
 */
export const authenticateJWT = (options: AuthOptions = {}) => {
  return (req: Request, _res: Response, next: NextFunction) => {
    const { required = true, allowAnonymous = false } = options;

    try {
      let token: string | undefined;

      // Extract JWT from Authorization header
      const authHeader = req.headers.authorization;
      if (authHeader && authHeader.startsWith('Bearer ')) {
        token = authHeader.substring(7);
      }

      if (!token) {
        if (allowAnonymous) {
          req.auth = {
            keyName: 'anonymous',
            permissions: ['read'],
            authenticated: false,
          };
          return next();
        }

        if (required) {
          throw createError('JWT token required', 401, 'MISSING_JWT');
        }

        return next();
      }

      // Verify JWT
      const payload = securityManager.verifyJWT(token);

      if (!payload) {
        logger.warn('JWT authentication failed: Invalid token', {
          ip: req.ip,
          userAgent: req.get('User-Agent'),
        });

        throw createError('Invalid JWT token', 401, 'INVALID_JWT');
      }

      // Set auth info on request
      req.auth = {
        keyName: payload.keyName || 'jwt-user',
        permissions: payload.permissions || ['read'],
        authenticated: true,
      };

      logger.info('JWT authentication successful', {
        keyName: payload.keyName,
        permissions: payload.permissions,
        ip: req.ip,
      });

      next();
    } catch (error) {
      next(error);
    }
  };
};

/**
 * Permission Check Middleware
 */
export const requirePermissions = (permissions: string[]) => {
  return (req: Request, _res: Response, next: NextFunction) => {
    if (!req.auth || !req.auth.authenticated) {
      throw createError('Authentication required', 401, 'AUTHENTICATION_REQUIRED');
    }

    const hasPermission = permissions.some(permission => 
      req.auth!.permissions.includes(permission) || req.auth!.permissions.includes('admin')
    );

    if (!hasPermission) {
      logger.warn('Permission check failed', {
        keyName: req.auth.keyName,
        requiredPermissions: permissions,
        userPermissions: req.auth.permissions,
        ip: req.ip,
      });

      throw createError('Insufficient permissions', 403, 'INSUFFICIENT_PERMISSIONS');
    }

    next();
  };
};

/**
 * SourceMod Extension Verification Middleware
 * Validates requests specifically from SourceMod extensions
 */
export const verifySourceModExtension = () => {
  return (req: Request, _res: Response, next: NextFunction) => {
    try {
      // Check for SourceMod-specific headers
      const userAgent = req.get('User-Agent');
      const sourceModHeader = req.get('X-SourceMod-Extension');
      const extensionVersion = req.get('X-Extension-Version');

      // Validate User-Agent contains SourceMod identifier
      if (!userAgent || !userAgent.includes('SourceMod-MongoDB-Extension')) {
        logger.warn('SourceMod verification failed: Invalid User-Agent', {
          userAgent,
          ip: req.ip,
        });

        throw createError('Invalid client', 403, 'INVALID_CLIENT');
      }

      // Validate SourceMod extension header
      if (!sourceModHeader || sourceModHeader !== 'MongoDB-HTTP-Extension') {
        logger.warn('SourceMod verification failed: Missing or invalid extension header', {
          sourceModHeader,
          ip: req.ip,
        });

        throw createError('Invalid SourceMod extension', 403, 'INVALID_EXTENSION');
      }

      // Log successful verification
      logger.info('SourceMod extension verified', {
        userAgent,
        extensionVersion,
        ip: req.ip,
      });

      next();
    } catch (error) {
      next(error);
    }
  };
};

/**
 * IP Whitelist Middleware
 */
export const ipWhitelist = (allowedIPs: string[]) => {
  return (req: Request, _res: Response, next: NextFunction) => {
    const clientIP = req.ip || req.connection.remoteAddress;

    if (!clientIP || !allowedIPs.includes(clientIP)) {
      logger.warn('IP whitelist check failed', {
        clientIP,
        allowedIPs,
        url: req.url,
      });

      throw createError('IP not allowed', 403, 'IP_NOT_ALLOWED');
    }

    next();
  };
};
