#!/usr/bin/ruby

require 'colorize'

def apply(cmd)
  puts " * COMMAND: #{cmd}".blue.bold
  system "#{cmd}"
  puts ""
end

$date = `date +%Y-%m-%d_%Hh%Mm%Ss`.chop

$mode = "full"

ARGV.each do |a|
  if a == "--small"
    $mode = "small"
  else
    $date = $date + "_" + a
  end
end

puts "[CLASSIFICATION BENCHMARK] Using #{$mode} data sets\n".green.bold

if $mode == "full"
  $mode = ""
  $date = "full_" + $date
else
  $mode = "--small"
  $date = "small_" + $date
end

puts "[COMPILING PROGRAMS]".red.bold

apply "cd build && make -j 6 && cd .."

puts "[RUNNING PROGRAMS]".red.bold

apply "mkdir log/#{$date}"
$logdir = "log/#{$date}"
$report = File.open("#{$logdir}/report.md", "w")
$raw = File.open("#{$logdir}/raw_report.csv", "w")

if $mode == "--small"
  $report.write("# Classification benchmark #{$date} (small)\n\n")
else
  $report.write("# Classification benchmark #{$date}\n\n")
end

$overview = "| __Data set__ | __Accuracy__ | __Mean IoU__ | __Timing (s)__ | __Memory (B)__ |\n"
$overview = $overview + "|--------------|--------------|--------------|----------------|----------------|\n"

first = true
Dir["config/*"].each do |file|
  $id = `grep \"ID = \" #{file}`.split("ID = ")[1].chop
  $logfile = "#{$logdir}/output_#{$id}.log"

  $report.write("## Data set \"#{$id}\"\n\n")

  apply "./build/benchmark #{$mode} --logdir=#{$logdir} #{file} >> #{$logfile} 2>&1"

  apply "gnuplot -c plot_memory.gnuplot \"#{$logdir}/memory_#{$id}.log\" \"#{$logdir}/memory_#{$id}.svg\""

  $report.write("![](memory_#{$id}.svg)\n\n")
  
  $acc = `cat #{$logfile} | grep Accuracy`.split(" = ")[1].chop.to_f
  $iou = `cat #{$logfile} | grep IoU`.split(" = ")[1].chop.to_f
  $time = `cat #{$logfile} | grep Timing`.split(" = ")[1].chop.to_f
  $mem = `cat #{$logfile} | grep Memory`.split(" = ")[1].chop.to_i

  $array = `cat #{$logfile} | grep \"|\"`
  $report.write("#{$array}")

  puts " -> #{$id}: accuracy = #{$acc} ; mean IoU =  #{$iou} ; time = #{$time} ; mem = #{$mem}"
  
  $report.write(" * __Accuracy:__ #{$acc}\n")
  $report.write(" * __Mean IoU:__ #{$iou}\n")
  $report.write(" * __Timing (s):__ #{$time}\n")
  $report.write(" * __Memory (B):__ #{$mem}\n\n")
  $overview = $overview + "| __#{$id}__ | #{$acc} | #{$iou} | #{$time} | #{$mem} |\n"
  if !first
    $raw.write(",")
  end
  $raw.write("#{$acc},#{$iou},#{$time},#{$mem}")
  first = false
end

$raw.write("\n")
$report.write("## Overview\n\n#{$overview}\n")
$report.close
apply "pandoc #{$logdir}/report.md -o #{$logdir}/report.html"
