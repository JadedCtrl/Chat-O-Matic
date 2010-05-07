#ifndef VCardManager_H_
#define VCardManager_H_

#include <Message.h>
#include <Path.h>
#include <Directory.h>

class JabberHandler;
class JabberPresence; 
class JabberContact;
class JabberVCard;
	
class VCardManager 
{
	public:
		VCardManager(JabberHandler* jabberHandler);
		
	protected:
	friend class JabberHandler;
		
		void	RefinePresence(JabberPresence*);
		void	VCardReceived(JabberContact*);
		
	private:
	
		void	SaveCache();
		
		JabberHandler*	fJabberHandler;
		BMessage	fCache;
		BPath		fCachePath;
		BDirectory	fCacheFolder;
		
};
#endif
