#ifdef QT_GUI 

#pragma once

#include <unordered_map>
#include <string>

struct infos
{
    std::string pid;
    std::string name;
};

class Ps
{

public:
    Ps();
    virtual ~Ps();
    void read_file_system_linux(std::string &, std::string &, long, bool = false);
    void ps_proc(std::unordered_map<std::string, infos> &umap);
};

#endif