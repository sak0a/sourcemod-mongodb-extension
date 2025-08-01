/**
 * Global error handling middleware
 */

import { Request, Response, NextFunction } from 'express';
import { logger } from '../utils/logger';
import { ErrorResponse } from '../types';

export interface AppError extends Error {
  statusCode?: number;
  code?: string;
  isOperational?: boolean;
}

export const errorHandler = (
  error: AppError,
  req: Request,
  res: Response,
  _next: NextFunction
): void => {
  // Log the error
  logger.error('API Error:', {
    message: error.message,
    stack: error.stack,
    url: req.url,
    method: req.method,
    ip: req.ip,
    userAgent: req.get('User-Agent'),
  });

  // Default error response
  let statusCode = error.statusCode || 500;
  let message = error.message || 'Internal Server Error';
  let code = error.code || 'INTERNAL_ERROR';

  // Handle specific error types
  if (error.name === 'ValidationError') {
    statusCode = 400;
    code = 'VALIDATION_ERROR';
  } else if (error.name === 'MongoError' || error.name === 'MongoServerError') {
    statusCode = 500;
    code = 'DATABASE_ERROR';
    message = 'Database operation failed';
  } else if (error.name === 'CastError') {
    statusCode = 400;
    code = 'INVALID_ID';
    message = 'Invalid ID format';
  } else if (error.message.includes('Connection')) {
    statusCode = 503;
    code = 'CONNECTION_ERROR';
    message = 'Database connection failed';
  }

  // Don't expose internal errors in production
  if (process.env['NODE_ENV'] === 'production' && statusCode === 500) {
    message = 'Internal Server Error';
  }

  const errorResponse: ErrorResponse = {
    success: false,
    error: message,
    code,
    timestamp: new Date().toISOString(),
  };

  // Include error details in development
  if (process.env['NODE_ENV'] === 'development') {
    errorResponse.details = {
      stack: error.stack,
      name: error.name,
    };
  }

  res.status(statusCode).json(errorResponse);
};

export const asyncHandler = (fn: Function) => {
  return (req: Request, res: Response, next: NextFunction) => {
    Promise.resolve(fn(req, res, next)).catch(next);
  };
};

export const createError = (message: string, statusCode: number = 500, code?: string): AppError => {
  const error = new Error(message) as AppError;
  error.statusCode = statusCode;
  if (code) error.code = code;
  error.isOperational = true;
  return error;
};
