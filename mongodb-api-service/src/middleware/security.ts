import rateLimit from 'express-rate-limit';
import slowDown from 'express-slow-down';
import { Request, Response, NextFunction } from 'express';
import { securityManager } from '../config/security';
import { createError } from './errorHandler';
import { logger } from '../utils/logger';

/**
 * Rate Limiting Middleware
 */
export const createRateLimit = () => {
  const config = securityManager.getConfig();
  
  return rateLimit({
    windowMs: config.rateLimiting.windowMs,
    max: config.rateLimiting.maxRequests,
    message: {
      success: false,
      error: 'Too many requests from this IP, please try again later',
      code: 'RATE_LIMIT_EXCEEDED',
      timestamp: new Date().toISOString(),
    },
    standardHeaders: true,
    legacyHeaders: false,
    keyGenerator: (req: Request) => {
      // Use API key name if authenticated, otherwise IP
      return req.auth?.keyName || req.ip || 'unknown';
    },
    skip: (req: Request) => {
      // Skip rate limiting for admin API keys
      return req.auth?.permissions?.includes('admin') || false;
    },
  });
};

/**
 * Slow Down Middleware (Progressive Delay)
 */
export const createSlowDown = () => {
  const config = securityManager.getConfig();
  
  return slowDown({
    windowMs: config.rateLimiting.slowDown.windowMs,
    delayAfter: config.rateLimiting.slowDown.delayAfter,
    delayMs: config.rateLimiting.slowDown.delayMs,
    keyGenerator: (req: Request) => {
      return req.auth?.keyName || req.ip || 'unknown';
    },
    skip: (req: Request) => {
      return req.auth?.permissions?.includes('admin') || false;
    },
  });
};

/**
 * Request Size Validation Middleware
 */
export const validateRequestSize = () => {
  return (req: Request, _res: Response, next: NextFunction) => {
    const contentLength = req.get('content-length');
    
    if (contentLength) {
      const size = parseInt(contentLength, 10);
      const maxSize = 10 * 1024 * 1024; // 10MB
      
      if (size > maxSize) {
        logger.warn('Request size too large', {
          size,
          maxSize,
          ip: req.ip,
          keyName: req.auth?.keyName,
        });
        
        throw createError('Request entity too large', 413, 'REQUEST_TOO_LARGE');
      }
    }
    
    next();
  };
};

/**
 * Input Sanitization Middleware
 */
export const sanitizeInput = () => {
  return (req: Request, _res: Response, next: NextFunction) => {
    try {
      // Sanitize query parameters
      if (req.query) {
        for (const [key, value] of Object.entries(req.query)) {
          if (typeof value === 'string') {
            // Remove potentially dangerous characters
            req.query[key] = value.replace(/[<>'"&]/g, '');
          }
        }
      }

      // Validate JSON body structure
      if (req.body && typeof req.body === 'object') {
        validateJsonStructure(req.body);
      }

      next();
    } catch (error) {
      logger.warn('Input sanitization failed', {
        error: error instanceof Error ? error.message : 'Unknown error',
        ip: req.ip,
        keyName: req.auth?.keyName,
      });
      
      throw createError('Invalid input data', 400, 'INVALID_INPUT');
    }
  };
};

/**
 * Validate JSON structure for MongoDB operations
 */
function validateJsonStructure(obj: any, depth: number = 0): void {
  const maxDepth = 10;
  const maxKeys = 100;
  
  if (depth > maxDepth) {
    throw new Error('JSON structure too deep');
  }
  
  if (typeof obj === 'object' && obj !== null) {
    const keys = Object.keys(obj);
    
    if (keys.length > maxKeys) {
      throw new Error('Too many keys in JSON object');
    }
    
    for (const key of keys) {
      // Validate key names
      if (typeof key !== 'string' || key.length > 100) {
        throw new Error('Invalid key name');
      }
      
      // Check for potentially dangerous operators
      if (key.startsWith('$') && !isAllowedMongoOperator(key)) {
        throw new Error(`Dangerous MongoDB operator: ${key}`);
      }
      
      // Recursively validate nested objects
      if (typeof obj[key] === 'object' && obj[key] !== null) {
        validateJsonStructure(obj[key], depth + 1);
      }
    }
  }
}

/**
 * Check if MongoDB operator is allowed
 */
function isAllowedMongoOperator(operator: string): boolean {
  const allowedOperators = [
    // Query operators
    '$eq', '$ne', '$gt', '$gte', '$lt', '$lte', '$in', '$nin',
    '$and', '$or', '$not', '$nor', '$exists', '$type', '$regex', '$options',
    '$text', '$search', '$language', '$caseSensitive', '$diacriticSensitive',
    '$mod', '$all', '$elemMatch', '$size',
    
    // Update operators
    '$set', '$unset', '$inc', '$mul', '$rename', '$min', '$max',
    '$addToSet', '$pop', '$pull', '$push', '$pullAll',
    '$currentDate', '$bit',
    
    // Aggregation operators
    '$match', '$group', '$sort', '$limit', '$skip', '$project',
    '$unwind', '$lookup', '$addFields', '$replaceRoot', '$facet',
    '$bucket', '$bucketAuto', '$sortByCount', '$count', '$sample',
    '$sum', '$avg', '$min', '$max', '$first', '$last', '$push', '$addToSet',
  ];
  
  return allowedOperators.includes(operator);
}

/**
 * HTTPS Enforcement Middleware
 */
export const enforceHttps = () => {
  return (req: Request, res: Response, next: NextFunction) => {
    const config = securityManager.getConfig();
    
    if (config.requireHttps && !req.secure && req.get('x-forwarded-proto') !== 'https') {
      logger.warn('HTTPS required but request is not secure', {
        ip: req.ip,
        url: req.url,
        protocol: req.protocol,
      });
      
      return res.redirect(301, `https://${req.get('host')}${req.url}`);
    }
    
    next();
  };
};

/**
 * Security Headers Middleware
 */
export const securityHeaders = () => {
  return (req: Request, res: Response, next: NextFunction) => {
    // Set security headers
    res.setHeader('X-Content-Type-Options', 'nosniff');
    res.setHeader('X-Frame-Options', 'DENY');
    res.setHeader('X-XSS-Protection', '1; mode=block');
    res.setHeader('Referrer-Policy', 'strict-origin-when-cross-origin');
    res.setHeader('Permissions-Policy', 'geolocation=(), microphone=(), camera=()');
    
    // Remove server information
    res.removeHeader('X-Powered-By');
    
    // Set HSTS header for HTTPS
    if (req.secure) {
      res.setHeader('Strict-Transport-Security', 'max-age=31536000; includeSubDomains');
    }
    
    next();
  };
};

/**
 * Request Logging Middleware for Security Events
 */
export const securityLogger = () => {
  return (req: Request, res: Response, next: NextFunction) => {
    const startTime = Date.now();
    
    res.on('finish', () => {
      const duration = Date.now() - startTime;
      
      // Log security-relevant requests
      if (req.url.includes('/connections') || req.url.includes('/admin') || res.statusCode >= 400) {
        logger.info('Security-relevant request', {
          method: req.method,
          url: req.url,
          statusCode: res.statusCode,
          duration,
          ip: req.ip,
          userAgent: req.get('User-Agent'),
          keyName: req.auth?.keyName,
          authenticated: req.auth?.authenticated || false,
        });
      }
    });
    
    next();
  };
};
