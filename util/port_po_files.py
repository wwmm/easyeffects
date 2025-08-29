#!/usr/bin/env python3

import os
import shutil
from pathlib import Path

def copy_po_files_to_qt_structure(source_po_dir, dest_translations_dir, project_name="easyeffects"):
    """
    Copies .po files from a source directory to a structured destination,
    suitable for a Qt/KDE project.

    Args:
        source_po_dir (str): Path to the source directory containing .po files.
        dest_translations_dir (str): Path to the destination directory where
                                     the new structure will be created.
        project_name (str): The base name for the output .po files.
    """
    source_path = Path(source_po_dir)
    dest_path = Path(dest_translations_dir)

    # Check if the source directory exists
    if not source_path.is_dir():
        print(f"Error: Source directory not found at '{source_po_dir}'")
        return

    print(f"Starting to copy language files from '{source_path}'...")
    print(f"Destination root: '{dest_path}'")
    
    # Iterate through all .po files in the source directory
    for po_file in source_path.glob("*.po"):
        # The language code is the file name without the .po extension
        lang_code = po_file.stem
        
        # Define the destination path based on the language code and project name
        # The structure will be translations/<lang_code>/<project_name>.po
        destination_subdir = dest_path / lang_code
        destination_file = destination_subdir / f"{project_name}.po"

        try:
            # Create the destination directory if it doesn't exist
            os.makedirs(destination_subdir, exist_ok=True)
            
            # Copy the file
            shutil.copy2(po_file, destination_file)
            print(f"Copied '{po_file.name}' to '{destination_file}'")
            
        except Exception as e:
            print(f"Failed to copy '{po_file.name}': {e}")
            
    print("Language file copying complete.")

if __name__ == "__main__":
    # --- IMPORTANT: EDIT THESE PATHS TO MATCH YOUR REPOSITORY LOCATIONS ---
    # Replace the placeholders below with the actual paths to your EasyEffects repositories.
    # e.g., source_po_directory = "/home/user/easyeffects_master/po"
    # e.g., dest_translations_directory = "/home/user/easyeffects_qt/translations"
    
    source_po_directory = "/tmp/easyeffects/po/news"
    dest_translations_directory = "/tmp/ee_qt_po"
    
    copy_po_files_to_qt_structure(source_po_directory, dest_translations_directory)