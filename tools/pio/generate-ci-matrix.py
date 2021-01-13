import json
from platformio.project.config import ProjectConfig

def get_jobs(cfg):
    for env in cfg.envs():
        platform = cfg.get("env:{}".format(env), "platform")
        if "espressif8266" in platform:
            yield {"chip": "esp8266", "env": env}
        elif "espressif32" in platform:
            yield {"chip": "esp32", "env": env}
        else:
            raise ValueError("Unknown `platform = {}` for `[env:{}]`".format(platform, env))

def filter_jobs(jobs, ignore=("spec_",)):
    for job in jobs:
        if job["env"].startswith(ignore):
            continue

        yield job

jobs = filter_jobs(get_jobs(ProjectConfig.get_instance()))

# XXX for testing

def get_one_of_each(jobs):
    seen = set()
    for job in jobs:
        if not job["chip"] in seen:
            seen.add(job["chip"])
            yield job

# ref. https://github.blog/changelog/2020-04-15-github-actions-new-workflow-features/
# we need to echo something like this:
# ::set-output name=matrix::{"include": [{"chip": "esp8266", "env": "custom_ESP8266_4M1M"}, {"chip": "esp32", "env": "custom_ESP32_4M316k"}]}

fmt = "::set-output name=matrix::{}"
out = fmt.format(json.dumps({"include": list(get_one_of_each(jobs))}))

print(out)

# XXX for testing

# ... Normal run will dump the whole jobs list as-is ...
