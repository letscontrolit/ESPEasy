In order to be able to build in Windows, we need to concatenate all .cpp files 
in some folders to reduce the number of compiled object files.
This is needed due to the limitation of Windows to have a command line longer than 32767 characters.
The linker command was becoming longer than this maximum.

However the DataStructs folder does have at least one templated .cpp file.
Since you may need to include a .cpp file if it contains a templated class, 
the DataStruct folder could not be concatenated into a single .cpp file.

Therefore the templated class(es) will be stored in a folder postfixed with 
_templ


