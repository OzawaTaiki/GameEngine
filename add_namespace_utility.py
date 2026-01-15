#!/usr/bin/env python3
"""
Utility層のファイルにnamespace Engineを追加するスクリプト
"""
import os

files = [
    r"c:\class\GameEngine\Engine\Utility\ConvertString\ConvertString.h",
    r"c:\class\GameEngine\Engine\Utility\ConvertString\ConvertString.cpp",
    r"c:\class\GameEngine\Engine\Utility\FileDialog\FileDialog.h",
    r"c:\class\GameEngine\Engine\Utility\FileDialog\FileDialog.cpp",
    r"c:\class\GameEngine\Engine\Utility\StringUtils\StringUitls.h",
    r"c:\class\GameEngine\Engine\Utility\StringUtils\StringUitls.cpp",
]

def add_namespace(file_path):
    """ファイルにnamespace Engineを追加"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        if 'namespace Engine' in content:
            print(f"Skip (already has namespace): {file_path}")
            return False

        lines = content.split('\n')

        # #includeや#pragmaの後にnamespaceを追加
        insert_idx = 0
        for i, line in enumerate(lines):
            stripped = line.strip()
            if stripped.startswith('#') or stripped.startswith('//') or stripped == '':
                insert_idx = i + 1
            else:
                break

        # namespace Engineを挿入
        lines.insert(insert_idx, '')
        lines.insert(insert_idx + 1, 'namespace Engine {')
        lines.insert(insert_idx + 2, '')

        # 最後に} // namespace Engineを追加
        while lines and lines[-1].strip() == '':
            lines.pop()

        lines.append('')
        lines.append('} // namespace Engine')
        lines.append('')

        new_content = '\n'.join(lines)

        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(new_content)

        print(f"Success: {os.path.basename(file_path)}")
        return True

    except Exception as e:
        print(f"Error: {file_path}: {e}")
        return False

if __name__ == "__main__":
    print("Processing Utility layer files...")
    print("=" * 60)

    success_count = 0
    for file_path in files:
        if add_namespace(file_path):
            success_count += 1

    print("=" * 60)
    print(f"Processed: {success_count}/{len(files)} files")
