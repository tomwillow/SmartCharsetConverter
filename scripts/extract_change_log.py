import argparse
import os
import re
import sys

# 设置 Python 的默认编码为 utf-8
os.environ['PYTHONIOENCODING'] = 'utf-8'

if hasattr(sys.stdout, 'reconfigure'):
    sys.stdout.reconfigure(encoding='utf-8')  # this works
else:
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

SELF_DIR = os.path.split(os.path.realpath(__file__))[0].replace("\\", "/")
SELF_BASENAME = os.path.split(os.path.realpath(__file__))[1]
REPO_DIR = os.path.abspath(f"{SELF_DIR}/..").replace("\\", "/")

CHANGE_LOG_FILENAME = f"{REPO_DIR}/build/CHANGELOG.txt"
README_FILENAME_CN = f"{REPO_DIR}/README.md"
README_FILENAME_EN = f"{REPO_DIR}/README-en.md"
VERSION_START_STR_CN = "# 版本记录"
VERSION_START_STR_EN = "# Version"

def ParseArguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tag", required=True)
    args = parser.parse_args()
    print(f"[{SELF_BASENAME}] args: {args}")
    return args

def SearchTag(filename: str, tag: str, versionStartStr: str) -> str:
    with open(filename, "r", encoding="utf-8") as f:
        lines = f.readlines()
    pos = 0
    found = False
    for i in range(len(lines)):
        line = lines[i]
        if line.find(versionStartStr) != -1:
            pos = i
            found = True
            break
    if not found:
        raise RuntimeError(f"version start string \"{versionStartStr}\" not found at {filename}")
    
    found = False
    for i in range(pos, len(lines)):
        line = lines[i]
        if line.startswith(tag):
            pos = i
            found = True
            break
    if not found:
        raise RuntimeError(f"tag \"{tag}\" not found at {filename}")
    
    ret = [line.strip()]
    pos += 1
    for i in range(pos, len(lines)):
        line = lines[i]
        if line == "\n" or line == "\r\n":
            break
        if re.match(r"v\d+\.\d+.*", line):
            break
        ret.append(line.strip())
    return "\n".join(ret)


def main(args: argparse.Namespace):
    cnContent = SearchTag(README_FILENAME_CN, args.tag, VERSION_START_STR_CN)
    enContent = SearchTag(README_FILENAME_EN, args.tag, VERSION_START_STR_EN)

    changeLog = ""
    changeLog += cnContent
    changeLog += "\n"
    changeLog += enContent
    print(f"[{SELF_BASENAME}] CHANGELOG.txt:")
    print("=" * 30)
    print(changeLog)
    print("=" * 30)
    with open(CHANGE_LOG_FILENAME, "w", encoding="utf-8") as f:
        f.write(changeLog)

if __name__ == "__main__":
    args = ParseArguments()
    main(args)
