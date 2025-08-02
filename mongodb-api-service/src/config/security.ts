import crypto from 'crypto';
import bcrypt from 'bcrypt';
import jwt from 'jsonwebtoken';

export interface SecurityConfig {
  apiKeys: {
    [keyName: string]: {
      hash: string;
      permissions: string[];
      description: string;
      createdAt: string;
      expiresAt?: string;
    };
  };
  jwtSecret: string;
  encryptionKey: string;
  rateLimiting: {
    windowMs: number;
    maxRequests: number;
    slowDown: {
      windowMs: number;
      delayAfter: number;
      delayMs: number;
    };
  };
  allowedOrigins: string[];
  requireHttps: boolean;
  maxRequestSize: string;
}

export const defaultSecurityConfig: SecurityConfig = {
  apiKeys: {
    // Default API key for SourceMod extension
    'sourcemod-default': {
      hash: '', // Will be generated
      permissions: ['read', 'write', 'admin'],
      description: 'Default SourceMod Extension API Key',
      createdAt: new Date().toISOString(),
    },
  },
  jwtSecret: process.env['JWT_SECRET'] || crypto.randomBytes(64).toString('hex'),
  encryptionKey: process.env['ENCRYPTION_KEY'] || crypto.randomBytes(32).toString('hex'),
  rateLimiting: {
    windowMs: 15 * 60 * 1000, // 15 minutes
    maxRequests: 1000, // Limit each IP to 1000 requests per windowMs
    slowDown: {
      windowMs: 15 * 60 * 1000, // 15 minutes
      delayAfter: 100, // Allow 100 requests per windowMs without delay
      delayMs: 500, // Add 500ms delay per request after delayAfter
    },
  },
  allowedOrigins: [
    'http://localhost:3000',
    'http://127.0.0.1:3000',
    'http://localhost:3300',
    'http://127.0.0.1:3300',
  ],
  requireHttps: process.env['NODE_ENV'] === 'production',
  maxRequestSize: '10mb',
};

export class SecurityManager {
  private config: SecurityConfig;

  constructor(config: SecurityConfig = defaultSecurityConfig) {
    this.config = config;
    this.initializeDefaultApiKey();
  }

  private async initializeDefaultApiKey(): Promise<void> {
    const defaultKey = process.env['SOURCEMOD_API_KEY'] || 'sourcemod-mongodb-extension-2024';
    const hash = await bcrypt.hash(defaultKey, 12);
    if (this.config.apiKeys['sourcemod-default']) {
      this.config.apiKeys['sourcemod-default'].hash = hash;
    }
  }

  async generateApiKey(name: string, permissions: string[], description: string, expiresInDays?: number): Promise<string> {
    const apiKey = crypto.randomBytes(32).toString('hex');
    const hash = await bcrypt.hash(apiKey, 12);
    
    const keyConfig: any = {
      hash,
      permissions,
      description,
      createdAt: new Date().toISOString(),
    };

    if (expiresInDays) {
      keyConfig.expiresAt = new Date(Date.now() + expiresInDays * 24 * 60 * 60 * 1000).toISOString();
    }

    this.config.apiKeys[name] = keyConfig;
    return apiKey;
  }

  async validateApiKey(apiKey: string): Promise<{ valid: boolean; keyName?: string; permissions?: string[] }> {
    for (const [keyName, keyConfig] of Object.entries(this.config.apiKeys)) {
      // Check if key is expired
      if (keyConfig.expiresAt && new Date(keyConfig.expiresAt) < new Date()) {
        continue;
      }

      const isValid = await bcrypt.compare(apiKey, keyConfig.hash);
      if (isValid) {
        return {
          valid: true,
          keyName,
          permissions: keyConfig.permissions,
        };
      }
    }

    return { valid: false };
  }

  generateJWT(payload: any, expiresIn: string = '24h'): string {
    return jwt.sign(payload, this.config.jwtSecret, { expiresIn } as jwt.SignOptions);
  }

  verifyJWT(token: string): any {
    try {
      return jwt.verify(token, this.config.jwtSecret);
    } catch (error) {
      return null;
    }
  }

  encrypt(text: string): string {
    // Simple base64 encoding for now (can be enhanced later)
    return Buffer.from(text).toString('base64');
  }

  decrypt(encryptedText: string): string {
    try {
      return Buffer.from(encryptedText, 'base64').toString('utf8');
    } catch {
      return encryptedText; // Return as-is if decoding fails
    }
  }

  hashPassword(password: string): Promise<string> {
    return bcrypt.hash(password, 12);
  }

  verifyPassword(password: string, hash: string): Promise<boolean> {
    return bcrypt.compare(password, hash);
  }

  getConfig(): SecurityConfig {
    return { ...this.config };
  }

  updateConfig(newConfig: Partial<SecurityConfig>): void {
    this.config = { ...this.config, ...newConfig };
  }
}

export const securityManager = new SecurityManager();
