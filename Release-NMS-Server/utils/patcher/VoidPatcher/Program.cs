using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Threading;
using System.Windows.Forms;

namespace VoidPatcher
{
	static class Program
	{
		public const string Version = "1.0.0";

		// Where manifest.txt and objects/ live. VoidPatcher.ini next to the exe can override it (url=...)
		const string DefaultUrl = "https://patch.voidforged.invalid/";

		static string logPath;
		static readonly object logLock = new object();

		[STAThread]
		static int Main(string[] args)
		{
			ServicePointManager.SecurityProtocol |= SecurityProtocolType.Tls12;
			ServicePointManager.DefaultConnectionLimit = 16;

			var gameDir = AppDomain.CurrentDomain.BaseDirectory.TrimEnd('\\');
			var settings = ReadSettings(Path.Combine(gameDir, "VoidPatcher.ini"));
			if (!settings.TryGetValue("url", out var url) || url.Length == 0) {
				url = DefaultUrl;
			}

			logPath = Path.Combine(gameDir, "VoidPatcher.log");
			try {
				File.WriteAllText(logPath, "");
			} catch {
				logPath = null;
			}

			WriteLog($"VoidPatcher {Version} in {gameDir}, patch server {url}");

			// --nogui: patch without a window and exit 0 on success; for testing and scripted installs
			if (args.Contains("--nogui")) {
				var p = new Patcher(gameDir, url);
				p.Log += WriteLog;
				try {
					var result = p.Run(args.Contains("--verify"), CancellationToken.None).GetAwaiter().GetResult();
					WriteLog("Result: " + result);
					return 0;
				} catch (Exception ex) {
					WriteLog("ERROR: " + ex.Message);
					return 1;
				}
			}

			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			Application.Run(new MainForm(gameDir, url));
			return 0;
		}

		public static void WriteLog(string msg)
		{
			if (logPath == null) {
				return;
			}

			lock (logLock) {
				try {
					File.AppendAllText(logPath, DateTime.Now.ToString("HH:mm:ss ") + msg + Environment.NewLine);
				} catch {
					// Logging never stops a patch
				}
			}
		}

		static Dictionary<string, string> ReadSettings(string path)
		{
			var d = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
			if (!File.Exists(path)) {
				return d;
			}

			foreach (var line in File.ReadAllLines(path)) {
				var i = line.IndexOf('=');
				if (i > 0 && !line.TrimStart().StartsWith(";") && !line.TrimStart().StartsWith("#")) {
					d[line.Substring(0, i).Trim()] = line.Substring(i + 1).Trim();
				}
			}

			return d;
		}
	}
}
