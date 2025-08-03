/**
 * TarStream : Create/receive a .tar file while streaming via http webserver
 * Copyright (c) 2023.. Ton Huisman for ESPEasy
 * Code is inspired by this example: https://github.com/esp8266/Arduino/issues/3966#issuecomment-351850298
 *
 * Changelog:
 * 2024-01-10 tonhuisman: Fix handling of 0-byte files (next files where shifted 1 byte forward for each 0-byte file)
 * 2023-08-27 tonhuisman: Add explicit check for / in filename, to avoid subdirectory/file to overwrite file (subdir is ignored by SPIFFS)
 *                        Check if file exists before trying to delete it, avoiding unneeded error log messages
 *                        Add link to the code that inspired this class
 * 2023-08-26 tonhuisman: Code improvements and de-duplication
 * 2023-08-24 tonhuisman: Implement streaming in a .tar storing all regular files in the chosen storage (Flash or SD), replacing existing
 *                        files, Flash: adding/replacing only if there is at least 2 blocks of storage available
 * 2023-08-23 tonhuisman: Implement streaming out a .tar via the read() method
 * 2023-08-19 tonhuisman: Initial setup
 */

#ifndef HELPERS_TAR_STREAM_H
#define HELPERS_TAR_STREAM_H

#include "../../ESPEasy_common.h"

#if FEATURE_TARSTREAM_SUPPORT

# include "../Helpers/ESPEasy_Storage.h"

# include <FS.h>
# include <vector>

// This are internal features only, to be used when debugging the code
# define TAR_STREAM_DEBUG    0               // Include/exclude some logging
# define TAR_STREAM_PEEK     0               // Include/exclude peek()
# define TAR_LOG_LEVEL_DEBUG LOG_LEVEL_DEBUG // Can use DEBUG or INFO

/* tar Header Block, from POSIX 1003.1-1990.  */

// Source: https://www.gnu.org/software/tar/manual/html_node/Standard.html

/* POSIX header.  */

struct posix_header       /* Using POSIX Tar-definitions for accuracy */
{                         /* byte offset */
  char name[100];         /*   0 */
  char mode[8];           /* 100 */
  char uid[8];            /* 108 */
  char gid[8];            /* 116 */
  char size[12];          /* 124 */
  char mtime[12];         /* 136 */
  char chksum[8];         /* 148 */
  char typeflag;          /* 156 */
  char linkname[100];     /* 157 */
  char magic[6];          /* 257 */
  char version[2];        /* 263 */
  char uname[32];         /* 265 */
  char gname[32];         /* 297 */
  char devmajor[8];       /* 329 */
  char devminor[8];       /* 337 */
  char prefix[155];       /* 345 */
                          /* 500 */
};

# define TMAGIC   "ustar" /* ustar and a null */
# define TMAGLEN  6
# define TVERSION "00"    /* 00 and no null */
# define TVERSLEN 2

/* Values used in typeflag field.  */
# define REGTYPE  '0'   /* regular file */
# define AREGTYPE '\0'  /* regular file */
# define LNKTYPE  '1'   /* link */
# define SYMTYPE  '2'   /* reserved */
# define CHRTYPE  '3'   /* character special */
# define BLKTYPE  '4'   /* block special */
# define DIRTYPE  '5'   /* directory */
# define FIFOTYPE '6'   /* FIFO special */
# define CONTTYPE '7'   /* reserved */

# define XHDTYPE  'x'   /* Extended header referring to the next file in the archive */
# define XGLTYPE  'g'   /* Global extended header */

/* Bits used in the mode field, values in octal.  */
# define TSUID    04000 /* set UID on execution */
# define TSGID    02000 /* set GID on execution */
# define TSVTX    01000 /* reserved */
                        /* file permissions */
# define TUREAD   00400 /* read by owner */
# define TUWRITE  00200 /* write by owner */
# define TUEXEC   00100 /* execute/search by owner */
# define TGREAD   00040 /* read by group */
# define TGWRITE  00020 /* write by group */
# define TGEXEC   00010 /* execute/search by group */
# define TOREAD   00004 /* read by other */
# define TOWRITE  00002 /* write by other */
# define TOEXEC   00001 /* execute/search by other */

# define TAR_HEADER_EXPECTED_SIZE 500
constexpr size_t TAR_HEADER_SIZE = sizeof(posix_header);
static_assert(TAR_HEADER_SIZE == TAR_HEADER_EXPECTED_SIZE, "TarStream: posix_header invalid size");
# undef TAR_HEADER_EXPECTED_SIZE

constexpr size_t TAR_BLOCK_SIZE = 512u;

struct TarFileInfo_struct {
  TarFileInfo_struct() {}

  TarFileInfo_struct(const String fname,
                     size_t       fsize);

  String fileName;
  size_t fileSize; // Actual file size in bytes
  size_t tarSize;  // File size rounded up to the next 512 byte block (tar block-size)
};

enum TarStreamState_e : uint8_t {
  Initial = 0u,  // Initial state
  ReadingHeader, // Processing a file header
  ReadingFile,   // Reading a file
  ReadingSlack,  // Reading the slack space after the file data is processed
  ReadingFinal,  // Reading the empty block after the last file
  WritingHeader, // Writing the header data
  WritingFile,   // Writing the file data
  WritingSlack,  // Writing nothing, receiving the slack space
  WritingFinal,  // Writing nothing, receiving the final block
  WritingDone,   // We're done, just skip any incoming bytes
  Error,
};

class TarStream : public Stream {
public:

  TarStream();
  TarStream(const String& fileName);
  TarStream(const String& fileName,
            FileDestination_e destination);
  virtual ~TarStream();

  virtual size_t      write(uint8_t ch);

  virtual size_t      write(const uint8_t *buf,
                            size_t         size);
  virtual int         available();
  virtual int         read();
  virtual int         peek();
  virtual void        flush();
  virtual size_t      size();
  virtual const char* name();

  bool                addFileIfExists(const String& fileName); // Check file and add to list if exists
  bool                addFile(const String& fileName,
                              size_t        fileSize);         // Add a file to the list and update _tarSize
  bool                isFileIncluded(const String& filename);  // Is this file included?
  size_t              getFileCount() const;                    // Actual number of files in the achive when uploading
  size_t              getFilesSizes() const {                  // Actual size of all files in bytes
    return _filesSizes;
  }

private:

  void     clearHeader();
  void     setupHeader();
  bool     validateHeader();
  uint32_t clearAndCalculateHeaderChecksum();

  union {
    posix_header _tarHeader;                                  // Header
    uint8_t      _tarData[TAR_HEADER_SIZE]{};                 // Access to the tarheader data
  };
  std::vector<TarFileInfo_struct>_filesList;                  // List of files
  String _fileName;                                           // Archive name
  size_t _tarSize        = 0u;                                // Total size of the .tar file
  size_t _tarRemaining   = 0u;                                // Remaining size of the .tar file (read)
  size_t _filesSizes     = 0u;                                // Total sizes of all files
  size_t _tarPosition    = 0u;                                // Current position in the _tarSize (read)
  size_t _writePosition  = 0u;                                // Current file written bytes
  size_t _headerPosition = 0u;                                // Offset into the current header
  int _fileIndex         = -1;                                // Current file in _filesList during write actions
  std::vector<TarFileInfo_struct>::iterator _currentIterator;
  TarFileInfo_struct _currentIndex;                           // File we're currently reading
  FileDestination_e _destination = FileDestination_e::ANY;    // Where to write the files
  TarStreamState_e _streamState  = TarStreamState_e::Initial; // Current stream state
  fs::File _currentFile;                                      // The file currently being handled
};

#endif // if FEATURE_TARSTREAM_SUPPORT

#endif // ifndef HELPERS_TAR_STREAM_H
