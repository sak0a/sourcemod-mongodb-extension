/**
 * Database and collection operation routes
 */

import { Router, Request, Response } from 'express';
import { body, param, validationResult } from 'express-validator';
import { ConnectionManager } from '../managers/ConnectionManager';
import { InsertOneRequest, FindRequest, ApiResponse, MongoDocument } from '../types';
import { asyncHandler, createError } from '../middleware/errorHandler';
import { logger } from '../utils/logger';
// import { ObjectId } from 'mongodb'; // Will be used later

const router = Router();

// Validation middleware
const validateConnectionId = [
  param('connectionId').isUUID().withMessage('Invalid connection ID format'),
];

const validateDbCollection = [
  param('db').isString().notEmpty().withMessage('Database name is required'),
  param('coll').isString().notEmpty().withMessage('Collection name is required'),
];

const validateInsertOne = [
  body('document').isObject().withMessage('Document must be an object'),
];

// const validateFind = [
//   body('filter').optional().isObject().withMessage('Filter must be an object'),
//   body('options.limit').optional().isInt({ min: 1, max: 1000 }).withMessage('Limit must be between 1 and 1000'),
//   body('options.skip').optional().isInt({ min: 0 }).withMessage('Skip must be >= 0'),
// ];

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

// Helper to get collection
const getCollection = (req: Request) => {
  const connectionManager: ConnectionManager = req.app.locals['connectionManager'];
  const { connectionId, db, coll } = req.params;

  const connection = connectionManager.getConnection(connectionId!);
  if (!connection) {
    throw createError('Connection not found', 404, 'CONNECTION_NOT_FOUND');
  }

  return connection.client.db(db!).collection(coll!);
};

/**
 * POST /:connectionId/databases/:db/collections/:coll/documents
 * Insert a single document
 */
router.post('/:connectionId/databases/:db/collections/:coll/documents',
  [...validateConnectionId, ...validateDbCollection, ...validateInsertOne],
  asyncHandler(async (req: Request, res: Response) => {
    const validationError = checkValidation(req, res);
    if (validationError) return;

    const collection = getCollection(req);
    const { document }: InsertOneRequest = req.body;

    logger.info('Inserting document', {
      connectionId: req.params['connectionId'],
      database: req.params['db'],
      collection: req.params['coll']
    });

    try {
      const result = await collection.insertOne(document);

      const response: ApiResponse<{ insertedId: string }> = {
        success: true,
        data: {
          insertedId: result.insertedId.toString()
        },
        timestamp: new Date().toISOString(),
      };

      res.status(201).json(response);
    } catch (error) {
      throw createError(
        error instanceof Error ? error.message : 'Insert operation failed',
        500,
        'INSERT_FAILED'
      );
    }
  })
);

/**
 * GET /:connectionId/databases/:db/collections/:coll/documents/findOne
 * Find a single document
 */
router.post('/:connectionId/databases/:db/collections/:coll/documents/findOne',
  [...validateConnectionId, ...validateDbCollection],
  asyncHandler(async (req: Request, res: Response) => {
    const validationError = checkValidation(req, res);
    if (validationError) return;

    const collection = getCollection(req);
    const { filter = {} }: FindRequest = req.body;

    logger.info('Finding document', {
      connectionId: req.params['connectionId'],
      database: req.params['db'],
      collection: req.params['coll'],
      filter
    });

    try {
      const document = await collection.findOne(filter);

      const response: ApiResponse<MongoDocument | null> = {
        success: true,
        data: document ? { ...document, _id: document._id.toString() } : null,
        timestamp: new Date().toISOString(),
      };

      res.json(response);
    } catch (error) {
      throw createError(
        error instanceof Error ? error.message : 'Find operation failed',
        500,
        'FIND_FAILED'
      );
    }
  })
);

/**
 * POST /:connectionId/databases/:db/collections/:coll/documents/find
 * Find multiple documents
 */
router.post('/:connectionId/databases/:db/collections/:coll/documents/find',
  [...validateConnectionId, ...validateDbCollection],
  asyncHandler(async (req: Request, res: Response) => {
    const validationError = checkValidation(req, res);
    if (validationError) return;

    const collection = getCollection(req);
    const { filter = {}, options = {} }: FindRequest = req.body;

    logger.info('Finding documents', {
      connectionId: req.params['connectionId'],
      database: req.params['db'],
      collection: req.params['coll'],
      filter,
      options
    });

    try {
      const cursor = collection.find(filter);

      // Apply options
      if (options.limit) cursor.limit(options.limit);
      if (options.skip) cursor.skip(options.skip);
      if (options.sort) cursor.sort(options.sort);
      if (options.projection) cursor.project(options.projection);

      const documents = await cursor.toArray();

      const response: ApiResponse<MongoDocument[]> = {
        success: true,
        data: documents.map((doc: any) => ({ ...doc, _id: doc._id.toString() })),
        timestamp: new Date().toISOString(),
      };

      res.json(response);
    } catch (error) {
      throw createError(
        error instanceof Error ? error.message : 'Find operation failed',
        500,
        'FIND_FAILED'
      );
    }
  })
);

/**
 * PUT /:connectionId/databases/:db/collections/:coll/documents/updateOne
 * Update a single document
 */
router.put('/:connectionId/databases/:db/collections/:coll/documents/updateOne',
  [...validateConnectionId, ...validateDbCollection],
  asyncHandler(async (req: Request, res: Response) => {
    const validationError = checkValidation(req, res);
    if (validationError) return;

    const collection = getCollection(req);
    const { filter, update, options = {} } = req.body;

    logger.info('Updating document', {
      connectionId: req.params['connectionId'],
      database: req.params['db'],
      collection: req.params['coll'],
      filter
    });

    try {
      const result = await collection.updateOne(filter, update, options);

      const response: ApiResponse<{
        matchedCount: number;
        modifiedCount: number;
        acknowledged: boolean;
        upsertedId?: string;
      }> = {
        success: true,
        data: {
          matchedCount: result.matchedCount,
          modifiedCount: result.modifiedCount,
          acknowledged: result.acknowledged,
          upsertedId: result.upsertedId?.toString(),
        },
        timestamp: new Date().toISOString(),
      };

      res.json(response);
    } catch (error) {
      throw createError(
        error instanceof Error ? error.message : 'Update operation failed',
        500,
        'UPDATE_FAILED'
      );
    }
  })
);

/**
 * PUT /:connectionId/databases/:db/collections/:coll/documents/updateMany
 * Update multiple documents
 */
router.put('/:connectionId/databases/:db/collections/:coll/documents/updateMany',
  [...validateConnectionId, ...validateDbCollection],
  asyncHandler(async (req: Request, res: Response) => {
    const validationError = checkValidation(req, res);
    if (validationError) return;

    const collection = getCollection(req);
    const { filter, update, options = {} } = req.body;

    logger.info('Updating multiple documents', {
      connectionId: req.params['connectionId'],
      database: req.params['db'],
      collection: req.params['coll'],
      filter
    });

    try {
      const result = await collection.updateMany(filter, update, options);

      const response: ApiResponse<{
        matchedCount: number;
        modifiedCount: number;
        acknowledged: boolean;
        upsertedCount: number;
      }> = {
        success: true,
        data: {
          matchedCount: result.matchedCount,
          modifiedCount: result.modifiedCount,
          acknowledged: result.acknowledged,
          upsertedCount: result.upsertedCount || 0,
        },
        timestamp: new Date().toISOString(),
      };

      res.json(response);
    } catch (error) {
      throw createError(
        error instanceof Error ? error.message : 'Update operation failed',
        500,
        'UPDATE_FAILED'
      );
    }
  })
);

/**
 * DELETE /:connectionId/databases/:db/collections/:coll/documents/deleteOne
 * Delete a single document
 */
router.delete('/:connectionId/databases/:db/collections/:coll/documents/deleteOne',
  [...validateConnectionId, ...validateDbCollection],
  asyncHandler(async (req: Request, res: Response) => {
    const validationError = checkValidation(req, res);
    if (validationError) return;

    const collection = getCollection(req);
    const { filter } = req.body;

    logger.info('Deleting document', {
      connectionId: req.params['connectionId'],
      database: req.params['db'],
      collection: req.params['coll'],
      filter
    });

    try {
      const result = await collection.deleteOne(filter);

      const response: ApiResponse<{
        deletedCount: number;
        acknowledged: boolean;
      }> = {
        success: true,
        data: {
          deletedCount: result.deletedCount,
          acknowledged: result.acknowledged,
        },
        timestamp: new Date().toISOString(),
      };

      res.json(response);
    } catch (error) {
      throw createError(
        error instanceof Error ? error.message : 'Delete operation failed',
        500,
        'DELETE_FAILED'
      );
    }
  })
);

/**
 * DELETE /:connectionId/databases/:db/collections/:coll/documents/deleteMany
 * Delete multiple documents
 */
router.delete('/:connectionId/databases/:db/collections/:coll/documents/deleteMany',
  [...validateConnectionId, ...validateDbCollection],
  asyncHandler(async (req: Request, res: Response) => {
    const validationError = checkValidation(req, res);
    if (validationError) return;

    const collection = getCollection(req);
    const { filter } = req.body;

    logger.info('Deleting multiple documents', {
      connectionId: req.params['connectionId'],
      database: req.params['db'],
      collection: req.params['coll'],
      filter
    });

    try {
      const result = await collection.deleteMany(filter);

      const response: ApiResponse<{
        deletedCount: number;
        acknowledged: boolean;
      }> = {
        success: true,
        data: {
          deletedCount: result.deletedCount,
          acknowledged: result.acknowledged,
        },
        timestamp: new Date().toISOString(),
      };

      res.json(response);
    } catch (error) {
      throw createError(
        error instanceof Error ? error.message : 'Delete operation failed',
        500,
        'DELETE_FAILED'
      );
    }
  })
);

/**
 * POST /:connectionId/databases/:db/collections/:coll/documents/count
 * Count documents
 */
router.post('/:connectionId/databases/:db/collections/:coll/documents/count',
  [...validateConnectionId, ...validateDbCollection],
  asyncHandler(async (req: Request, res: Response) => {
    const validationError = checkValidation(req, res);
    if (validationError) return;

    const collection = getCollection(req);
    const { filter = {} } = req.body;

    logger.info('Counting documents', {
      connectionId: req.params['connectionId'],
      database: req.params['db'],
      collection: req.params['coll'],
      filter
    });

    try {
      const count = await collection.countDocuments(filter);

      const response: ApiResponse<{ count: number }> = {
        success: true,
        data: { count },
        timestamp: new Date().toISOString(),
      };

      res.json(response);
    } catch (error) {
      throw createError(
        error instanceof Error ? error.message : 'Count operation failed',
        500,
        'COUNT_FAILED'
      );
    }
  })
);

export { router as databaseRoutes };
