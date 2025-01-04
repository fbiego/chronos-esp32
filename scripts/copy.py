
import shutil
import os

Import("env")

def copy_files_recursive(src_dir, dest_dir):
    """Recursively copies files and directories from the source to the destination.

    Args:
        src_dir: The source directory.
        dest_dir: The destination directory.
    """
    print(f"Updating source files to {dest_dir}")

    for item in os.listdir(src_dir):
        src_item = os.path.join(src_dir, item)
        dst_item = os.path.join(dest_dir, item)

        if os.path.isdir(src_item):
            shutil.copytree(src_item, dst_item, dirs_exist_ok=True)
        else:
            shutil.copy2(src_item, dst_item)

try:
    pioenv = env._dict['PIOENV']
    print(f"PlatformIO environment: {pioenv}")
except (KeyError, AttributeError):
    print("PlatformIO environment not found in the SCons Environment.")
    pioenv = "devkit"

sep = os.sep
copy_files_recursive(f"src{sep}", f".pio{sep}libdeps{sep}{pioenv}{sep}src{sep}")

