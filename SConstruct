import subprocess
env = Environment(CPPPATH=["./third_party/include", './include/'],
                  CXX='g++',
                  CXXFLAGS='-O3  -msse4.2 -std=c++11 -fno-builtin-memcmp -g -Wall -Wextra -DNDEBUG -Wno-unused-parameter -Wno-unused-variable -Wno-sign-compare', 
                  LIBPATH=['./third_party/lib'],
                  LIBS=['gflags', 'glog', 'gtest', 'boost_system', 'boost_filesystem', 'boost_regex', 'jemalloc', 'gomp', 'pthread', 'rt', 'tbb'],
                  RPATH=['third_party/lib'])

#`env.Program('test', ["src/tbb_pipeline.cc"])
env.Program('test_queue', ["src/test_queue.cc"])
