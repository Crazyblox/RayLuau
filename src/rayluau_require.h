#pragma once
#include <cstdlib>
#include <cstring>
#include <string>
#include "Luau/Require.h"

class RequireContext {
public:
    // RequireContext(std::string path, bool Sandbox) : root(root), path(path) {}
    RequireContext(std::string inputPath, bool Sandbox) : root(""), path(inputPath) {
        if (Sandbox)
        {   // Treat 'inputPath' as hard root boundary so filesystem access can't occur outside of the given project.
            root = path;
            path = "";
        }
    }
    virtual ~RequireContext() {}
    
    // Shorthand for full path
    std::string fullPath() const { return root + path; }

    // Trims RequireContext.path to where the last '/' is found
    std::string trimPath() const {
        return path.substr(0, path.find_last_of('/') + 1);
    }

    std::string Child(std::string childName) const {
        return path + "/" + childName;
    }

    // Traverses path up by 1
    std::string Parent() const {
        size_t lastSlash = path.find_last_of('/');
        if (lastSlash == std::string::npos) return "";
        return path.substr(0, lastSlash);
    }

    // getters
    std::string getRoot() const { return root; }
    std::string getPath() const { return path; }
    const char* getRoot_c() const { return root.c_str(); }
    const char* getPath_c() const { return path.c_str(); }
    
    // setters (Root is intended to be immutable wrt sandboxing, so we don't provide functions for it.)
    void setPath(const std::string& p) { path = p; }
    void setPath_c(const char* p) { path = std::string(p); }

private:
    std::string root;
    std::string path;
};

void initRequireConfig(luarequire_Configuration* config);