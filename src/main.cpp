#include "ver.h"
#include "MapReduce.h"
#include "Mapper.h"
#include "Reducer.h"

#include <iostream>
#include <string>

int main(int argc, char const *argv[])
{
  std::cout << "yamr version: "
            << ver_major() << "."
            << ver_minor() << "."
            << ver_patch() << std::endl;

  if(argc != 4) {
    std::cerr << "Wrong number of arguments (expected 2). Usage: yamr <src> <mnum> <rnum>. \n" << std::endl;
    return EXIT_FAILURE;
  }

  std::string filename = argv[1];

  auto mnum = std::strtoll(argv[2], nullptr, 0);
  if(mnum <= 0) {
    std::cerr << "mnum must be integer greater than 0.\n";
    return EXIT_FAILURE;
  }

  auto rnum = std::strtoll(argv[3], nullptr, 0);
  if(rnum <= 0) {
    std::cerr << "rnum must be integer greater than 0.\n";
    return EXIT_FAILURE;
  }

  try {
    mr::MapReduce<mr::Mapper, mr::Reducer> map_reduce(filename, static_cast<size_t>(mnum), static_cast<size_t>(rnum));
    map_reduce.process();
  } catch(std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
