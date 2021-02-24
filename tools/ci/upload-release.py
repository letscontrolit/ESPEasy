import os
import pathlib
from itertools import chain
from github import Github


class NoZipFilesFound(Exception):
    pass


def find_zip(directory):
    return pathlib.Path(directory).glob("**/*.zip")


if __name__ == "__main__":
    # Binaries/ - .elf + .bin
    # artifact/ - docs, upload tools, etc.
    archives = [
        str(zip_file) for zip_file in chain(find_zip("Binaries"), find_zip("artifact"))
    ]
    if not archives:
        raise NoZipFilesFound

    tag = os.environ["GITHUB_REF"][len("refs/tags/") :]
    message = os.environ.get("RELEASE_NOTES", tag)

    print("Prepared for tag={} and archives={}".format(tag, archives))

    gh = Github(os.environ["GITHUB_TOKEN"])
    repo = gh.get_repo(os.environ["GITHUB_REPOSITORY"])

    release = repo.create_git_release(tag=tag, name=tag, message=message)

    for path in archives:
        release.upload_asset(path)
