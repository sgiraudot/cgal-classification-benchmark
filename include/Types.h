#ifndef TYPES_H
#define TYPES_H

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Classification.h>

typedef CGAL::Simple_cartesian<float> Kernel;
typedef Kernel::Point_3 Point;
typedef CGAL::Point_set_3<Point> Point_set;
typedef typename Point_set::Point_map Point_map;
typedef typename Point_set::Property_map<int> Int_map;
typedef typename Point_set::Property_map<float> Float_map;

namespace Classif = CGAL::Classification;
typedef Classif::Label_handle                                            Label_handle;
typedef Classif::Feature_handle                                          Feature_handle;
typedef Classif::Label_set                                               Label_set;
typedef Classif::Feature_set                                             Feature_set;

typedef Classif::ETHZ_random_forest_classifier Classifier;
typedef Classif::Point_set_feature_generator<Kernel, Point_set, Point_map> Feature_generator;


#endif // TYPES_H
