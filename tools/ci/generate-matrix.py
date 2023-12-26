# ref. https://github.blog/changelog/2020-04-15-github-actions-new-workflow-features/
#
# This script is expected to print out something like this to supply the next Actions step with environments to build in parallel:
# ::set-output name=matrix::{"include": [{"chip": "esp8266", "env": "custom_ESP8266_4M1M"}, {"chip": "esp32", "env": "custom_ESP32_4M316k"}]}
#
# These are the same environment names as with tools/build_ESPeasy.sh (i.e. all of them)

import json
import os
import re
from platformio.project.config import ProjectConfig


def get_jobs(cfg):
    regex = re.compile(r".*(ESP[^_]*).*")
    for env in cfg.envs():
        platform = cfg.get("env:{}".format(env), "platform")
        match = regex.match(env)
        if type(match) == re.Match:
            typ = match.group(1)
            if "ESP8285" == typ:
                typ = "ESP8266" # Generalize ESP8285 into ESP8266
        else:
            typ = "ESP8266" # Catch WROOM02 and some other 'hard_' builds
        if "espressif8266" in platform:
            yield {"chip": typ.lower(), "env": env}
        elif "espressif32" in platform:
            yield {"chip": typ.lower(), "env": env}
        else:
            raise ValueError(
                "Unknown `platform = {}` for `[env:{}]`".format(platform, env)
            )


def filter_jobs(jobs, ignore=("spec_",)):
    for job in jobs:
        if job["env"].startswith(ignore):
            continue

        yield job


if __name__ == "__main__":
    jobs = list(filter_jobs(get_jobs(ProjectConfig.get_instance())))

    sort = []
    for job in jobs:
        if job["chip"] == "esp8266":
            sort.append(job["env"])
        sort.sort(key=str.casefold)

    serialized = json.dumps({"include": jobs})
    with open(os.environ['GITHUB_OUTPUT'], 'a') as fh:
        print("matrix={}".format(serialized), file=fh)
