#!/usr/bin/env python3
"""
指定したディレクトリの全.hと.cppファイルにnamespace Engineを追加するスクリプト
"""
import os
import sys

def add_namespace(file_path):
    """ファイルにnamespace Engineを追加"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        if 'namespace Engine' in content:
            return False, "already has namespace"

        lines = content.split('\n')

        # #includeや#pragmaの後にnamespaceを追加
        insert_idx = 0
        for i, line in enumerate(lines):
            stripped = line.strip()
            if stripped.startswith('#') or stripped.startswith('//') or stripped == '' or stripped.startswith('using'):
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

        return True, "success"

    except Exception as e:
        return False, f"error: {e}"

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python add_namespace_module.py <directory>")
        sys.exit(1)

    target_dir = sys.argv[1]

    if not os.path.exists(target_dir):
        print(f"Error: Directory not found: {target_dir}")
        sys.exit(1)

    # ファイルを収集
    files = []
    for root, dirs, filenames in os.walk(target_dir):
        for filename in filenames:
            if filename.endswith('.h') or filename.endswith('.cpp'):
                files.append(os.path.join(root, filename))

    print(f"Found {len(files)} files in {target_dir}")
    print("=" * 70)

    success_count = 0
    skip_count = 0
    error_count = 0

    for file_path in files:
        success, msg = add_namespace(file_path)
        rel_path = os.path.relpath(file_path, target_dir)

        if success:
            print(f"[OK] {rel_path}")
            success_count += 1
        elif "already" in msg:
            print(f"[SKIP] {rel_path}")
            skip_count += 1
        else:
            print(f"[ERROR] {rel_path}: {msg}")
            error_count += 1

    print("=" * 70)
    print(f"Success: {success_count}, Skipped: {skip_count}, Errors: {error_count}")
    print(f"Total: {len(files)} files")
