#include "RefDataLoader.h"
#include "ReferenceDataMgr.h"

namespace GameServer {

RefDataLoader::RefDataLoader() : refMgr(nullptr) {

}

void RefDataLoader::loadData(ReferenceDataMgr *refMgr) {
    this->refMgr = refMgr;
    load();
}

void RefDataLoader::dataLoaded() {
    refMgr->signalDataLoaded(this);
}

}
