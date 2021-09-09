#pragma once
#define RAD_TO_DEG(x) x*-57.2958f

class FileMgr
{
public:
    FileMgr() = delete;
    FileMgr(FileMgr&) = delete;

    static void ExportIPL(const char* fileName);
    static void ImportIPL(std::string fileName);
};

