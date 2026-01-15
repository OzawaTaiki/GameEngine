#!/usr/bin/env python3
"""
namespace Engineの配置が正しいかチェックするスクリプト
#include の後に namespace Engine { があるべき
"""
import os
import sys
import re

def check_file(file_path):
    """ファイルの namespace 配置をチェック"""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()

        namespace_line = -1
        last_include_line = -1

        for i, line in enumerate(lines):
            stripped = line.strip()

            # namespace Engine { を探す
            if stripped == 'namespace Engine {':
                namespace_line = i
                break

            # #include を探す（コメント内でないもの）
            if stripped.startswith('#include'):
                last_include_line = i

        # namespace が見つからない場合はスキップ
        if namespace_line == -1:
            return True, "no namespace"

        # namespace の後に #include がある場合は問題
        for i in range(namespace_line + 1, min(namespace_line + 20, len(lines))):
            stripped = lines[i].strip()
            if stripped.startswith('#include'):
                return False, f"#include found at line {i+1} after namespace at line {namespace_line+1}"

        return True, "ok"

    except Exception as e:
        return False, f"error: {e}"

if __name__ == "__main__":
    target_dirs = [
        r"c:\class\GameEngine\Engine\Core",
        r"c:\class\GameEngine\Engine\System",
        r"c:\class\GameEngine\Engine\Framework",
        r"c:\class\GameEngine\Engine\Debug",
        r"c:\class\GameEngine\Engine\Settings",
        r"c:\class\GameEngine\Engine\Utility",
    ]

    print("Checking namespace placement...")
    print("=" * 70)

    problem_files = []

    for target_dir in target_dirs:
        if not os.path.exists(target_dir):
            continue

        for root, dirs, files in os.walk(target_dir):
            for filename in files:
                if filename.endswith('.cpp') or filename.endswith('.h'):
                    file_path = os.path.join(root, filename)
                    is_ok, msg = check_file(file_path)

                    if not is_ok:
                        rel_path = os.path.relpath(file_path, r"c:\class\GameEngine\Engine")
                        print(f"[PROBLEM] {rel_path}: {msg}")
                        problem_files.append(file_path)

    print("=" * 70)
    if problem_files:
        print(f"Found {len(problem_files)} files with problems")
        engine_dir = r"c:\class\GameEngine\Engine"
        for f in problem_files:
            print(f"  - {os.path.relpath(f, engine_dir)}")
    else:
        print("No problems found!")
