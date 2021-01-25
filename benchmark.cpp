#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#define CGAL_CLASSIFICATION_VERBOSE
#define DO_NOT_USE_EIGEN_FEATURES
#define DO_NOT_USE_HSV_FEATURES

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Classification.h>
#include <CGAL/Point_set_3.h>
#include <CGAL/Point_set_3/IO.h>
#include <CGAL/random_simplify_point_set.h>
#include <CGAL/Real_timer.h>

#include "Types.h"
#include "Args.h"
#include "Memory_thread.h"
#include "Filtered_range.h"


int main (int argc, char** argv)
{
  srand(0);

  Args args (argc, argv);
  Memory_thread mem (args.memory_log_filename());
  std::thread mem_thread (mem);

  std::ifstream input_file (args.input_filename());
  Point_set points;
  input_file >> points;
  std::cout << points.size() << " points read" << std::endl;

  Int_map gt_labels;
  bool gt_info;
  boost::tie (gt_labels, gt_info) = points.property_map<int>("label");
  if (!gt_info)
  {
    std::cout << "No ground truth info" << std::endl;
    return EXIT_FAILURE;
  }

  CGAL::Real_timer t;
  t.start();

  Label_set labels;
  args.fill_labels (labels);
  Feature_set features;
  args.fill_features (points, features);

  std::cout << "Features: " << std::endl;

  for (std::size_t i = 0; i < features.size(); ++ i)
    std::cerr << "#" << i << ": " << features[i]->name() << std::endl;

  std::cout << "Classification on " << labels.size() << " labels and "
            << features.size() << " features" << std::endl;
  Classifier classifier (labels, features);

  std::cout << "Training with " << args.number_of_trees()
            << " and max depth of " << args.maximum_depth() << std::endl;

  CGAL::Real_timer t_atom;
  t_atom.start();

  Filtered_range training_set (points, true);
  Filtered_range test_set (points, false);

  std::size_t nb_training = 0;
  std::size_t nb_test = 0;

  for (const int& l : training_set)
    if (l != -1)
      ++ nb_training;
  for (const int& l : test_set)
    if (l != -1)
      ++ nb_test;
  std::cout << nb_training << " points selected for training" << std::endl
            << nb_test << " points selected for testing" << std::endl;

  classifier.train (training_set, true, args.number_of_trees(), args.maximum_depth());

  t_atom.stop();
  std::cout << "Done in " << t_atom.time() << " second(s)" << std::endl;
  t_atom.reset();

  std::cout << "Classifying" << std::endl;
  t_atom.start();

  std::vector<int> result (points.size(), -1);
  Classif::classify<CGAL::Parallel_tag> (points, labels, classifier, result);
  t_atom.stop();
  std::cout << "Done in " << t_atom.time() << " second(s)" << std::endl;
  t_atom.reset();

  std::cout << "| __Label__ | __Precision__ | __Recall__ | __IoU__ |" << std::endl;
  std::cout << "|-----------|---------------|------------|---------|" << std::endl;
  Classif::Evaluation eval (labels, test_set, result);

  for (std::size_t i = 0; i < labels.size(); ++ i)
  {
    std::cout << "| __\"" << labels[i]->name() << "\"__ | "
              << eval.precision(labels[i]) << " | "
              << eval.recall(labels[i]) << " | "
              << eval.intersection_over_union(labels[i]) << " |" << std::endl;
  }

  std::cout << "Accuracy = " << eval.accuracy() << std::endl
            << "Mean F1 score = " << eval.mean_f1_score() << std::endl
            << "Mean IoU = " << eval.mean_intersection_over_union() << std::endl;

  mem.stop();
  mem_thread.join();

  t.stop();

  std::cout << "Timing = " << t.time() << std::endl
            << "Memory = " << mem.max_size() << std::endl;

  return EXIT_SUCCESS;
}
