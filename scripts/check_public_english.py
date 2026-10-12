#!/usr/bin/env python3
"""Verify public-facing repository documentation is written in English.

No external dependencies, online translation service, npm, or GitHub API.
The check focuses on distinctive Portuguese vocabulary and UTF-8 accents;
technical identifiers, proper names, historic tags and URLs remain intact.
"""
from __future__ import annotations

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
PUBLIC_FILES = [ROOT / "README.md", *sorted((ROOT / "docs").rglob("*.md"))]
PORTUGUESE = re.compile(
    r"(?iu)\b(?:"
    r"não|versão|atualização|instalação|validação|documentação|"
    r"aceito|aceita|aprovado|aprovados|corrigido|corrigida|"
    r"mudanças|alterações|alteração|preservados|mantidos|"
    r"quando|também|usuário|usuários|pareamento|tecla|tela|"
    r"somente|apenas|nenhum|nenhuma|qualquer|necessário|"
    r"liberação|conectado|botão|último|configurações|"
    r"primeiro|primeira|selecione|mostram|funcionalidades|"
    r"dicas|conforme|permanecem|realizada|publicação|"
    r"removido|substitui|anteriormente|atualizada|"
    r"físico|física|físicos|fisicamente|revisão|"
    r"opção|opções|mudança|ajuda|corrigir|corrige|"
    r"roteiro|clique|concluído|recompila|"
    r")\b"
)
HTML_LOCALE = re.compile(r'(?i)<html\s[^>]*\blang\s*=\s*["\']pt(?:-BR)?["\']')

def main() -> int:
    problems: list[str] = []
    for path in PUBLIC_FILES:
        if not path.is_file():
            problems.append(f"Missing file: {path.relative_to(ROOT)}")
            continue
        source = path.read_text(encoding="utf-8")
        for line_number, line in enumerate(source.splitlines(), 1):
            # Ignore URLs, code identifiers and literals containing slashes.
            # We still check quoted prose and Markdown headings/bullets.
            prose = re.sub(r"https?://\S+|\`[^\`]*\`", "", line)
            match = PORTUGUESE.search(prose)
            if match:
                problems.append(
                    f"{path.relative_to(ROOT)}:{line_number}: "
                    f"non-English text ({match.group(0)!r})"
                )
            if HTML_LOCALE.search(line):
                problems.append(f"{path.relative_to(ROOT)}:{line_number}: lang must be en")
    if problems:
        print("\n".join(problems))
        print(f"FAIL: {len(problems)} potentially Portuguese lines")
        return 1
    releases = sorted((ROOT / "docs" / "releases").glob("v*.md"))
    if not releases:
        print("FAIL: no release note documents found")
        return 1
    print(
        f"PASS: English-only documentation check: "
        f"{len(PUBLIC_FILES)} Markdown pages, {len(releases)} release note pages"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
