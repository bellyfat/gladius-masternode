#pragma once

#include <GeoLite2PP.hpp>
#include "nanoflann.hpp"
#include <folly/Synchronized.h>

#include "MasternodeConfig.h"
#include "EdgeNode.h"

typedef folly::Synchronized<std::vector
    <std::shared_ptr<EdgeNode>>> LockedNodeList;

using namespace nanoflann;

struct PointCloud {

    std::vector<std::shared_ptr<EdgeNode>> pts;

	// Must return the number of data points
	inline size_t kdtree_get_point_count() const { return pts.size(); }

	// Returns the dim'th component of the idx'th point in the class:
	// Since this is inlined and the "dim" argument is typically an immediate value, the
	//  "if/else's" are actually solved at compile time.
	inline double kdtree_get_pt(const size_t idx, const size_t dim) const {
		if (dim == 0) return pts.at(idx)->getLocation().x;
		else if (dim == 1) return pts.at(idx)->getLocation().y;
		else return pts.at(idx)->getLocation().z;
	}

	template <class BBOX>
	bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }
};

typedef KDTreeSingleIndexAdaptor<L2_Simple_Adaptor<double, PointCloud>,PointCloud,3> kd_tree_t;

struct TreeData {
	PointCloud cloud;
	std::shared_ptr<kd_tree_t> tree;
};

// This class handles installing the maxmind geoip database
// and provides utility methods wrapping the libmaxmind library
// to access the database. This class will also hold a reference
// to a KD tree of edge nodes to facilitate looking up nodes
// that are nearest to a given client.
class Geo {
    public:
		Geo();
        explicit Geo(std::string);
		
        Location lookupCoordinates(std::string);
        std::shared_ptr<TreeData> buildTreeData(const std::vector<std::shared_ptr<EdgeNode>>&);
		void setTreeData(std::shared_ptr<TreeData> tree);
		std::shared_ptr<TreeData> getTree();
		std::vector<size_t> getNearestNodes(Location l, int n);
    private:
        // reference to maxmind geoip database
        std::unique_ptr<GeoLite2PP::DB> db_;
        folly::Synchronized<std::shared_ptr<TreeData>> treeData_;
};


