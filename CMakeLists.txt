cmake_minimum_required(VERSION 3.10)
project(QTree)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-O0 -g -fpic -march=native -Wall")


SET(CMAKE_BUILD_TYPE "Debug")

#3.head file path，头文件目录
INCLUDE_DIRECTORIES(
        src
        /home/jyh/libs/usr/local/include
)
link_directories(/home/jyh/libs/usr/local/lib)
link_libraries("/home/jyh/libs/usr/local/lib/libpfm.so.4")

file(COPY config.cfg DESTINATION ./)

#4.source directory，源文件目录
AUX_SOURCE_DIRECTORY(src DIR_SRCS)

#5.set environment variable，设置环境变量，编译用到的源文件全部都要放到这里，否则编译能够通过，但是执行的时候会出现各种问题，比如"symbol lookup error xxxxx , undefined symbol"
SET(TEST_MATH
        ${DIR_SRCS}
        src/index/QTree.c src/Tool/Tools.c src/Tool/ArrayList.h src/Tool/Arraylist.c)


add_library(query src/query/QueryRange.c src/query/QueryMeta.c src/query/QueryRange.h src/query/QueryMeta.h)
add_library(index src/index/Node.c src/index/QTree.h src/index/QTree.c src/index/Node.c src/index/LeafNode.c src/index/InternalNode.c)
target_link_libraries(index PUBLIC query)

add_library(main  src/main.c)
target_link_libraries(main PUBLIC index -lpthread -lpapi)

#6.add executable file，添加要编译的可执行文件
ADD_EXECUTABLE(${PROJECT_NAME}  ${TEST_MATH})

#add_library(qtree src/Tool/Tools.h src/Tool/Tools.c src/Tool/ArrayList.h src/Tool/Arraylist.c
#        src/query/QueryRange.c src/query/QueryMeta.c src/query/QueryRange.h src/query/QueryMeta.h
#        src/index/Node.c src/index/QTree.h src/index/QTree.c src/index/LeafNode.c src/index/InternalNode.c
#        src/common.h  src/main.c)
#set_target_properties(qtree PROPERTIES output_name "qtree")

#7.add link library，添加可执行文件所需要的库，比如我们用到了libm.so（命名规则：lib+name+.so），就添加该库的名称
#TARGET_LINK_LIBRARIES(${PROJECT_NAME} m)
target_link_libraries(${PROJECT_NAME}  PUBLIC main  -lconfig -lm )

#add_executable(QTree main.cpp src/query/QueryTemplate.cpp include/query/QueryTemplate.h src/query/QueryMeta.cpp include/query/QueryMeta.h include/common.h include/query/Tools.h include/holder/RangeHolder.h include/index/QTree.h include/index/Node.h include/index/LeafNode.h include/index/InternalNode.h src/query/QueryRange.cpp include/query/QueryRange.h)


install(TARGETS ${PROJECT_NAME}
        EXPORT QTreeTargets
        LIBRARY DESTINATION lib  # 动态库安装路径
        ARCHIVE DESTINATION lib  # 静态库安装路径
        RUNTIME DESTINATION bin  # 可执行文件安装路径
        PUBLIC_HEADER DESTINATION include  # 头文件安装路径
        )
#install(src/index/QTree.h DESTINATION include/index )
#install(FILES src/query/QueryMeta.h src/query/QueryRange.h DESTINATION include /query )
