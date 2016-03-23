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
        template <typename T>
        Json *mk_item(T &&v);
        void display() const;
        Json *get_json_ptr() {return cppjson;}
};
bool Ctrl_Json::first = true;
class Json{
        friend  Ctrl_Json;
    public:
        virtual ~Json(){};
        virtual void operator << (Ctrl_Json &c) =0;
        virtual JsonType get_type() const =0 ;
        virtual void *get_data() =0;
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
        virtual void *get_data() {return &data;};
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
        virtual void *get_data() {return &data;};
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
        virtual void *get_data() {return &data;};
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
        virtual void *get_data() {return &data;};
};
////////////////////////////////////////////////////////////
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
void JsonObject::operator<<(Ctrl_Json &c){
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
    else if (ch == ':'){
        return nullptr;
    }
    else {
        std::cout << "error\n" << std::endl;
        exit(-1);
    }
}
void Ctrl_Json::display() const{
    if (cppjson != nullptr){
        cppjson->show();
    }
}
//////////////////////////////////////////////////
class Factory{
    private:
        std::string res;
    public:
        Factory() : res(""){}
        ~Factory(){}
        void produce_json(Json *);
        std::string &get_res(){return res;}
        void deal_object(Json *);
        void deal_array(Json *);
};

void Factory::produce_json(Json *ptr){
    switch(ptr->get_type()){    
        case Int:  
            res += std::to_string(*static_cast<int *>(ptr->get_data())); 
            break;
        case String: res += "\""; 
                     res += *(static_cast<std::string *>(ptr->get_data()));
                     res += "\""; 
                     break;
        case Object:deal_object(ptr);break;
        case Array:deal_array(ptr);break;
        default:break;
    }
}
void Factory::deal_array(Json *ptr){
    res += "[";    
    auto _list = static_cast<std::list<Json *>*>(ptr->get_data());
    for (auto e : *_list){
        produce_json(e);
        res += ",";
    }
    res += "]";
}
void Factory::deal_object(Json *ptr){
    res += "{";
    auto _list = static_cast<std::list<std::pair<std::string, Json *>> *>(ptr->get_data());
    for (auto i=_list->begin(); i!=_list->end(); ++i){
        res += "\"";
        res += i->first;
        res += "\"";
        res += ":";
        produce_json(i->second);
        res += ",";
    }
    res.pop_back();
    res += "}";
}
//////////////////////////////////////////////////////
int main(){
    int fd = open("./file.json", O_RDONLY);
    if (fd != -1){
        char buff[1024] = "";
        read(fd, buff, 1023);
        Ctrl_Json json(buff);
        json.parse_json();
        Factory fa;
        fa.produce_json(json.get_json_ptr());
        json.display();
        std::cout << fa.get_res() << std::endl;
    }
    close(fd);
    return 0;
}
