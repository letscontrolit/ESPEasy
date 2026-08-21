Import("env")
import os


# ToDo: Use suggested code by Jason2866
# https://github.com/letscontrolit/ESPEasy/issues/4943#issuecomment-1986831198

try:
    from pygit2 import Repository
except ImportError:
    # Activate the Python environment penv and install the requirements there when not yet available
    # Windows is different from other OS-es, assuming Powershell as default shell, not CMD (where separator: & instead of ;)
    if os.name == "nt":
        env.Execute(".\\.platformio\\penv\\Scripts\\activate")
        env.Execute("uv pip install -r requirements.txt")
    else:
        env.Execute(". .platformio/penv/bin/activate ; uv pip install -r requirements.txt")
