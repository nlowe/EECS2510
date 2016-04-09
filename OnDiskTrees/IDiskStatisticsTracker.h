#pragma once

// An interface for keeping track of the number of disk reads and writes
class IDiskStatisticsTracker
{
public:
	// The number of disk write operations performed during the lifetime of this object
	size_t GetWriteCount() const { return writeCount; }

	// The number of disk read operations performed during the lifetime of this object
	size_t GetReadCount() const { return readCount; }

protected:
	size_t writeCount = 0;
	size_t readCount = 0;
};