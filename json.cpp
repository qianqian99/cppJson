/*************************************************************************
 > File Name: json.cpp
 > What I should do is fighting !!! 
 > hgg 
 > Created Time: 2016年03月20日 星期日 09时39分41秒
 ************************************************************************/

#include <iostream>
#include <iterator>
#include <list>
#include <algorithm>
#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
enum JsonType{
    String,
    Int,
    Object,
    Array
};
class Json;
class Ctrl_Json{
    public:
        std::string str;
        Json *cppjson;
        static bool first;
    public:
        Ctrl_Json(std::string s) : str(s), cppjson(nullptr){}
        ~Ctrl_Json(){}
        void skip_space();
        Json *parse_json();
        std::string produce_json();
        template <typename T>
        Json *mk_item(T &&v);
        void display() const;
        int &get_int() ;
        std::string &get_string() ;
        std::list<Json *> &get_array() ;
        std::list<std::pair<std::string, Json *>> &get_object();
};
bool Ctrl_Json::first = true;
class Json{
    protected:
        friend  Ctrl_Json;
    public:
        virtual ~Json(){};
        virtual void operator << (Ctrl_Json &c) =0;
        virtual JsonType get_type() const =0 ;
        virtual void show() const=0;
};
class JsonInt : public Json{
    private:
        int data;
        friend Ctrl_Json;
    public:
        JsonInt(int v=0) : data(v){}
        virtual ~JsonInt(){};
        virtual JsonType get_type() const{return Int;}
        void operator << (Ctrl_Json &c) override;
        virtual void show() const override;
};
class JsonString : public Json{
    private:
        std::string data;
        friend  Ctrl_Json;
    public:
        JsonString(std::string s="") : data(s){}
        virtual ~JsonString(){}
        virtual JsonType get_type() const{return String;}
        virtual void show() const;
        std::string string_data(){
            return data;
        }
        void operator << (Ctrl_Json &c) override;
};
class JsonObject : public Json{
    private:
        std::list<std::pair<std::string, Json*>> data;
        friend  Ctrl_Json;
    public:
        virtual ~JsonObject(){}
        virtual JsonType get_type() const{return Object;}
        void operator << (Ctrl_Json &c) override;
        virtual void show() const;
};
class JsonArray : public Json{
    private:
        std::list<Json *> data;
        friend Ctrl_Json;
    public:
        virtual ~JsonArray(){}
        JsonArray()=default;
        virtual JsonType get_type() const{return Array;}
        void operator << (Ctrl_Json &c) override;
        virtual void show() const;
};
////////////////////////////////////////////////////////////
//Simple show 
void JsonInt::show() const{
    std::cout << data << std::endl;
}
void JsonString::show() const {
    std::cout << data << std::endl;
}
void JsonArray::show() const{
    for (auto i : data){
        i->show();
    }
}
void JsonObject::show() const{
    for (auto i=data.begin(); i!=data.end(); ++i) {
        std::cout << i->first << " : ";
        i->second->show();
        std::cout << std::endl;
    }
}
////////////////////////////////////////////////////////////
//Chnage string to value
void JsonObject::operator<<(Ctrl_Json &c){
    char buff[256] = "";
    c.str.erase(c.str.begin());
    char ch = c.str.front();
    std::string tmp;
    while (ch != '}'){
        Json *p = c.parse_json();
        if (p != nullptr) {
            if (String == p->get_type()) {
                tmp = static_cast<JsonString *>(p)->string_data();
            }
        }
        if (ch == ':') {
            c.str.erase(c.str.begin());
            Json *p = nullptr;
            while (p == nullptr)
                p = c.parse_json();
            data.insert(data.end(), {tmp, p});
        }
        ch = c.str.front();
        if (ch == ',') {
            c.str.erase(c.str.begin());
        }
        ch = c.str.front();
    }
    c.str.erase(c.str.begin());
}
void JsonArray::operator<<(Ctrl_Json &c) {
    c.str.erase(c.str.begin());
    char ch = c.str.front();
    while (ch != ']') {
        Json *p = c.parse_json();
        if (p != nullptr) data.push_back(p);
        ch = c.str.front();
        if (ch == ',') {
            c.str.erase(c.str.begin());
            ch = c.str.front();
        }
    }
    c.str.erase(c.str.begin());
}
/////hello
void JsonString::operator<<(Ctrl_Json &c) {
    auto i = c.str.begin() + 1;
    while (i != c.str.end() && (*i != '\"' && *i != '\'')){
        ++i;
    }
    if (*i == '\"' || *i == '\''){
        std::copy(c.str.begin()+1, i, std::back_inserter(data));
        c.str.erase(c.str.begin(), i+1);
    }
}
void JsonInt::operator<<(Ctrl_Json &c) {
    auto i = c.str.begin();
    while (i != c.str.end() && isdigit(*i)) {
        ++i;
    }
    data = stoi(c.str);
    c.str.erase(c.str.begin(), i);
}
/////////////////////////////////////////////
//Total parsing
template <typename T>
Json* Ctrl_Json::mk_item(T &&v){
    Json &p = *new T(v);
    if (Ctrl_Json::first == true) {
        cppjson = &p;
        Ctrl_Json::first = false;
    }
    p << *this;
    return &p;
}
void Ctrl_Json::skip_space(){
    auto i = str.begin();
    while (i!=str.end() && (*i==' ' || *i == '\r' || *i == '\n' || *i == '\t')){
        ++i;
    }
    str.erase(str.begin(), i);
}
Json *Ctrl_Json::parse_json(){
    char ch = str.front();
    if (ch == '{') {
        return mk_item(JsonObject());
    }
    else if (ch == '['){
        return mk_item(JsonArray());
    }
    else if (isdigit(ch)) {
        return mk_item(JsonInt());
    }
    else if (ch == '\"' || ch == '\'') {
        return  mk_item(JsonString());
    }
    else if (ch == ' ' || ch == '\n' || ch == '\r') {
        skip_space();
        return nullptr;
    }
    else {
        return nullptr;
    }
}
/////////////////////////////////////////////////////////
// From here to get data
int &Ctrl_Json::get_int() {
    return static_cast<JsonInt *>(cppjson)->data;
}
std::string &Ctrl_Json::get_string() {
    return static_cast<JsonString *>(cppjson)->data;
}

std::list<Json *> &Ctrl_Json::get_array() {
    return static_cast<JsonArray *>(cppjson)->data;
}
std::list<std::pair<std::string, Json *>> &Ctrl_Json::get_object() {
    return static_cast<JsonObject *>(cppjson)->data;
}
//////////////////////////////////////////////////////////////
void Ctrl_Json::display() const{
    if (cppjson != nullptr){
        cppjson->show();
    }
}
/////////////////////////////////////////////////////////
int main(){
    int fd = open("./file.json", O_RDONLY);
    if (fd != -1){
        char buff[1024] = "";
        read(fd, buff, 1023);
        Ctrl_Json json(buff);
        json.parse_json();
        json.display();
    }
    close(fd);
    return 0;
}
