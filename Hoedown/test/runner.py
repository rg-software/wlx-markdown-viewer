#!/usr/bin/env python
# -*- coding: utf-8 -*-

import difflib
import json
import os
import re
import subprocess
import unittest
from pathlib import Path
import shutil

TEST_ROOT = os.path.dirname(__file__)
PROJECT_ROOT = os.path.dirname(TEST_ROOT)


def _find_hoedown_executable():
    """Return the preferred hoedown executable path, raising if missing."""
    candidates = [
        Path(PROJECT_ROOT) / 'hoedown',  # Unix-style build
        Path(PROJECT_ROOT) / 'hoedown.exe',
        Path(PROJECT_ROOT).parent / 'x64' / 'Release' / 'hoedown.exe',
        Path(PROJECT_ROOT).parent / 'x64' / 'Debug' / 'hoedown.exe',
        Path(PROJECT_ROOT).parent / 'Win32' / 'Release' / 'hoedown.exe',
        Path(PROJECT_ROOT).parent / 'Win32' / 'Debug' / 'hoedown.exe',
    ]

    tried = []

    for path in candidates:
        if not path:
            continue

        if not path.exists():
            tried.append(f"{path} (missing)")
            continue

        if path.is_dir():
            tried.append(f"{path} (directory)")
            continue

        if not os.access(path, os.X_OK):
            tried.append(f"{path} (not executable)")
            continue

        return [str(path.resolve())]

    raise FileNotFoundError(
        'Unable to locate hoedown executable. Build the project first or '
        'place hoedown(.exe) in the Hoedown directory or in a Release/Debug folder. '
        'Tried paths: ' + ', '.join(tried)
    )


def _build_tidy_command():
    tidy_path = shutil.which('tidy')
    if not tidy_path:
        return None
    return [tidy_path, '--show-body-only', '1', '--show-warnings', '0', '--quiet', '1']


HOEDOWN = _find_hoedown_executable()
TIDY = _build_tidy_command()
CONFIG_PATH = os.path.join(TEST_ROOT, 'config.json')
SLUGIFY_PATTERN = re.compile(r'\W')


def with_metaclass(meta, *bases):
    """Metaclass injection utility from six.

    See: https://pythonhosted.org/six/
    """
    class metaclass(meta):
        def __new__(cls, name, this_bases, d):
            return meta(name, bases, d)
    return type.__new__(metaclass, 'temporary_class', (), {})


class TestFailed(AssertionError):
    def __init__(self, name, expected, got):
        super(TestFailed, self).__init__(self)
        if isinstance(expected, bytes):
            expected = expected.decode('utf-8')
        if isinstance(got, bytes):
            got = got.decode('utf-8')
        diff = difflib.unified_diff(
            expected.splitlines(), got.splitlines(),
            fromfile='Expected', tofile='Got',
        )
        self.description = '{name}\n{diff}'.format(
            name=name, diff='\n'.join(diff),
        )

    def __str__(self):
        return self.description


def _tidy_bytes(data):
    if TIDY is None:
        return data.strip()

    tidy_proc = subprocess.Popen(
        TIDY,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
    )
    tidy_output, _ = tidy_proc.communicate(input=data)
    return tidy_output.strip()


def _tidy_file(file_path):
    if TIDY is None:
        with open(file_path, 'rb') as fh:
            return fh.read().strip()

    tidy_proc = subprocess.Popen(
        TIDY + [file_path],
        stdout=subprocess.PIPE,
    )
    tidy_output, _ = tidy_proc.communicate()
    return tidy_output.strip()


def _test_func(test_case):
    flags = test_case.get('flags') or []
    hoedown_proc = subprocess.Popen(
        HOEDOWN + flags + [os.path.join(TEST_ROOT, test_case['input'])],
        stdout=subprocess.PIPE,
    )
    stdoutdata = hoedown_proc.communicate()[0]

    got = _tidy_bytes(stdoutdata)
    expected = _tidy_file(os.path.join(TEST_ROOT, test_case['output']))

    # Cleanup.
    hoedown_proc.stdout.close()

    try:
        assert expected == got
    except AssertionError:
        raise TestFailed(test_case['input'], expected, got)


def _make_test(test_case):
    return lambda self: _test_func(test_case)


class MarkdownTestsMeta(type):
    """Meta class for ``MarkdownTestCase`` to inject test cases on the fly.
    """
    def __new__(meta, name, bases, attrs):
        with open(CONFIG_PATH) as f:
            config = json.load(f)

        for test in config['tests']:
            input_name = test['input']
            attr_name = 'test_' + SLUGIFY_PATTERN.sub(
                '_', os.path.splitext(input_name)[0].lower(),
            )
            func = _make_test(test)
            func.__doc__ = input_name
            if test.get('skip', False):
                func = unittest.skip(input_name)(func)
            if test.get('fail', False):
                func = unittest.expectsFailure(func)
            attrs[attr_name] = func
        return type.__new__(meta, name, bases, attrs)


class MarkdownTests(with_metaclass(MarkdownTestsMeta, unittest.TestCase)):
    pass


if __name__ == '__main__':
    unittest.main()
