#ifndef _INCLUDE_PHYSFS_HPP_
#define _INCLUDE_PHYSFS_HPP_

/*

The code below is from the following fork of PhysFS++: https://github.com/rh101/physfs-cpp/tree/fix-write-stream-issue

License:

   Copyright (c) 2013 Kevin Howell and others.

   This software is provided 'as-is', without any express or implied warranty.
   In no event will the authors be held liable for any damages arising from
   the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software in a
   product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source distribution.

       Kevin Howell <kevin.adam.howell@gmail.com>

This library wraps the C library PhysicsFS, written by Ryan C. Gordon and
others. See http://icculus.org/physfs/. Its LICENSE (from the time of writing)
is reproduced below:

Copyright (c) 2001-2022 Ryan C. Gordon <icculus@icculus.org> and others.

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

*/

#include <physfs.h>
#include <string>
#include <vector>
#include <iostream>

namespace PhysFS {

    typedef enum {
        READ,
        WRITE,
        APPEND
    } mode;

    using std::string;

    typedef std::vector<string> StringList;

    typedef PHYSFS_uint8 uint8;

    typedef PHYSFS_sint8 sint8;

    typedef PHYSFS_uint16 uint16;

    typedef PHYSFS_sint16 sint16;

    typedef PHYSFS_uint32 uint32;

    typedef PHYSFS_sint32 sint32;

    typedef PHYSFS_uint64 uint64;

    typedef PHYSFS_sint64 sint64;

    typedef PHYSFS_StringCallback StringCallback;

    typedef PHYSFS_EnumerateCallback EnumFilesCallback;

    typedef PHYSFS_Version Version;

    typedef PHYSFS_Allocator Allocator;

    typedef PHYSFS_ArchiveInfo ArchiveInfo;

    typedef std::vector<ArchiveInfo> ArchiveInfoList;

    typedef PHYSFS_Stat Stat;

    typedef uint64 size_t;

    class base_fstream {
    protected:
        PHYSFS_File* file;
    public:
        base_fstream(PHYSFS_File* file);
        virtual ~base_fstream();
        size_t length();

        virtual void open(std::string const& filename, mode openMode) = 0;
        virtual void close();
    };

    class ifstream : public base_fstream, public std::istream {
    public:
        explicit ifstream(string const& filename);
        virtual ~ifstream();

        void open(std::string const& filename, mode openMode = READ) override;
        void close() override;
    };

    class ofstream : public base_fstream, public std::ostream {
    public:
        explicit ofstream(string const& filename, mode writeMode = WRITE);
        virtual ~ofstream();

        void open(std::string const& filename, mode openMode = WRITE) override;
        void close() override;
    };

    class fstream : public base_fstream, public std::iostream {
    public:
        explicit fstream(string const& filename, mode openMode = READ);
        virtual ~fstream();

        void open(std::string const& filename, mode openMode = READ) override;
        void close() override;
    };

    Version getLinkedVersion();

    void init(char const* argv0);

    void deinit();

    ArchiveInfoList supportedArchiveTypes();

    string getDirSeparator();

    void permitSymbolicLinks(bool allow);

    StringList getCdRomDirs();

    void getCdRomDirs(StringCallback callback, void* extra);

    string getBaseDir();

    string getPrefDir(const string& org, const string& app);

    string getWriteDir();

    void setWriteDir(string const& newDir);

    StringList getSearchPath();

    void getSearchPath(StringCallback callback, void* extra);

    void setSaneConfig(string const& orgName, string const& appName, string const& archiveExt, bool includeCdRoms, bool archivesFirst);

    int mkdir(string const& dirName);

    int deleteFile(string const& filename);

    string getRealDir(string const& filename);

    StringList enumerateFiles(string const& directory);

    int enumerateFiles(string const& directory, EnumFilesCallback callback, void* extra);

    bool exists(string const& filename);

    Stat getStat(string const& filename);

    bool isDirectory(string const& filename);

    bool isSymbolicLink(string const& filename);

    sint64 getLastModTime(string const& filename);

    bool isInit();

    bool symbolicLinksPermitted();

    void setAllocator(Allocator const* allocator);

    void mount(string const& newDir, string const& mountPoint, bool appendToPath);

    void unmount(string const& oldDir);

    string getMountPoint(string const& dir);

    namespace Util {

        sint16 swapSLE16(sint16 value);

        uint16 swapULE16(uint16 value);

        sint32 swapSLE32(sint32 value);

        uint32 swapULE32(uint32 value);

        sint64 swapSLE64(sint64 value);

        uint64 swapULE64(uint64 value);

        sint16 swapSBE16(sint16 value);

        uint16 swapUBE16(uint16 value);

        sint32 swapSBE32(sint32 value);

        uint32 swapUBE32(uint32 value);

        sint64 swapSBE64(sint64 value);

        uint64 swapUBE64(uint64 value);

        string utf8FromUcs4(uint32 const* src);

        string utf8ToUcs4(char const* src);

        string utf8FromUcs2(uint16 const* src);

        string utf8ToUcs2(char const* src);

        string utf8FromLatin1(char const* src);

    }

}

#endif /* _INCLUDE_PHYSFS_HPP_ */
