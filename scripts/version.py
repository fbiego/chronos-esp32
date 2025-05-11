import re
import json
import sys
from pathlib import Path

def extract_version_from_properties(file_path):
    with open(file_path, "r") as f:
        for line in f:
            if line.strip().startswith("version="):
                return line.strip().split("=", 1)[1]
    raise ValueError("Version not found in library.properties")

def extract_version_from_json(file_path):
    with open(file_path, "r") as f:
        data = json.load(f)
        return data.get("version")

def extract_version_from_header(file_path):
    content = Path(file_path).read_text()
    major = re.search(r"#define\s+CHRONOSESP_VERSION_MAJOR\s+(\d+)", content)
    minor = re.search(r"#define\s+CHRONOSESP_VERSION_MINOR\s+(\d+)", content)
    patch = re.search(r"#define\s+CHRONOSESP_VERSION_PATCH\s+(\d+)", content)
    if not (major and minor and patch):
        raise ValueError("Version macros not found in header")
    return f"{major.group(1)}.{minor.group(1)}.{patch.group(1)}"

def main():
    props_ver = extract_version_from_properties("library.properties")
    json_ver = extract_version_from_json("library.json")
    header_ver = extract_version_from_header("src/ChronosESP32.h")

    print(f"📦 library.properties version: {props_ver}")
    print(f"📝 library.json version:      {json_ver}")
    print(f"🔣 Header macro version:     {header_ver}")

    if props_ver != json_ver or props_ver != header_ver:
        print("❌ Version mismatch detected!")
        sys.exit(1)

    print("✅ All version numbers match!")

if __name__ == "__main__":
    main()
