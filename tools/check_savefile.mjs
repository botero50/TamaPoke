#!/usr/bin/env node
// Tests web/savefile.js with made-up saves: a good one must pass, and every kind of
// damaged one must be refused. Run by tools/check_installer.py.
import { createRequire } from 'node:module';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

const require = createRequire(import.meta.url);
const here = path.dirname(fileURLToPath(import.meta.url));
const { checkSave, crc16 } = require(path.join(here, '..', 'web', 'savefile.js'));

const hexOf = (b) => [...b].map((v) => v.toString(16).padStart(2, '0').toUpperCase()).join('');

function makeSave(size = 400, chunk = 48) {
  const b = new Uint8Array(size);
  b.set([0x54, 0x4B, 0x50, 0x53, 1, 0, 0, 0]);          // "TKPS", version, no keys
  for (let i = 8; i < size - 2; i++) b[i] = (i * 37) & 0xFF;
  const c = crc16(b, size - 2);
  b[size - 2] = c & 0xFF; b[size - 1] = c >> 8;
  const lines = [`# TamaPoke save, ${size} bytes. Paste this whole block back.`];
  for (let i = 0; i < size; i += chunk) lines.push('IMPORT ' + hexOf(b.subarray(i, i + chunk)));
  lines.push('IMPORT');
  return lines;
}

let bad = 0;
function ck(ok, what) {
  console.log((ok ? 'PASS  ' : 'FAIL  ') + what);
  if (!ok) bad++;
}
const passes = (lines, need = true) => checkSave(lines.join('\n'), need).ok;

const good = makeSave();
const r = checkSave(good.join('\n'), true);
ck(r.ok && r.bytes === 400, 'a good save passes');
ck(r.lines.length === good.length - 1 && r.lines.at(-1) === 'IMPORT', 'and gives back the lines to send');
ck(checkSave(good.join('\r\n'), true).ok, 'Windows line endings are fine');

ck(!passes(good.slice(0, -1)), 'no closing line is refused');
ck(!passes([...good.slice(0, 5), 'IMPORT']), 'whole lines missing from the middle are refused');
{
  const l = [...good]; l[3] = l[3].slice(0, -2);
  ck(!passes(l), 'a line cut short is refused, by its size');
  ck(!passes(l, false) , 'and without the size line, by its checksum');
}
{
  const l = [...good], d = l[4].at(-1) === '0' ? '1' : '0';
  l[4] = l[4].slice(0, -1) + d;
  ck(!passes(l), 'one changed digit is refused');
}
{
  const l = [...good]; l[0] = '# TamaPoke save, 999 bytes.';
  ck(!passes(l), 'a wrong byte count is refused');
}
{
  const l = [...good]; l[2] = 'IMPORT ZZ';
  ck(!passes(l), 'a line that is not hex is refused');
  const o = [...good]; o[2] = o[2] + '0';
  ck(!passes(o), 'a line with half a byte is refused');
}
ck(!passes(good.slice(1)), 'a new backup with no size line is refused');
ck(passes(good.slice(1), false), 'a file to restore with no size line is judged by its checksum');
ck(!checkSave('hello', true).ok && !checkSave('', true).ok, 'a file that is not a save is refused');
{
  const b = makeSave(); b[1] = 'IMPORT 00000000' + b[1].slice('IMPORT 00000000'.length);
  ck(!passes(b), 'the wrong first bytes are refused');
}

if (bad) { console.log(`FAILURES: ${bad}`); process.exit(1); }
console.log('the backup check works');
