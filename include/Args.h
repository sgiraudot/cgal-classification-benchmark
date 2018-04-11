#ifndef ARGS_H
#define ARGS_H

#include "Types.h"

class Args
{
  std::string m_logdir;
  std::string m_input_file;
  std::string m_mem_logfile;
  bool m_small_data_sets;
  std::vector<std::string> m_labels;
  bool m_use_echo;
  bool m_use_colors;
  bool m_use_intensity;
  std::size_t m_nb_trees;
  std::size_t m_max_depth;
  float m_voxel_size;
  std::size_t m_nb_scales;
  mutable boost::shared_ptr<Feature_generator> m_generator;

  struct Color_map
  {
    Point_set::Property_map<unsigned char> red_map;
    Point_set::Property_map<unsigned char> green_map;
    Point_set::Property_map<unsigned char> blue_map;
    bool found;
    
    Color_map (Point_set& points)
    {
      boost::tie (red_map, found) = points.template property_map<unsigned char>("red");
      boost::tie (green_map, found) = points.template property_map<unsigned char>("green");
      boost::tie (blue_map, found) = points.template property_map<unsigned char>("blue");
    }

    friend CGAL::Classification::RGB_Color get (const Color_map& cm, const Point_set::Index& idx)
    {
      CGAL::Classification::RGB_Color out;
      out[0] = cm.red_map[idx]; 
      out[1] = cm.green_map[idx]; 
      out[2] = cm.blue_map[idx];
      return out;
    }
  };

public:
  
  const char* input_filename() const { return m_input_file.c_str(); }
  const char* memory_log_filename() const { return m_mem_logfile.c_str(); }
  std::size_t number_of_trees() const { return m_nb_trees; }
  std::size_t maximum_depth() const { return m_max_depth; }
  float voxel_size() const { return m_voxel_size; }
  std::size_t nb_scales() const { return m_nb_scales; }
  
  Args (int argc, char** argv) : m_small_data_sets (false)
  {
    std::string configfile;
    for (int i = 1; i < argc; ++ i)
    {
      std::string arg (argv[std::size_t(i)]);
      if (arg == "--small")
        m_small_data_sets = true;
      else if (arg.find("--logdir=") == 0)
        m_logdir = std::string (arg.begin() + 9, arg.end());
      else
        configfile = arg;
    }
    if (configfile == "")
    {
      std::cerr << "Error: no config file given." << std::endl;
      exit(-1);
    }
    std::ifstream file(configfile.c_str());
    init (file);
   }

  void fill_labels (Label_set& labels) const
  {
    for (std::size_t i = 0; i < m_labels.size(); ++ i)
      labels.add (m_labels[i].c_str());
  }

  void fill_features (Point_set& points, Feature_set& features) const
  {
    Color_map color_map (points);
    Point_set::Property_map<boost::uint8_t> echo_map;
    bool echo_found;
    boost::tie (echo_map, echo_found)
      = points.template property_map<boost::uint8_t>("number_of_returns");
    
    if (!m_use_colors && !m_use_echo)
      m_generator = boost::make_shared<Feature_generator>
        (features, points, points.point_map(), m_nb_scales,
         CGAL::Default(), CGAL::Default(), CGAL::Default(), m_voxel_size);
    else if (!m_use_colors && m_use_echo)
    {
      if (!echo_found)
      {
        std::cerr << "Error: can't find echo" << std::endl;
        exit(-1);
      }
      m_generator = boost::make_shared<Feature_generator>
        (features, points, points.point_map(), m_nb_scales,
         CGAL::Default(), CGAL::Default(), echo_map, m_voxel_size);
    }
    else if (m_use_colors && !m_use_echo)
    { 
      if (!color_map.found)
      {
        std::cerr << "Error: can't find colors" << std::endl;
        exit(-1);
      }
      m_generator = boost::make_shared<Feature_generator>
        (features, points, points.point_map(), m_nb_scales,
         CGAL::Default(), color_map, CGAL::Default(), m_voxel_size);
    }
    else if (m_use_colors && m_use_echo)
    { 
      if (!echo_found)
      {
        std::cerr << "Error: can't find echo" << std::endl;
        exit(-1);
      }
      if (!color_map.found)
      {
        std::cerr << "Error: can't find colors" << std::endl;
        exit(-1);
      }
      m_generator = boost::make_shared<Feature_generator>
        (features, points, points.point_map(), m_nb_scales,
         CGAL::Default(), color_map, echo_map, m_voxel_size);
    }

    if (m_use_intensity)
    {
      Point_set::Property_map<unsigned short> unsigned_intensity;
      Point_set::Property_map<short> signed_intensity;
      bool unsigned_okay = false;
      bool signed_okay = false;
      
      boost::tie (unsigned_intensity, unsigned_okay)
        = points.property_map<unsigned short>("intensity");
      boost::tie (signed_intensity, signed_okay)
        = points.property_map<short>("intensity");
      if (!unsigned_okay && !signed_okay)
      {
        std::cerr << "Error: can't find intensity" << std::endl;
        exit(-1);
      }
      if (unsigned_okay)
        features.add<CGAL::Classification::Feature::Simple_feature
                     <Point_set, Point_set::Property_map<unsigned short> > >
                     (points, unsigned_intensity, "intensity");
      else
        features.add<CGAL::Classification::Feature::Simple_feature
                     <Point_set, Point_set::Property_map<short> > >
                     (points, signed_intensity, "intensity");
    }
  }

private:
  
  void init (std::ifstream& file)
  {
    std::string line;
    std::istringstream iss;

    std::size_t nb = 0;

    std::string id;
    
    while(getline(file,line))
    {
      ++ nb;
       
      iss.clear();
      iss.str(line);
      std::string tag;
      char equal_sign;      
      if (!(iss >> tag >> equal_sign))
        continue;
      
      if (tag[0] == '#') // Skip comments
        continue;
      
      if (equal_sign != '=')
      {
        std::cerr << "Configuration error (line " << nb << "): expected \" = \" symbol" << std::endl;
        exit(-1);
      }

      if (tag == "LABELS")
      {
        std::string name;
        while (iss >> name)
        {
          if (name[0] == '#')
            break;
          m_labels.push_back (name);
        }
      }
      else if (tag == "FULL_DATA_SET")
      {
        if (!m_small_data_sets)
        {
          if (!(iss >> m_input_file))
          {
            std::cerr << "Configuration error (line "
                      << nb << "): expected input file name" << std::endl;
            exit(-1);
          }
        }
      }
      else if (tag == "SMALL_DATA_SET")
      {
        if (m_small_data_sets)
        {
          if (!(iss >> m_input_file))
          {
            std::cerr << "Configuration error (line "
                      << nb << "): expected input file name" << std::endl;
            exit(-1);
          }
        }
      }
      else if (tag == "ID")
      {
        if (!(iss >> id))
        {
          std::cerr << "Configuration error (line " << nb << "): expected id" << std::endl;
          exit(-1);
        }
      }
      else if (tag == "USE_ECHO")
      {
        std::string boolean;
        if (!(iss >> boolean))
        {
          std::cerr << "Configuration error (line " << nb << "): expected boolean" << std::endl;
          exit(-1);
        }
        if (boolean == "true")
          m_use_echo = true;
        else if (boolean == "false")
          m_use_echo = false;
        else
        {
          std::cerr << "Configuration error (line "
                    << nb << "): expected true or false" << std::endl;
          exit(-1);
        }
      }
      else if (tag == "USE_COLORS")
      {
        std::string boolean;
        if (!(iss >> boolean))
        {
          std::cerr << "Configuration error (line " << nb << "): expected boolean" << std::endl;
          exit(-1);
        }
        if (boolean == "true")
          m_use_colors = true;
        else if (boolean == "false")
          m_use_colors = false;
        else
        {
          std::cerr << "Configuration error (line "
                    << nb << "): expected true or false" << std::endl;
          exit(-1);
        }
      }
      else if (tag == "USE_INTENSITY")
      {
        std::string boolean;
        if (!(iss >> boolean))
        {
          std::cerr << "Configuration error (line " << nb << "): expected boolean" << std::endl;
          exit(-1);
        }
        if (boolean == "true")
          m_use_intensity = true;
        else if (boolean == "false")
          m_use_intensity = false;
        else
        {
          std::cerr << "Configuration error (line "
                    << nb << "): expected true or false" << std::endl;
          exit(-1);
        }
      }
      else if (tag == "NUMBER_OF_TREES")
      {
        if (!(iss >> m_nb_trees))
        {
          std::cerr << "Configuration error (line " << nb << "): expected number of trees" << std::endl;
          exit(-1);
        }
      }
      else if (tag == "MAXIMUM_DEPTH")
      {
        if (!(iss >> m_max_depth))
        {
          std::cerr << "Configuration error (line " << nb << "): expected maximum depth" << std::endl;
          exit(-1);
        }
      }
      else if (tag == "VOXEL_SIZE")
      {
        if (!(iss >> m_voxel_size))
        {
          std::cerr << "Configuration error (line " << nb << "): expected voxel size" << std::endl;
          exit(-1);
        }
      }
      else if (tag == "NUMBER_OF_SCALES")
      {
        if (!(iss >> m_nb_scales))
        {
          std::cerr << "Configuration error (line " << nb << "): expected number of scales" << std::endl;
          exit(-1);
        }
      }
      else
        std::cerr << "Configuration warning (line " << nb << "): unknown tag \"" << tag << "\"" << std::endl;
    }

    if (id == "")
    {
      std::cerr << "Error: no id" << std::endl;
      exit(-1);
    }

    m_mem_logfile = m_logdir + "/memory_" + id + ".log";
  }
};

#endif // ARGS_H
