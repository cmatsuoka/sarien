#ifndef BROWSE_H
#define BROWSE_H

#define BF_SELECTEXISTING 0x00000001

struct EXTIMAGE
{
	LPCTSTR	pExt;
	DWORD	nImageIndex;
};

struct BROWSEFILES
{
	LPCTSTR szCaption;
	DWORD	dwFlags;
	DWORD	nTemplateId;
	LPCTSTR	szStartDir;	  // Can be absolute or relative. '.' is accepted but
						  // not '..'
	LPCTSTR szDefExt;	  // Default extension to add if user did not append one
	LPTSTR	pBuffer;	  // Fully qualified (absolute) filename.
	DWORD	nBufSize;
	LPTSTR  pBufferShort; // Resulting filename without full path or extension.
						  // Can be NULL.
	DWORD	nBufShortSize;
	DWORD	nImageId;
	EXTIMAGE* pExtImages; // The last structure must have NULL in pExt field
						  // nImageIndex from that last one is used for directories.
};

bool BrowseFiles(BROWSEFILES* pbf);

#endif
