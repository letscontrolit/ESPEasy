Import("env")

import os


def get_github_actions_git_tag():
    ref = os.environ.get("GITHUB_REF", "")
    if ref and ref.startswith("refs/tags/"):
        return ref[len("refs/tags/") :]

    return ""


if "true" == os.environ.get("CI", "false"):
    defines = [
        "CONTINUOUS_INTEGRATION",
        ("BUILD_GIT", '\\\"{}\\\"'.format(get_github_actions_git_tag())),
    ]
else:
    defines = [("BUILD_GIT", '\\\"\\\"')]

env.Append(CPPDEFINES=defines)
