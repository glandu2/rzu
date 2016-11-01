#ifndef AUCTIONUID_H
#define AUCTIONUID_H

#include <stdint.h>

struct AuctionUid {
	explicit AuctionUid(const uint32_t t_) : uid(t_) {};
	AuctionUid(){};
	AuctionUid(const AuctionUid & uid_) : uid(uid_.uid){}
	AuctionUid & operator=(const AuctionUid & rhs) { uid = rhs.uid; return *this; }
	bool operator==(const AuctionUid & rhs) const { return uid == rhs.uid; }
	bool operator!=(const AuctionUid & rhs) const { return uid != rhs.uid; }
	uint32_t get() const { return uid; }

private:
	uint32_t uid;

};

#endif
