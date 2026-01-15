#!/usr/bin/env python3
"""
.cppファイルにnamespace Engineを追加するスクリプト
"""
import os
import sys

def add_namespace_to_cpp(file_path):
    """cppファイルにnamespace Engineを追加"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        # 既にnamespaceがある場合はスキップ
        if 'namespace Engine' in content:
            print(f"Skip (already has namespace): {file_path}")
            return False

        lines = content.split('\n')

        # #includeの後にnamespaceを追加
        include_end_idx = 0
        for i, line in enumerate(lines):
            if line.strip().startswith('#include') or line.strip().startswith('//') or line.strip() == '':
                include_end_idx = i
            else:
                break

        # namespace Engineを挿入
        lines.insert(include_end_idx + 1, '')
        lines.insert(include_end_idx + 2, 'namespace Engine {')
        lines.insert(include_end_idx + 3, '')

        # 最後に} // namespace Engineを追加
        # 最後の空行を削除してから追加
        while lines and lines[-1].strip() == '':
            lines.pop()

        lines.append('')
        lines.append('} // namespace Engine')
        lines.append('')

        new_content = '\n'.join(lines)

        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(new_content)

        print(f"Success: {file_path}")
        return True

    except Exception as e:
        print(f"Error processing {file_path}: {e}")
        return False

if __name__ == "__main__":
    # Mathディレクトリ内の全cppファイルを処理
    math_dir = r"c:\class\GameEngine\Engine\Math"

    cpp_files = []
    for root, dirs, files in os.walk(math_dir):
        for file in files:
            if file.endswith('.cpp'):
                cpp_files.append(os.path.join(root, file))

    print(f"Found {len(cpp_files)} .cpp files in Math directory")
    print("=" * 60)

    success_count = 0
    for cpp_file in cpp_files:
        if add_namespace_to_cpp(cpp_file):
            success_count += 1

    print("=" * 60)
    print(f"Processed: {success_count}/{len(cpp_files)} files")
