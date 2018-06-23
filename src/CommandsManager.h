#include <map>
#define INPUT_COMMAND_SIZE          80

// Forward declarations.
typedef bool (*ExecuteCommandCall)(struct EventStruct *event, const char* line);

class CommandsManager
{
    private:    
    struct char_cmp { 
        bool operator () (const char *a,const char *b) const 
        {
            int ca, cb;
            do
            {
                ca = (unsigned char) *a++;
                cb = (unsigned char) *b++;
                ca = tolower(toupper(ca));
                cb = tolower(toupper(cb));
            } while (ca == cb && ca != '\0');
            return ca - cb < 0;
        }
    };
    std::map<const char *,ExecuteCommandCall,char_cmp> commandsRegistry;
    public:
    bool Register(const char * id, ExecuteCommandCall command);
    bool Execute(const char * id, struct EventStruct *event, const char* line);
};
