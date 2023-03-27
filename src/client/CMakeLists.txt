# 将该目录下的所有源文件名赋值给SRC_LIST变量
aux_source_directory(. SRC_LIST)

# 生成可执行函数
add_executable(ChatClient ${SRC_LIST})
# 绑定库
target_link_libraries(ChatClient pthread)