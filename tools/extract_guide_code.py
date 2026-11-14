#!/usr/bin/env python3
"""Markdown 문서의 C++ 코드 블록을 컴파일할 수 있는 .cpp 파일로 뽑는다.

사용자 설명서, README, API 레퍼런스 첫 쪽(docs/mainpage.md)의 코드가 API와 어긋나지 않게 하는 것이
목적이다. 이 스크립트는 파일만 만든다. tests/guide/CMakeLists.txt가 configure 때 이 스크립트를
실행하고, 만들어진 파일은 테스트 빌드에서 컴파일된다.

    python tools/extract_guide_code.py --root . --out build/guide_code \
        README.md docs/mainpage.md docs/guide/*.md

```cpp 블록만 뽑는다. 블록마다 파일 하나를 만들며, 종류는 두 가지다.

- 완성 프로그램: `int main(`이 있는 블록. 그대로 컴파일하고 meno와 링크한다.
- 조각: 그 밖의 블록. 설명서가 앞에서 만들어 두었다고 가정하는 이름(game, renderer 등)을 매개변수로
  받는 함수 안에 넣어 컴파일만 한다. 조각이 다른 이름을 가정해야 하면 FRAGMENT_PROLOGUE의 매개변수에
  더한다.

블록 앞에 #line을 넣으므로, 컴파일 오류는 생성된 파일이 아니라 Markdown 파일의 줄을 가리킨다.
내용이 바뀐 파일만 다시 쓰므로, 문서 하나를 고쳐도 다른 블록은 다시 컴파일되지 않는다.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

BLOCK = re.compile(r"^```cpp[ \t]*\n(.*?)^```[ \t]*$", re.MULTILINE | re.DOTALL)

PROGRAM_PROLOGUE = """\
// {source}의 {index}번째 C++ 블록(완성 프로그램)이다. tools/extract_guide_code.py가 만든
// 파일이므로 직접 고치지 말고, 원본인 Markdown 파일을 고친다.
"""

FRAGMENT_PROLOGUE = """\
// {source}의 {index}번째 C++ 블록(조각)이다. tools/extract_guide_code.py가 만든 파일이므로
// 직접 고치지 말고, 원본인 Markdown 파일을 고친다.
#include <meno/Meno.hpp>

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <string>

// 설명서의 조각 코드가 이미 있다고 가정하는 이름들이다.
void {function}([[maybe_unused]] meno::Game& game, [[maybe_unused]] meno::Renderer& renderer,
        [[maybe_unused]] meno::Scene& play, [[maybe_unused]] meno::GameObject& player,
        [[maybe_unused]] std::shared_ptr<const meno::Font> font,
        [[maybe_unused]] meno::Vec2f& position, [[maybe_unused]] meno::Vec2f& velocity,
        [[maybe_unused]] double fixedDeltaTime) {{
    {{
"""

FRAGMENT_EPILOGUE = """\
    }
}
"""


def extract(root: Path, source: Path) -> list[tuple[str, str]]:
    """문서 하나에서 (파일 이름, 내용) 목록을 만든다."""
    relative = source.resolve().relative_to(root.resolve()).as_posix()
    text = source.read_text(encoding="utf-8")
    stem = re.sub(r"\W", "_", source.stem)
    files = []
    for index, match in enumerate(BLOCK.finditer(text), start=1):
        code = match.group(1)
        # 블록 내용은 여는 펜스 다음 줄부터 시작한다.
        first_line = text.count("\n", 0, match.start(1)) + 1
        line_directive = f'#line {first_line} "{relative}"\n'
        if "int main(" in code:
            name = f"program_{stem}_{index}.cpp"
            content = PROGRAM_PROLOGUE.format(source=relative, index=index) + line_directive + code
        else:
            name = f"fragment_{stem}_{index}.cpp"
            content = (FRAGMENT_PROLOGUE.format(source=relative, index=index,
                                                function=f"guide_fragment_{stem}_{index}")
                       + line_directive + code + FRAGMENT_EPILOGUE)
        files.append((name, content))
    return files


def main() -> int:
    # Windows의 출력 인코딩은 시스템 코드 페이지를 따른다. 한글이 없는 코드 페이지(영문 CI 러너
    # 등)에서는 한글 메시지를 출력하다 예외로 멈추므로 UTF-8로 바꾼다.
    for stream in (sys.stdout, sys.stderr):
        stream.reconfigure(encoding="utf-8", errors="replace")

    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--root", type=Path, required=True,
                        help="저장소 루트. #line에 적는 경로의 기준이다.")
    parser.add_argument("--out", type=Path, required=True, help=".cpp 파일을 만들 폴더")
    parser.add_argument("sources", type=Path, nargs="+", help="Markdown 파일")
    args = parser.parse_args()

    args.out.mkdir(parents=True, exist_ok=True)
    wanted: dict[str, str] = {}
    for source in args.sources:
        for name, content in extract(args.root, source):
            if name in wanted:
                print(f"error: 확장자를 뺀 이름이 같은 문서가 둘 있다: {source}", file=sys.stderr)
                return 1
            wanted[name] = content
    # 블록을 하나도 찾지 못하면(펜스 형식이 바뀐 경우 등) 검사가 아무것도 하지 않고 통과하므로
    # 실패로 본다.
    if not wanted:
        print("error: C++ 블록을 하나도 찾지 못했다.", file=sys.stderr)
        return 1

    for name, content in wanted.items():
        path = args.out / name
        if not path.exists() or path.read_text(encoding="utf-8") != content:
            path.write_text(content, encoding="utf-8", newline="\n")
    # 문서에서 사라진 블록의 파일을 지운다. 남겨 두면 CMake가 계속 컴파일한다.
    for path in args.out.glob("*.cpp"):
        if path.name not in wanted:
            path.unlink()

    programs = sum(name.startswith("program_") for name in wanted)
    print(f"{len(wanted)} C++ blocks: {programs} programs, {len(wanted) - programs} fragments")
    return 0


if __name__ == "__main__":
    sys.exit(main())
