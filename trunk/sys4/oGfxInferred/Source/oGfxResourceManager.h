// $(header)
#pragma once
#ifndef oGfxResourceManager_h
#define oGfxResourceManager_h

interface oGfxLoader
{
	Open the file
	Read the header
	Allocate space/object for the file
	Chunk read in the file

	Close the file
};

interface oGfxResourceManager
{
	virtual oStringID Load(const char* _URIReference, oGfxResource::TYPE _Type);
	
	virtual oStringID GetID(const oGfxResource* _pResource) const threadsafe = 0;
	virtual const char* GetURIReference(oStringID _StringID) const threadsafe = 0;
	virtual const char* GetSourceURI(oStringID _StringID) const threadsafe = 0;
	virtual const char* GetCacheURI(oStringID _StringID) const threadsafe = 0;
	virtual oGfxResource* GetResource(oStringID _StrindID) const threadsafe = 0;
};

oAPI bool oGfxCreateResourceManager(oGfxDevice* _pDevice, oGfxResourceManager** _ppManager);

#endif
