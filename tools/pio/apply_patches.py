Import("env")

PLATFORM_DIR = env.PioPlatform().get_dir()
patchflag_path = join(PLATFORM_DIR, ".patching-done")

# patch file only if we didn't do it before
if not isfile(join(PLATFORM_DIR, ".patching-done")):
    original_file = join(PLATFORM_DIR, "builder", "main.py")
    patched_file = join("patches", "gcc-ar-ranlib.patch")

    assert isfile(original_file) and isfile(patched_file)

    env.Execute("patch %s %s" % (original_file, patched_file))

    def _touch(path):
        with open(path, "w") as fp:
            fp.write("")

    env.Execute(lambda *args, **kwargs: _touch(patchflag_path))