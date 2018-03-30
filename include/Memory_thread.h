#ifndef MEMORY_THREAD_H
#define MEMORY_THREAD_H

#include <chrono>
#include <thread>

#include <CGAL/Memory_sizer.h>

class Memory_thread
{
private:
  struct Content
  {
    std::size_t max_size;
    bool running;
    std::ofstream log_file;

    Content (const char* log_file_name) 
      : max_size (0), running (true), log_file (log_file_name)
    {
    }
  };

  boost::shared_ptr<Content> m_content;

public:
  Memory_thread (const char* log_file_name)
    : m_content (new Content(log_file_name))
  { }

  std::size_t max_size() const { return m_content->max_size; }
  void stop() { m_content->running = false; }

  void operator()()
  {
    std::chrono::seconds dura (1);
    while (m_content->running)
    {
      std::this_thread::sleep_for (dura);
      std::size_t size = CGAL::Memory_sizer().virtual_size();
      m_content->max_size = std::max (size, m_content->max_size);
      m_content->log_file << (size >> 20) << std::endl;
    }
  }
};

#endif // MEMORY_THREAD_H
