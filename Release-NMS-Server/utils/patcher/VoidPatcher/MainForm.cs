using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace VoidPatcher
{
	class MainForm : Form
	{
		readonly string gameDir;
		readonly string url;

		readonly TextBox log = new TextBox();
		readonly ProgressBar bar = new ProgressBar();
		readonly Label status = new Label();
		readonly Button patchButton = new Button();
		readonly Button playButton = new Button();
		readonly CheckBox verifyAll = new CheckBox();

		CancellationTokenSource cts;
		bool busy;

		public MainForm(string gameDir, string url)
		{
			this.gameDir = gameDir;
			this.url = url;

			Text = "Voidforged Patcher " + Program.Version;
			ClientSize = new Size(640, 400);
			FormBorderStyle = FormBorderStyle.FixedSingle;
			MaximizeBox = false;
			StartPosition = FormStartPosition.CenterScreen;
			Font = new Font("Segoe UI", 9f);
			try {
				Icon = Icon.ExtractAssociatedIcon(Application.ExecutablePath);
			} catch {
				// Default icon is fine
			}

			log.Multiline = true;
			log.ReadOnly = true;
			log.ScrollBars = ScrollBars.Vertical;
			log.BackColor = Color.FromArgb(20, 20, 28);
			log.ForeColor = Color.Gainsboro;
			log.Font = new Font("Consolas", 9f);
			log.SetBounds(12, 12, 616, 270);

			status.SetBounds(12, 292, 616, 20);
			status.AutoEllipsis = true;

			bar.SetBounds(12, 316, 616, 22);
			bar.Maximum = 1000;

			verifyAll.Text = "Check every file (slower, fixes damaged files)";
			verifyAll.SetBounds(12, 356, 330, 24);

			patchButton.Text = "Patch";
			patchButton.SetBounds(400, 352, 110, 32);
			patchButton.Click += (s, e) => {
				if (busy) {
					cts?.Cancel();
				} else {
					StartPatch();
				}
			};

			playButton.Text = "Play";
			playButton.SetBounds(518, 352, 110, 32);
			playButton.Enabled = false;
			playButton.Click += (s, e) => Play();

			Controls.AddRange(new Control[] { log, status, bar, verifyAll, patchButton, playButton });

			Shown += (s, e) => StartPatch();
			FormClosing += (s, e) => cts?.Cancel();
		}

		void StartPatch()
		{
			if (!File.Exists(Path.Combine(gameDir, "eqgame.exe"))) {
				var answer = MessageBox.Show(this,
					"There is no EverQuest in this folder:\n\n" + gameDir +
					"\n\nDownload the full Voidforged client here? It is about 10 GB.\n\n" +
					"Tip: put VoidPatcher.exe in its own folder first, somewhere like C:\\Games\\Voidforged (not Program Files).",
					"Install Voidforged", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
				if (answer != DialogResult.Yes) {
					status.Text = "Put VoidPatcher.exe in your EverQuest folder, or in an empty folder to install there.";
					return;
				}
			}

			if (Process.GetProcessesByName("eqgame").Length > 0) {
				MessageBox.Show(this, "EverQuest is running. Close it, then press Patch.", "Voidforged Patcher",
					MessageBoxButtons.OK, MessageBoxIcon.Warning);
				status.Text = "Close EverQuest, then press Patch.";
				return;
			}

			_ = RunPatch();
		}

		async Task RunPatch()
		{
			busy = true;
			cts = new CancellationTokenSource();
			patchButton.Text = "Cancel";
			playButton.Enabled = false;
			verifyAll.Enabled = false;
			bar.Value = 0;

			var p = new Patcher(gameDir, url);
			p.Log += msg => Ui(() => AppendLog(msg));
			p.Status += msg => Ui(() => status.Text = msg);
			p.Progress += (done, total) => Ui(() => bar.Value = total > 0 ? (int) Math.Max(0, Math.Min(1000, done * 1000 / total)) : 0);

			bool ok = false;
			try {
				var result = await Task.Run(() => p.Run(verifyAll.Checked, cts.Token));
				if (result == PatchResult.Restarting) {
					Close();
					return;
				}

				ok = true;
				AppendLog(result == PatchResult.UpToDate ? "Everything is up to date." : "Patching complete.");
			} catch (OperationCanceledException) {
				status.Text = "Cancelled. Press Patch to continue where you left off.";
				AppendLog("Cancelled.");
			} catch (Exception ex) {
				status.Text = "Patching failed. Press Patch to try again.";
				AppendLog("ERROR: " + ex.Message);
				if (ex.InnerException != null) {
					AppendLog("  " + ex.InnerException.Message);
				}
			} finally {
				busy = false;
				patchButton.Text = "Patch";
				verifyAll.Enabled = true;
				verifyAll.Checked = false;
			}

			playButton.Enabled = ok;
			if (ok) {
				bar.Value = 1000;
				playButton.Focus();
			}
		}

		void Play()
		{
			try {
				Process.Start(new ProcessStartInfo(Path.Combine(gameDir, "eqgame.exe"), "patchme") {
					WorkingDirectory = gameDir,
					UseShellExecute = false,
				});
				Close();
			} catch (Exception ex) {
				MessageBox.Show(this, "Could not start EverQuest: " + ex.Message, "Voidforged Patcher",
					MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}

		void AppendLog(string msg)
		{
			Program.WriteLog(msg);
			log.AppendText(msg + Environment.NewLine);
		}

		void Ui(Action a)
		{
			if (IsDisposed) {
				return;
			}

			try {
				BeginInvoke(a);
			} catch (InvalidOperationException) {
				// Window closed mid-patch
			}
		}
	}
}
