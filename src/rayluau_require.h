#pragma once
#include <cstdlib>
#include <cstring>
#include <string>
#include "Luau/Require.h"

class RequireContext {
public:
    RequireContext(std::string root, std::string path) : root(root), path(path) {}
    virtual ~RequireContext() {}
    
    // Shorthand for full path
    std::string fullPath() const { return root + path; }

    // Trims RequireContext.path to where the last '/' is found
    std::string trimPath() const {
        return path.substr(0, path.find_last_of('/') + 1);
    }

    // getters
    std::string getRoot() const { return root; }
    std::string getPath() const { return path; }
    const char* getRoot_c() const { return root.c_str(); }
    const char* getPath_c() const { return path.c_str(); }
    
    // setters
    void setRoot(const std::string& r) { root = r; }
    void setPath(const std::string& p) { path = p; }
    void setRoot_c(const char* r) { root = std::string(r); }
    void setPath_c(const char* p) { path = std::string(p); }

private:
    std::string root;
    std::string path;
};

void initRequireConfig(luarequire_Configuration* config);