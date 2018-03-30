#ifndef FILTERED_RANGE_H
#define FILTERED_RANGE_H

#include <CGAL/Random.h>

#include "Types.h"

class Filtered_range
{
  Point_set& points;
  mutable CGAL::Random predicate;
  Point_set::Property_map<int> label;
  bool training;

public:

  Filtered_range (Point_set& points, bool training)
    : points (points), predicate (0), training (training)
  {
    label = points.property_map<int> ("label").first;
  }

  std::size_t size() const { return points.size(); }

  int operator[] (const std::size_t& idx) const
  {
    bool t = (predicate.get_double() < 0.1);
    if (training == t)
      return label[idx];
    return -1;
  }
};

#endif // FILTERED_RANGE_H
