import os
import enum
import subprocess


class CannotArchive(Exception):
    pass


class UnhandledEvent(Exception):
    pass


def cmd(*, env, pio_can_fail):
    built = False
    try:
        subprocess.check_call(["platformio", "run", "-e", env])
        built = True
    except subprocess.CalledProcessError:
        if not pio_can_fail:
            raise

    output = "build_output"
    dirs = [os.path.join(output, "bin"), os.path.join(output, "debug")]

    # Notice that we also have build_output/reject containing .elf that cannot
    # be made into a flashable .bin

    archive = "ESPEasy_{}.zip".format(env)
    if built:
        if not all([os.path.isdir(d) for d in dirs]):
            raise CannotArchive(
                "Built, but build_output does not have expected directories"
            )
        cmd = ["zip", "-q", "-rr", archive]
        cmd.extend(dirs)

        subprocess.check_call(cmd)


if __name__ == "__main__":
    ref = os.environ["GITHUB_REF"]
    event = os.environ["GITHUB_EVENT_NAME"]

    # allow release build to fail and not produce an artifact
    opts = {"env": os.environ["ENV"], "pio_can_fail": False}

    if event == "push" and "/tags/" in ref:
        opts["pio_can_fail"] = True
    elif event == "push" or event == "pull_request":
        opts["pio_can_fail"] = False
    else:
        raise UnhandledEvent("Cannot handle GITHUB_EVENT_NAME={}".format(event))

    cmd(**opts)
