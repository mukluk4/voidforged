using System;
using System.Collections.Generic;
using System.IO;

namespace VoidPatcher
{
	// One line per entry, tab separated so paths can hold spaces:
	//   version <tab> 2026.10.28.1
	//   message <tab> text shown in the patcher window
	//   patcher <tab> md5 <tab> size
	//   F <tab> md5 <tab> size <tab> path   always kept matching the server copy
	//   I <tab> md5 <tab> size <tab> path   installed only when missing (player settings files)
	//   D <tab> path                        deleted when present
	// Files are served by content at objects/<first two md5 chars>/<md5>, so a CDN never hands out a stale copy
	class ManifestEntry
	{
		public bool OnlyIfMissing;
		public string Md5;
		public long Size;
		public string Path;
	}

	class Manifest
	{
		public string Version = "";
		public List<string> Messages = new List<string>();
		public ManifestEntry Patcher;
		public List<ManifestEntry> Files = new List<ManifestEntry>();
		public List<string> Deletes = new List<string>();

		public static string ObjectPath(string md5) => "objects/" + md5.Substring(0, 2) + "/" + md5;

		public static Manifest Parse(string text)
		{
			var m = new Manifest();
			int lineNo = 0;
			foreach (var raw in text.Split('\n')) {
				lineNo++;
				var line = raw.TrimEnd('\r');
				if (line.Length == 0 || line[0] == '#') {
					continue;
				}

				var p = line.Split('\t');
				switch (p[0]) {
					case "version":
						m.Version = p.Length > 1 ? p[1] : "";
						break;
					case "message":
						m.Messages.Add(p.Length > 1 ? p[1] : "");
						break;
					case "patcher":
						Require(p, 3, lineNo);
						m.Patcher = new ManifestEntry { Md5 = CheckMd5(p[1], lineNo), Size = long.Parse(p[2]), Path = "VoidPatcher.exe" };
						break;
					case "F":
					case "I":
						Require(p, 4, lineNo);
						m.Files.Add(new ManifestEntry {
							OnlyIfMissing = p[0] == "I",
							Md5 = CheckMd5(p[1], lineNo),
							Size = long.Parse(p[2]),
							Path = CheckPath(p[3], lineNo),
						});
						break;
					case "D":
						Require(p, 2, lineNo);
						m.Deletes.Add(CheckPath(p[1], lineNo));
						break;
					default:
						// Unknown keys are skipped so a newer manifest still works with an older patcher
						break;
				}
			}

			if (m.Files.Count == 0) {
				throw new InvalidDataException("The patch list from the server is empty.");
			}

			return m;
		}

		static void Require(string[] p, int n, int lineNo)
		{
			if (p.Length < n) {
				throw new InvalidDataException($"Bad patch list line {lineNo}.");
			}
		}

		static string CheckMd5(string s, int lineNo)
		{
			if (s.Length != 32) {
				throw new InvalidDataException($"Bad checksum on patch list line {lineNo}.");
			}

			foreach (var c in s) {
				if (!Uri.IsHexDigit(c)) {
					throw new InvalidDataException($"Bad checksum on patch list line {lineNo}.");
				}
			}

			return s.ToLowerInvariant();
		}

		// Only plain relative paths inside the game folder are allowed
		static string CheckPath(string s, int lineNo)
		{
			var path = s.Replace('\\', '/');
			if (path.Length == 0 || path.StartsWith("/") || path.Contains(":")) {
				throw new InvalidDataException($"Bad path on patch list line {lineNo}.");
			}

			foreach (var part in path.Split('/')) {
				if (part.Length == 0 || part == "." || part == "..") {
					throw new InvalidDataException($"Bad path on patch list line {lineNo}.");
				}
			}

			return path;
		}
	}
}
