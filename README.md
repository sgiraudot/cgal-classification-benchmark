# CGAL Classification Benchmark

The CGAL Classification package is in constant evolution and we need
to keep track of where we are / where we are going in terms of quality
of output and of computation time.

## Principle

 1. Read point cloud
 2. Compute features
 3. Select 10% of points are training set, 90% as test set
 4. Train
 5. Apply training
 6. Evaluate ground truth, get accuracy and IoU
 7. Get timing and memory usage
 8. Generate a report

When trying to add a feature or to change something in the package, we
just need to apply the same benchmark again and to compare the results
to check if there is an improvement (or if, at least, there is no
regression).

## How it works

 * Each files in `config/` is used as one test. A template is
   given. Basically, a configuration file will contain the name of the
   input file to be tested + some parameters.

 * The input file must be a `PLY` file that contains an `int` property
   called `label` that contains the number of the assigned label (for
   example, if my labels are `ground`, `vegetation` and `buildings`,
   points belonging to the ground are assigned a `label` value of `0`,
   points belonging to the trees a value of `1`, and building points a
   value of `2`).
 
 * The script generates some files in `log/[full/small]_[date]_[id]`
   where `[date]` contains the date + hours of when the script is
   launched and `[id]` is an optional user-specified
   identifier. Mainly, it generates a web page `report.html` that can
   be seen in a web browser. It shows the memory usage along time, the
   classifiction performances for each label and the overall
   evaluation (accuracy, intersection over union, etc.)
 
## Dependencies

The benchmark was created and only tested using GNU/Linux. It might
work on other platforms, but I haven't tested. It depends on the
following:

 * CGAL 4.12
 * ruby (with the package `colorize`)
 * gnuplot
 * pandoc

## Running the benchmark

 1. Put your config files in `config/` and put the data sets where the
    config files say they are
 2. Run `ruby run.rb` or `ruby run.rb --small` to run the benchmark on
    the small data sets. You can run `ruby run.rb --small my_test` to
    identify your tests with `my_test`.
 3. If all goes well, a detailed report is generated in
    `log/[full/small]_[date]_[id]/report.html`

For example, if I run `ruby run.rb new_feature` on the 13th of April
in the morning, my log files will be saved in a directory such as
`log/full_2018-04-13_11h08m32s_new_feature`.
