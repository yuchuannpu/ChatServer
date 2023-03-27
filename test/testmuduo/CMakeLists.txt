# g++ -o -g server xxx.cpp -I/xxx -L/xxx -lxxx
# g++ -o -g server muduo_server.cpp -lmuduo_net -lmuduo_base -lpthread
cmake_minimum_required(VERSION 3.0)
project(testmuduo)

# 配置编译选项
# set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 配置需要编译的源文件
set(SRC_LIST ./main.cpp)
# aux_source_directory(. SRC_LIST)

# 配置头文件搜索路径
# include_directories()

# 配置库文件搜索路径
# link_directories()

# 生成库文件
# add_library()

# 设置可执行文件的输出路径、生成可执行文件
add_executable(server ${SRC_LIST})
# 要链接的库
target_link_libraries(server muduo_net muduo_base pthread)