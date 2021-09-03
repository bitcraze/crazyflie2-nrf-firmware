#!/usr/bin/env python

import argparse
import os
import subprocess

version = {}


def extract_information_from_git(base):
    print(f"{base}")
    revision = (
        subprocess.check_output(["git", "-C", base, "rev-parse", "HEAD"])
        .decode("utf-8")
        .strip()
    )

    version["revision"] = revision[0:12]
    version["irevision0"] = "0x" + revision[0:8]
    version["irevision1"] = "0x" + revision[8:12]
    version["productionRelease"] = "false"

    try:
        identify = subprocess.check_output(
            ["git", "-C", base, "describe", "--abbrev=12", "--tags", "HEAD"]
        ).decode("utf-8")
        identify = identify.split("-")

        if len(identify) > 2:
            version["local_revision"] = identify[len(identify) - 2]
        else:
            version["local_revision"] = "0"

        version["tag"] = identify[0]
        for x in range(1, len(identify) - 2):
            version["tag"] += "-"
            version["tag"] += identify[x]
    except subprocess.CalledProcessError:
        # We are maybe running from a shallow tree
        version["local_revision"] = "0"
        version["tag"] = "NA"

    version["tag"] = version["tag"].strip()

    if version["local_revision"] != "0":
        version["tag"] = version["tag"] + " +" + version["local_revision"]

    branch = (
        subprocess.check_output(
            ["git", "-C", base, "rev-parse", "--abbrev-ref", "HEAD"]
        )
        .decode("utf-8")
        .strip()
    )
    version["branch"] = branch

    subprocess.call(["git", "-C", base, "update-index", "-q", "--refresh"])
    changes = (
        subprocess.check_output(
            ["git", "-C", base, "diff-index", "--name-only", "HEAD", "--"]
        )
        .decode("utf-8")
        .strip()
    )
    if len(changes):
        version["modified"] = "true"
    else:
        version["modified"] = "false"


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--crazyflie-base",
        help="base folder of the crazyflie firmware",
        action="store",
        default="./",
    )
    parser.add_argument(
        "--output", help="name of output header", action="store", default="Include/version.h"
    )
    args = parser.parse_args()

    extract_information_from_git(args.crazyflie_base)

    header = (
        "#pragma once\n\n"
        f'#define V_SLOCAL_REVISION "{version["local_revision"]}"\n'
        f'#define V_SREVISION "{version["revision"]}"\n'
        f'#define V_STAG "{version["tag"]}"\n'
        f'#define V_MODIFIED {version["modified"]}\n'
    )

    with open(os.path.join(args.crazyflie_base, args.output), "w") as f:
        f.write(header)
