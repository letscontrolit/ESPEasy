#include <functional>
typedef struct s_fileinfo
{
  String Name;
  int Size;
  bool isDirectory = false;
} fileInfo;

typedef std::function<bool(fileInfo)> HandlerFileInfo;
