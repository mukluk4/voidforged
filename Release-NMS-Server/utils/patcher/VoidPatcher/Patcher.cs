using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace VoidPatcher
{
	enum PatchResult { UpToDate, Patched, Restarting }

	class Patcher
	{
		const int Workers = 4;
		const int Retries = 6;
		static readonly TimeSpan StallTimeout = TimeSpan.FromSeconds(60);

		readonly string gameDir;
		readonly string baseUrl;
		readonly HttpClient http;

		public event Action<string> Log;
		public event Action<string> Status;
		// done, total; total 0 means unknown
		public event Action<long, long> Progress;

		public Manifest Manifest { get; private set; }

		public Patcher(string gameDir, string baseUrl)
		{
			this.gameDir = gameDir;
			this.baseUrl = baseUrl.EndsWith("/") ? baseUrl : baseUrl + "/";
			http = new HttpClient { Timeout = Timeout.InfiniteTimeSpan };
			http.DefaultRequestHeaders.UserAgent.ParseAdd("VoidPatcher/" + Program.Version);
		}

		public async Task<PatchResult> Run(bool verifyAll, CancellationToken ct)
		{
			Status?.Invoke("Checking for updates...");
			var text = await GetString("manifest.txt?t=" + DateTime.UtcNow.Ticks, ct);
			Manifest = Manifest.Parse(text);
			foreach (var msg in Manifest.Messages) {
				Log?.Invoke(msg);
			}

			Log?.Invoke($"Patch version {Manifest.Version}");

			if (await SelfUpdate(ct)) {
				return PatchResult.Restarting;
			}

			foreach (var rel in Manifest.Deletes) {
				var full = FullPath(rel);
				if (File.Exists(full)) {
					File.SetAttributes(full, FileAttributes.Normal);
					File.Delete(full);
					Log?.Invoke("Removed " + rel);
				}
			}

			var cache = new HashCache(Path.Combine(gameDir, "VoidPatcher.cache"));
			if (!verifyAll) {
				cache.Load();
			}

			var needed = await Scan(cache, ct);
			if (needed.Count == 0) {
				cache.Save();
				Progress?.Invoke(1, 1);
				Status?.Invoke("Up to date.");
				return PatchResult.UpToDate;
			}

			long total = needed.Sum(e => e.Size);
			Log?.Invoke($"{needed.Count} file(s) to download, {FormatBytes(total)}");
			try {
				await Download(needed, total, cache, ct);
			} finally {
				// Keep what finished so a cancelled or failed run picks up where it stopped
				cache.Save();
			}

			Status?.Invoke("Patching complete.");
			return PatchResult.Patched;
		}

		async Task<List<ManifestEntry>> Scan(HashCache cache, CancellationToken ct)
		{
			var needed = new List<ManifestEntry>();
			int i = 0;
			foreach (var e in Manifest.Files) {
				ct.ThrowIfCancellationRequested();
				if (++i % 50 == 0) {
					Status?.Invoke($"Checking files... {i} / {Manifest.Files.Count}");
					Progress?.Invoke(i, Manifest.Files.Count);
				}

				var full = FullPath(e.Path);
				var info = new FileInfo(full);
				if (!info.Exists) {
					needed.Add(e);
					continue;
				}

				if (e.OnlyIfMissing) {
					continue;
				}

				if (info.Length != e.Size) {
					needed.Add(e);
					continue;
				}

				var md5 = cache.Get(e.Path, info);
				if (md5 == null) {
					md5 = await Task.Run(() => HashFile(full), ct);
					cache.Set(e.Path, info, md5);
				}

				if (md5 != e.Md5) {
					needed.Add(e);
				}
			}

			return needed;
		}

		async Task Download(List<ManifestEntry> needed, long total, HashCache cache, CancellationToken ct)
		{
			long done = 0;
			int filesDone = 0;
			var queue = new Queue<ManifestEntry>(needed.OrderByDescending(e => e.Size));
			var failures = new List<string>();
			var started = Stopwatch.StartNew();
			long lastReport = -1000;

			// About ten window updates a second, however many chunks and files land in between
			void Report(bool force)
			{
				var now = started.ElapsedMilliseconds;
				var last = Interlocked.Read(ref lastReport);
				if (!force && (now - last < 100 || Interlocked.CompareExchange(ref lastReport, now, last) != last)) {
					return;
				}

				var d = Interlocked.Read(ref done);
				var rate = d / Math.Max(1.0, started.Elapsed.TotalSeconds);
				Progress?.Invoke(d, total);
				Status?.Invoke($"Downloading... {filesDone} / {needed.Count} files, {FormatBytes(d)} of {FormatBytes(total)} ({FormatBytes((long) rate)}/s)");
			}

			async Task Worker()
			{
				while (true) {
					ManifestEntry e;
					lock (queue) {
						if (queue.Count == 0) {
							return;
						}

						e = queue.Dequeue();
					}

					try {
						await DownloadFile(e, n => {
							Interlocked.Add(ref done, n);
							Report(false);
						}, ct);
						cache.Set(e.Path, new FileInfo(FullPath(e.Path)), e.Md5);
						Interlocked.Increment(ref filesDone);
						Report(false);
					} catch (OperationCanceledException) when (ct.IsCancellationRequested) {
						throw;
					} catch (Exception ex) {
						lock (failures) {
							failures.Add(e.Path);
						}

						Log?.Invoke($"FAILED {e.Path}: {ex.Message}");
					}
				}
			}

			await Task.WhenAll(Enumerable.Range(0, Workers).Select(_ => Task.Run(Worker, ct)));
			Report(true);

			if (failures.Count > 0) {
				throw new Exception($"{failures.Count} file(s) could not be downloaded. Press Patch to try again.");
			}
		}

		async Task DownloadFile(ManifestEntry e, Action<long> onBytes, CancellationToken ct)
		{
			var full = FullPath(e.Path);
			// Named by content, so a partial download is only ever resumed into the same version of the file
			var tmp = full + "." + e.Md5.Substring(0, 8) + ".voidtmp";
			Directory.CreateDirectory(Path.GetDirectoryName(full));

			// Bytes of this file currently counted on the progress bar
			long reported = 0;
			void Count(long have)
			{
				onBytes(have - reported);
				reported = have;
			}

			for (int attempt = 1; ; attempt++) {
				try {
					// A partial download, from an earlier attempt or an earlier run, picks up where it stopped
					long have = File.Exists(tmp) ? new FileInfo(tmp).Length : 0;
					if (have > e.Size) {
						File.Delete(tmp);
						have = 0;
					}

					Count(have);
					if (e.Size == 0) {
						// Nothing to fetch for an empty file
						File.WriteAllBytes(tmp, new byte[0]);
					}
					else if (have < e.Size) {
						using (var stall = CancellationTokenSource.CreateLinkedTokenSource(ct))
						using (var req = new HttpRequestMessage(HttpMethod.Get, baseUrl + Manifest.ObjectPath(e.Md5))) {
							if (have > 0) {
								req.Headers.Range = new RangeHeaderValue(have, null);
							}

							stall.CancelAfter(StallTimeout);
							using (var resp = await http.SendAsync(req, HttpCompletionOption.ResponseHeadersRead, stall.Token)) {
								resp.EnsureSuccessStatusCode();

								// A host that ignores Range sends the whole file again
								if (have > 0 && resp.StatusCode != HttpStatusCode.PartialContent) {
									have = 0;
									Count(0);
								}

								using (var src = await resp.Content.ReadAsStreamAsync())
								using (var dst = new FileStream(tmp, have > 0 ? FileMode.Append : FileMode.Create, FileAccess.Write, FileShare.None, 1 << 16, true)) {
									var buf = new byte[1 << 16];
									int n;
									while ((n = await src.ReadAsync(buf, 0, buf.Length, stall.Token)) > 0) {
										stall.CancelAfter(StallTimeout);
										await dst.WriteAsync(buf, 0, n, stall.Token);
										have += n;
										Count(have);
									}
								}
							}
						}
					}

					if (have != e.Size) {
						throw new IOException("the download ended early");
					}

					if (HashFile(tmp) != e.Md5) {
						TryDelete(tmp);
						Count(0);
						throw new InvalidDataException("the download was damaged in transit");
					}

					if (File.Exists(full)) {
						File.SetAttributes(full, FileAttributes.Normal);
						File.Delete(full);
					}

					File.Move(tmp, full);
					return;
				} catch (Exception) when (!ct.IsCancellationRequested && attempt < Retries) {
					await Task.Delay(TimeSpan.FromSeconds(Math.Min(30, 3 * attempt)), ct);
				} catch (Exception) {
					// The partial file stays for the next run to resume
					Count(0);
					throw;
				}
			}
		}

		// Swap in a newer patcher: Windows lets a running exe be renamed but not overwritten
		async Task<bool> SelfUpdate(CancellationToken ct)
		{
			var exe = Process.GetCurrentProcess().MainModule.FileName;
			TryDelete(exe + ".old");
			if (Manifest.Patcher == null || HashFile(exe) == Manifest.Patcher.Md5) {
				return false;
			}

			Status?.Invoke("Updating the patcher...");
			Log?.Invoke("A new patcher is available, updating it first.");
			var tmp = exe + ".new";
			using (var resp = await http.GetAsync(baseUrl + Manifest.ObjectPath(Manifest.Patcher.Md5), ct)) {
				resp.EnsureSuccessStatusCode();
				var bytes = await resp.Content.ReadAsByteArrayAsync();
				using (var md5 = MD5.Create()) {
					if (Hex(md5.ComputeHash(bytes)) != Manifest.Patcher.Md5) {
						throw new InvalidDataException("The patcher update was damaged in transit.");
					}
				}

				File.WriteAllBytes(tmp, bytes);
			}

			File.Move(exe, exe + ".old");
			File.Move(tmp, exe);
			var args = string.Join(" ", Environment.GetCommandLineArgs().Skip(1));
			Process.Start(new ProcessStartInfo(exe, args) { WorkingDirectory = gameDir, UseShellExecute = false });
			return true;
		}

		async Task<string> GetString(string rel, CancellationToken ct)
		{
			using (var resp = await http.GetAsync(baseUrl + rel, ct)) {
				resp.EnsureSuccessStatusCode();
				var bytes = await resp.Content.ReadAsByteArrayAsync();
				return Encoding.UTF8.GetString(bytes);
			}
		}

		string FullPath(string rel) => Path.Combine(gameDir, rel.Replace('/', Path.DirectorySeparatorChar));

		public static string HashFile(string path)
		{
			using (var md5 = MD5.Create())
			using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite | FileShare.Delete, 1 << 20)) {
				return Hex(md5.ComputeHash(fs));
			}
		}

		static string Hex(byte[] b)
		{
			var sb = new StringBuilder(b.Length * 2);
			foreach (var x in b) {
				sb.Append(x.ToString("x2"));
			}

			return sb.ToString();
		}

		static void TryDelete(string path)
		{
			try {
				if (File.Exists(path)) {
					File.Delete(path);
				}
			} catch {
				// Still locked by the old process; the next run cleans it up
			}
		}

		public static string FormatBytes(long n)
		{
			if (n >= 1L << 30) {
				return $"{n / (double) (1L << 30):0.00} GB";
			}

			if (n >= 1L << 20) {
				return $"{n / (double) (1L << 20):0.0} MB";
			}

			return $"{n / 1024.0:0} KB";
		}
	}

	// Remembers each file's md5 by size and write time so later runs only hash what changed
	class HashCache
	{
		readonly string path;
		readonly Dictionary<string, (long size, long ticks, string md5)> map =
			new Dictionary<string, (long, long, string)>(StringComparer.OrdinalIgnoreCase);

		public HashCache(string path)
		{
			this.path = path;
		}

		public void Load()
		{
			try {
				foreach (var line in File.ReadAllLines(path)) {
					var p = line.Split('\t');
					if (p.Length == 4) {
						map[p[0]] = (long.Parse(p[1]), long.Parse(p[2]), p[3]);
					}
				}
			} catch {
				// Missing or damaged cache just means a full check
				map.Clear();
			}
		}

		public string Get(string rel, FileInfo info)
		{
			lock (map) {
				return map.TryGetValue(rel, out var v) && v.size == info.Length && v.ticks == info.LastWriteTimeUtc.Ticks ? v.md5 : null;
			}
		}

		public void Set(string rel, FileInfo info, string md5)
		{
			lock (map) {
				map[rel] = (info.Length, info.LastWriteTimeUtc.Ticks, md5);
			}
		}

		public void Save()
		{
			try {
				var sb = new StringBuilder();
				lock (map) {
					foreach (var kv in map) {
						sb.Append(kv.Key).Append('\t').Append(kv.Value.size).Append('\t').Append(kv.Value.ticks).Append('\t').Append(kv.Value.md5).Append('\n');
					}
				}

				File.WriteAllText(path, sb.ToString());
			} catch {
				// Not worth failing a patch over
			}
		}
	}
}
