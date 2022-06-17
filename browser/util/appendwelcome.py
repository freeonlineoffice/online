#!/usr/bin/env python3
"""Append Welcome dialog strings to LOOL UI pot file"""

import sys
import polib

welcome = polib.pofile(sys.argv[1],
                  autodetect_encoding=False,
                  encoding="utf-8",
                  wrapwidth=-1)
loolui = polib.pofile(sys.argv[2],
                  autodetect_encoding=False,
                  encoding="utf-8",
                  wrapwidth=78)
# Filter out unnecessary strings of meta tags from html2po output
for entry in welcome:
    if 'html.head.meta' in entry.occurrences[0][0]:
        continue
    if entry.msgid == '':
        continue
    loolui.append(entry)
loolui.metadata['Content-Type'] = 'text/plain; charset=UTF-8'
loolui.save(sys.argv[2])
