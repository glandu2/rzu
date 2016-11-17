#ifndef IAUCTIONDATA_H
#define IAUCTIONDATA_H

#include <stdint.h>
#include "Extern.h"
#include "AuctionUid.h"
#include "Core/Object.h"

class RZAUCTION_EXTERN IAuctionData : public Object {
	DECLARE_CLASS(IAuctionData)
public:
	IAuctionData(AuctionUid uid, int32_t category, uint64_t timeMin, uint64_t timeMax)
	    : uid(uid), category(category), timeMin(timeMin), timeMax(timeMax) {}
	virtual ~IAuctionData() {}

	virtual bool outputInPartialDump() = 0;

	virtual bool isInFinalState() const = 0;

	AuctionUid getUid() const { return uid; }
	uint32_t getCategory() const { return category; }
	uint64_t getTimeMin() const { return timeMin; }
	uint64_t getTimeMax() const { return timeMax; }

	void setTimeMin(uint64_t time) {
		timeMin = time;
	}
protected:
	void updateTime(uint64_t time) {
		timeMax = time;
	}

	void advanceTime() {
		timeMin = timeMax;
	}

private:
	AuctionUid uid;
	int32_t category;

	uint64_t timeMin;
	uint64_t timeMax;
};

#endif
