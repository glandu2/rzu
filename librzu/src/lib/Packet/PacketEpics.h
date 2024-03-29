#pragma once

class packet_version_t {
public:
	packet_version_t(unsigned int version) : version(version) {}
	packet_version_t(packet_version_t const& other) : version(other.version) {}

	int compare(unsigned int epic) const { return (this->version & 0x00FFFFFF) - epic; }
	int compare(packet_version_t other) const { return this->version - other.version; }

	bool operator==(unsigned int epic) const { return compare(epic) == 0; }
	bool operator==(packet_version_t other) const { return compare(other) == 0; }

	bool operator!=(unsigned int epic) const { return !operator==(epic); }
	bool operator!=(packet_version_t other) const { return !operator==(other); }

	bool operator>=(unsigned int epic) const { return compare(epic) >= 0; }
	bool operator>=(packet_version_t other) const { return compare(other) >= 0; }

	bool operator>(unsigned int epic) const { return compare(epic) > 0; }
	bool operator>(packet_version_t other) const { return compare(other) > 0; }

	bool operator<=(unsigned int epic) const { return compare(epic) <= 0; }
	bool operator<=(packet_version_t other) const { return compare(other) <= 0; }

	bool operator<(unsigned int epic) const { return compare(epic) < 0; }
	bool operator<(packet_version_t other) const { return compare(other) < 0; }

	int flag(unsigned int flag) const { return (version >> 24) & flag; }
	unsigned int getAsInt() { return version; }

	packet_version_t& operator=(unsigned int epic) {
		this->version = epic;
		return *this;
	}

	packet_version_t& operator=(packet_version_t other) {
		this->version = other.version;
		return *this;
	}

private:
	unsigned int version;
};

// Epics

#define EPIC_2 0x020100
#define EPIC_3 0x030100
#define EPIC_4_1 0x040100
#define EPIC_4_1_1 0x040101
#define EPIC_5_1 0x050100
#define EPIC_5_2 0x050200
#define EPIC_6_1 0x060100
#define EPIC_6_2 0x060200
#define EPIC_6_3 0x060300
#define EPIC_7_1 0x070100
#define EPIC_7_2 0x070200
#define EPIC_7_3 0x070300
#define EPIC_7_4 0x070400
#define EPIC_8_1 0x080100
#define EPIC_8_1_1_RSA 0x080101
#define EPIC_8_2 0x080200
#define EPIC_8_3 0x080300
#define EPIC_9_1 0x090100
#define EPIC_9_2 0x090200
#define EPIC_9_3 0x090300
#define EPIC_9_4 0x090400
#define EPIC_9_4_AR 0x090401
#define EPIC_9_4_2 0x090402  // newer login packet id
#define EPIC_9_5 0x090500
#define EPIC_9_5_1 0x090501  // 2017/09/26
#define EPIC_9_5_2 0x090502  // GS Version packet modified, 2018 02 27
#define EPIC_9_5_3 0x090503  // GS Version packet modified, 20181211
#define EPIC_9_6 0x090600    // GS Version packet modified, 20190102

// KTS 9.6 20190123 doesn't support the changes in TS_SC_INVENTORY, so remap them as 9.6.1 instead of 9.6
#define EPIC_9_6_1 0x090601

// Defined from FR version on 2020-03-07
// Tested packets: 0, 2, 3, 8, 9, 11, 12, 13, 20, 22, 23, 25, 26, 27, 51, 55, 56, 62, 63, 64, 67, 71, 72, 100, 101, 102,
// 150, 200, 201, 202, 204, 207, 210, 211, 212, 216, 217, 221, 222, 223, 240, 250, 251, 253, 255, 256, 257, 264, 282,
// 283, 286, 287, 301, 303, 305, 324, 351, 400, 401, 402, 403, 404, 406, 407, 500, 503, 505, 507, 508, 509, 511, 514,
// 516, 517, 600, 604, 625, 626, 627, 629, 631, 636, 702, 703, 704, 707, 708, 900, 901, 902, 1000, 1001, 1002, 1003,
// 1004, 1005, 1100, 1101, 1300, 1301, 2004, 2005, 2007, 3000, 3001, 3002, 4250, 4251, 4252, 4253, 4700, 5000, 5001,
// 7000, 7001, 7002, 8000, 9001, 9999, 10000, 10001, 10003, 10004, 10010, 10014, 10021, 10022, 10023, 10024
// Notes: some items have new ID now, which breaks old client that doesn't know how to handle these.
//        also, NPC dialogs are broken: the text is ok but there is no choice available
#define EPIC_9_6_2 0x090602

// Packet ID mapping:
// >= 0 && < 2000: += 1000
// >= 2000 < 3000: += 400
// >= 9000 < 9999: -= 900
// >= 10000: -= 1000
#define EPIC_9_6_3 0x090603  // GS packet ID modified with version 20200713

#define EPIC_9_6_4 0x090604  // GS packet ID modified with version 20200922

#define EPIC_9_6_5 0x090605  // some int16_t fields were replaced with int32_t

#define EPIC_9_6_6 0x090606  // GS packet ID modified with version 20210128

#define EPIC_9_6_7 0x090607  // changes to player and item data

#define EPIC_9_6_8 0x090608  // no changes in packets, but change in how duration_type is handled in auctions messages

#define EPIC_9_7_0 0x090700  // TS_SC_STAT_INFO changed

#define EPIC_9_7_1 0x090701  // GS version changed to 20220210

#define EPIC_9_7_2 0x090702  // Changed wear_info in TS_SC_CHARACTER_LIST and TS_SC_WEAR_INFO and GS version to 20220622

#define EPIC_9_8_0 0x090800  // GS version changed to 20230126, TS_ITEM_AWAKEN_OPTION increased from 5 to 7 stats

#define EPIC_9_8_1 0x090801  // new fields in TS_SC_INVENTORY

// Latest released epic
#define EPIC_LATEST EPIC_9_8_1

// Flags

#define EPIC_FLAG_TESTSERVER 1
