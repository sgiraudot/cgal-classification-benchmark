#ifndef FILTERED_RANGE_H
#define FILTERED_RANGE_H

#include <CGAL/Random.h>

#include <boost/iterator/transform_iterator.hpp>

#include "Types.h"

class Filtered_range
{
  typedef boost::transform_iterator<std::function<int(Point_set::Index)>, Point_set::const_iterator> iterator;

  const Point_set& points;
  Point_set::Property_map<int> label;
  std::vector<bool> is_training;
  bool training;
  std::function<int(Point_set::Index)> transform;

public:

  Filtered_range (Point_set& points, bool training)
    : points(points), training (training)
  {
    CGAL::Random predicate(0);
    is_training.reserve(points.size());
    for (std::size_t i = 0; i < points.size(); ++ i)
      is_training.push_back (predicate.get_double() < 0.1);

    label = points.property_map<int> ("label").first;

    transform = [&](const Point_set::Index& idx) -> int
                {
                  if (is_training[idx] == training)
                    return label[idx];
                  // else
                  return -1;
                };
  }

  iterator begin() const { return boost::make_transform_iterator (points.begin(), transform); }
  iterator end() const { return boost::make_transform_iterator (points.end(), transform); }

};

#endif // FILTERED_RANGE_H
