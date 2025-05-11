import re
import sys

def extract_keywords(header_file_path, library_name="ChronosESP32", output_file="keywords.txt"):
    with open(header_file_path, "r") as file:
        lines = file.readlines()

    output_lines = [f"{library_name}\tKEYWORD1\n"]
    content_up_to_private = []

    private_markers = [
        re.compile(r'^\s*private:', re.I),
        re.compile(r'^\s*/\*\s*private', re.I),
        re.compile(r'^\s*#ifdef\s+INTERNAL_API', re.I),
        re.compile(r'^\s*#ifndef\s+PUBLIC_API', re.I),
    ]

    for line in lines:
        if any(marker.search(line) for marker in private_markers):
            break
        content_up_to_private.append(line)

    public_section = ''.join(content_up_to_private)
    public_section = re.sub(r"//.*?$|/\*.*?\*/", "", public_section, flags=re.DOTALL | re.MULTILINE)

    # Ordered function name extraction
    func_pattern = re.compile(
        r"^[ \t]*((?!static)(?!inline)[a-zA-Z_][\w\s\*\(\)]*?)\b([a-zA-Z_][a-zA-Z0-9_]*)\s*\([^;]*\)\s*;",
        re.MULTILINE
    )
    seen = set()
    functions_in_order = []
    for match in func_pattern.finditer(public_section):
        func_name = match.group(2)
        if func_name not in seen:
            seen.add(func_name)
            functions_in_order.append(func_name)

    for func in functions_in_order:
        output_lines.append(f"{func}\tKEYWORD2")

    output_lines.append("")

    # Full content for structs/enums
    full_content = ''.join(lines)
    full_content = re.sub(r"//.*?$|/\*.*?\*/", "", full_content, flags=re.DOTALL | re.MULTILINE)

    struct_enum_pattern = re.compile(r"\b(struct|enum)\s+([a-zA-Z_]\w*)\b")
    struct_enum_names = []
    seen_structs = set()
    for match in struct_enum_pattern.finditer(full_content):
        name = match.group(2)
        if name not in seen_structs:
            seen_structs.add(name)
            struct_enum_names.append(name)

    for name in struct_enum_names:
        output_lines.append(f"{name}\tLITERAL1")

    output_lines.append("")

    # Enum values
    enum_blocks = re.findall(r"enum\s+[a-zA-Z_]\w*\s*{([^}]+)}", full_content, re.DOTALL)
    for block in enum_blocks:
        enum_values = re.findall(r"\b([A-Z_][A-Z0-9_]*)\b", block)
        for val in enum_values:
            output_lines.append(f"{val}\tLITERAL1")
        output_lines.append("")

    with open(output_file, "w") as out_file:
        out_file.write("\n".join(output_lines))

    print(f"[✓] Extracted keywords saved to: {output_file}")

# Example usage
if __name__ == "__main__":
    if len(sys.argv) < 1:
        print("Usage: python extract_keywords.py <header_file.h> <LibraryName>")
    else:
        extract_keywords(sys.argv[1])
