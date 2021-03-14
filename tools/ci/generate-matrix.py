import json
from platformio.project.config import ProjectConfig

# This will select the same environments as tools/build_ESPeasy.sh


def get_jobs(cfg):
    for env in cfg.envs():
        platform = cfg.get("env:{}".format(env), "platform")
        if "espressif8266" in platform:
            yield {"chip": "esp8266", "env": env}
        elif "espressif32" in platform:
            yield {"chip": "esp32", "env": env}
        else:
            raise ValueError(
                "Unknown `platform = {}` for `[env:{}]`".format(platform, env)
            )


def filter_jobs(jobs, ignore=("spec_",)):
    for job in jobs:
        if job["env"].startswith(ignore):
            continue

        yield job


# ref. https://github.blog/changelog/2020-04-15-github-actions-new-workflow-features/
# we need to echo something like this:
# ::set-output name=matrix::{"include": [{"chip": "esp8266", "env": "custom_ESP8266_4M1M"}, {"chip": "esp32", "env": "custom_ESP32_4M316k"}]}

if __name__ == "__main__":
    jobs = list(filter_jobs(get_jobs(ProjectConfig.get_instance())))

    sort = []
    for job in jobs:
        if job["chip"] == "esp8266":
            sort.append(job["env"])
        sort.sort(key=str.casefold)

    serialized = json.dumps({"include": jobs})
    print("::set-output name=matrix::{}".format(serialized))
