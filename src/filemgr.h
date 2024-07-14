#pragma once

class FileMgr {
  public:
    FileMgr() = delete;
    FileMgr(FileMgr&) = delete;

    static void ExportIPL(const char* fileName);
    static void ImportIPL(std::string fileName, bool logImports = false);
};

