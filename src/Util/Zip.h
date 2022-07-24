#pragma once

#include <unzip.h>
#include <AUI/IO/AFileInputStream.h>

namespace unzip {

    struct File {
        unzFile unz;

        zlib_filefunc_def fileFuncs;
        File(_<AFileInputStream> stream): mStream(std::move(stream)) {
            fileFuncs.opaque = this;
            fileFuncs.zopen_file = openFileFunc;
            fileFuncs.zread_file = readFileFunc;
            fileFuncs.zwrite_file = writeFileFunc;
            fileFuncs.ztell_file = tellFileFunc;
            fileFuncs.zseek_file = seekFileFunc;
            fileFuncs.zclose_file = closeFileFunc;
            fileFuncs.zerror_file = errorFileFunc;

            unz = unzOpen2("tmp", &fileFuncs);
        }
        ~File() {
            unzClose(unz);
        }
        operator unzFile() const {
            return unz;
        }

    private:
        _<AFileInputStream> mStream;

        static voidpf openFileFunc (voidpf opaque, const char* filename, int mode) {
            return opaque;
        }
        static uLong readFileFunc (voidpf opaque, voidpf stream, void* buf, uLong size) {
            return reinterpret_cast<File*>(opaque)->mStream->read(static_cast<char*>(buf), size);
        }
        static uLong writeFileFunc (voidpf opaque, voidpf stream, const void* buf, uLong size) {
            return -1;
        }
        static long tellFileFunc (voidpf opaque, voidpf stream) {
            return reinterpret_cast<File*>(opaque)->mStream->tell();
        }
        static long seekFileFunc (voidpf opaque, voidpf stream, uLong offset, int origin) {
            AFileInputStream::Seek originImpl;
            switch (origin)
            {
                case ZLIB_FILEFUNC_SEEK_CUR :
                    originImpl = AFileInputStream::Seek::CURRENT;
                    break;
                case ZLIB_FILEFUNC_SEEK_END :
                    originImpl = AFileInputStream::Seek::END;
                    break;
                case ZLIB_FILEFUNC_SEEK_SET :
                    originImpl = AFileInputStream::Seek::BEGIN;
                    break;
                default: return -1;
            }
            reinterpret_cast<File*>(opaque)->mStream->seek(offset, originImpl);
            return 0;
        }
        static int closeFileFunc (voidpf opaque, voidpf stream) {
            return 0;
        }
        static int errorFileFunc(voidpf opaque, voidpf stream) {
            return 0;
        }

    };
}