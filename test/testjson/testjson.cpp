/*
json序列化示例
*/

#include "json.hpp"

#include <iostream>
#include <string>

using json = nlohmann::json;

// json序列化函数
void func() {
    json js;
    js["name"] = "zhang san";
    js["msg"]["zhang san"] = "hello world";
    js["msg"]["liu shuo"] = "hello china";
    js["yc_msg"] = {{"yc1", "hello world1"}, {"yc2", "hello world2"}};

    std::cout << js << std::endl;

    std::string js_str = js.dump();
    std::cout << js_str << std::endl;
}

int main() {
    func();

    return 0;
}