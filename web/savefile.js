// Checks a pet backup: the same check when a backup is taken and when one is
// restored, so a cut-off file is caught while the pet is still safe on the board.
// Used by index.html and by tools/check_savefile.mjs, which tests it. The rules
// match save.cpp: the blob ends in a little-endian CRC-16 of everything before it.
(function (root) {
  function crc16(bytes, n) {
    let c = 0xFFFF;
    for (let i = 0; i < n; i++) {
      c ^= bytes[i] << 8;
      for (let b = 0; b < 8; b++) c = (c & 0x8000) ? ((c << 1) ^ 0x1021) & 0xFFFF : (c << 1) & 0xFFFF;
    }
    return c;
  }

  // needHeader: a backup just taken must carry the board's "N bytes" line. A file
  // given to restore may have lost it, and then the checksum alone decides.
  // Returns { ok: true, bytes, lines } with the IMPORT lines to send, or { ok: false, reason }.
  function checkSave(text, needHeader) {
    const all = String(text).split(/\r?\n/).map((l) => l.trim());
    const lines = all.filter((l) => l === 'IMPORT' || l.startsWith('IMPORT '));
    const fail = (reason) => ({ ok: false, reason });

    if (!lines.length) return fail('it has no save data in it');
    if (lines[lines.length - 1] !== 'IMPORT') return fail('it is cut off: the closing line is missing');

    const head = all.find((l) => l.startsWith('# TamaPoke save'));
    const m = head && head.match(/^# TamaPoke save, (\d+) bytes/);
    if (needHeader && !m) return fail('the line saying its size is missing');

    let hex = '';
    for (const l of lines.slice(0, -1)) {
      const chunk = l.slice('IMPORT '.length);
      if (!/^[0-9a-fA-F]+$/.test(chunk) || chunk.length % 2) return fail('a line is damaged');
      hex += chunk;
    }
    const bytes = hex.length / 2;
    if (m && bytes !== Number(m[1])) return fail(`it is incomplete: ${bytes} of ${m[1]} bytes arrived`);
    if (bytes < 10) return fail('it is too short to be a save');

    const b = new Uint8Array(bytes);
    for (let i = 0; i < bytes; i++) b[i] = parseInt(hex.substr(i * 2, 2), 16);
    if (b[0] !== 0x54 || b[1] !== 0x4B || b[2] !== 0x50 || b[3] !== 0x53) return fail('it is not a TamaPoke save');
    if (crc16(b, bytes - 2) !== (b[bytes - 2] | (b[bytes - 1] << 8))) return fail('its checksum is wrong, so the data is damaged');
    return { ok: true, bytes, lines };
  }

  if (typeof module !== 'undefined' && module.exports) module.exports = { checkSave, crc16 };
  else root.checkSave = checkSave;
})(this);
