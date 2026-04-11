Import("env")


# ToDo: Use suggested code by Jason2866
# https://github.com/letscontrolit/ESPEasy/issues/4943#issuecomment-1986831198

try:
    from pygit2 import Repository
except ImportError:
    env.Execute("$PYTHONEXE -m pip install -r requirements.txt")
