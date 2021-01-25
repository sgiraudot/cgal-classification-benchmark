#ifndef FILTERED_RANGE_H
#define FILTERED_RANGE_H

#include <CGAL/Random.h>

#include <boost/iterator/transform_iterator.hpp>

#include "Types.h"

class Filtered_range
{
  typedef boost::transform_iterator<std::function<int(Point_set::Index)>, Point_set::const_iterator> iterator;

  const Point_set& m_points;
  Point_set::Property_map<int> m_label;
  std::vector<bool> m_is_training;
  bool m_training;
  std::function<int(Point_set::Index)> m_transform;

public:

  Filtered_range (Point_set& points, bool training)
    : m_points(points), m_training (training)
  {
    CGAL::Random predicate(0);
    m_is_training.reserve(m_points.size());
    for (std::size_t i = 0; i < m_points.size(); ++ i)
      m_is_training.push_back (predicate.get_double() < 0.1);

    m_label = m_points.property_map<int> ("label").first;

    m_transform = [&](const Point_set::Index& idx) -> int
                  {
                    if (m_is_training[idx] == m_training)
                      return m_label[idx];
                    // else
                    return -1;
                  };
  }

  iterator begin() const { return boost::make_transform_iterator (m_points.begin(), m_transform); }
  iterator end() const { return boost::make_transform_iterator (m_points.end(), m_transform); }

};

#endif // FILTERED_RANGE_H
