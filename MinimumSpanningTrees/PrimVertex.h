#pragma once
#include "WeightedGraph.h"

struct PrimVertex : Vertex
{
	explicit PrimVertex(Vertex* v) : Vertex(v->ID, v->Key), QKey(INT64_MAX), pi(nullptr) {}

	bool operator==(const PrimVertex& rhs) const { return rhs.Key == Key; }

	int64_t QKey;
	PrimVertex* pi;
};
