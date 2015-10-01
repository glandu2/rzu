#ifndef REFDATALOADER_H
#define REFDATALOADER_H

#include "Core/Object.h"

namespace GameServer {

class ReferenceDataMgr;

class RefDataLoader : public Object
{
public:
	RefDataLoader();

	void loadData(ReferenceDataMgr* refMgr);
	virtual void load() = 0;

protected:
	void dataLoaded();

private:
	ReferenceDataMgr* refMgr;
};

}

#endif // REFDATALOADER_H
