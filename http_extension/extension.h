/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod HTTP MongoDB Extension
 * Copyright (C) 2024 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#ifndef _INCLUDE_SOURCEMOD_HTTP_MONGODB_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_HTTP_MONGODB_EXTENSION_PROPER_H_

#include "smsdk_ext.h"
#include <memory>

// Forward declarations
class HTTPClient;
class JSONStructureManager;
class MongoDBAPILayer;

/**
 * @brief HTTP MongoDB Extension for SourceMod
 * 
 * This extension provides MongoDB functionality through HTTP API calls,
 * allowing 32-bit SourceMod servers to use MongoDB without requiring
 * 32-bit MongoDB drivers.
 */
class HTTPMongoDBExtension : public SDKExtension
{
public:
	/**
	 * @brief This is called after the initial loading sequence has been processed.
	 *
	 * @param error		Error message buffer.
	 * @param maxlength	Size of error message buffer.
	 * @param late		Whether or not the module was loaded after map load.
	 * @return			True to succeed, false to fail.
	 */
	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
	
	/**
	 * @brief This is called right before the extension is unloaded.
	 */
	virtual void SDK_OnUnload();
	
	/**
	 * @brief This is called once all known extensions have been loaded.
	 * Note: It is is a good idea to add natives here, if any are provided.
	 */
	virtual void SDK_OnAllLoaded();

	/**
	 * @brief Called when the pause state is changed.
	 */
	virtual void SDK_OnPauseChange(bool paused);

	/**
	 * @brief this is called when Core wants to know if your extension is working.
	 *
	 * @param error		Error message buffer.
	 * @param maxlength	Size of error message buffer.
	 * @return			True if working, false otherwise.
	 */
	virtual bool QueryRunning(char *error, size_t maxlength);

	/**
	 * @brief Called when the extension is loaded by SourceMod.
	 *
	 * @param me		Extension interface.
	 * @param sys		Share system interface.
	 * @param error		Error message buffer.
	 * @param maxlength	Size of error message buffer.
	 * @param late		Whether or not the extension was loaded late.
	 * @return			True to succeed, false to fail.
	 */
	virtual bool OnExtensionLoad(IExtension *me, IShareSys *sys, char *error, size_t maxlength, bool late);

public:
#if defined SMEXT_CONF_METAMOD
	/**
	 * @brief Called when Metamod is attached, before the extension version is called.
	 *
	 * @param error			Error buffer.
	 * @param maxlength		Maximum length of error buffer.
	 * @param late			Whether or not Metamod considers this a late load.
	 * @return				True to succeed, false to fail.
	 */
	virtual bool SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlength, bool late);

	/**
	 * @brief Called when Metamod is detaching, after the extension version is called.
	 * NOTE: By default this is blocked unless sent from SourceMod.
	 *
	 * @param error			Error buffer.
	 * @param maxlength		Maximum length of error buffer.
	 * @return				True to succeed, false to fail.
	 */
	virtual bool SDK_OnMetamodUnload(char *error, size_t maxlength);

	/**
	 * @brief Called when Metamod's pause state is changing.
	 * NOTE: By default this is blocked unless sent from SourceMod.
	 *
	 * @param paused		Pause state being set.
	 * @param error			Error buffer.
	 * @param maxlength		Maximum length of error buffer.
	 * @return				True to succeed, false to fail.
	 */
	virtual bool SDK_OnMetamodPauseChange(bool paused, char *error, size_t maxlength);
#endif

public:
	// Getters for managers
	HTTPClient* GetHTTPClient() const { return m_pHTTPClient.get(); }
	JSONStructureManager* GetJSONManager() const { return m_pJSONManager.get(); }
	MongoDBAPILayer* GetMongoAPI() const { return m_pMongoAPI.get(); }

private:
	std::unique_ptr<HTTPClient> m_pHTTPClient;
	std::unique_ptr<JSONStructureManager> m_pJSONManager;
	std::unique_ptr<MongoDBAPILayer> m_pMongoAPI;
	
	bool m_bInitialized;
	std::string m_lastError;
};

extern HTTPMongoDBExtension g_HTTPMongoDBExtension;

#endif // _INCLUDE_SOURCEMOD_HTTP_MONGODB_EXTENSION_PROPER_H_
