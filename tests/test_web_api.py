#!/usr/bin/env python3
"""Prueba de extremo a extremo del servidor HTTP local (sin bibliotecas externas)."""
import json
import socket
import subprocess
import sys
import time
import urllib.error
import urllib.parse
import urllib.request


def main(binary):
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        port = sock.getsockname()[1]
    base = f"http://127.0.0.1:{port}"
    proc = subprocess.Popen([binary, "--gui", "--port", str(port)],
                            stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)

    def call(path, values=None):
        raw = urllib.parse.urlencode(values).encode() if values is not None else None
        with urllib.request.urlopen(base + path, data=raw, timeout=4) as response:
            data = response.read()
        return json.loads(data) if path.startswith('/api/') else data.decode()

    def until(predicate, seconds=6):
        limit = time.monotonic() + seconds
        while time.monotonic() < limit:
            value = call('/api/status')
            if predicate(value):
                return value
            time.sleep(.06)
        raise AssertionError('Tiempo de espera agotado')

    try:
        for _ in range(60):
            try:
                call('/api/status')
                break
            except (urllib.error.URLError, ConnectionError):
                time.sleep(.04)
        else:
            raise AssertionError('El servidor web no inicio')
        assert '<html' in call('/')
        assert 'function render' in call('/app.js')
        assert 'root' in call('/styles.css')
        initial = call('/api/status')
        assert not initial['running']
        try:
            call('/api/start', {'vehicles': '0'})
        except urllib.error.HTTPError as e:
            assert e.code == 400
        else:
            raise AssertionError('Se acepto una configuracion invalida')

        call('/api/start', {'vehicles': 2, 'arrival': 10, 'crossing': 20,
                            'green': 100, 'red': 20, 'mode': 'adaptive'})
        done = until(lambda s: s['finished'] and not s['running'])
        assert done['generated'] == done['served'] == 4
        assert done['pending'] == 0
        assert done['error'] == ''
        assert done['events']
        assert not done['stopped']
        call('/api/reset', {})
        cleared = call('/api/status')
        assert cleared['generated'] == 0 and not cleared['finished']

        call('/api/start', {'vehicles': 60, 'arrival': 400, 'crossing': 250,
                            'green': 1000, 'red': 200, 'mode': 'fixed'})
        until(lambda s: s['running'] and s['generated'] >= 1)
        call('/api/stop', {})
        interrupted = until(lambda s: s['finished'] and not s['running'], 6)
        assert interrupted['stopped']
        assert interrupted['served'] <= interrupted['generated']
        call('/api/reset', {})
        print('OK: HTTP, HTML, inicio, validacion, cierre, parada y reinicio')
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=2)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait(timeout=2)


if __name__ == '__main__':
    main(sys.argv[1])
