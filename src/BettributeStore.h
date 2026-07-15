#ifndef BETTRIBUTE_STORE_H
#define BETTRIBUTE_STORE_H

#include <SupportDefs.h>

#include <stddef.h>


class BNode;


class BettributeSnapshot {
public:
	BettributeSnapshot();
	~BettributeSnapshot();

	bool HasAttribute() const;
	type_code Type() const;
	const void* Data() const;
	ssize_t Size() const;

private:
	friend class BettributeStore;

	void Reset();

	BettributeSnapshot(const BettributeSnapshot&) = delete;
	BettributeSnapshot& operator=(const BettributeSnapshot&) = delete;

	bool fHasAttribute;
	type_code fType;
	char* fData;
	ssize_t fSize;
};


class BettributeStore {
public:
	static status_t Write(BNode& node, const char* name, type_code type,
		const void* data, size_t size);
	static status_t Capture(const BNode& node, const char* name,
		BettributeSnapshot& snapshot);
	static status_t Restore(BNode& node, const char* name,
		const BettributeSnapshot& snapshot);
};


#endif
